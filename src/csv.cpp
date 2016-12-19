// (C) 2016 Leonardo Silvestri
//
// This file is part of ztsdb.
//
// ztsdb is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ztsdb is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ztsdb.  If not, see <http://www.gnu.org/licenses/>.



#include <cstdio>
#include <cstdlib>
#include <type_traits>
#include <double-conversion.h>
#include "csv.hpp"
#include "cow_ptr.hpp"
#include "globals.hpp"
#include "misc.hpp"
#include "display.hpp"
#include "config.hpp"
#include "timezone/ztime.hpp"


static const size_t BUFSZ = 64000;


template<typename T>
struct ToChar {
  size_t toChar(T t, char* buf, size_t sz) { ASSERT(false); return 0; };
};
template<typename T>
struct FromChar {
  size_t fromChar(const char* buf, int sz, T& t) { ASSERT(false); return 0; };
};

// double:
template<>
struct ToChar<double> {
  ToChar() : dtosc(double_conversion::
                   DoubleToStringConverter::
                   EMIT_POSITIVE_EXPONENT_SIGN,
                   "Inf", "NaN", 'e', -6, 6, 6, 6) { }

  size_t toChar(double t, char* buf, size_t sz) {
    double_conversion::StringBuilder sb(buf, sz);
    dtosc.ToShortest(t, &sb);
    auto res = sb.position();     // can only be called before finalization
    sb.Finalize(); // have to call it else it will be done in the descructor!
    return res;
  }
private:
  double_conversion::DoubleToStringConverter dtosc;
};
template<>
struct FromChar<double> {
  FromChar() :  sdc(0,
                    0.0,
                    NAN,
                    "Inf",
                    "NaN") { }

  size_t fromChar(const char* buf, int sz, double& t) {
    int processed_characters_count;
    t = sdc.StringToDouble(buf, sz, &processed_characters_count);
    return processed_characters_count;
  }
  double_conversion::StringToDoubleConverter sdc;
};

// bool
template<>
struct ToChar<bool> {
  size_t toChar(bool t, char* buf, size_t sz) {
    return ltostr_r(t, buf, sz);
  }
};
template<>
struct FromChar<bool> {
  size_t fromChar(const char* buf, int sz, bool& t) {
    char* endptr;
    t = strtol(buf, &endptr, 10);
    return endptr - buf;
  }
};

// dtime
template<>
struct ToChar<Global::dtime> {
  // grab the format from cfgmap LLL
  size_t toChar(Global::dtime t, char* buf, size_t sz) {
    auto s = tz::to_string(t, "", "UTC");

    memcpy(buf, s.c_str(), s.length());
    return s.length();
  }
};
template<>
struct FromChar<Global::dtime> {
  size_t fromChar(const char* buf, int sz, Global::dtime& t) {
    // not efficient to pass by string, LLL
    t = tz::dtime_from_string(string(buf, sz));
    return sz;
  }
};


static std::unique_ptr<arr::AllocFactory> getAllocFactory(const string& mmapfile) {
  if (mmapfile.size()) {
    return std::make_unique<arr::MmapAllocFactory>(mmapfile, false);
  }
  else {
    return std::make_unique<arr::MemAllocFactory>();
  }
}



int readToken(int fd, char buf[], char*& b, char*& e,
              const char sep, size_t& offset, ssize_t& bytes_read, bool& quoted) {
  if (quoted) ++e;
  quoted = *++e=='"';
  if (quoted) ++e;
  b = e;
  for (;;) {
    // read more if necessary:
    if (e >= buf + offset + bytes_read) {
      // copy the remaining bytes of the buffer at the beginning and
      // set offset for the next read:
      offset = e - b;
      memcpy(buf, b, offset);
      b = buf;
      e = buf + offset;
      bytes_read=read(fd, e, BUFSZ - (e - b));
      if (bytes_read <  0) return -1;
      if (bytes_read == 0) return offset ? 0 : -1;
    }
    if (quoted) {
      if (e < buf + offset - 1 && *e=='"' && (e[1] == sep || e[1] == '\n')) break;
    }
    else if (*e == sep || *e == '\n') break;
    ++e;
  }
  std::cout << "token: " << std::string(b, e-b) << std::endl;
  return *e;
}


