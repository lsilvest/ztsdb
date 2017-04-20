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


#include <cstdint>
#include <random>
#include "base_funcs.hpp"
#include "base_types.hpp"
#include "valuevar.hpp"
#include "conversion_funcs.hpp"
#include "display.hpp"


// #define DEBUG_BFA

/// Define type equivalence for the purpose of 'c', 'abind', etc.
static val::ValType equiv(val::ValType vt)  {
  switch (vt) {
  case val::vt_clos:	
  case val::vt_builting:	
  case val::vt_connection:
  case val::vt_timer:
    return val::vt_list;
  default:
    return vt;
  }
}


// evaluate what the final type of c should be. Here again we follow
// R's casts:
// R's hierarchy raw < logical < integer < double < complex < character < list
static val::ValType getCType(vector<val::VBuiltinG::arg_t>::const_iterator b,
                             vector<val::VBuiltinG::arg_t>::const_iterator e) {
  return accumulate(b,
                    e, 
                    val::vt_null, 
                    [](val::ValType x, const val::VBuiltinG::arg_t& y) {
                      auto yt = equiv(static_cast<val::ValType>(val::getVal(y).which()));
                      return funcs::operator<(x, yt) ? yt : x; 
                    });
} 


// conversions to Value
namespace arr {
  template<> val::Value convert(const double& u) { return val::make_array(u); }
  template<> val::Value convert(const bool& u) { return val::make_array(u); }
  template<> val::Value convert(const Global::dtime& u) { return val::make_array(u); }
  template<> val::Value convert(const Global::duration& u) { return val::make_array(u); }
  template<> val::Value convert(const tz::period& u) { return val::make_array(u); }
  template<> val::Value convert(const tz::interval& u) { return val::make_array(u); }
  template<> val::Value convert(const arr::zstring& u) { return val::make_array(u); }
}

template <typename T>
static Array<T> makeVector(arr::Array<T>& r,
                           vector<val::VBuiltinG::arg_t>::iterator begin,
                           vector<val::VBuiltinG::arg_t>::iterator end) {
#ifdef DEBUG_BFA
  cout << "makeVector<T>: " << endl;
#endif

  for (auto iter=begin; iter!=end; ++iter) {
    auto& e = *iter;
#ifdef DEBUG_BFA
    std::cout << "| name: " << val::getName(e) << std::endl;
#endif
    switch (val::getVal(e).which()) {
    case val::vt_double: 
      r.concat(vectorize(*get<val::SpVAD>(val::getVal(e))), val::getName(e));
      break;
    case val::vt_bool: 
      r.concat(vectorize(*get<val::SpVAB>(val::getVal(e))), val::getName(e));
      break;
    case val::vt_string: 
      r.concat(vectorize(*get<val::SpVAS>(val::getVal(e))), val::getName(e));
      break;
    case val::vt_time: 
      r.concat(vectorize(*get<val::SpVADT>(val::getVal(e))), val::getName(e));
      break;
    case val::vt_duration: 
      r.concat(vectorize(*get<val::SpVADUR>(val::getVal(e))), val::getName(e));
      break;
    case val::vt_period: 
      r.concat(vectorize(*get<val::SpVAPRD>(val::getVal(e))), val::getName(e));
      break;
    case val::vt_interval: 
      r.concat(vectorize(*get<val::SpVAIVL>(val::getVal(e))), val::getName(e));
      break;
    case val::vt_list: 
      r.concat(vectorize(get<val::SpVList>(val::getVal(e))->a), val::getName(e));
      break;
    case val::vt_builting:
    case val::vt_clos:
    case val::vt_connection:
    case val::vt_timer:
    case val::vt_zts:
      r.concat(val::getVal(e), val::getName(e));    
      break;
    case val::vt_null:          // like in R, NULLs are just ignored in 'c'
      break;
    default:
      throw interp::EvalException("invalid type", val::getLoc(e));
    }
  }
  return r;
}


template <val::ValType vt>
static val::Value& cHelper(val::Value& val, vector<val::VBuiltinG::arg_t>& v) {
  using T  = typename val::getelttype<vt>::TP;
  using SP = typename val::gettype<vt>::TP;
  if (val.which() == vt && val::isRef(val)) {
    auto& a = get<SP>(val);
    a->addprefix(val::getName(v[0]), 0);
    makeVector<T>(*a, v.begin()+1, v.end()); // potentially inplace
    return val;
  }
  else {
    if (val::isRef(val)) {
      throw interp::EvalException("cannot convert the type of a reference", val::getLoc(v[0]));
    }
    auto a = Array<T>(rsv, Vector<idx_type>{0});
    val = arr::make_cow<Array<T>>(arr::NOFLAGS, makeVector<T>(a, v.begin(), v.end())); // copy
    return val;
  }
}


template <>
val::Value& cHelper<val::vt_list>(val::Value& val, vector<val::VBuiltinG::arg_t>& v) {
  if (val.which() == val::vt_list && val::isRef(val)) {
    auto& l = get<val::SpVList>(val);
    l->a.addprefix(val::getName(v[0]), 0);
    makeVector<val::Value>(l->a, v.begin()+1, v.end()); // potentially inplace
    return val;
  }
  else {
    if (val::isRef(val)) {
      throw interp::EvalException("cannot convert the type of a reference", val::getLoc(v[0]));
    }
    auto a = Array<val::Value>(rsv, Vector<idx_type>{0});
    val = arr::make_cow<val::VList>(arr::NOFLAGS,
                                    makeVector<val::Value>(a, v.begin(), v.end())); // copy
    return val;
  }
}


