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
#include "valuevar.hpp"
#include "conversion_funcs.hpp"
#include "index.hpp"
#include "display.hpp"
#include "interp_ctx.hpp"


static size_t minus1(double d) {
  if (d == 0) throw std::range_error("subscript out of bounds");
  if (d < 0)  throw std::range_error("mixed positive and negative subscripts");
  return static_cast<size_t>(d) - 1;
}

static size_t negminus1(double d) {
  if (d == 0) throw std::range_error("subscript out of bounds");
  if (d > 0)  throw std::range_error("mixed positive and negative subscripts");
  return static_cast<size_t>(-d) - 1;
}

static vector<Index> convertToIndex(const vector<val::VBuiltinG::arg_t>::const_iterator& begin, 
                                    const vector<val::VBuiltinG::arg_t>::const_iterator& end,
                                    const zts& z) {
  vector<Index> vi;
  unsigned j = 0;
  for (auto e = begin; e != end; ++e) {
    switch (getVal(*e).which()) {
    case val::vt_null:
      vi.push_back(NullIndex{z.getdim(j)});
      break;      
    case val::vt_double: {
      // - 1, like in R: indices start at 1:
      const auto& idx = get<val::SpVAD>(getVal(*e));
      if (!signbit((*idx)[0])) {
        const auto idx0 = applyf<double,double>(*idx, minus1).getcol(0);
        vi.push_back(IntIndex(Vector<size_t>(idx0.begin(), idx0.end())));
      }
      else {
        const auto idx0 = applyf<double,double>(*idx, negminus1).getcol(0);
        vi.push_back(IntIndexNeg(Vector<size_t>(idx0.begin(), idx0.end()), z.getdim(j)));
      }
      break; }
    case val::vt_bool: {
      const auto& idx = get<val::SpVAB>(getVal(*e));
      if (!idx->isVector()) {
        throw interp::EvalException("boolean index is not a vector", getLoc(*e));
      }
      if (z.getdim(j) != idx->getdim(0)) {
        throw interp::EvalException("boolean index not equal to array extent", getLoc(*e));
      }
      vi.push_back(BoolIndex{idx->getcol(0)});
      break; }
    case val::vt_string: {
      const auto& idx = get<val::SpVAS>(getVal(*e));
      vi.push_back(NameIndex{arr::stdvector<string, arr::zstring>(*idx), z.getArray().getnames(j)});
      break;
    }
    case val::vt_time: {
      if (e != begin) {
        throw interp::EvalException("zts: can't use a time index into dimension > 1", getLoc(*e));
      }
      const auto& idx = get<val::SpVADT>(getVal(*e));
      vi.push_back(DtimeIndex{idx->getcol(0), z.getIndex().getcol(0)});
      break;
    }
    case val::vt_interval: {
      if (e != begin) {
        throw interp::EvalException("zts: can't use an interval index into dimension > 1", 
                                    getLoc(*e));
      }
      const auto& idx = get<val::SpVAIVL>(getVal(*e));
      vi.push_back(IntervalIndex{idx->getcol(0), z.getIndex().getcol(0)});
      break;
    }
    default:
      throw interp::EvalException("convertToIndex incorrect type", getLoc(*e));
    }
    ++j;
  }
  return vi;
}



// these set of function are breakouts from 'convertToIndex' because for
// 'DtimeIndex' and 'IntervalIndex' we need different behaviour
// depending on the type of array into which we are indexing.
template<typename T>
static void push_back_dtime_index(vector<Index>& vi, const val::VArrayDT& i, 
                                  const yy::location& iloc, const Array<T>& a) {
  throw interp::EvalException("time point index into non time point vector", iloc);
}

template<>
void push_back_dtime_index(vector<Index>& vi, const val::VArrayDT& i, 
                           const yy::location& iloc, const Array<Global::dtime>& a) {
  vi.push_back(DtimeIndex{i.getcol(0), *a.v[0]});
}

