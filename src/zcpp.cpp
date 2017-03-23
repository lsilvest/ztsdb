// Copyright (C) 2017 Leonardo Silvestri
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


#include "zcpp.hpp"
#include "zcpp_stdlib.hpp"
#include "zc.h"
#include "net_handler.hpp"


static const auto MAXLEN = 255;

size_t arr::getHeaderLength(const std::vector<std::string>& names) {
  // note + 2: 1 byte for size and one byte for terminating 0
  auto combined_length = accumulate(names.begin(), names.end(), 0.0,
                           [](size_t s, const std::string& e) { return s + e.size() + 2; });
  const unsigned alen = getAlignedLength(combined_length, Global::STRALIGN);
  return net::INIT_OFFSET + sizeof(Global::MsgType) + sizeof(size_t) + alen;
}



static void writeHeaderHeader(Global::buflen_pair& buf,
                              Global::MsgType msgtype,
                              size_t& offset) {
  auto magicnb = hton64(Global::MAGICNB);
  memcpy(buf.first.get() + offset, &magicnb, sizeof(Global::MAGICNB));
  offset += sizeof(Global::MAGICNB);
  auto totalsz = hton64(buf.second);
  memcpy(buf.first.get() + offset, &totalsz, sizeof(totalsz));
  offset += sizeof(totalsz);
  auto msgtype64 = hton64(static_cast<uint64_t>(msgtype));
  memcpy(buf.first.get() + offset, &msgtype64, sizeof(msgtype));
  offset += sizeof(msgtype);
}


static void writeLengthAndPadding(size_t nameslen,
                                  Global::buflen_pair& buf,
                                  size_t& offset,
                                  uint64_t namesz_loc) {
  size_t paddingsz = getAlignedLength(offset - namesz_loc, Global::STRALIGN) - (offset - namesz_loc);
  memset(buf.first.get() + offset, 0, paddingsz);

  // now write the number of strings and the total length:
  const uint64_t namessz = (offset - namesz_loc + paddingsz) | (nameslen << 32);
  const uint64_t namessz_hton = hton64(namessz);
  memcpy(buf.first.get() + namesz_loc, &namessz_hton, sizeof(namessz_hton));
  offset += sizeof(namessz);
}


int arr::writeHeader(Global::buflen_pair& buf,
                     Global::MsgType msgtype,
                     const std::vector<std::string>& names) {
  size_t offset = 0;
  writeHeaderHeader(buf, msgtype, offset);
  
  const uint64_t namesz_loc = offset; // remember where to write length information
  offset += sizeof(uint64_t);
  for (const auto& n : names) {
    if (n.size() > MAXLEN) {
      return -1;
    }
    buf.first.get()[offset++] = n.size();
    memcpy(buf.first.get() + offset, n.c_str(), n.size());
    offset += n.size();
    buf.first.get()[offset++] = 0;
  }
  
  writeLengthAndPadding(names.size(), buf, offset, namesz_loc);
  return 0;
}


// C linkage functions -------------------

static size_t getHeaderLength(const char** names, size_t nameslen) {
  // note + 2: 1 byte for size and one byte for terminating 0

  size_t combined_length = 0;
  for (size_t i=0; i<nameslen; ++i) {
    combined_length += strnlen(names[i], MAXLEN) + 2;
  }
  const unsigned alen = getAlignedLength(combined_length, Global::STRALIGN);
  return net::INIT_OFFSET + sizeof(Global::MsgType) + sizeof(size_t) + alen;
}

static int writeHeader(Global::buflen_pair& buf,
                       Global::MsgType msgtype,
                       const char** names,
                       size_t nameslen) {
  size_t offset = 0;
  writeHeaderHeader(buf, msgtype, offset);
  
  const uint64_t namesz_loc = offset; // remember where to write length information
  offset += sizeof(uint64_t);
  for (size_t i = 0; i<nameslen; ++i) {
    auto slen = strnlen(names[i], MAXLEN+1);
    if (slen == MAXLEN+1) {
      return -1;
    }
    buf.first.get()[offset++] = slen;
    memcpy(buf.first.get() + offset, names[i], slen);
    offset += slen;
    buf.first.get()[offset++] = 0;
  }
  
  writeLengthAndPadding(nameslen, buf, offset, namesz_loc);
  return 0;
}