val::Value funcs::c(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
#ifdef DEBUG_BFA
  std::cout << "c..." << std::endl;
#endif
  auto vt = getCType(v.cbegin(), v.cend()); // examines all elts to determine type of vector
  if (vt == val::vt_zts) {
    vt = val::vt_list;
  }
  
  switch (vt) {
  case val::vt_double:
    return cHelper<val::vt_double>(val::getVal(v[0]), v);
  case val::vt_bool:
    return cHelper<val::vt_bool>(val::getVal(v[0]), v);
  case val::vt_string:
    return cHelper<val::vt_string>(val::getVal(v[0]), v);
  case val::vt_time:
    return cHelper<val::vt_time>(val::getVal(v[0]), v);
  case val::vt_duration:
    return cHelper<val::vt_duration>(val::getVal(v[0]), v);
  case val::vt_period:
    return cHelper<val::vt_period>(val::getVal(v[0]), v);
  case val::vt_interval:
    return cHelper<val::vt_interval>(val::getVal(v[0]), v);
  case val::vt_list:
    return cHelper<val::vt_list>(val::getVal(v[0]), v);
  case val::vt_null:
    return val::VNull();
  default:
    throw interp::EvalException("invalid type", val::getLoc(v[0]));
  }
}


val::Value funcs::length(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return val::make_array(static_cast<double>(size(val::getVal(v[0]))));
}


val::Value funcs::t(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  switch (val::getVal(v[0]).which()) {
  case val::vt_double:
    return arr::make_cow<val::VArrayD>(false, arr::transpose(*get<val::SpVAD>(val::getVal(v[0]))));
    break;
  case val::vt_bool:
    return arr::make_cow<val::VArrayB>(false, arr::transpose(*get<val::SpVAB>(val::getVal(v[0]))));
    break;
  case val::vt_string:
    return arr::make_cow<val::VArrayS>(false, arr::transpose(*get<val::SpVAS>(val::getVal(v[0]))));
    break;
  case val::vt_time:
    return arr::make_cow<val::VArrayDT>(false, arr::transpose(*get<val::SpVADT>(val::getVal(v[0]))));
    break;
  case val::vt_duration:
    return arr::make_cow<val::VArrayDUR>(false, arr::transpose(*get<val::SpVADUR>(val::getVal(v[0]))));
    break;
  case val::vt_period:
    return arr::make_cow<val::VArrayPRD>(false, arr::transpose(*get<val::SpVAPRD>(val::getVal(v[0]))));
    break;
  case val::vt_interval:
    return arr::make_cow<val::VArrayIVL>(false, arr::transpose(*get<val::SpVAIVL>(val::getVal(v[0]))));
    break;
  default: 
    throw interp::EvalException("argument is not a matrix", val::getLoc(v[0]));
  }
}
  

static vector<Vector<zstring>> convertToDnames(const val::Value& v) {
  auto vvs = vector<Vector<zstring>>();
  // dimnames can be VNull, VList or VArrayS (for an n x 1 matrix
  switch (v.which()) {
  case val::vt_null:
    break;
  case val::vt_list: {
    auto l = get<val::SpVList>(v);
    for (size_t j=0; j<l->size(); ++j) {
      auto vs = Vector<zstring>();
      switch (l->a[j].which()) {
      case val::vt_null:
        break;
      case val::vt_string: {
        const auto& as = *get<val::SpVAS>(l->a[j]);
        for (auto se:(*as.v[0])) {
          vs.push_back(static_cast<string>(se));
        }
        break;
      }
      default:
        throw std::domain_error("incorrect data type for dimension name");        
      }
      vvs.push_back(vs);
    }
    break;
  }
  case val::vt_string: {
    const auto& vs = arrvector<zstring,zstring>(*get<val::SpVAS>(v));
    vvs.push_back(vs);
    break;
  }
  default:
    throw std::domain_error("incorrect data type for dimension name");
  }
  return vvs;
} 


static std::unique_ptr<arr::AllocFactory> getAllocFactory(const arr::zstring& filename) {
  if (filename.size()) {
   return std::make_unique<arr::MmapAllocFactory>(std::string(filename), false);
  }
  else {
    return  std::make_unique<arr::MemAllocFactory>();
  }
}


val::Value make_array_helper(const val::Value& v, 
                             Vector<arr::idx_type> idx,
                             const std::vector<arr::Vector<arr::zstring>>& dimnames,
                             const arr::zstring& filename) 
{
  /// use 'try' and create specific array exceptions so we can have better error reporting LLL
  unsigned flags = filename.size() ? arr::LOCKED : arr::NOFLAGS;
  std::unique_ptr<arr::AllocFactory> alloc = getAllocFactory(filename);
  switch (v.which()) {
  case val::vt_double:
    return arr::make_cow<val::VArrayD>(flags, idx, *get<val::SpVAD>(v), dimnames, std::move(alloc));
  case val::vt_bool:
    return arr::make_cow<val::VArrayB>(flags, idx, *get<val::SpVAB>(v), dimnames, std::move(alloc));
  case val::vt_string: 
    return arr::make_cow<val::VArrayS>(flags, idx, *get<val::SpVAS>(v), dimnames, std::move(alloc));
  case val::vt_time:
    return arr::make_cow<val::VArrayDT>(flags, idx, *get<val::SpVADT>(v), dimnames, std::move(alloc));
  case val::vt_duration:
    return arr::make_cow<val::VArrayDUR>(flags, idx, *get<val::SpVADUR>(v), dimnames, std::move(alloc));
  case val::vt_interval:
    return arr::make_cow<val::VArrayIVL>(flags, idx, *get<val::SpVAIVL>(v), dimnames, std::move(alloc));
  case val::vt_period:
    return arr::make_cow<val::VArrayPRD>(flags, idx, *get<val::SpVAPRD>(v), dimnames, std::move(alloc));
  default: 
    throw std::domain_error("incorrect data type");
  }   
}


