// Copyright (C) 2015 Leonardo Silvestri
//
// This file is part of ztsdb.
//
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
#include "net_handler.hpp"


size_t arr::getHeaderLength(const std::string& name) {
  const unsigned alen = getAlignedLength(name, Global::STRALIGN);
  return net::INIT_OFFSET + sizeof(Global::MsgType) + sizeof(size_t) + alen;
}


void arr::writeHeader(arr::buflen_pair& buf, Global::MsgType msgtype, const std::string& name) {
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
  size_t paddingsz = getAlignedLength(name, Global::STRALIGN) - name.size();
  memset(buf.first.get() + offset, 0, paddingsz);
}


arr::buflen_pair arr::make_append_msg(const string& name, const arr::zts& z) {
  auto buf = z.to_buffer(getHeaderLength(name));
  writeHeader(buf, Global::MsgType::APPEND, name);
  return buf;
}


arr::buflen_pair arr::make_append_msg(const std::string& name, 
                                      const arr::Vector<Global::dtime> idx, 
                                      const arr::Vector<double>& v) {
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
  memcpy(buf.first.get() + headersz, idx.getRawVectorPtr(), rawvecsz);
  memcpy(buf.first.get() + headersz + rawvecsz, &idx.front(), idxdatasz);    
  memcpy(buf.first.get() + headersz + rawvecsz + idxdatasz, v.getRawVectorPtr(), rawvecsz);
  memcpy(buf.first.get() + headersz + rawvecsz + idxdatasz + rawvecsz, &v.front(), datasz);    
  return buf;
}