int make_append_msg(const char* names[], size_t nameslen,
                    const int64_t* idx, size_t idxlen,
                    const double* data, size_t datalen,
                    char** buf, size_t* buflen)
{
  if (idxlen == 0 || datalen % idxlen) {
    cout << idxlen << " " << datalen << endl;
    return -1;
  }
  for (size_t i=1; i<idxlen; ++i) {
    if (idx[i-1] >= idx[i]) {
      return -2;
    }
  }

  const auto headersz  = getHeaderLength(names, nameslen);
  const auto datasz    = datalen*sizeof(double);
  const auto rawvecsz  = sizeof(arr::RawVector<double>);
  const auto idxdatasz = idxlen*sizeof(Global::dtime);
  const auto totalsz   = headersz + rawvecsz + idxdatasz + rawvecsz + datasz;
  auto buf_p = std::make_pair(std::make_unique<char[]>(totalsz), totalsz);
  if (writeHeader(buf_p, Global::MsgType::APPEND_VECTOR, names, nameslen) < 0) {
    return -3;
  }

  const arr::RawVector<Global::dtime> idx_rv{arr::TypeNumber<Global::dtime>::n, idxlen, 1};
  memcpy(buf_p.first.get() + headersz, &idx_rv, rawvecsz);
  memcpy(buf_p.first.get() + headersz + rawvecsz, idx, idxdatasz);    

  const arr::RawVector<double> v_rv{arr::TypeNumber<double>::n, datalen, 1};
  memcpy(buf_p.first.get() + headersz + rawvecsz + idxdatasz, &v_rv, rawvecsz);
  memcpy(buf_p.first.get() + headersz + rawvecsz + idxdatasz + rawvecsz, data, datasz);    

  *buf = buf_p.first.release();
  *buflen = buf_p.second;

  return 0;
}


// C++ functions with declaration using only C++ stdlib -------------------

Global::buflen_pair arr::make_append_msg(const std::vector<std::string>& names, 
                                         const arr::Vector<Global::dtime>& idx, 
                                         const arr::Vector<double>& v)
{
  if (idx.size() == 0) {
    throw std::out_of_range("make_append_msg: idx has size 0");
  }
  if (v.size() % idx.size()) {
    throw std::out_of_range("make_append_msg: idx and v have incompatible lengths");
  }
  if (!idx.isOrdered()) {
    throw std::out_of_range("make_append_msg: idx must be sorted");
  }
  
  const auto headersz  = getHeaderLength(names);
  const auto datasz    = v.size()*sizeof(double);
  const auto rawvecsz  = sizeof(RawVector<double>);
  const auto idxdatasz = idx.size()*sizeof(Global::dtime);
  const auto totalsz   = headersz + rawvecsz + idxdatasz + rawvecsz + datasz;
  auto buf = std::make_pair(std::make_unique<char[]>(totalsz), totalsz);
  writeHeader(buf, Global::MsgType::APPEND_VECTOR, names);
  memcpy(buf.first.get() + headersz, idx.getRawVectorPtr(), rawvecsz);
  memcpy(buf.first.get() + headersz + rawvecsz, &idx.front(), idxdatasz);    
  memcpy(buf.first.get() + headersz + rawvecsz + idxdatasz, v.getRawVectorPtr(), rawvecsz);
  memcpy(buf.first.get() + headersz + rawvecsz + idxdatasz + rawvecsz, &v.front(), datasz);    
  return buf;
}


Global::buflen_pair arr::make_append_msg(const std::vector<std::string>& names, 
                                         const std::vector<Global::dtime>& idx, 
                                         const std::vector<double>& v)
{
  if (idx.size() == 0) {
    throw std::out_of_range("make_append_msg: idx has size 0");
  }
  if (v.size() % idx.size()) {
    throw std::out_of_range("make_append_msg: idx and v have incompatible lengths");
  }
  for (size_t i=1; i<idx.size(); ++i) {
    if (idx[i-1] >= idx[i]) {
      throw std::out_of_range("make_append_msg: idx must be sorted");
    }
  }
  
  const auto headersz  = getHeaderLength(names);
  const auto datasz    = v.size()*sizeof(double);
  const auto rawvecsz  = sizeof(RawVector<double>);
  const auto idxdatasz = idx.size()*sizeof(Global::dtime);
  const auto totalsz   = headersz + rawvecsz + idxdatasz + rawvecsz + datasz;
  auto buf = std::make_pair(std::make_unique<char[]>(totalsz), totalsz);
  writeHeader(buf, Global::MsgType::APPEND_VECTOR, names);

  const RawVector<Global::dtime> idx_rv{TypeNumber<Global::dtime>::n, idx.size(), 1};
  memcpy(buf.first.get() + headersz, &idx_rv, rawvecsz);
  memcpy(buf.first.get() + headersz + rawvecsz, &idx.front(), idxdatasz);    

  const RawVector<double> v_rv{TypeNumber<double>::n, v.size(), 1};
  memcpy(buf.first.get() + headersz + rawvecsz + idxdatasz, &v_rv, rawvecsz);
  memcpy(buf.first.get() + headersz + rawvecsz + idxdatasz + rawvecsz, &v.front(), datasz);    
  return buf;
}