val::Value funcs::make_matrix(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
#ifdef DEBUG_BFA
  std::cout << "make_matrix..." << std::endl;
#endif
  enum { DATA, NROW, NCOL, BYROW, DIMNAMES, FILE };

  auto idx = Vector<idx_type>{static_cast<idx_type>(val::get_scalar<double>(val::getVal(v[NROW]))), 
                              static_cast<idx_type>(val::get_scalar<double>(val::getVal(v[NCOL])))};
  auto byrow = val::get_scalar<bool>(val::getVal(v[BYROW]));
  if (byrow) {
    throw interp::EvalException("'byrow' not implemented yet", val::getLoc(v[BYROW]));
  }
  const auto& dimnames = convertToDnames(val::getVal(v[DIMNAMES]));
  const auto& filename = val::get_scalar<arr::zstring>(val::getVal(v[FILE]));
 
  return make_array_helper(val::getVal(v[DATA]), idx, dimnames, filename);
}


val::Value funcs::make_array(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { DATA, DIM, DIMNAMES, FILE };

  auto& idxOrig = *get<val::SpVAD>(val::getVal(v[DIM]));
  // doesn't Meyer warn about this way of using auto: LLL
  const auto& idx = Vector<arr::idx_type>(idxOrig.v[0]->begin(), idxOrig.v[0]->end());
  const auto& dimnames = convertToDnames(val::getVal(v[DIMNAMES]));
  const auto& filename = val::get_scalar<arr::zstring>(val::getVal(v[FILE]));
 
  return make_array_helper(val::getVal(v[DATA]), idx, dimnames, filename);
}


template<typename T>
static val::Value make_vector_default(size_t length, const arr::zstring& dirname) {
  unsigned flags = dirname.size() ? arr::LOCKED : arr::NOFLAGS;
  return make_cow<Array<T>>(flags, 
                            arr::Vector<idx_type>{length}, 
                            arr::Vector<T>(length), 
                            std::vector<Vector<zstring>>(),
                            getAllocFactory(dirname));
}


static val::Value make_vector_helper(const arr::zstring& mode, 
                                     const yy::location& modeLoc,
                                     size_t length, 
                                     const arr::zstring& dirname) {
  if (mode == "double") {
    return make_vector_default<double>(length, dirname);
  } else if (mode == "logical") {
    return make_vector_default<bool>(length, dirname);
  } else if (mode == "character") {
    return make_vector_default<arr::zstring>(length, dirname);
  } else if (mode == "time") {
    return make_vector_default<Global::dtime>(length, dirname);
  } else if (mode == "interval") {
    return make_vector_default<tz::interval>(length, dirname);
  } else if (mode == "period") {
    return make_vector_default<tz::period>(length, dirname);
  } else if (mode == "duration") {
    return make_vector_default<Global::duration>(length, dirname);
  } else {
    throw interp::EvalException("unknown mode", modeLoc);
  }
}


val::Value funcs::make_vector(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { MODE, LENGTH, FILE };
  const auto& mode     = val::get_scalar<arr::zstring>(val::getVal(v[MODE]));
  const auto& modeLoc  = val::getLoc(v[MODE]);
  const auto& length   = val::get_scalar<double>(val::getVal(v[LENGTH]));
  const auto& filename = val::get_scalar<arr::zstring>(val::getVal(v[FILE]));
  vector<Vector<zstring>> dimnames;
  const Vector<idx_type> idx{static_cast<idx_type>(length)};

  return make_vector_helper(mode, modeLoc, length, filename);
}


// nrow, ncol -----------------
template<int I, typename A>
struct ndim_I {
  static val::Value f(const val::Value& val) {
    const auto& vv = get<A>(val);
    if (vv->getdim().size() > I) {
      return val::make_array(static_cast<double>(vv->getdim()[I]));
    }
    else {
      return val::VNull();
    }
  }
};

template <typename A>
using ndim_row = ndim_I<0, A>;
template <typename A>
using ndim_col = ndim_I<1, A>;


val::Value funcs::ncol(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return apply_to_types_null<ndim_col, 
                             val::vt_double, 
                             val::vt_bool, 
                             val::vt_time, 
                             val::vt_duration, 
                             val::vt_interval, 
                             val::vt_string, 
                             val::vt_zts>(val::getVal(v[0]), val::getLoc(v[0]));
}

val::Value funcs::nrow(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return apply_to_types_null<ndim_row, 
                             val::vt_double, 
                             val::vt_bool, 
                             val::vt_time, 
                             val::vt_duration, 
                             val::vt_interval, 
                             val::vt_string, 
                             val::vt_zts>(val::getVal(v[0]), val::getLoc(v[0]));
}


// dim -----------------
template<typename T>
struct dim_helper {
  static val::Value f(val::Value v) {
    const auto& vv = get<T>(v);
    return arr::make_cow<val::VArrayD>(false, 
                                       val::VArrayD({vv->getdim().size()}, 
                                                    arr::Vector<double>(vv->getdim().begin(), 
                                                                        vv->getdim().end()))); 
  }
};