template<typename T>
static void push_back_dtime_index(vector<Index>& vi, const Global::dtime& i, 
                                  const yy::location& iloc, const Array<T>& a) {
  throw interp::EvalException("time point index into non time point vector", iloc);
}

template<typename T>
static void push_back_interval_index(vector<Index>& vi, const val::VArrayIVL& i, 
                                     const yy::location& iloc, const Array<T>& a) {
  throw interp::EvalException("interval index into non time point vector", iloc);
}

template<>
void push_back_interval_index(vector<Index>& vi, const val::VArrayIVL& i, const yy::location& iloc, 
                              const Array<Global::dtime>& a) {
  vi.push_back(IntervalIndex{i.getcol(0), *a.v[0]});
}

template<typename T>
static void push_back_interval_index(vector<Index>& vi, const tz::interval& i, 
                                     const yy::location& iloc, const Array<T>& a) {
  throw interp::EvalException("interval index into non time point vector", iloc);
}


template<typename T>
static vector<Index> convertToIndex(const vector<val::VBuiltinG::arg_t>::const_iterator& begin, 
                                    const vector<val::VBuiltinG::arg_t>::const_iterator& end,
                                    const Array<T>& a) {
  vector<Index> vi;
  unsigned j = 0;


  // if there is only one null index, then the intent is to select
  // the whole of the array, even in an n>1 case; so we push back
  // dim.size() Nulls, and we are done:
  if (end - begin == 1 && getVal(*begin).which() == val::vt_null) {
    for (unsigned i=0; i<a.getdim().size(); ++i) {
      vi.push_back(NullIndex{a.getdim(i)});
    }
  }
  else if (end-begin > 1 && static_cast<size_t>(end-begin) != a.getdim().size()) {
    throw std::range_error("incorrect number of dimensions");
  }
  else {
    for (auto e = begin; e != end; ++e) {
      switch (getVal(*e).which()) {
      case val::vt_null:
        vi.push_back(NullIndex{a.getdim(j)});
        break;      
      case val::vt_double: {
        const auto& idx = get<val::SpVAD>(getVal(*e));
        if ((*idx)[0] >= 0) {
          // - 1 because indices are 1-based:
          const auto idx0 = applyf<double,double>(*idx, minus1).getcol(0);
          vi.push_back(IntIndex(Vector<size_t>(idx0.begin(), idx0.end())));
        }
        else {
          // if we are indexing as in a vector, then the length with
          // which we initialize the index must be the number of
          // elements in the array rather than the dimension in which
          // the index is operating as is the case when we have one
          // index per dimension:
          arr::idx_type sz = end-begin == 1 && a.getdim().size() > 1 ? a.size() : a.getdim(j);
          const auto idx0 = applyf<double,double>(*idx, negminus1).getcol(0);
          vi.push_back(IntIndexNeg(Vector<size_t>(idx0.begin(), idx0.end()), sz));
        }
        break; 
      }
      case val::vt_bool: {
        const auto& idx = get<val::SpVAB>(getVal(*e));
        if (a.getdim(j) != idx->getdim(0) && a.size() != idx->size()) {
          throw interp::EvalException("boolean index not equal to array extent", getLoc(*e));
        }        
        vi.push_back(BoolIndex{idx->getcol(0)});
        break; 
      }
      case val::vt_string: {
        const auto& idx = get<val::SpVAS>(getVal(*e));
        vi.push_back(NameIndex{arr::stdvector<string, arr::zstring>(*idx), a.getnames(j)});
        break; 
      }
      case val::vt_time: {
        if (!a.isVector()) {
          throw interp::EvalException("time point index into non-vector", getLoc(*e));
        }
        const auto& idx = get<val::SpVADT>(getVal(*e));
        push_back_dtime_index(vi, *idx, getLoc(*e), a); 
        break;
      }
      case val::vt_interval: {
        if (!a.isVector()) {
          throw interp::EvalException("interval index into non-vector", getLoc(*e));
        }
        const auto& idx = get<val::SpVAIVL>(getVal(*e));
        push_back_interval_index(vi, *idx, getLoc(*e), a); 
        break;
      }
        /// case val::vt_arraydur:
        /// can't make an index from a duration
      default:
        throw interp::EvalException("incorrect index type", getLoc(*e));
      }
      ++j;
    }
  }
  return vi;
}


