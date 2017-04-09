// (C) 2016-2017 Leonardo Silvestri
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


#include "zts.hpp"

static void checkDims(const arr::Array<double>& a,
                      const arr::Array<Global::dtime>& idx) {
  if (idx.getdim().size() != 1) {
    throw range_error("zts index must be a vector");
  }
  // note that we accept the special case of the null array:
  if (idx.size() != (a.getdim().size() ? a.getdim(0) : idx.size())) {
    std::cout << "idx.size(): " << idx.size() << ", a.getdim(0): " << a.getdim(0) << std::endl;
    throw range_error("mismatched dimensions for index and data");
  }
}

arr::zts::zts(arr::rsv_t,
              const arr::Vector<arr::idx_type>& dim_p,
              const std::vector<Vector<zstring>> cnames) :
  a(std::make_shared<arr::Array<double>>(rsv, dim_p, cnames)), 
  idx(std::make_shared<arr::Array<Global::dtime>>(rsv, arr::Vector<arr::idx_type>({dim_p[0]})))
{
  drop(*idx);
  checkDims(*a, *idx);
}


arr::zts::zts(const arr::Vector<arr::idx_type>& dim_p, 
              const arr::Vector<Global::dtime>& idx_p,
              const arr::Vector<double>& v_p, 
              const std::vector<Vector<zstring>> cnames,
              std::unique_ptr<AllocFactory>&& allocf_a,
              std::unique_ptr<AllocFactory>&& allocf_idx) :
  a(std::make_shared<arr::Array<double>>(dim_p, v_p, cnames, std::move(allocf_a))), 
  idx(std::make_shared<arr::Array<Global::dtime>>
      (arr::Vector<arr::idx_type>{idx_p.size()}, 
       idx_p, 
       vector<Vector<zstring>>(), 
       std::move(allocf_idx)))
{
  drop(*idx);
  checkDims(*a, *idx);
  if (!idx->isOrdered()) {
    throw range_error("index is not in ascending order");
  }
}


arr::zts::zts(arr::Array<Global::dtime> idx_p, 
              arr::Array<double> a_p,
              std::unique_ptr<AllocFactory>&& allocf_a,
              std::unique_ptr<AllocFactory>&& allocf_idx) :
  a(std::make_shared<arr::Array<double>>(a_p, std::move(allocf_a))), 
  idx(std::make_shared<arr::Array<Global::dtime>>(idx_p, std::move(allocf_idx)))
{
  drop(*idx);
  checkDims(*a, *idx);
  if (!idx->isOrdered()) {
    throw range_error("index is not in ascending order");
  }
}


// note, this constructor will throw in the initialization phase if
// the array 'a' can't get constructed (e.g. directory doesn't exist)
// or inside the constructor body if 'idx' doesn't exist, etc.
arr::zts::zts(std::unique_ptr<AllocFactory>&& allocf_a,
              std::unique_ptr<AllocFactory>&& allocf_idx) : 
  a(std::make_shared<arr::Array<double>>(std::move(allocf_a))), 
  idx(std::make_shared<arr::Array<Global::dtime>>(std::move(allocf_idx)))
{
  drop(*idx);
  checkDims(*a, *idx);
  if (!idx->isOrdered()) {
    throw range_error("index is not in ascending order");
  }  
}


arr::zts::zts(const zts& z,
              std::unique_ptr<AllocFactory>&& allocf_a,
              std::unique_ptr<AllocFactory>&& allocf_idx) 
  : a(std::make_shared<arr::Array<double>>(*z.a, std::move(allocf_a))),
    idx(std::make_shared<arr::Array<Global::dtime>>(*z.idx, std::move(allocf_idx)))
{
  // std::cout << "making a zts copy!" << std::endl;
  drop(*idx);
  checkDims(*a, *idx);
}


arr::zts::zts(zts&& z) {
  swap(a, z.a);
  swap(idx, z.idx);
}


arr::zts& arr::zts::append(const char* buf, size_t buflen, size_t& offset) {
  if (a->getdim().size() == 0) {
    throw std::out_of_range("append on null zts not implemented");        
  }
  idx->append(buf, buflen, offset);  // time array
  if (!idx->isOrdered()) {
    // stay in a coherent state:
    idx->resize(0, a->getdim(0));    
    idx->getcol(0).forceOrdered();   // use 'getcol', so 'forceOrdered()' 
                                     // doesn't also leak into array:
                                     // it's ugly enough like that
    throw std::range_error("index not ascending");
  }

  try {
    a->append(buf+offset, buflen-offset, offset); // followed by a double array
  } 
  catch (...) {
    idx->resize(0, a->getdim(0));    // stay in a coherent state!
    throw;
  }

  return *this;
}


arr::zts& arr::zts::appendVector(const char* buf, size_t buflen) {
  if (a->getdim().size() == 0) {
    // figure out if we want to support that...
    /// LLL, yes, we do!!!
    throw std::out_of_range("append vector on null zts not implemented");        
  }
  auto i = Vector<Global::dtime>(const_cast<char*>(buf), buflen);
  if (i.size() == 0) {
    throw std::out_of_range("arr::zts::appendVector: time vector has size 0");
  }  
  for (auto& e : i) {
    if (idx->size() && e <= (*idx)[idx->size()-1]) {
      idx->resize(0, a->getdim(0));
      throw std::out_of_range("append index not ascending");
    }
    idx->concat(e);
  }
  
  size_t idxsz = sizeof(RawVector<Global::dtime>) + i.size()*sizeof(Global::dtime);
  auto data = Vector<double>(const_cast<char*>(buf) + idxsz, buflen - idxsz);
  if (data.size() % i.size()) {
    idx->resize(0, a->getdim(0));
    throw std::out_of_range("mismatch between index and data");        
  }
  arr::idx_type nrows = data.size() / a->ncols();
  if (nrows != i.size()) { 
    idx->resize(0, a->getdim(0));
    throw std::out_of_range("size mismatch between idx and array");        
  }
  try {
    a->appendVector(const_cast<char*>(buf) + idxsz, buflen - idxsz);
  }
  catch (...) {
    idx->resize(0, a->getdim(0));
    throw;
  }
  return *this;
}


Global::buflen_pair arr::zts::to_buffer(size_t offset) const {
  // get the time index vector encoded length:
  size_t totalsz = offset + idx->getBufferSize() + a->getBufferSize();
  // allocate the buffer and encode the array:
  auto buf = std::make_unique<char[]>(totalsz);
  offset += idx->to_buffer(buf.get() + offset);
  offset += a->to_buffer(buf.get() + offset);
  return std::make_pair(std::move(buf), offset);
}


arr::zts& arr::zts::resize(arr::idx_type d, arr::idx_type sz, arr::idx_type from) {
  idx->resize(0, sz, from);
  a->resize(0, sz, from);
  return *this;
}


arr::zts arr::zts::subsetRows(arr::idx_type from, arr::idx_type to, bool dummy) const {
  return zts(idx->subsetRows(from, to, false), a->subsetRows(from, to, false));
}


arr::zts& arr::zts::abind(const zts& z, idx_type d, const string& prefix) {
  if (d == 0) {
    // if binding vertically, also bind the index:
    idx->concat(z.getIndex());
  }
  a->abind(*z.a, d, prefix);
  return *this;
}


arr::zts& arr::zts::abind(const Array<double>& u, idx_type d, const string& prefix) {
  if (d == 0) {
    throw range_error("can't bind an array to a zts in first dimension");
  }
  a->abind(u, d, prefix);
  return *this;
}