val::Value funcs::dim(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return apply_to_types_null<dim_helper, 
                             val::vt_double, 
                             val::vt_bool, 
                             val::vt_time, 
                             val::vt_duration, 
                             val::vt_interval, 
                             val::vt_string, 
                             val::vt_zts>(val::getVal(v[0]), val::getLoc(v[0]));
}


// bind --------------------
template <typename T>
static Array<T>* bindVector(Array<T>& r,
                            vector<val::VBuiltinG::arg_t>::const_iterator begin, 
                            vector<val::VBuiltinG::arg_t>::const_iterator end,
                            idx_type dim)
{
#ifdef DEBUG_BFA
  cout << "bindVector<T>: " << endl;
  cout << "| dim: " << dim << endl; 
#endif
  
  for (auto e = begin; e != end; ++e) {
    switch (val::getVal(*e).which()) {
    case val::vt_double: {
      const auto& a = get<val::SpVAD>(val::getVal(*e));
      r.abind(*a, dim, val::getName(*e));
      break; }
    case val::vt_bool: {
      const auto& a = get<val::SpVAB>(val::getVal(*e));
      r.abind(*a, dim, val::getName(*e));
      break; }
    case val::vt_string: {
      const auto& a = get<val::SpVAS>(val::getVal(*e));
      r.abind(*a, dim, val::getName(*e));
        break; }
    case val::vt_time: {
      const auto& a = get<val::SpVADT>(val::getVal(*e));
      r.abind(*a, dim, val::getName(*e));
      break; }
    case val::vt_duration: {
      const auto& a = get<val::SpVADUR>(val::getVal(*e));
      r.abind(*a, dim, val::getName(*e));
      break; }
    case val::vt_interval: {
      const auto& a = get<val::SpVAIVL>(val::getVal(*e));
      r.abind(*a, dim, val::getName(*e));
      break; }
    default:
      throw std::domain_error("bindVector<T> incorrect type");
    }
  }
  return &r;
}


static zts* bindVector(zts& r,
                       vector<val::VBuiltinG::arg_t>::const_iterator begin, 
                       vector<val::VBuiltinG::arg_t>::const_iterator end,
                       idx_type dim)
{
#ifdef DEBUG_BFA
  cout << "bindVector<T>: " << endl;
  cout << "| dim: " << dim << endl; 
#endif
  
  for (auto e = begin; e != end; ++e) {
    switch (val::getVal(*e).which()) {
    case val::vt_double: {
      const auto& a = get<val::SpVAD>(val::getVal(*e));
      r.abind(a->isVector() && dim==0 ? transpose(*a) : *a, 
              dim,                 // in which dimension
              val::getName(*e));   // the prefix to add to the bind
      break; }
    case val::vt_zts: {
      const auto& z = get<val::SpZts>(val::getVal(*e));
      r.abind(*z, dim, val::getName(*e));
      break; }
    default:
      throw std::domain_error("bindVector<T> incorrect type");
    }
  }
  return &r;
}


static void checkZtsBindInDim0(vector<val::VBuiltinG::arg_t>::const_iterator begin, 
                               vector<val::VBuiltinG::arg_t>::const_iterator end) 
{
  const arr::zts* zp_prev = nullptr;
  const arr::zts* zp_cur   = nullptr;
  for (auto iter=begin; iter!=end; ++iter) {
    if (val::getVal(*iter).which() != val::vt_zts) {
      throw interp::EvalException("'zts' bind in first dimension requires zts 'argument'", 
                                  val::getLoc(*iter));    
    }
    zp_prev = zp_cur;
    zp_cur = get<val::SpZts>(val::getVal(*iter)).get();
    if (zp_prev) {
      if (!Array<double>::checkdims(zp_prev->getdim(), zp_cur->getdim(), 0)) {
        throw interp::EvalException("invalid dimensions for abind", val::getLoc(*iter));    
      }
      idx_type prev_rows = zp_prev->getdim(0);
      idx_type cur_rows  = zp_cur->getdim(0);

      if (prev_rows && cur_rows && 
          zp_prev->getIndex()[zp_prev->getIndex().size()-1] >= zp_cur->getIndex()[0]) {
        throw interp::EvalException("non-ascending index for rbind", val::getLoc(*iter));
      }
    }
  }
}


static const arr::Array<Global::dtime>* 
checkZtsBindInDimGt0(vector<val::VBuiltinG::arg_t>::const_iterator begin, 
                     vector<val::VBuiltinG::arg_t>::const_iterator end,
                     arr::idx_type dim) 
{
  const arr::Array<Global::dtime>* idx = nullptr;
  const arr::Array<double>* a = nullptr;

  // traverse the list and find the first zts; assign its array and idx
  for (auto iter=begin; iter!=end; ++iter) {
    if (val::getVal(*iter).which() == val::vt_zts) {
      a = &get<val::SpZts>(val::getVal(*iter))->getArray();
      idx = &get<val::SpZts>(val::getVal(*iter))->getIndex();
      break;
    }
  }
  if (a == nullptr) {
    // this should never happen because if we get here it means there
    // was no zts in the argument list:
    std::logic_error("no zts in list");
  }

  for (auto iter=begin+1; iter!=end; ++iter) {
    if (val::getVal(*iter).which() == val::vt_zts) {
      const auto& zcur = get<val::SpZts>(val::getVal(*iter));
      if (zcur->getIndex().getcol(0) != idx->getcol(0)) {
        throw interp::EvalException("'zts' index mismatch for abind", val::getLoc(*iter));
      }
      if (!Array<double>::checkdims(a->getdim(), zcur->getdim(), dim)) {
        throw interp::EvalException("invalid dimensions for abind", val::getLoc(*iter));
      }
    }
    else if (val::getVal(*iter).which() == val::vt_double) {
      const auto& acur = get<val::SpVAD>(val::getVal(*iter));
      if (!Array<double>::checkdims(a->getdim(), acur->getdim(), dim)) {
        throw interp::EvalException("invalid dimensions for abind", val::getLoc(*iter));
      }
    }
  }
  return idx;
}


