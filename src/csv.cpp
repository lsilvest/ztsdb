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
  size_t toChar(T t, char* buf, size_t sz) {
    throw std::out_of_range("csv.write not implemented for type");
  };
};
template<typename T>
struct FromChar {
  size_t fromChar(const char* buf, int sz, T& t) {
    throw std::out_of_range("csv.read not implemented for type");
  };
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
    sb.Finalize(); // have to call it else it will be done in the destructor!
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

// zstring
template<>
struct ToChar<arr::zstring> {
  // grab the format from cfgmap LLL
  size_t toChar(arr::zstring t, char* buf, size_t sz) {
    memcpy(buf, t.c_str(), t.length());
    return t.length();
  }
};
template<>
struct FromChar<arr::zstring> {
  size_t fromChar(const char* buf, int sz, arr::zstring& t) {
    t = t + arr::zstring(buf, buf + sz);
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


static ssize_t readMore(int fd, char buf[], char*& tb, char*& te) {
  size_t offset = te - tb;
  if (offset >= BUFSZ) {
    throw std::range_error("token larger than buffer size");
  }
  memcpy(buf, tb, offset); // copy the remaining bytes of the buffer at the beginning
  tb = buf;
  te = buf + offset;
  ssize_t bytes_read = read(fd, te, BUFSZ - (te - tb));
  if (bytes_read <  0) return -1;
  if (bytes_read == 0) return offset ? 0 : -1;
  return offset + bytes_read;
}


int readToken(int fd,       ///< file descriptor
              char buf[],   ///< read buffer
              ssize_t& len, ///< length of buffer
              char*& tb,    ///< token begin
              char*& te,    ///< token end
              const char sep,
              bool& quoted) {
  if (te == 0) {
    if ((len = readMore(fd, buf, tb, te)) <= 0) return -1;
  }
  else {
    if (quoted) {
      if (++te >= buf + len && (len = readMore(fd, buf, tb, te)) <= 0) return -1;
    }
    if (++te >= buf + len && (len = readMore(fd, buf, tb, te)) <= 0) return -1;
  }
  quoted = *te=='"';
  if (quoted) {
    if (++te >= buf + len && (len = readMore(fd, buf, tb, te)) <= 0) return -1;
  }

  tb = te;
  for (;;) {
    if (quoted) {
      if (*te=='"') {
        if (te+1 >= buf + len && (len = readMore(fd, buf, tb, te)) <= 0) return len;
        if (te[1] == sep || te[1] == '\n') return te[1];
        else throw std::out_of_range("quote does not terminate token");
      }
    }
    else if (*te == sep || *te == '\n') return *te;
    ++te;
    if (te >= buf + len && (len = readMore(fd, buf, tb, te)) <= 0) return len;
  }
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

  try {
    char buf[BUFSZ];
    ssize_t len = 0;
    char *b = nullptr, *e = nullptr;
    bool quoted = false;
  
    auto ap = arr::make_cow<arr::Array<T>>(true, 
                                           arr::Vector<arr::idx_type>{}, 
                                           arr::Vector<T>(), 
                                           vector<arr::Vector<arr::zstring>>(), 
                                           getAllocFactory(mmapfile));
    auto& a = *ap.get();          // get so we don't make a copy
  
    if (header) {
      for (;;) {
        auto sep_read = readToken(fd, buf, len, b, e, sep, quoted);
        if (sep_read < 0) break;
        a.cbind(arr::Array<T>({0,1}, arr::Vector<T>(), {{}, {std::string(b, e-b)}}));
        if (sep_read == '\n') break;
      }
    }
    else {
      for (;;) {
        T d;
        auto sep_read = readToken(fd, buf, len, b, e, sep, quoted);    
        if (sep_read < 0) break;
        int processed_chars = FromChar<T>().fromChar(b, e-b, d);
        if (processed_chars != e-b) {
          throw std::out_of_range("can't parse '" + std::string(b, e-b));
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
      for (size_t j=0; j<a.ncols(); ++j) {
        sep_read = readToken(fd, buf, len, b, e, sep, quoted);    
        if (sep_read < 0) break;
        T d;
        int processed_chars = FromChar<T>().fromChar(b, e-b, d);
        if (processed_chars != e-b) {
          throw std::out_of_range("can't parse '" + std::string(b, e-b) +
                                  "', col " + std::to_string(j+1));
        }
        a.getcol(j).push_back(d); 
      }
      if (sep_read < 0) break;
      if (sep_read > 0 && sep_read != '\n') {
        throw std::out_of_range("incorrect number of elements");
      }
      ++row;
    }    
    a.resize(0, header ? row-1 : row);

    return ap;
  }
  catch (std::exception& e) {
    throw std::out_of_range(e.what() + " on row "s + std::to_string(row+1));
  }  
}


arr::cow_ptr<arr::zts> arr::readcsv_zts(const string& file,
                                        bool header,
                                        const char sep,
                                        const string& mmapfile) {

  arr::idx_type row = 0;

  try {
    int fd = open(file.c_str(), O_RDONLY);
    if (fd < 0) {
      throw std::system_error(std::error_code(errno, std::system_category()), "open " + file);
    }

    char buf[BUFSZ];
    char *b = nullptr, *e = nullptr;
    ssize_t len = 0;
    bool quoted = false;
  
    auto z = arr::make_cow<arr::zts>(true, 
                                     arr::Vector<arr::idx_type>{0,0}, 
                                     arr::Vector<Global::dtime>(), 
                                     arr::Vector<double>(), 
                                     vector<arr::Vector<arr::zstring>>(), 
                                     getAllocFactory(mmapfile));

  
    if (header) {
      auto sep_read = readToken(fd, buf, len, b, e, sep, quoted);
      if (sep_read < 0) return z; // it's an empty file..., throw? LLL
      for (;;) {
        auto sep_read = readToken(fd, buf, len, b, e, sep, quoted);
        if (sep_read < 0) break;
        z->abind(arr::Array<double>({0,1}, arr::Vector<double>(), {{}, {std::string(b, e-b)}}), 1);
        if (sep_read == '\n') break;
      }
    }
    else {
      auto sep_read = readToken(fd, buf, len, b, e, sep, quoted);
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
        auto sep_read = readToken(fd, buf, len, b, e, sep, quoted);    
        if (sep_read < 0) break;
        int processed_chars = FromChar<double>().fromChar(b, e-b, d);
        if (processed_chars != e-b) {
          throw std::out_of_range("can't parse '" + std::string(b, e-b));
        }
        z->getArrayPtr()->abind(arr::Array<double>({1,1}, arr::Vector<double>{d}), 1); 
        if (sep_read == '\n') break;
      }
    }
    ++row;
    
    for (;;) {
      auto sep_read = readToken(fd, buf, len, b, e, sep, quoted);    
      if (sep_read < 0) break;
      Global::dtime dt;
      int processed_chars = FromChar<Global::dtime>().fromChar(b, e-b, dt);
      if (processed_chars != e-b) {
        throw std::out_of_range("can't parse datetime '" + std::string(b, e-b));
      }
      z->getIndexPtr()->getcol(0).push_back(dt);
      for (size_t j=0; j<z->getArray().ncols(); ++j) {
        sep_read = readToken(fd, buf, len, b, e, sep, quoted);
        if (sep_read < 0) break;
        double d;
        int processed_chars = FromChar<double>().fromChar(b, e-b, d);
        if (processed_chars != e-b) {
          throw std::out_of_range("can't parse '" + std::string(b, e-b) +
                                  "', col " + std::to_string(j+1));
        }
        z->getArrayPtr().get()->getcol(j).push_back(d); 
      }
      if (sep_read >= 0 && sep_read != '\n') {
        throw std::out_of_range("incorrect number of elements");
      }
      ++row;
    }    
    z->getIndexPtr()->resize(0, header ? row-1 : row);
    z->getArrayPtr()->resize(0, header ? row-1 : row);

    return z;
  }
  catch (std::exception& e) {
    throw std::out_of_range(e.what() + " on row "s + std::to_string(row+1));
  }  
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
        
        p += tc.toChar(a.getcol(j)[i], p, (buf + BSZ) - p);
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
  int fd = open(file.c_str(),  O_RDWR | O_CREAT | O_TRUNC
                  , S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (!fd) {
    throw std::system_error(std::error_code(errno, std::system_category()), "open " + file);
  }

  try {
    static const size_t FMAX = 4000;
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
          if (write(fd, buf, p - buf) < 0) {
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

    // now the header is done, continue with the data:
    for (arr::idx_type i=0; i<z.getArray().nrows(); ++i) {
      if (BSZ - (p - buf) < FMAX) {
        if (write(fd, buf, p - buf) < 0) {
          throw std::system_error(std::error_code(errno, std::system_category()), "write");
        }
        p = buf;
      }
      
      p += ToChar<Global::dtime>().toChar(z.getIndex()[i], p, (buf + BSZ) - p);
      *p++ = z.getArray().ncols() ? sep : '\n';      

      for (arr::idx_type j=0; j<z.getArray().ncols(); ++j) {
        if (BSZ - (p - buf) < FMAX) {
          if (write(fd, buf, p - buf) < 0) {
            throw std::system_error(std::error_code(errno, std::system_category()), "write");
          }
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
// character
template 
arr::cow_ptr<arr::Array<arr::zstring>> arr::readcsv_array<arr::zstring>(const string&, bool, const char sep, const string&);
template 
void arr::writecsv_array(const Array<arr::zstring>&, const string&, bool, const char);
// interval
template 
arr::cow_ptr<arr::Array<tz::interval>> arr::readcsv_array<tz::interval>(const string&, bool, const char sep, const string&);
template 
void arr::writecsv_array(const Array<tz::interval>&, const string&, bool, const char);
// period
template 
arr::cow_ptr<arr::Array<tz::period>> arr::readcsv_array<tz::period>(const string&, bool, const char sep, const string&);
template 
void arr::writecsv_array(const Array<tz::period>&, const string&, bool, const char);
