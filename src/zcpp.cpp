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


size_t arr::getHeaderLength(const std::string& name) {
  const unsigned alen = getAlignedLength(name.size(), Global::STRALIGN);
  return net::INIT_OFFSET + sizeof(Global::MsgType) + sizeof(size_t) + alen;
}


void arr::writeHeader(Global::buflen_pair& buf, Global::MsgType msgtype, const std::string& name) {
  size_t offset = 0;
  auto magicnb = hton64(Global::MAGICNB);
  memcpy(buf.first.get() + offset, &magicnb, sizeof(Global::MAGICNB));
  offset += sizeof(Global::MAGICNB);
  auto totalsz = hton64(buf.second);
  memcpy(buf.first.get() + offset, &totalsz, sizeof(totalsz));
  offset += sizeof(totalsz);
  auto msgtype64 = hton64(static_cast<uint64_t>(msgtype));
  memcpy(buf.first.get() + offset, &msgtype64, sizeof(msgtype));
  offset += sizeof(msgtype);

  const size_t namesz = hton64(name.size());
  memcpy(buf.first.get() + offset, &namesz, sizeof(namesz));
  offset += sizeof(namesz);
  memcpy(buf.first.get() + offset, name.c_str(), name.size());
  offset += name.size();
  size_t paddingsz = getAlignedLength(name.size(), Global::STRALIGN) - name.size();
  memset(buf.first.get() + offset, 0, paddingsz);
}


// C linkage functions -------------------

int make_append_msg(const char* name,
                    const int64_t* idx, size_t ilen,
                    const double* data, size_t len,
                    char** buf, size_t* buflen)
{
  if (ilen == 0 || len % ilen) {
    return -1;
  }
  for (size_t i=1; i<ilen; ++i) {
    if (idx[i-1] >= idx[i]) {
      return -2;
    }
  }

  const auto headersz  = arr::getHeaderLength(name);
  const auto datasz    = len*sizeof(double);
  const auto rawvecsz  = sizeof(arr::RawVector<double>);
  const auto idxdatasz = ilen*sizeof(Global::dtime);
  const auto totalsz   = headersz + rawvecsz + idxdatasz + rawvecsz + datasz;
  auto buf_p = std::make_pair(std::make_unique<char[]>(totalsz), totalsz);
  arr::writeHeader(buf_p, Global::MsgType::APPEND_VECTOR, name);

  const arr::RawVector<Global::dtime> idx_rv{arr::TypeNumber<Global::dtime>::n, ilen, 1};
  memcpy(buf_p.first.get() + headersz, &idx_rv, rawvecsz);
  memcpy(buf_p.first.get() + headersz + rawvecsz, idx, idxdatasz);    

  const arr::RawVector<double> v_rv{arr::TypeNumber<double>::n, len, 1};
  memcpy(buf_p.first.get() + headersz + rawvecsz + idxdatasz, &v_rv, rawvecsz);
  memcpy(buf_p.first.get() + headersz + rawvecsz + idxdatasz + rawvecsz, data, datasz);    

  *buf = buf_p.first.release();
  *buflen = buf_p.second;

  return 0;
}


// C++ functions with declaration using only C++ stdlib -------------------

Global::buflen_pair arr::make_append_msg(const std::string& name, 
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
  
  const auto headersz  = getHeaderLength(name);
  const auto datasz    = v.size()*sizeof(double);
  const auto rawvecsz  = sizeof(RawVector<double>);
  const auto idxdatasz = idx.size()*sizeof(Global::dtime);
  const auto totalsz   = headersz + rawvecsz + idxdatasz + rawvecsz + datasz;
  auto buf = std::make_pair(std::make_unique<char[]>(totalsz), totalsz);
  writeHeader(buf, Global::MsgType::APPEND_VECTOR, name);
  memcpy(buf.first.get() + headersz, idx.getRawVectorPtr(), rawvecsz);
  memcpy(buf.first.get() + headersz + rawvecsz, &idx.front(), idxdatasz);    
  memcpy(buf.first.get() + headersz + rawvecsz + idxdatasz, v.getRawVectorPtr(), rawvecsz);
  memcpy(buf.first.get() + headersz + rawvecsz + idxdatasz + rawvecsz, &v.front(), datasz);    
  return buf;
}


Global::buflen_pair arr::make_append_msg(const std::string& name, 
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
  
  const auto headersz  = getHeaderLength(name);
  const auto datasz    = v.size()*sizeof(double);
  const auto rawvecsz  = sizeof(RawVector<double>);
  const auto idxdatasz = idx.size()*sizeof(Global::dtime);
  const auto totalsz   = headersz + rawvecsz + idxdatasz + rawvecsz + datasz;
  auto buf = std::make_pair(std::make_unique<char[]>(totalsz), totalsz);
  writeHeader(buf, Global::MsgType::APPEND_VECTOR, name);

  const RawVector<Global::dtime> idx_rv{TypeNumber<Global::dtime>::n, idx.size(), 1};
  memcpy(buf.first.get() + headersz, &idx_rv, rawvecsz);
  memcpy(buf.first.get() + headersz + rawvecsz, &idx.front(), idxdatasz);    

  const RawVector<double> v_rv{TypeNumber<double>::n, v.size(), 1};
  memcpy(buf.first.get() + headersz + rawvecsz + idxdatasz, &v_rv, rawvecsz);
  memcpy(buf.first.get() + headersz + rawvecsz + idxdatasz + rawvecsz, &v.front(), datasz);    
  return buf;
}