static val::Value bindHelper(vector<val::VBuiltinG::arg_t>::iterator begin, 
                             vector<val::VBuiltinG::arg_t>::iterator end,
                             idx_type dim)
{
  // figure out if the first element is pass by reference; if yes, we
  // bind on that element (which also means that its type cannot be
  // changed as it can in a copy bind):

  // LLL ahead of time, check the validity of the bind, so we don't
  // end up with partially modified first arg !!!
  switch (val::getVal(*begin).which()) {
  case val::vt_double: {
    auto& a = get<val::SpVAD>(val::getVal(*begin));
    if (a.isRef()) {
      a.get()->addprefix(val::getName(*begin), a.get()->isVector() ? 0 : dim);
      bindVector(*a, begin+1, end, dim);
      return a;
    } else {
      break;
    }
  }
  case val::vt_bool: {
    auto& a = get<val::SpVAB>(val::getVal(*begin));
    if (a.isRef()) {
      a.get()->addprefix(val::getName(*begin), a->isVector() ? 0 : dim);
      bindVector(*a, begin+1, end, dim);
      return a;
    } else {
      break;
    }
  }
  case val::vt_time: {
    auto& a = get<val::SpVADT>(val::getVal(*begin));
    if (a.isRef()) {
      a.get()->addprefix(val::getName(*begin), a->isVector() ? 0 : dim);
      bindVector(*a, begin+1, end, dim);
      return a;
    } else {
      break;
    }
  }
  case val::vt_duration: {
    auto& a = get<val::SpVADUR>(val::getVal(*begin));
    if (a.isRef()) {
      a.get()->addprefix(val::getName(*begin), a->isVector() ? 0 : dim);
      bindVector(*a, begin+1, end, dim);
      return a;
    } else {
      break;
    }
  }
  case val::vt_interval: {
    auto& a = get<val::SpVAIVL>(val::getVal(*begin));
    if (a.isRef()) {
      a.get()->addprefix(val::getName(*begin), a->isVector() ? 0 : dim);
      bindVector(*a, begin+1, end, dim);
      return a;
    } else {
      break;
    }
  }
  case val::vt_string: {
    auto& a = get<val::SpVAS>(val::getVal(*begin));
    if (a.isRef()) {
      a.get()->addprefix(val::getName(*begin), a->isVector() ? 0 : dim);
      bindVector(*a, begin+1, end, dim);
      return a;
    } else {
      break;
    }
  }
  case val::vt_zts: {
    auto& z = get<val::SpZts>(val::getVal(*begin));
    if (z.isRef()) {
      if (dim == 0) {
        checkZtsBindInDim0(begin, end);
      }
      else {
        checkZtsBindInDimGt0(begin, end, dim);
        z.get()->addprefix(val::getName(*begin), dim);
      }
      bindVector(*z, begin+1, end, dim);
      return z;
    } else {
      break;
    }
  }
  default: 
    ; // continue
  }

  // we get here when the pointer to the array is not a reference; in
  // this case, as in R, we allow a cast to occur as part of the
  // bind; we examine all elts to determine the type of vector
  auto vt = getCType(begin, end);

  switch (vt) {
  case val::vt_double: {
    auto first = make_cow<val::VArrayD>(true, rsv, Vector<idx_type>());
    bindVector(*first, begin, end, dim);
    return first;
  }
  case val::vt_bool: {
    auto first = make_cow<val::VArrayB>(true, rsv, Vector<idx_type>());
    bindVector(*first, begin, end, dim);
    return first;
  }
  case val::vt_time: {
    auto first = make_cow<val::VArrayDT>(true, rsv, Vector<idx_type>());
    bindVector(*first, begin, end, dim);
    return first;
  }
  case val::vt_duration: {
    auto first = make_cow<val::VArrayDUR>(true, rsv, Vector<idx_type>());
    bindVector(*first, begin, end, dim);
    return first;
  }
  case val::vt_interval: {
    auto first = make_cow<val::VArrayIVL>(true, rsv, Vector<idx_type>());
    bindVector(*first, begin, end, dim);
    return first;
  }
  case val::vt_string: {
    auto first = make_cow<val::VArrayS>(true, rsv, Vector<idx_type>());
    bindVector(*first, begin, end, dim);
    return first;
  }
  case val::vt_zts: {
    if (dim == 0) {
      checkZtsBindInDim0(begin, end);
      auto z = get<val::SpZts>(val::getVal(*begin));
      auto first = make_cow<arr::zts>(true, z->getIndex(), z->getArray()); // copy!
      bindVector(*first, begin+1, end, dim);
      return first;
    }
    else {    
      auto idx = checkZtsBindInDimGt0(begin, end, dim);
      if (!idx) {
        throw std::logic_error("unspecified zts-related bind");
      }
      auto first = make_cow<arr::zts>(true, *idx, Array<double>(rsv, Vector<idx_type>()));
      bindVector(*first, begin, end, dim);
      return first;
    }
  }
  case val::vt_null:
    return val::VNull();
    // note we don't handle binding of lists:
    // case val::vt_list:
  default:
    throw std::domain_error("cbind incorrect type"); // need to unify also this LLL
  }
}


