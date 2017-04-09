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

  // if there is only one null index, then the intent is to select
  // the whole of the array, even in an n>1 case; so we push back
  // dim.size() Nulls, and we are done:
  if (end - begin == 1 && val::getVal(*begin).which() == val::vt_null) {
    for (unsigned i=0; i<z.getArray().getdim().size(); ++i) {
      vi.push_back(NullIndex{z.getArray().getdim(i)});
    }
  }
  else if (end-begin > 1 && static_cast<size_t>(end-begin) != z.getArray().getdim().size()) {
    throw std::range_error("incorrect number of dimensions");
  }
  else {
    for (auto e = begin; e != end; ++e) {
      switch (val::getVal(*e).which()) {
      case val::vt_null:
        vi.push_back(NullIndex{z.getdim(j)});
        break;      
      case val::vt_double: {
        // - 1, like in R: indices start at 1:
        const auto& idx = get<val::SpVAD>(val::getVal(*e));
        if (idx->size() == 0) {
          vi.push_back(IntIndex(Vector<size_t>()));
        }
        else if (!signbit((*idx)[0])) {
          const auto idx0 = applyf<double,double>(*idx, minus1).getcol(0);
          vi.push_back(IntIndex(Vector<size_t>(idx0.begin(), idx0.end())));
        }
        else {
          const auto idx0 = applyf<double,double>(*idx, negminus1).getcol(0);
          vi.push_back(IntIndexNeg(Vector<size_t>(idx0.begin(), idx0.end()), z.getdim(j)));
        }
        break; }
      case val::vt_bool: {
        const auto& idx = get<val::SpVAB>(val::getVal(*e));
        if (!idx->isVector()) {
          throw interp::EvalException("boolean index is not a vector", val::getLoc(*e));
        }
        if (z.getdim(j) != idx->getdim(0)) {
          throw interp::EvalException("boolean index not equal to array extent", val::getLoc(*e));
        }
        vi.push_back(BoolIndex{idx->getcol(0)});
        break; }
      case val::vt_string: {
        const auto& idx = get<val::SpVAS>(val::getVal(*e));
        vi.push_back(NameIndex{arr::stdvector<string, arr::zstring>(*idx), z.getArray().getnames(j)});
        break;
      }
      case val::vt_time: {
        if (e != begin) {
          throw interp::EvalException("zts: can't use a time index into dimension > 1", val::getLoc(*e));
        }
        const auto& idx = get<val::SpVADT>(val::getVal(*e));
        vi.push_back(DtimeIndex{idx->getcol(0), z.getIndex().getcol(0)});
        break;
      }
      case val::vt_interval: {
        if (e != begin) {
          throw interp::EvalException("zts: can't use an interval index into dimension > 1", 
                                      val::getLoc(*e));
        }
        const auto& idx = get<val::SpVAIVL>(val::getVal(*e));
        vi.push_back(IntervalIndex{idx->getcol(0), z.getIndex().getcol(0)});
        break;
      }
      default:
        throw interp::EvalException("convertToIndex incorrect type", val::getLoc(*e));
      }
      ++j;
    }
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
  if (end - begin == 1 && val::getVal(*begin).which() == val::vt_null) {
    for (unsigned i=0; i<a.getdim().size(); ++i) {
      vi.push_back(NullIndex{a.getdim(i)});
    }
  }
  else if (end-begin > 1 && static_cast<size_t>(end-begin) != a.getdim().size()) {
    throw std::range_error("incorrect number of dimensions");
  }
  else {
    for (auto e = begin; e != end; ++e) {
      switch (val::getVal(*e).which()) {
      case val::vt_null:
        vi.push_back(NullIndex{a.getdim(j)});
        break;      
      case val::vt_double: {
        const auto& idx = get<val::SpVAD>(val::getVal(*e));
        if (idx->size() == 0) {
          vi.push_back(IntIndex(Vector<size_t>()));
        }
        else if ((*idx)[0] >= 0) {
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
        const auto& idx = get<val::SpVAB>(val::getVal(*e));
        if (a.getdim(j) != idx->getdim(0) && a.size() != idx->size()) {
          throw interp::EvalException("boolean index not equal to array extent", val::getLoc(*e));
        }        
        vi.push_back(BoolIndex{idx->getcol(0)});
        break; 
      }
      case val::vt_string: {
        const auto& idx = get<val::SpVAS>(val::getVal(*e));
        vi.push_back(NameIndex{arr::stdvector<string, arr::zstring>(*idx), a.getnames(j)});
        break; 
      }
      case val::vt_time: {
        if (!a.isVector()) {
          throw interp::EvalException("time point index into non-vector", val::getLoc(*e));
        }
        const auto& idx = get<val::SpVADT>(val::getVal(*e));
        push_back_dtime_index(vi, *idx, val::getLoc(*e), a); 
        break;
      }
      case val::vt_interval: {
        if (!a.isVector()) {
          throw interp::EvalException("interval index into non-vector", val::getLoc(*e));
        }
        const auto& idx = get<val::SpVAIVL>(val::getVal(*e));
        push_back_interval_index(vi, *idx, val::getLoc(*e), a); 
        break;
      }
        /// case val::vt_arraydur:
        /// can't make an index from a duration
      default:
        throw interp::EvalException("incorrect index type", val::getLoc(*e));
      }
      ++j;
    }
  }
  return vi;
}


/// subset an array: 
val::Value funcs::subset(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { DROP, A };             // and the following ones are the indices (from the ellipsis)
  auto& a = val::getVal(v[A]);

  bool drop_p;
  try {
    drop_p = val::get_scalar<bool>(funcs::convert_logical(val::getVal(v[DROP])));
  }
  catch (std::exception& e) {
    throw std::range_error("cannot convert 'drop' parameter to boolean");
  }

  switch (a.which()) {
  case val::vt_zts: {
    const auto& z = get<val::SpZts>(a);
    auto i = convertToIndex(v.begin()+2, v.end(), *z);
    return arr::make_cow<arr::zts>(false, (*z)(i, drop_p));
  }
  case val::vt_double: {
    const auto& ad = get<val::SpVAD>(a);
    auto i = convertToIndex(v.begin()+2, v.end(), *ad);
    auto r = (*ad)(i, drop_p);
    return arr::make_cow<val::VArrayD>(false, std::move(r));
  }
  case val::vt_list: {
    const auto& l = get<val::SpVList>(a);
    auto i = convertToIndex(v.begin()+2, v.end(), l->a);
    auto r = (l->a)(i, drop_p);
    // as in R, list does not revert to scalars or it would lose its list semantics:
    return arr::make_cow<val::VList>(false, std::move(r));
  }    
  case val::vt_string: {
    const auto& ai = get<val::SpVAS>(a); // const necessary for disambiguation of subset function
    auto i = convertToIndex(v.begin()+2, v.end(), *ai);
    auto r = (*ai)(i, drop_p);
    return val::Value(arr::make_cow<val::VArrayS>(false, std::move(r)));
  }
  case val::vt_bool: {
    const auto& ai = get<val::SpVAB>(a);
    auto i = convertToIndex(v.begin()+2, v.end(), *ai);
    auto r = (*ai)(i, drop_p);
    return val::Value(arr::make_cow<val::VArrayB>(false, std::move(r)));
  }
  case val::vt_time: {
    const auto& adt = get<val::SpVADT>(a);
    auto i = convertToIndex(v.begin()+2, v.end(), *adt);
    auto r = (*adt)(i, drop_p);
    return val::Value(arr::make_cow<val::VArrayDT>(false, std::move(r)));
  }
  case val::vt_duration: {
    const auto& adt = get<val::SpVADUR>(a);
    auto i  = convertToIndex(v.begin()+2, v.end(), *adt);
    auto r = (*adt)(i, drop_p);
    return val::Value(arr::make_cow<val::VArrayDUR>(false, std::move(r)));
  }
  case val::vt_period: {
    const auto& adt = get<val::SpVAPRD>(a);
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
static val::Value dblsubsetHelper(val::Value& a,
                                  const yy::location& loc,
                                  std::vector<val::VBuiltinG::arg_t>::iterator begin,
                                  std::vector<val::VBuiltinG::arg_t>::iterator end) {

  switch (a.which()) {
  case val::vt_zts: {
    const auto& z = get<val::SpZts>(a);
    auto i = convertToIndex(begin, end, *z);
    checkScalarIndex(i);
    return arr::make_cow<arr::zts>(arr::NOFLAGS, (*z)(i, true));
  }
  case val::vt_double: {
    const auto& ad = get<val::SpVAD>(a);
    auto i = convertToIndex(begin, end, *ad);
    checkScalarIndex(i);
    auto r = (*ad)(i, true);
    return arr::make_cow<val::VArrayD>(arr::NOFLAGS, std::move(r));
  }
  case val::vt_list: {
    auto& l = get<val::SpVList>(a);
    // raise exception if more than one index LLL
    auto i = convertToIndex(begin, begin+1, l.get()->a); // don't make a copy of l!
    idx_type iv=0, ii=0; 
    if (!i[0].getfirst(iv, ii)) {
      throw interp::EvalException("invalid index", loc);
    }
    auto* a = &l.get()->a.getcol(0)[iv]; // don't make a copy of l!
    for (unsigned u=1; u<i[0].size(); ++u) {
      if (!i[0].getnext(iv, ii)) break;
      if (a->which() == val::vt_list) {
        auto& l = get<val::SpVList>(*a);
        a = &l.get()->a.getcol(0)[iv];
      }
      else {
        using arg_t = val::VBuiltinG::arg_t;
        auto arg = vector<arg_t>{arg_t{"", val::make_array(static_cast<double>(iv+1)), loc}};
        return dblsubsetHelper(*a, loc, arg.begin(), arg.end());
      }
    }
    return val::VPtr(*a);
  }    
  case val::vt_string: {
    const auto& ai = get<val::SpVAS>(a); // const necessary for disambiguation of subset function
    auto i = convertToIndex(begin, end, *ai);
    checkScalarIndex(i);
    auto r = (*ai)(i, true);
    return val::Value(arr::make_cow<val::VArrayS>(arr::NOFLAGS, std::move(r)));
  }
  case val::vt_bool: {
    const auto& ai = get<val::SpVAB>(a);
    auto i = convertToIndex(begin, end, *ai);
    checkScalarIndex(i);
    auto r = (*ai)(i, true);
    return val::Value(arr::make_cow<val::VArrayB>(arr::NOFLAGS, std::move(r)));
  }
  case val::vt_time: {
    const auto& adt = get<val::SpVADT>(a);
    auto i = convertToIndex(begin, end, *adt);
    checkScalarIndex(i);
    auto r = (*adt)(i, true);
    return val::Value(arr::make_cow<val::VArrayDT>(arr::NOFLAGS, std::move(r)));
  }
  case val::vt_duration: {
    const auto& adt = get<val::SpVADUR>(a);
    auto i  = convertToIndex(begin, end, *adt);
    checkScalarIndex(i);
    auto r = (*adt)(i, true);
    return val::Value(arr::make_cow<val::VArrayDUR>(arr::NOFLAGS, std::move(r)));
  }
  case val::vt_interval: {
    const auto& adt = get<val::SpVAIVL>(a);
    auto i = convertToIndex(begin, end, *adt);
    checkScalarIndex(i);
    auto r = (*adt)(i, true);
    return val::Value(arr::make_cow<val::VArrayIVL>(arr::NOFLAGS, std::move(r)));
  }
  default:
    throw std::domain_error(apply_visitor(val::Typeof(), a) + " cannot be double subsetted");
  }
}

val::Value funcs::dblsubset(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return dblsubsetHelper(val::getVal(v[0]), val::getLoc(v[0]), v.begin()+1, v.end());
}



/// make subassignment when type of a > type of b, i.e. result must be of type a.
template<typename A>
static void doSubassign(A& a, 
                        const val::Value& b, 
                        const vector<Index> i) 
{
  switch (b.which()) {
  case val::vt_double: {
    const auto& bi = get<val::SpVAD>(b);
    bi->size() > 1 ? a(i, *bi) : a(i, (*bi)[0]);
    break;
  }
  case val::vt_bool: {
    const auto& bi = get<val::SpVAB>(b);
    bi->size() > 1 ? a(i, *bi) : a(i, (*bi)[0]);
    break;
  }
  case val::vt_time: {
    const auto& bi = get<val::SpVADT>(b);
    bi->size() > 1 ? a(i, *bi) : a(i, (*bi)[0]);
    break;
  }
  case val::vt_interval: {
    const auto& bi = get<val::SpVAIVL>(b);
    bi->size() > 1 ? a(i, *bi) : a(i, (*bi)[0]);
    break;
  }
  case val::vt_duration: {
    const auto& bi = get<val::SpVADUR>(b);
    bi->size() > 1 ? a(i, *bi) : a(i, (*bi)[0]);
    break;
  }
  case val::vt_period: {
    const auto& bi = get<val::SpVAPRD>(b);
    bi->size() > 1 ? a(i, *bi) : a(i, (*bi)[0]);
    break;
  }
  case val::vt_string: {
    const auto& bi = get<val::SpVAS>(b);
    bi->size() > 1 ? a(i, *bi) : a(i, (*bi)[0]);
    break;
  }
  case val::vt_list: {
    const auto& l = get<val::SpVList>(b);
    l->size() > 1 ?  a(i, l->a) : a(i, l->a[0]);
    break;
  }
  case val::vt_null: {
    std::cout << "going through NULL" << std::endl;
    const auto& n = get<val::VNull>(b);
    a(i, n);
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
static val::Value subassignHelper(val::Value& a,
                                  const val::Value& b,
                                  std::vector<val::VBuiltinG::arg_t>::iterator begin,
                                  std::vector<val::VBuiltinG::arg_t>::iterator end)
{  
  switch (a.which()) {
  case val::vt_zts: {
    auto& z = *get<val::SpZts>(a);
    auto i = convertToIndex(begin, end, z);
    doSubassign(z, b, i);
    break;
  }
  case val::vt_double: {
    auto& ai = *get<val::SpVAD>(a);
    auto i = convertToIndex(begin, end, ai);
    doSubassign(ai, b, i);
    break;
  }
  case val::vt_list: {
    auto& l = get<val::SpVList>(a);
    auto i = convertToIndex(begin, end, l->a);
    std::set<size_t> delidx;
    if (b.which() == val::vt_null) {
      // assigning NULL actually means deleting list elements:
      idx_type iv=0, ii=0;
      if (!i.size() || !i[0].getfirst(iv, ii)) {
        throw std::out_of_range("invalid index");
      }
      delidx.insert(iv);
      for (size_t u=1; u<i[0].size(); ++u) {
        if (!i[0].getnext(iv, ii))
          break;
        delidx.insert(iv);
      }
      size_t n = 0;
      for (auto j=delidx.begin(); j != delidx.end(); ++j) {
        l->remove(*j - n++);
      }      
    }
    else {
      doSubassign(l->a, b, i);
    }
    break;
  }
  case val::vt_bool: {
    auto& ai = *get<val::SpVAB>(a);
    auto i = convertToIndex(begin, end, ai);
    doSubassign(ai, b, i);
    break;
  }
  case val::vt_time: {
    auto& ai = *get<val::SpVADT>(a);
    auto i = convertToIndex(begin, end, ai);
    doSubassign(ai, b, i);
    break;
  }
  case val::vt_duration: {
    auto& ai = *get<val::SpVADUR>(a);
    auto i = convertToIndex(begin, end, ai);
    doSubassign(ai, b, i);
    break;
  }
  case val::vt_interval: {
    auto& ai = *get<val::SpVAIVL>(a);
    auto i = convertToIndex(begin, end, ai);
    doSubassign(ai, b, i);
    break;
  }
  case val::vt_period: {
    auto& ai = *get<val::SpVAPRD>(a);
    auto i = convertToIndex(begin, end, ai);
    doSubassign(ai, b, i);
    break;
  }
  case val::vt_string: {
    auto& ai = *get<val::SpVAS>(a);
    auto i = convertToIndex(begin, end, ai);
    doSubassign(ai, b, i);
    break;
  }
  default:
    throw std::domain_error(apply_visitor(val::Typeof(), a) + " cannot be subassigned");
  }
  return a;
}

val::Value funcs::subassign(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  // handle case a[] <- x, when dim(a) > 1 LLL

  // handle loc LLL
  
  enum { A, B };
  //  const auto name_a = val::get_scalar<arr::zstring>(val::getVal(v[NAME_A]));
  //auto& a = name_a.size() == 0 ? val::getVal(v[A]) : ic.s->k->next->r->findR(name_a);
  auto& a = val::getVal(v[A]);
  auto& b = val::getVal(v[B]);

  ic.s->k->next->next->atype |= interp::Kont::SILENT;
  return subassignHelper(a, b, v.begin()+2, v.end());
}


static void dblsubassignHelper(val::Value& val,
                               const val::Value& b,
                               const yy::location& loc,
                               std::vector<val::VBuiltinG::arg_t>::iterator begin,
                               std::vector<val::VBuiltinG::arg_t>::iterator end) {
  if (val.which() == val::vt_list) {
    val::Value* a = &val;
    auto& l = get<val::SpVList>(val);
    const auto i = convertToIndex(begin, begin+1, l.get()->a);
    idx_type iv=0, ii=0;
    unsigned u=0;
    try {
      if (!i.size() || !i[0].getfirst(iv, ii)) {
        throw interp::EvalException("invalid index", loc);
      }
      a = &l.get()->a.getcol(0)[iv]; // don't make a copy of l!
      for (u=1; u<i[0].size(); ++u) {
        if (!i[0].getnext(iv, ii))
          break;
        if (a->which() == val::vt_list) {
          auto& l = get<val::SpVList>(*a);
          a = &l.get()->a.getcol(0)[iv];
        }
        else if (u == i[0].size() - 1) {
          // if this is the last index we allow indexing into any type of 'Value':
          using arg_t = val::VBuiltinG::arg_t;
          auto arg = vector<arg_t>{arg_t{"", val::make_array(static_cast<double>(iv+1)), loc}};
          dblsubassignHelper(*a, b, loc, arg.begin(), arg.end());
        }
        else throw std::out_of_range("recursive indexing failed at level "s + std::to_string(u+1));
      }
      if (b.which() == val::vt_null) {
        l->remove(iv);
      }
      else {
        *a = b;
      }
    }
    catch (...) {
      // in the case of an index character that doesn't exist, we
      // extend the list in the last position:
      if (u == i[0].size() - 1 && i[0].idx.which() == arr::Index::it_names) {
        const auto& name = get<arr::NameIndex>(i[0].idx).vs[iv];
        if (a->which() == val::vt_list) {
          auto& l = get<val::SpVList>(*a);
          l->a.concat(b, name);
        }
        else throw std::out_of_range("recursive indexing failed at level "s + std::to_string(u+1));
      }
      else throw std::out_of_range("recursive indexing failed at level "s + std::to_string(u+1));
    }
  }
  else {
    subassignHelper(val, b, begin, end);
  } 
}

val::Value funcs::dblsubassign(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { A, B };
  ic.s->k->next->next->atype |= interp::Kont::SILENT;
  try {
    auto& a = val::getVal(v[A]);
    const auto& b = val::getVal(v[B]);
    dblsubassignHelper(a, b, val::getLoc(*(v.begin()+2)), v.begin()+2, v.end());
    return b;
  }
  catch (interp::EvalException&) {
    throw;
  }
  catch (std::exception& e) {
    throw interp::EvalException(e.what(), val::getLoc(*(v.begin()+2)));
  }
}