/// subset an array: 
val::Value funcs::subset(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { DROP, A };             // and the following ones are the indices (from the ellipsis)
  auto a = getVal(v[A]);

  bool drop_p;
  try {
    drop_p = val::get_scalar<bool>(funcs::convert_logical(getVal(v[DROP])));
  }
  catch (std::exception& e) {
    throw std::range_error("cannot convert 'drop' parameter to boolean");
  }

  switch (a.which()) {
  case val::vt_zts: {
    const auto z = get<val::SpZts>(a);
    auto i = convertToIndex(v.begin()+2, v.end(), *z);
    return arr::make_cow<arr::zts>(false, (*z)(i, drop_p));
  }
  case val::vt_double: {
    const auto ad = get<val::SpVAD>(a);
    auto i = convertToIndex(v.begin()+2, v.end(), *ad);
    auto r = (*ad)(i, drop_p);
    return arr::make_cow<val::VArrayD>(false, std::move(r));
  }
  case val::vt_list: {
    const auto l = get<val::SpVList>(a);
    auto i = convertToIndex(v.begin()+2, v.end(), l->a);
    auto r = (l->a)(i, drop_p);
    // as in R, list does not revert to scalars or it would lose its list semantics:
    return arr::make_cow<val::VList>(false, std::move(r));
  }    
  case val::vt_string: {
    const auto ai = get<val::SpVAS>(a); // const necessary for disambiguation of subset function
    auto i = convertToIndex(v.begin()+2, v.end(), *ai);
    auto r = (*ai)(i, drop_p);
    return val::Value(arr::make_cow<val::VArrayS>(false, std::move(r)));
  }
  case val::vt_bool: {
    const auto ai = get<val::SpVAB>(a);
    auto i = convertToIndex(v.begin()+2, v.end(), *ai);
    auto r = (*ai)(i, drop_p);
    return val::Value(arr::make_cow<val::VArrayB>(false, std::move(r)));
  }
  case val::vt_time: {
    const auto adt = get<val::SpVADT>(a);
    auto i = convertToIndex(v.begin()+2, v.end(), *adt);
    auto r = (*adt)(i, drop_p);
    return val::Value(arr::make_cow<val::VArrayDT>(false, std::move(r)));
  }
  case val::vt_duration: {
    const auto adt = get<val::SpVADUR>(a);
    auto i  = convertToIndex(v.begin()+2, v.end(), *adt);
    auto r = (*adt)(i, drop_p);
    return val::Value(arr::make_cow<val::VArrayDUR>(false, std::move(r)));
  }
  case val::vt_period: {
    const auto adt = get<val::SpVAPRD>(a);
    auto i  = convertToIndex(v.begin()+2, v.end(), *adt);
    auto r = (*adt)(i, drop_p);
    return val::Value(arr::make_cow<val::VArrayPRD>(false, std::move(r)));
  }
  case val::vt_interval: {
    const auto adt = get<val::SpVAIVL>(a);
    auto i = convertToIndex(v.begin()+2, v.end(), *adt);
    auto r = (*adt)(i, drop_p);
    return val::Value(arr::make_cow<val::VArrayIVL>(false, std::move(r)));
  }
  default:
    throw std::domain_error(apply_visitor(val::Typeof(), a) + " cannot be subsetted");
  }
}


/// Check that all indices yield exactly one element. Raise an
/// exception if it's not the case.
static void checkScalarIndex(const vector<Index>& idx) {
  for (auto& i: idx) {
    if (i.trueSize() != 1) {
      throw std::range_error("attempt to select more than one element");
    }
  }
}