val::Value funcs::abind(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  auto dim = val::get_scalar<double>(val::getVal(v[0])) - 1; // -1 as R starts from 1
  return bindHelper(v.begin()+1, v.end(), dim);
}


val::Value funcs::rbind(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return bindHelper(v.begin(), v.end(), 0);
}


val::Value funcs::cbind(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return bindHelper(v.begin(), v.end(), 1);
}


static std::unique_ptr<arr::AllocFactory> getAllocFactoryZts(const fsys::path& filename) {
  if (filename.string().size()) {
    return std::make_unique<arr::MmapAllocFactory>(filename, false);
  }
  else {
    return std::make_unique<arr::FlexAllocFactory>();
  }
}


val::Value funcs::make_zts(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { IDX, DATA, FILE };

  const auto& tidx = get<val::SpVADT>(val::getVal(v[IDX]));
  const auto& data = get<val::SpVAD>(val::getVal(v[DATA]));
  const auto& filename = fsys::path(std::string(val::get_scalar<arr::zstring>(val::getVal(v[FILE]))));
  const auto& filename_idx = filename.string().size() ? filename / "idx" : filename;
  try {
    unsigned flags = filename.string().size() ? arr::LOCKED: arr::NOFLAGS; // with TMP instead of 0, avoid a copy? LLL
    auto allocf = getAllocFactoryZts(filename);
    auto allocf_idx = getAllocFactoryZts(filename_idx);
    return arr::make_cow<arr::zts>(flags, 
                                   *tidx, 
                                   *data, 
                                   std::move(allocf),
                                   std::move(allocf_idx));
  }
  catch (arr::UnorderedIndex& e) {
    throw interp::EvalException(e.what(), val::getLoc(v[IDX]));    
  }
  catch (arr::LengthMismatch& e) {
    throw interp::EvalException(e.what(), val::getLoc(v[DATA]));
  }
  catch (...) {
    throw;
  }
}


val::Value funcs::zts_idx(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { ZTS };
  auto& zts = *get<val::SpZts>(val::getVal(v[ZTS]));
  return arr::cow_ptr<val::VArrayDT>(arr::LOCKED | arr::CONSTREF, zts.getIndexPtr());
}


val::Value funcs::zts_data(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { ZTS };
  auto& zts = *get<val::SpZts>(val::getVal(v[ZTS]));
  return arr::cow_ptr<val::VArrayD>(arr::LOCKED | arr::CONSTREF, zts.getArrayPtr());
}




val::Value funcs::load(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  const string dirname = val::get_scalar<arr::zstring>(val::getVal(v[0]));
  struct stat st = {0};
  if (stat((fsys::path(dirname) / "idx").c_str(), &st) == 0) {
    auto indexdir = fsys::path(dirname) / "idx";    
    auto allocfidx = std::make_unique<MmapAllocFactory>(indexdir, true);
    auto allocfdata = std::make_unique<MmapAllocFactory>(dirname, true);
    return arr::make_cow<arr::zts>(false, std::move(allocfdata), std::move(allocfidx));
  }
  // else it's an array:
  else {
    auto datafilename = fsys::path(dirname) / "0";
    if (stat(datafilename.c_str(), &st) != 0) {
      throw range_error("no data in directory " + dirname);
    }
    auto file = fopen(datafilename.c_str(), "rb");
    if (!file) {
      throw std::system_error(std::error_code(errno, std::system_category()), 
                              "cannot fopen "s + datafilename.string());
    }
    RawVector<double> v;
    auto res = fread(&v, sizeof(RawVector<double>), 1, file);
    if (res < 1) {
      fclose(file);
      throw std::system_error(std::error_code(errno, std::system_category()), 
                              "cannot fread "s + datafilename.string());
    }
    fclose(file);

    auto allocf = std::make_unique<MmapAllocFactory>(dirname, true);

    // switch on the type number we have to determine which type of array to construct:
    switch (v.typenumber) {
    case TypeNumber<double>::n:
      return arr::make_cow<val::VArrayD>(false, std::move(allocf));
    case TypeNumber<bool>::n:
      return arr::make_cow<val::VArrayB>(false, std::move(allocf));
    case TypeNumber<Global::dtime>::n:
      return arr::make_cow<val::VArrayDT>(false, std::move(allocf));
    case TypeNumber<Global::duration>::n:
      return arr::make_cow<val::VArrayDUR>(false, std::move(allocf));
    case TypeNumber<tz::interval>::n:
      return arr::make_cow<val::VArrayIVL>(false, std::move(allocf));
    case TypeNumber<arr::zstring>::n:
      return arr::make_cow<val::VArrayS>(false, std::move(allocf));
    case TypeNumber<tz::period>::n:
      return arr::make_cow<val::VArrayPRD>(false, std::move(allocf));
    default:
      throw std::domain_error("unknown type number: " + std::to_string(v.typenumber));
    }
  }
}