template<typename T> 
arr::cow_ptr<arr::Array<T>> arr::readcsv_array(const string& file, 
                                               bool header,
                                               const char sep,
                                               const string& mmapfile) 
{
  int fd = open(file.c_str(), O_RDONLY);
  if (fd < 0) {
    throw std::system_error(std::error_code(errno, std::system_category()), "open " + file);
  }

  arr::idx_type row = 0;
  char buf[BUFSZ];
  ssize_t bytes_read = 0;
  char* b = buf;
  char* e = buf;
  size_t offset = 0;
  bool quoted = false;
  
  auto ap = arr::make_cow<arr::Array<T>>(true, 
                                        arr::Vector<arr::idx_type>{0,0}, 
                                        arr::Vector<T>(), 
                                        vector<arr::Vector<arr::zstring>>(), 
                                        getAllocFactory(mmapfile));
  auto& a = *ap.get();          // get so we don't make a copy
  
  if (header) {
    for (;;) {
      auto sep_read = readToken(fd, buf, b, e, sep, offset, bytes_read, quoted);
      if (sep_read < 0) break;
      a.cbind(arr::Array<T>({0,1}, arr::Vector<T>(), {{}, {std::string(b, e-b)}}));
      if (sep_read == '\n') break;
    }
  }
  else {
    for (;;) {
      T d;
      auto sep_read = readToken(fd, buf, b, e, sep, offset, bytes_read, quoted);    
      if (sep_read < 0) break;
      int processed_chars = FromChar<T>().fromChar(b, e-b, d);
      if (processed_chars != e-b) {
        throw std::out_of_range("can't parse '" + std::string(b, e-b) +
                                "' on row " + std::to_string(row+1));
      }
      a.cbind(arr::Array<T>({1,1}, arr::Vector<T>{d})); 
      if (sep_read == '\n') break;
    }
  }
  ++row;

  for (;;) {
    // from earlier measurements:
    //
    // char* endptr;
    // with no opt:
    // 20s without anything:
    // 38s with just (*a->v[col])[row] = 1.0
    // 1m24s with the full line
    // with opt:
    // 7s without anything
    // 11s with just (*a->v[col])[row]
    // 49s with (*a->v[col])[row] = strtod(b, &endptr);
    // (*a->v[col])[row] = strtod(b, &endptr);
    // 1m6s with StringToDouble
    char sep_read;
    std::cout << "a.ncols: " << a.ncols() << std::endl;
    for (size_t j=0; j<a.ncols(); ++j) {
      sep_read = readToken(fd, buf, b, e, sep, offset, bytes_read,quoted);    
      if (sep_read < 0) break;
      T d;
      int processed_chars = FromChar<T>().fromChar(b, e-b, d);
      if (processed_chars != e-b) {
        throw std::out_of_range("can't parse '" + std::string(b, e-b) + "' on row " +
                                std::to_string(row+1) + ", col " + std::to_string(j+1));
      }
      a.getcol(j).push_back(d); 
    }
    std::cout << "read data sep_read: " << char(sep_read) << std::endl;
    if (sep_read < 0) break;
    if (sep_read >= 0 && sep_read != '\n') {
      throw std::out_of_range("incorrect number of elements in row " + std::to_string(row+1));
    }
    ++row;
  }    
  a.resize(0, header ? row-1 : row);

  return ap;
}