/// Double subset an array e.g. do 'x[[y]]'. 
val::Value funcs::dblsubset(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  auto a = getVal(v[0]);

  switch (a.which()) {
  case val::vt_zts: {
    const auto z = get<val::SpZts>(a);
    auto i = convertToIndex(v.begin()+1, v.end(), *z);
    checkScalarIndex(i);
    return arr::make_cow<arr::zts>(false, (*z)(i, true));
  }
  case val::vt_double: {
    const auto ad = get<val::SpVAD>(a);
    auto i = convertToIndex(v.begin()+1, v.end(), *ad);
    checkScalarIndex(i);
    auto r = (*ad)(i, true);
    return arr::make_cow<val::VArrayD>(false, std::move(r));
  }
  case val::vt_list: {
    const auto l = get<val::SpVList>(a);
    auto i = convertToIndex(v.begin()+1, v.end(), l->a);
    checkScalarIndex(i);
    auto r = (l->a)(i, true);
    return r[0];
  }    
  case val::vt_string: {
    const auto ai = get<val::SpVAS>(a); // const necessary for disambiguation of subset function
    auto i = convertToIndex(v.begin()+1, v.end(), *ai);
    checkScalarIndex(i);
    auto r = (*ai)(i, true);
    return val::Value(arr::make_cow<val::VArrayS>(false, std::move(r)));
  }
  case val::vt_bool: {
    const auto ai = get<val::SpVAB>(a);
    auto i = convertToIndex(v.begin()+1, v.end(), *ai);
    checkScalarIndex(i);
    auto r = (*ai)(i, true);
    return val::Value(arr::make_cow<val::VArrayB>(false, std::move(r)));
  }
  case val::vt_time: {
    const auto adt = get<val::SpVADT>(a);
    auto i = convertToIndex(v.begin()+1, v.end(), *adt);
    checkScalarIndex(i);
    auto r = (*adt)(i, true);
    return val::Value(arr::make_cow<val::VArrayDT>(false, std::move(r)));
  }
  case val::vt_duration: {
    const auto adt = get<val::SpVADUR>(a);
    auto i  = convertToIndex(v.begin()+1, v.end(), *adt);
    checkScalarIndex(i);
    auto r = (*adt)(i, true);
    return val::Value(arr::make_cow<val::VArrayDUR>(false, std::move(r)));
  }
  case val::vt_interval: {
    const auto adt = get<val::SpVAIVL>(a);
    auto i = convertToIndex(v.begin()+1, v.end(), *adt);
    checkScalarIndex(i);
    auto r = (*adt)(i, true);
    return val::Value(arr::make_cow<val::VArrayIVL>(false, std::move(r)));
  }
  default:
    throw std::domain_error(apply_visitor(val::Typeof(), a) + " cannot be double subsetted");
  }
}


/// make subassignment when type of a > type of b, i.e. result must be of type a.
template<typename A>
static void doSubassign(A& a, 
                        const val::Value& b, 
                        const vector<Index> i,
                        zcore::InterpCtx& ic) 
{
  switch (b.which()) {
  case val::vt_double: {
    const auto& bi = *get<val::SpVAD>(b);
    bi.size() > 1 ? a(i, bi) : a(i, bi[0]);
    break;
  }
  case val::vt_bool: {
    auto& bi = *get<val::SpVAB>(b);
    bi.size() > 1 ? a(i, bi) : a(i, bi[0]);
    break;
  }
  case val::vt_time: {
    auto& bi = *get<val::SpVADT>(b);
    bi.size() > 1 ? a(i, bi) : a(i, bi[0]);
    break;
  }
  case val::vt_interval: {
    auto& bi = *get<val::SpVAIVL>(b);
    bi.size() > 1 ? a(i, bi) : a(i, bi[0]);
    break;
  }
  case val::vt_duration: {
    auto& bi = *get<val::SpVADUR>(b);
    bi.size() > 1 ? a(i, bi) : a(i, bi[0]);
    break;
  }
  case val::vt_string: {
    auto& bi = *get<val::SpVAS>(b);
    bi.size() > 1 ? a(i, bi) : a(i, bi[0]);
    break;
  }
  case val::vt_list: {
    auto& l = get<val::SpVList>(b);
    a(i, l);
    break;
  }
  default:
    throw std::domain_error("invalid type for subassign");
  }
} 