// sort -----------------
template<typename T, typename A1>
struct sort_wrapper {
  static val::Value f(val::Value v, A1 a1, 
                      const yy::location& dummy1, const yy::location& dummy2) { 
    if (!a1) {
      get<T>(v)->sort();
    }
    else {
      typedef typename val::rmptr<T>::TP::value_type E; // E is the array element type
      get<T>(v)->template sort<std::greater<E>>();
    }
    return v;
  }
};


val::Value funcs::_sort(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { X, DECREASING };
  auto decreasing = val::get_scalar<bool>(val::getVal(v[DECREASING]));
  return apply_to_types2<sort_wrapper, 
                         bool,   // type of argument 1 for sort_wrapper::f()
                         val::vt_double, 
                         val::vt_bool, 
                         val::vt_time, 
                         val::vt_string, 
                         val::vt_duration, 
//                         val::vt_zts,
                         val::vt_interval>(val::getVal(v[X]), decreasing, 
                                           val::getLoc(v[X]), yy::missing_loc());
}


val::Value funcs::is_ordered(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  const auto& val = val::getVal(v[0]);
  switch (val.which()) {
  case val::vt_double:
    return val::make_array(get<val::SpVAD>(val).get()->isOrdered());
  case val::vt_time:
    return val::make_array(get<val::SpVADT>(val).get()->isOrdered());
  case val::vt_interval:
    return val::make_array(get<val::SpVAIVL>(val).get()->isOrdered());
  case val::vt_duration:
    return val::make_array(get<val::SpVADUR>(val).get()->isOrdered());
  case val::vt_string:
    return val::make_array(get<val::SpVAS>(val).get()->isOrdered());
  default:
    throw interp::EvalException("incorrect argument type", val::getLoc(v[0]));
};


}

template<typename T, typename A1>
struct sort_idx_wrapper {
  static val::Value f(val::Value v, A1 a1, const yy::location& l, const yy::location& dummy) { 
    if (!a1) {
      return arr::make_cow<val::VArrayD>(false, (get<T>(v))->template sort_idx<double>(1));
    }
    else {
      typedef typename val::rmptr<T>::TP::value_type E;  // E is the array element type
      return arr::make_cow<val::VArrayD>(false, 
                                         (get<T>(v))->template sort_idx<double, std::greater<E>>(1));
    }
  }
};


val::Value funcs::_sort_idx(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { X, DECREASING };
  auto decreasing = val::get_scalar<bool>(val::getVal(v[DECREASING]));
  return apply_to_types2<sort_idx_wrapper, 
                         bool,   // type of argument 1 for sort_wrapper_idx::f()
                         val::vt_double, 
                         val::vt_time, 
//                         val::vt_zts,
                         val::vt_duration, 
                         val::vt_interval, 
                         val::vt_string, 
                         val::vt_interval>(val::getVal(v[X]), decreasing, 
                                           val::getLoc(v[X]), yy::missing_loc());
}


template <typename T, typename A1>
struct head_helper {
  static val::Value f(const val::Value& x, A1 n, const yy::location& lx, const yy::location& ln) {
    const auto& a = get<T>(x);
    if (n >= 0) {
      if (static_cast<size_t>(n) >= a->getdim(0)) {
        return x;
      }
      return make_cow<typename val::rmptr<T>::TP>(false, a->subsetRows(n, 0));
    }
    else if (n < 0) {
      if (static_cast<size_t>(-n) >= a->getdim(0)) {
        n = -a->getdim(0);
      }
      return make_cow<typename val::rmptr<T>::TP>(false, a->subsetRows(a->getdim(0) + n, 0));
    }
    else {
      throw interp::EvalException("0 is not a valid value for 'n'", ln);
    }
  }
};



val::Value funcs::head(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) 
{
  enum { X, N };

  auto n = val::get_scalar<double>(val::getVal(v[N]));
  return apply_to_types2<head_helper, 
                         ssize_t,   // type of argument 1 for head_helper::f()
                         val::vt_double, 
                         val::vt_zts,
                         val::vt_bool, 
                         val::vt_time, 
                         val::vt_duration, 
                         val::vt_interval, 
                         val::vt_string, 
                         val::vt_list>(val::getVal(v[X]), n, val::getLoc(v[X]), val::getLoc(v[N]));
}


template <typename T, typename A1, typename A2>
struct tail_helper {
  static val::Value f(const val::Value& x, A1 n, A2 addrownums, 
                      const yy::location& lx, const yy::location& ln) {
    const auto& a = get<T>(x);
    if (a->getdim().size() <= 1) {
      addrownums = false;
    }
    if (n >= 0) {
      if (static_cast<size_t>(n) >= a->getdim(0)) {
        n = a->getdim(0);
      }
      return make_cow<typename val::rmptr<T>::TP>(false, a->subsetRows(n, 
                                                                       a->getdim(0)-n,
                                                                       addrownums));
    }
    else if (n < 0) {
      if (static_cast<size_t>(-n) >= a->getdim(0)) {
        n = -a->getdim(0);
      }
      return make_cow<typename val::rmptr<T>::TP>(false, a->subsetRows(a->getdim(0)+n, 
                                                                       -n,
                                                                       addrownums));
    }
    else {
      throw interp::EvalException("0 is not a valid value for 'n'", ln);
    }
  }
};