arr::cow_ptr<arr::zts> arr::readcsv_zts(const string& file,
                                        bool header,
                                        const char sep,
                                        const string& mmapfile) {
  int fd = open(file.c_str(), O_RDONLY);
  if (fd < 0) {
    throw std::system_error(std::error_code(errno, std::system_category()), "open " + file);
  }

  arr::idx_type row = 0;
  char buf[BUFSZ];
  ssize_t bytes_read = 0;
  char* b = buf;
  char* e = buf;
  size_t offset = 0;
  bool quoted = false;
  
  auto z = arr::make_cow<arr::zts>(true, 
                                   arr::Vector<arr::idx_type>{0,0}, 
                                   arr::Vector<Global::dtime>(), 
                                   arr::Vector<double>(), 
                                   vector<arr::Vector<arr::zstring>>(), 
                                   getAllocFactory(mmapfile));

  
  if (header) {
    auto sep_read = readToken(fd, buf, b, e, sep, offset, bytes_read, quoted);
    if (sep_read < 0) return z; // it's an empty file..., throw? LLL
    for (;;) {
      auto sep_read = readToken(fd, buf, b, e, sep, offset, bytes_read, quoted);
      if (sep_read < 0) break;
      z->abind(arr::Array<double>({0,1}, arr::Vector<double>(), {{}, {std::string(b, e-b)}}), 1);
      if (sep_read == '\n') break;
    }
  }
  else {
    auto sep_read = readToken(fd, buf, b, e, sep, offset, bytes_read, quoted);
    if (sep_read < 0) return z; // it's an empty file..., throw? LLL
    Global::dtime dt;
    int processed_chars = FromChar<Global::dtime>().fromChar(b, e-b, dt);
    if (processed_chars != e-b) {
      throw std::out_of_range("can't parse '" + std::string(b, e-b) +
                              "' on row " + std::to_string(row+1));
    }
    z->getIndexPtr()->getcol(0).push_back(dt);
    for (;;) {
      double d;
      auto sep_read = readToken(fd, buf, b, e, sep, offset, bytes_read, quoted);    
      if (sep_read < 0) break;
      int processed_chars = FromChar<double>().fromChar(b, e-b, d);
      if (processed_chars != e-b) {
        throw std::out_of_range("can't parse '" + std::string(b, e-b) +
                                "' on row " + std::to_string(row));
      }
      z->getArrayPtr()->abind(arr::Array<double>({1,1}, arr::Vector<double>{d}), 1); 
      if (sep_read == '\n') break;
    }
  }
  ++row;
    
  for (;;) {
    auto sep_read = readToken(fd, buf, b, e, sep, offset, bytes_read, quoted);    
    if (sep_read < 0) break;
    Global::dtime dt;
    int processed_chars = FromChar<Global::dtime>().fromChar(b, e-b, dt);
    if (processed_chars != e-b) {
      throw std::out_of_range("can't parse datetime '" + std::string(b, e-b) +
                              "' on row " + std::to_string(row+1));
    }
    z->getIndexPtr()->getcol(0).push_back(dt);
    for (size_t j=0; j<z->getArray().ncols(); ++j) {
      sep_read = readToken(fd, buf, b, e, sep, offset, bytes_read,quoted);    
      if (sep_read < 0) break;
      double d;
      int processed_chars = FromChar<double>().fromChar(b, e-b, d);
      if (processed_chars != e-b) {
        throw std::out_of_range("can't parse '" + std::string(b, e-b) + "' on row " +
                                std::to_string(row+1) + ", col " + std::to_string(j+1));
      }
      z->getArrayPtr().get()->getcol(j).push_back(d); 
    }
    std::cout << "read data sep_read: " << char(sep_read) << std::endl;
    if (sep_read >= 0 && sep_read != '\n') {
      throw std::out_of_range("incorrect number of elements in row " + std::to_string(row+1));
    }
    ++row;
  }    
  z->getIndexPtr()->resize(0, header ? row-1 : row);
  z->getArrayPtr()->resize(0, header ? row-1 : row);

  return z;
}