/// R allows assigns of types that have priority over the type of the
/// array; this means the array gets converted "up" to the new type;
/// obviously, in a database setting this is a bad idea; we could end
/// up assigning to an element and having the whole array modified
/// (potentially many gigabytes on disk!). So in ztsdb we return an
/// error on such assignments. We silently convert "down" when the
/// type can be converted. Another particularity of subassign (and
/// this is also the case in R) is that it always modifies the array
/// (i.e. it never makes a copy).
val::Value funcs::subassign(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  // handle case a[] <- x, when dim(a) > 1 LLL

  enum { A, B };
  //  const auto name_a = val::get_scalar<arr::zstring>(getVal(v[NAME_A]));
  //auto& a = name_a.size() == 0 ? getVal(v[A]) : ic.s->k->next->r->findR(name_a);
  auto& a = getVal(v[A]);
  auto& b = getVal(v[B]);
  const size_t IDXOFF = 2;

  switch (a.which()) {
  case val::vt_zts: {
    auto& z = *get<val::SpZts>(a);
    auto i = convertToIndex(v.begin()+IDXOFF, v.end(), z);
    doSubassign(z, b, i, ic);
    break;
  }
  case val::vt_double: {
    auto& ai = *get<val::SpVAD>(a);
    auto i = convertToIndex(v.begin()+IDXOFF, v.end(), ai);
    doSubassign(ai, b, i, ic);
    break;
  }
  case val::vt_list: {
    auto l = get<val::SpVList>(a);
    auto i = convertToIndex(v.begin()+IDXOFF, v.end(), l->a);
    doSubassign(l->a, b, i, ic);
    break;
  }
  case val::vt_bool: {
    auto& ai = *get<val::SpVAB>(a);
    auto i = convertToIndex(v.begin()+IDXOFF, v.end(), ai);
    doSubassign(ai, b, i, ic);
    break;
  }
  case val::vt_time: {
    auto& ai = *get<val::SpVADT>(a);
    auto i = convertToIndex(v.begin()+IDXOFF, v.end(), ai);
    doSubassign(ai, b, i, ic);
    break;
  }
  case val::vt_duration: {
    auto& ai = *get<val::SpVADUR>(a);
    auto i = convertToIndex(v.begin()+IDXOFF, v.end(), ai);
    doSubassign(ai, b, i, ic);
    break;
  }
  case val::vt_interval: {
    auto& ai = *get<val::SpVAIVL>(a);
    auto i = convertToIndex(v.begin()+IDXOFF, v.end(), ai);
    doSubassign(ai, b, i, ic);
    break;
  }
  case val::vt_string: {
    auto& ai = *get<val::SpVAS>(a);
    auto i = convertToIndex(v.begin()+IDXOFF, v.end(), ai);
    doSubassign(ai, b, i, ic);
    break;
  }
  default:
    throw std::domain_error(apply_visitor(val::Typeof(), a) + " cannot be subassigned");
  }
  ic.s->k->next->next->atype |= interp::Kont::SILENT;
  return std::move(a);
}


val::Value funcs::dblsubassign(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { A, B };
  ic.s->k->next->next->atype |= interp::Kont::SILENT;
  auto& a = getVal(v[A]);
  if (a.which() == val::vt_list) {
    auto& b = getVal(v[B]);
    auto& al = get<val::SpVList>(a);
    auto i = convertToIndex(v.begin()+2, v.end(), al->a);
    al->a.operator()<Index, val::Value>(i, b);
    return a;
  }
  else {
    return subassign(v, ic);
  }
}