val::Value funcs::tail(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) 
{
  enum { X, N, ADDROWNUMS };

  auto n = val::get_scalar<double>(val::getVal(v[N]));
  auto addrownums = val::get_scalar<bool>(val::getVal(v[ADDROWNUMS]));
  return apply_to_types3<tail_helper, 
                         ssize_t,   // type of argument 1 for tail_helper::f()
                         bool,      // type of argument 2 for tail_helper::f()
                         val::vt_double,
                         val::vt_zts,
                         val::vt_bool, 
                         val::vt_time, 
                         val::vt_duration, 
                         val::vt_interval, 
                         val::vt_string, 
                         val::vt_time, 
                         val::vt_list>(val::getVal(v[X]), n, addrownums, val::getLoc(v[X]), val::getLoc(v[N]));
}


static std::random_device rd;
static std::mt19937_64 gen(rd());

val::Value funcs::set_seed(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { X };
  auto x = static_cast<idx_type>(val::get_scalar<double>(val::getVal(v[X])));
  gen.seed(x);
  return val::VNull();
}

template <typename DIS>
static val::Value random_helper(vector<val::VBuiltinG::arg_t>& v, DIS& dis) {
  enum { X };
  // if x is scalar, then we create a new vector of specified size
  if (val::getVal(v[X]).which() == val::vt_double) {
    auto x = get<val::SpVAD>(val::getVal(v[X]));
    if (x->isScalar()) {
      auto len = static_cast<idx_type>(val::get_scalar<double>(val::getVal(v[X])));
      auto a = arr::make_cow<val::VArrayD>(false, noinit_tag, Vector<idx_type>{len});
      for (size_t i = 0; i<len; ++i) {
        setv(*a, i, dis(gen));
      }
      return a;
    }
    else {
      for (size_t i = 0; i<x->size(); ++i) {
        arr::setv(*x, i, dis(gen));     /// \todo not very efficient
      }
      return x;
    }
  }
  else {
    auto x = get<val::SpZts>(val::getVal(v[X]));
    for (size_t i = 0; i<x->getArray().size(); ++i) {
      arr::setv(*x->getArrayPtr(), i, dis(gen));     /// \todo not very efficient
      // define setv for zts! LLL
    }
    return x;
  }
}

val::Value funcs::runif(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { X, MIN, MAX };

  auto minp = val::get_scalar<double>(val::getVal(v[MIN]));
  auto maxp = val::get_scalar<double>(val::getVal(v[MAX]));
  std::uniform_real_distribution<> dis(minp, maxp);
  return random_helper(v, dis);
}


// combine this code with the code for unif
val::Value funcs::rnorm(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { N, MEAN, SD };

  auto mean = val::get_scalar<double>(val::getVal(v[MEAN]));
  auto sd   = val::get_scalar<double>(val::getVal(v[SD]));
  std::normal_distribution<double> dis(mean, sd);
  return random_helper(v, dis);
}


template<typename T>
struct alloc_dirname_wrapper {
  static val::Value f(const val::Value& val, const yy::location& loc_val) {
    const auto& v = get<T>(val);
    try {
      return val::make_array(static_cast<arr::zstring>(v->getAllocfDirname().c_str()));
    }
    catch (const std::exception& e) {
      throw interp::EvalException(e.what(), loc_val);
    }
  }
};


val::Value funcs::alloc_dirname(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { X };
  return apply_to_types<alloc_dirname_wrapper,
                        val::vt_double, 
                        val::vt_bool, 
                        val::vt_time, 
                        val::vt_string, 
                        val::vt_duration, 
                        val::vt_zts,
                        val::vt_interval>(val::getVal(v[X]), val::getLoc(v[X]));    
}


val::Value funcs::islocked(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { X };
  return val::make_array<bool>(val::isLocked(val::getVal(v[X])));
}


static ssize_t getIndex(const arr::zts& z, const val::Value& i) {
  if (i.which() == val::vt_double) {
    auto val = val::get_scalar<double>(i) - 1;
    if (val > z.getdim(0)) val = z.getdim(0);
    return val;
  }
  else {
    const auto dt = val::get_scalar<Global::dtime>(i);
    const auto& v = z.getIndex().getcol(0);
    const auto iter = std::lower_bound(v.begin(), v.end(), dt);
    return iter - v.begin();
  }
}


val::Value funcs::zts_resize(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { X, START, END };
    
  auto& x = get<val::SpZts>(val::getVal(v[X]));
  if (!x.get()->getdim().size()) return x;
  auto start_idx = getIndex(*x.get(), val::getVal(v[START]));
  auto end_idx   = getIndex(*x.get(), val::getVal(v[END]));

  if (start_idx < 0) {
    throw interp::EvalException("start out of bounds", val::getLoc(v[START]));  
  }
  if (end_idx < 0) {
    throw interp::EvalException("end out of bounds", val::getLoc(v[END]));  
  }
  if (start_idx > end_idx) {
    throw interp::EvalException("start must be <= to end", val::getLoc(v[START]));  
  }

  auto sz = end_idx - start_idx; // include the end index
  x->resize(0, sz, start_idx);
  return x;
}


val::Value funcs::zts_truncate(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { X, END };
  auto x = get<val::SpZts>(val::getVal(v[X]));
  auto end_idx = getIndex(*x.get(), val::getVal(v[END]));
  if (end_idx < 0) {
    throw interp::EvalException("argument cannot be negative", val::getLoc(v[END]));
  }
  auto nrows = x.get()->getdim(0); // get() to avoid a copy!
  if (static_cast<size_t>(end_idx) > nrows) {
    throw interp::EvalException("end out of bounds", val::getLoc(v[END]));  
  }
  x->resize(0, end_idx);
  return x;
}