template<typename T>
void arr::writecsv_array(const Array<T>& a, const string& file, bool header, const char sep) {
  
  auto fd = open(file.c_str(),  O_RDWR | O_CREAT | O_TRUNC
                  , S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (!fd) {
    throw std::system_error(std::error_code(errno, std::system_category()), "open " + file);
  }

  try {
    static const size_t FMAX = 32;
    static const size_t BSZ = 64000;
    static_assert(BSZ > 10 * arr::zstring::STRING_SIZE, 
                  "for efficiency reasons, csv buf must be at least 10x max zstring");
    char buf[BSZ];
    char* p = buf;
    if (header) {
      for (arr::idx_type j=0; j<a.ncols(); ++j) {
        if (BSZ - (p - buf) < arr::zstring::STRING_SIZE) {
          // flush the buffer:
          auto res = write(fd, buf, p - buf);
          if (res < 0) {
            throw std::system_error(std::error_code(errno, std::system_category()), "write");
          }
          p = buf;              // go back to the start of the buffer
        }

        if (a.dim.size() > 2) {
          // we don't have a name per column, so we just write the numbers out:
          const string c = std::to_string(j);
          memcpy(p, c.c_str(), c.length());
          p += c.length();
        }
        else if (a.dim.size() > 1 && a.getnames(1).hasNames()) {
          size_t len = a.getnames(1)[j].length();
          memcpy(p, a.getnames(1)[j].c_str(), min(BSZ - (p - buf), len));
          p += len;
        }
        *p++ = j < a.ncols() - 1 ? sep : '\n';
      }
    }

    // now the header is done, continue with the data:
    ToChar<T> tc;                              
    for (arr::idx_type i=0; i<a.nrows(); ++i) {
      for (arr::idx_type j=0; j<a.ncols(); ++j) {

        if (BSZ - (p - buf) < FMAX) {
          auto res = write(fd, buf, p - buf);
          if (res < 0) {
            throw std::system_error(std::error_code(errno, std::system_category()), "write");
          }
          p = buf;
        }
        
        p += tc.toChar((*a.v[j])[i], p, (buf + BSZ) - p);
        *p++ = j < a.ncols() - 1 ? sep : '\n';
      }
    }
    
    if (p != buf) {
      auto res = write(fd, buf, p - buf);  // write what's left
      if (res < 0) {
        throw std::system_error(std::error_code(errno, std::system_category()), "write");
      }
    }
    close(fd);
  } catch (...) {
    close(fd);
    throw;
  }  
}


void arr::writecsv_zts(const zts& z, const string& file, bool header, const char sep) {
  std::cout << "write_zts" << std::endl; 
  int fd = open(file.c_str(),  O_RDWR | O_CREAT | O_TRUNC
                  , S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (!fd) {
    throw std::system_error(std::error_code(errno, std::system_category()), "open " + file);
  }

  try {
    static const size_t FMAX = 32;
    static const size_t BSZ = 64000;
    static_assert(BSZ > 10 * arr::zstring::STRING_SIZE, 
                  "for efficiency reasons, csv buf must be at least 10x max zstring");
    char buf[BSZ];
    char* p = buf;
    if (header) {
      size_t len = z.getIndex().getnames(0)[0].length();
      memcpy(p, z.getIndex().getnames(0)[0].c_str(), min(BSZ - (p - buf), len));
      *p++ = z.getArray().ncols() ? sep : '\n';      

      for (arr::idx_type j=0; j<z.getArray().ncols(); ++j) {
        if (BSZ - (p - buf) < arr::zstring::STRING_SIZE) {
          // flush the buffer:
          auto res = write(fd, buf, p - buf);
          if (res < 0) {
            throw std::system_error(std::error_code(errno, std::system_category()), "write");
          }
          p = buf;              // go back to the start of the buffer
        }

        if (z.getdim().size() > 1 && z.getArray().getnames(1).hasNames()) {
          size_t len = z.getArray().getnames(1)[j].length();
          memcpy(p, z.getArray().getnames(1)[j].c_str(), min(BSZ - (p - buf), len));
          p += len;
        }
        *p++ = j < z.getArray().ncols() - 1 ? sep : '\n';
      }
    }

    if (BSZ - (p - buf) < FMAX && write(fd, buf, p - buf) < 0) {
      throw std::system_error(std::error_code(errno, std::system_category()), "write");
      p = buf;
    }

    // now the header is done, continue with the data:
    for (arr::idx_type i=0; i<z.getArray().nrows(); ++i) {
      p += ToChar<Global::dtime>().toChar(z.getIndex()[i], p, (buf + BSZ) - p);
      *p++ = z.getArray().ncols() ? sep : '\n';      

      for (arr::idx_type j=0; j<z.getArray().ncols(); ++j) {

        if (BSZ - (p - buf) < FMAX && write(fd, buf, p - buf) < 0) {
          throw std::system_error(std::error_code(errno, std::system_category()), "write");
          p = buf;
        }
        
        p += ToChar<double>().toChar(z.getArray().getcol(j)[i], p, (buf + BSZ) - p);
        *p++ = j < z.getArray().ncols() - 1 ? sep : '\n';
      }
    }
    
    if (p != buf && write(fd, buf, p - buf) < 0) { // write what's left
      throw std::system_error(std::error_code(errno, std::system_category()), "write");
    }
    close(fd);
  } catch (...) {
    close(fd);
    throw;
  }
}


// double:
template 
arr::cow_ptr<arr::Array<double>> arr::readcsv_array<double>(const string&, bool, const char sep, const string&);
template 
void arr::writecsv_array(const Array<double>&, const string&, bool, const char);
// bool:
template 
arr::cow_ptr<arr::Array<bool>> arr::readcsv_array<bool>(const string&, bool, const char sep, const string&);
template 
void arr::writecsv_array(const Array<bool>&, const string&, bool, const char);
// Global::dtime:
template 
arr::cow_ptr<arr::Array<Global::dtime>> arr::readcsv_array<Global::dtime>(const string&, bool, const char sep, const string&);
template 
void arr::writecsv_array(const Array<Global::dtime>&, const string&, bool, const char);
