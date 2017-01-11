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


#include "unop_binop_funcs.hpp"
#include "parser_utils.hpp"
#include "timezone/ztime.hpp"
#include "parser.hpp"           // bison-generated

extern tz::Zones tzones;


static const string types_to_string(int t1, int t2, int op) {
  return "(" + val::vt_to_string.at(t1) + " " + ztsdb::op_to_string.at(op) + " " + 
    val::vt_to_string.at(t2) + ")";
}


// -------------- unop -----------------------------

// The issue with 'std::negate' is that it creates a true '-0' and
// then we have -0 != 0; we follow the R convention here with -0 == 0:
template <typename T>
struct unary_minus {
  T operator()(const T& t) { return -t; }
};

// ---------------------------------------
// templates for not in-place unop evaluation:
template<typename T, typename R, int OP> struct do_unop {
  static val::Value f(const arr::Array<T>&) { 
    throw range_error("invalid type for unary operator"); } };
template<typename T, typename R> struct do_unop<T, R, yy::parser::token::PLUS> {
  static val::Value f(const arr::Array<T>& d) { 
    return make_cow<arr::Array<R>>(false, d); } };
template<typename T, typename R> struct do_unop<T, R, yy::parser::token::MINUS> {
  static val::Value f(const arr::Array<T>& d) { 
    return make_cow<arr::Array<R>>
      (false, apply<unary_minus<T>, R, typename arr::Array<T>::comparator>(d)); } };
template<typename T, typename R> struct do_unop<T, R, yy::parser::token::NOT> {
  static val::Value f(const arr::Array<T>& d) { 
    return make_cow<arr::Array<R>>
      (false, apply<std::logical_not<T>, R, typename arr::Array<R>::comparator>(d)); } };


template<typename T, typename R, typename... OP>
inline val::Value evalunop_array(const arr::Array<T>& d, int op) {
  throw std::range_error("invalid type for unary operator");
}
template <typename T, typename R, int H, int... OP>
inline val::Value evalunop_array(const arr::Array<T>& d, int op) {
  return H == op ? 
    do_unop<T, R, H>::f(d) : 
    evalunop_array<T,R,OP...>(d, op);
}

// ---------------------------------------
// templates for inplace unop evaluation:
template<typename T, int OP> struct unop_inplace {
  static void f(arr::Array<T>&) {
    throw range_error("invalid type for unary operator"); } };
template<typename T> struct unop_inplace<T, yy::parser::token::PLUS> {
  static void f(arr::Array<T>& d) { }; };
template<typename T> struct unop_inplace<T, yy::parser::token::MINUS> {
  static void f(arr::Array<T>& d) { 
    d.template apply<unary_minus<T>>(); } };
template<typename T> struct unop_inplace<T, yy::parser::token::NOT> {
  static void f(arr::Array<T>& d) { 
    d.template apply<std::logical_not<T>>(); } };

template<typename T, typename... OP>
inline void evalunop_inplace(arr::Array<T>& d, int op) {
  throw std::range_error("invalid type for unary operator");
}
template <typename T, int H, int... OP>
inline void evalunop_inplace(arr::Array<T>& d, int op) {
  H == op ? 
    unop_inplace<T, H>::f(d) : 
    evalunop_inplace<T,OP...>(d, op);
}

val::Value funcs::evalunop(val::Value v, int op) {
  static std::set<int> arithmetic{
    yy::parser::token::PLUS, 
    yy::parser::token::MINUS};

  switch (v.which()) {
  case val::vt_double: {
    const auto& t = get<val::SpVAD>(v);
    if (t.getFlags() == arr::REF && arithmetic.find(op) != arithmetic.end()) {
      auto& t = get<val::SpVAD>(v);
      evalunop_inplace<double,
                       yy::parser::token::PLUS,
                       yy::parser::token::MINUS>(*t, op);
      return t;
    }
    else {
      if (arithmetic.find(op) != arithmetic.end()) {
        return evalunop_array<double, double, 
                              yy::parser::token::PLUS, yy::parser::token::MINUS>(*t, op);
      }
      else {
        return evalunop_array<double, bool, yy::parser::token::NOT>(*t, op);
      }
    }
  }
  case val::vt_duration: {
    const auto& t = get<val::SpVADUR>(v);
    if (t.getFlags() == arr::REF && arithmetic.find(op) != arithmetic.end()) {
      auto& t = get<val::SpVADUR>(v);
      evalunop_inplace<Global::duration,
                       yy::parser::token::PLUS,
                       yy::parser::token::MINUS>(*t, op);
      return t;
    }
    else {
      return evalunop_array<Global::duration, Global::duration, 
                            yy::parser::token::PLUS, yy::parser::token::MINUS>(*t, op);
    }
  }
  case val::vt_period: {
    const auto& t = get<val::SpVAPRD>(v);
    if (t.getFlags() == arr::REF && arithmetic.find(op) != arithmetic.end()) {
      auto& t = get<val::SpVAPRD>(v);
      evalunop_inplace<tz::period,
                       yy::parser::token::PLUS,
                       yy::parser::token::MINUS>(*t, op);
      return t;
    }
    else {
      return evalunop_array<tz::period, tz::period, 
                            yy::parser::token::PLUS, yy::parser::token::MINUS>(*t, op);
    }
  }
  case val::vt_bool: {
    const auto& t = get<val::SpVAB>(v);
    if (t.getFlags() == arr::REF && op == yy::parser::token::NOT) {
      auto& t = get<val::SpVAB>(v);
      evalunop_inplace<bool, yy::parser::token::NOT>(*t, op);
      return t;
    }
    else {
      return evalunop_array<bool, bool, yy::parser::token::NOT>(*t, op);
    }
  }
  default:
    throw range_error("invalid type for unary operator");
  }
}


// -------------- binop -----------------------------


// ---------------------------------------
// templates for not in-place binop evaluation:
template<typename T, typename U, typename R, int OP> struct doop {
  static val::Value f(const arr::Array<T>&, const arr::Array<U>&) { 
    throw range_error("invalid type for binary operator"); } };
template<typename T, typename U, typename R> struct doop<T, U, R, yy::parser::token::PLUS> {
  static val::Value f(const arr::Array<T>& d1, const arr::Array<U>& d2) { 
    return make_cow<arr::Array<R>>(false, apply<T, U, R, ztsdb::plus<T,U,R>>(d1, d2)); } };
template<typename T, typename U, typename R> struct doop<T, U, R, yy::parser::token::MINUS> {
  static val::Value f(const arr::Array<T>& d1, const arr::Array<U>& d2) { 
    return make_cow<arr::Array<R>>(false, apply<T, U, R, ztsdb::minus<T,U,R>>(d1, d2)); } };
template<typename T, typename U, typename R> struct doop<T, U, R, yy::parser::token::MUL> {
  static val::Value f(const arr::Array<T>& d1, const arr::Array<U>& d2) { 
    return make_cow<arr::Array<R>>(false, apply<T, U, R, ztsdb::multiplies<T,U,R>>(d1, d2)); } };
template<typename T, typename U, typename R> struct doop<T, U, R, yy::parser::token::DIV> {
  static val::Value f(const arr::Array<T>& d1, const arr::Array<U>& d2) { 
    return make_cow<arr::Array<R>>(false, apply<T, U, R, ztsdb::divides<T,U,R>>(d1, d2)); } };
template<typename T, typename U, typename R> struct doop<T, U, R, yy::parser::token::MOD> {
  static val::Value f(const arr::Array<T>& d1, const arr::Array<U>& d2) { 
    return make_cow<arr::Array<R>>(false, apply<T, U, R, ztsdb::modulus<T,U,R>>(d1, d2)); } };
template<typename T, typename U, typename R> struct doop<T, U, R, yy::parser::token::POWER> {
  static val::Value f(const arr::Array<T>& d1, const arr::Array<U>& d2) { 
    return make_cow<arr::Array<R>>(false, apply<T, U, R, funcs::power<T>>(d1, d2)); } };
// COLON is now transformed by the parser into a call to 'seq'
// template<typename T, typename U, typename R> struct doop<T, U, R, yy::parser::token::COLON> {
//   static val::Value f(const arr::Array<T>& d1, const arr::Array<U>& d2) { 
//     return make_cow<arr::Array<R>>(false, arr::seq_to, d1[0], d2[0], 1.0); } };
template<typename T, typename U> struct doop<T, U, bool, yy::parser::token::LE> {
  static val::Value f(const arr::Array<T>& d1, const arr::Array<U>& d2) { 
    return make_cow<val::VArrayB>(false, apply<T, U, bool, std::less_equal<T>>(d1, d2)); } };
template<typename T, typename U> struct doop<T, U, bool, yy::parser::token::LT> {
  static val::Value f(const arr::Array<T>& d1, const arr::Array<U>& d2) { 
    return make_cow<val::VArrayB>(false, apply<T, U, bool, std::less<T>>(d1, d2)); } };
template<typename T, typename U> struct doop<T, U, bool, yy::parser::token::EQ> {
  static val::Value f(const arr::Array<T>& d1, const arr::Array<U>& d2) { 
    return make_cow<val::VArrayB>(false, apply<T, U, bool, std::equal_to<T>>(d1, d2)); } };
template<typename T, typename U> struct doop<T, U, bool, yy::parser::token::NE> {
  static val::Value f(const arr::Array<T>& d1, const arr::Array<U>& d2) { 
    return make_cow<val::VArrayB>(false, apply<T, U, bool, std::not_equal_to<T>>(d1, d2)); } };
template<typename T, typename U> struct doop<T, U, bool, yy::parser::token::GE> {
  static val::Value f(const arr::Array<T>& d1, const arr::Array<U>& d2) { 
    return make_cow<val::VArrayB>(false, apply<T, U, bool, std::greater_equal<T>>(d1, d2)); } };
template<typename T, typename U> struct doop<T, U, bool, yy::parser::token::GT> {
  static val::Value f(const arr::Array<T>& d1, const arr::Array<U>& d2) { 
    return make_cow<val::VArrayB>(false, apply<T, U, bool, std::greater<T>>(d1, d2)); } };
template<typename T, typename U> struct doop<T, U, bool, yy::parser::token::AND> {
  static val::Value f(const arr::Array<T>& d1, const arr::Array<U>& d2) { 
    return make_cow<val::VArrayB>(false, apply<T, U, bool, std::logical_and<T>>(d1, d2)); } };
template<typename T, typename U> struct doop<T, U, bool, yy::parser::token::AND2> {
  static val::Value f(const arr::Array<T>& d1, const arr::Array<U>& d2) { 
    return val::make_array((d1.size() && d1[0]) && (d2.size() && d2[0])); } };
template<typename T, typename U> struct doop<T, U, bool, yy::parser::token::OR> {
  static val::Value f(const arr::Array<T>& d1, const arr::Array<U>& d2) { 
    return make_cow<val::VArrayB>(false, apply<T, U, bool, std::logical_or<T>>(d1, d2)); } };
template<typename T, typename U> struct doop<T, U, bool, yy::parser::token::OR2> {
  static val::Value f(const arr::Array<T>& d1, const arr::Array<U>& d2) { 
    return val::make_array((d1.size() && d1[0]) || (d2.size() && d2[0])); } };


template<typename T, typename U, typename R, typename... OP>
inline val::Value evalbinop_array_array_(const arr::Array<T>& d1, const arr::Array<U>& d2, int op) {
  throw std::range_error("invalid type for binary operator2");
}
template <typename T, typename U, typename R, int H, int... OP>
inline val::Value evalbinop_array_array_(const arr::Array<T>& d1, const arr::Array<U>& d2, int op) {
  return H == op ? 
    doop<T, U, R, H>::f(d1, d2) : 
    evalbinop_array_array_<T,U,R,OP...>(d1, d2, op);
}

template<typename T, typename U, typename R>
struct pplus {
  inline R operator()(const T& t, const U& u, const tz::Zone& z) const {
    return tz::plus(t, u, z);
  }
};
template<typename T, typename U, typename R>
struct period_minus {
  inline R operator()(const T& t, const U& u, const tz::Zone& z) const {
    return tz::minus(t, u, z);
  }
};

// templates for the third argument when using period:
template<typename T, typename U, typename R, int OP> struct doop_prd {
  static val::Value f(const arr::Array<T>&, const arr::Array<U>&, const tz::Zone& tz) { 
    throw range_error("invalid type for binary operator3"); } };
template<typename T, typename U, typename R> struct doop_prd<T, U, R, yy::parser::token::PLUS> {
  static val::Value f(const arr::Array<T>& d1, const arr::Array<U>& d2, const tz::Zone& tz) { 
    return make_cow<arr::Array<R>>(false, apply_attrib<T, U, R, pplus<T,U,R>, 
                                   tz::Zone>(d1, d2, tz)); } };
template<typename T, typename U, typename R> struct doop_prd<T, U, R, yy::parser::token::MINUS> {
  static val::Value f(const arr::Array<T>& d1, const arr::Array<U>& d2, const tz::Zone& tz) { 
    return make_cow<arr::Array<R>>(false, apply_attrib<T, U, R, period_minus<T,U,R>, 
                                   tz::Zone>(d1, d2, tz)); } };


template<typename T, typename U, typename R, typename... OP>
inline val::Value evalbinop_array_array_prd(const arr::Array<T>& d1, 
                                            const arr::Array<U>& d2, 
                                            int op,
                                            const tz::Zone& tz) {
  throw std::range_error("invalid type for binary operator");
}
template <typename T, typename U, typename R, int H, int... OP>
inline val::Value evalbinop_array_array_prd(const arr::Array<T>& d1, 
                                            const arr::Array<U>& d2, 
                                            int op,
                                            const tz::Zone& tz) {
  return H == op ? 
    doop_prd<T, U, R, H>::f(d1, d2, tz) : 
    evalbinop_array_array_prd<T,U,R,OP...>(d1, d2, op, tz);
}



// ---------------------------------------
// templates for inplace binop evaluation:
template<typename T, typename U, int OP> struct doop_inplace {
  static void f(arr::Array<T>&, const arr::Array<U>&) {
    throw range_error("invalid type for binary operator"); } };
template<typename T, typename U> struct doop_inplace<T, U, yy::parser::token::PLUS> {
  static void f(arr::Array<T>& d1, const arr::Array<U>& d2) { 
    d1.template apply<ztsdb::plus<T,U,T>, const arr::Array<U>>(d2); } };
template<typename T, typename U> struct doop_inplace<T, U, yy::parser::token::MINUS> {
  static void f(arr::Array<T>& d1, const arr::Array<U>& d2) { 
    d1.template apply<ztsdb::minus<T,U,T>, const arr::Array<U>>(d2); } };
template<typename T, typename U> struct doop_inplace<T, U, yy::parser::token::MUL> {
  static void f(arr::Array<T>& d1, const arr::Array<U>& d2) { 
    d1.template apply<ztsdb::multiplies<T,U,T>, const arr::Array<U>>(d2); } };
template<typename T, typename U> struct doop_inplace<T, U, yy::parser::token::DIV> {
  static void f(arr::Array<T>& d1, const arr::Array<U>& d2) { 
    d1.template apply<ztsdb::divides<T,U,T>, const arr::Array<U>>(d2); } };
template<typename T, typename U> struct doop_inplace<T, U, yy::parser::token::MOD> {
  static void f(arr::Array<T>& d1, const arr::Array<U>& d2) { 
    d1.template apply<ztsdb::modulus<T,U,T>, const arr::Array<U>>(d2); } };
template<typename T, typename U> struct doop_inplace<T, U, yy::parser::token::POWER> {
  static void f(arr::Array<T>& d1, const arr::Array<U>& d2) { 
    d1.template apply<funcs::power<T>, const arr::Array<U>>(d2); } };
template<typename T, typename U> struct doop_inplace<T, U, yy::parser::token::OR> {
  static void f(arr::Array<T>& d1, const arr::Array<U>& d2) { 
    d1.template apply<std::logical_or<T>, const arr::Array<U>>(d2); } };
template<typename T, typename U> struct doop_inplace<T, U, yy::parser::token::AND> {
  static void f(arr::Array<T>& d1, const arr::Array<U>& d2) { 
    d1.template apply<std::logical_and<T>, const arr::Array<U>>(d2); } };

template<typename T, typename U, typename... OP>
inline void evalbinop_array_array_inplace_(arr::Array<T>& d1, const arr::Array<U>& d2, int op) {
  throw std::range_error("invalid type for binary operator");
}
template <typename T, typename U, int H, int... OP>
inline void evalbinop_array_array_inplace_(arr::Array<T>& d1, const arr::Array<U>& d2, int op) {
  H == op ? 
    doop_inplace<T, U, H>::f(d1, d2) : 
    evalbinop_array_array_inplace_<T,U,OP...>(d1, d2, op);
}


// templates for the third argument when using period:
template<typename T, typename U, int OP> struct doop_inplace_prd {
  static void f(arr::Array<T>&, const arr::Array<U>&) {
    throw range_error("invalid type for binary operator7"); } };
template<typename T, typename U> struct doop_inplace_prd<T, U, yy::parser::token::PLUS> {
  static void f(arr::Array<T>& d1, const arr::Array<U>& d2, const tz::Zone& tz) { 
    d1.template apply_attrib<pplus<T,U,T>, U, tz::Zone>(d2, tz); } };
template<typename T, typename U> struct doop_inplace_prd<T, U, yy::parser::token::MINUS> {
  static void f(arr::Array<T>& d1, const arr::Array<U>& d2, const tz::Zone& tz) { 
    d1.template apply_attrib<period_minus<T,U,T>, U, tz::Zone>(d2, tz); } };


template<typename T, typename U, typename... OP>
inline void evalbinop_array_array_inplace_prd(arr::Array<T>& d1, 
                                              const arr::Array<U>& d2, 
                                              int op,
                                              const tz::Zone& tz) {
  throw std::range_error("invalid type for binary operator8");
}
template <typename T, typename U, int H, int... OP>
inline void evalbinop_array_array_inplace_prd(arr::Array<T>& d1, 
                                              const arr::Array<U>& d2, 
                                              int op,
                                              const tz::Zone& tz) {
  H == op ? 
    doop_inplace_prd<T, U, H>::f(d1, d2, tz) : 
    evalbinop_array_array_inplace_prd<T,U,OP...>(d1, d2, op, tz);
}


/// Compare an 'Array<Value>' (i.e. a list) to 'Value'. This function
/// does not implement comparison or logical operators.
val::Value evalbinop_array_scalar(const arr::Array<val::Value>& d1, val::Value d2, int op) { 
  switch (op) {
  case yy::parser::token::EQ : 
    return make_cow<val::VArrayB>(false, 
                                  apply_scalar<val::Value, 
                                  val::Value, bool, std::equal_to<val::Value>>(d1, d2));
  case yy::parser::token::NE : 
    return make_cow<val::VArrayB>(false, 
                                  apply_scalar<val::Value, 
                                  val::Value, bool, std::not_equal_to<val::Value>>(d1, d2));
  default: throw std::range_error("invalid type for binary operator"); 
  }
}


static val::Value evalbinop_value_value_comp(const val::Value d1, const val::Value d2, int op) { 
  if (d1.which() != d2.which()) {
    switch (op) {
    case yy::parser::token::EQ : 
      return val::make_array(false);
    case yy::parser::token::NE : 
      return val::make_array(true);
    default: throw range_error("invalid type for binary operator " 
                               + types_to_string(d1.which(), d2.which(), op));
    }
  }
  else {
    throw range_error("invalid type for binary operator " 
                      + types_to_string(d1.which(), d2.which(), op));
  }
}


// zts ------------------------

template<typename U, typename R>
static val::Value evalbinop_zts(const arr::Array<Global::dtime>& idx, 
                                const arr::Array<double>& d1, 
                                const arr::Array<U>& d2, 
                                int op) { 
  switch (op) {
  case yy::parser::token::PLUS : 
    return make_cow<arr::zts>(false, idx, apply<double, U, R, ztsdb::plus<double, U, R>>(d1, d2));
  case yy::parser::token::MINUS:                                                                      
    return make_cow<arr::zts>(false, idx, apply<double, U, R, ztsdb::minus<double, U, R>>(d1, d2));
  case yy::parser::token::MUL  :                                                                      
    return make_cow<arr::zts>(false, idx, apply<double, U, R, ztsdb::multiplies<double, U, R>>(d1, d2));
  case yy::parser::token::DIV  :                                                                      
    return make_cow<arr::zts>(false, idx, apply<double, U, R, ztsdb::divides<double, U, R>>(d1, d2));
  case yy::parser::token::MOD  :                                                                      
    return make_cow<arr::zts>(false, idx, apply<double, U, R, ztsdb::modulus<double, U, R>>(d1, d2));
  case yy::parser::token::POWER  :                                                                    
    return make_cow<arr::zts>(false, idx, apply<double, U, R, funcs::power<double>>(d1, d2));
  default: 
    return evalbinop_array_array_<double, U, R,
                                  yy::parser::token::LE,
                                  yy::parser::token::LT,
                                  yy::parser::token::EQ,
                                  yy::parser::token::NE,
                                  yy::parser::token::GE,
                                  yy::parser::token::GT
                                  >(d1, d2, op);
  }
}  


// -------------------------------------------------------------------------------

template <typename T, typename U>
static bool isInplace(const arr::cow_ptr<T>& t, const arr::cow_ptr<U>& u) {
  return t.getFlags() == arr::REF && (t->size() == u->size() || u->size() == 1);
}


static const tz::Zone& getTzone(const val::Value& val) {
  if (val.which() == val::vt_null) {
    throw std::range_error("time zone must be specified");
  }
  const auto& tzstr = val::get_scalar<arr::zstring>(val); // throws if not scalar
  return tzones.find(tzstr);                              // throws if not found
}


/// This is the main switch function for binary operations on the Variant 'Value'.
val::Value funcs::evalbinop(val::Value v1, const val::Value& v2, int op, const val::Value& attrib) {
  static std::set<int> arithmetic{
    yy::parser::token::PLUS, 
    yy::parser::token::MINUS,
    yy::parser::token::MUL,
    yy::parser::token::DIV, 
    yy::parser::token::MOD, 
    yy::parser::token::POWER}; 

  static std::set<int> arithmetic_colon{
    yy::parser::token::PLUS, 
    yy::parser::token::MINUS,
    yy::parser::token::MUL,
    yy::parser::token::DIV, 
    yy::parser::token::MOD, 
    yy::parser::token::POWER
    // yy::parser::token::COLON
  }; 

  static std::set<int> plus_minus{
    yy::parser::token::PLUS, 
    yy::parser::token::MINUS};

  static std::set<int> plus_only{
    yy::parser::token::PLUS};

  static std::set<int> mul_div{
    yy::parser::token::MUL, 
    yy::parser::token::DIV};

  static std::set<int> mul_only{
    yy::parser::token::MUL};

  static std::set<int> boolean{
    yy::parser::token::LE,
    yy::parser::token::LT,
    yy::parser::token::EQ,
    yy::parser::token::NE,
    yy::parser::token::GE,
    yy::parser::token::GT,
    yy::parser::token::AND,
    yy::parser::token::AND2,
    yy::parser::token::OR,
    yy::parser::token::OR2};
 
  static std::set<int> or_and{
    yy::parser::token::AND,
    yy::parser::token::OR};

  static std::set<int> eq_ne{
    yy::parser::token::EQ,
    yy::parser::token::NE};

  static std::set<int> comp{
    yy::parser::token::LE,
    yy::parser::token::LT,
    yy::parser::token::EQ,
    yy::parser::token::NE,
    yy::parser::token::GE,
    yy::parser::token::GT};


  // take care of the case where the second arg is a list, otherwise
  // this code will be repeated in each case:
  if (v2.which() == val::vt_list) {
    const auto& t2 = get<val::SpVList>(v2);
    if (v1.which() == val::vt_list) {
      throw range_error("invalid type for binary operator " 
                        + types_to_string(v1.which(), v2.which(), op));
    }
    return evalbinop_array_scalar(t2->a, v1, op);
  }
  

  switch (v1.which()) {

  // VArrayD ----------------------- 
  case val::vt_double: {
    const auto& t1 = get<val::SpVAD>(v1);
    switch (v2.which()) {
    case val::vt_double: {
      const auto& t2 = get<val::SpVAD>(v2);
      // if it's a pure reference (not locked, not const) and if the
      // sizes are compatible for inplace assignment, modify in place:
      if (isInplace(t1, t2) && arithmetic.find(op) != arithmetic.end()) {
        auto& t1 = get<val::SpVAD>(v1);
        evalbinop_array_array_inplace_<double, double,
                                       yy::parser::token::PLUS,
                                       yy::parser::token::MINUS,
                                       yy::parser::token::MUL,
                                       yy::parser::token::DIV,
                                       yy::parser::token::MOD,
                                       yy::parser::token::POWER
                                       >(*t1, *t2, op);
        return v1;
      }
      // otherwise continue using a const t1:
      else if (arithmetic_colon.find(op) != arithmetic_colon.end()) {
        return evalbinop_array_array_<double, double, double,
                                      yy::parser::token::PLUS,
                                      yy::parser::token::MINUS,
                                      yy::parser::token::MUL,
                                      yy::parser::token::DIV,
                                      yy::parser::token::MOD,
                                      yy::parser::token::POWER
                                      // yy::parser::token::COLON
                                      >(*t1, *t2, op);
      }
      else if (boolean.find(op) != boolean.end()) {
        return evalbinop_array_array_<double, double, bool,
                                      yy::parser::token::LE,
                                      yy::parser::token::LT,
                                      yy::parser::token::EQ,
                                      yy::parser::token::NE,
                                      yy::parser::token::GE,
                                      yy::parser::token::GT,
                                      yy::parser::token::AND,
                                      yy::parser::token::AND2,
                                      yy::parser::token::OR,
                                      yy::parser::token::OR2
                                      >(*t1, *t2, op);
      }
      break;
    }
    case val::vt_zts: {
      const auto& t2 = get<val::SpZts>(v2);
      if (arithmetic.find(op) != arithmetic.end()) {
        return evalbinop_zts<double, double>(t2->getIndex(), *t1, t2->getArray(), op);
      }
      else if (boolean.find(op) != boolean.end()) {
        return evalbinop_array_array_<double, double, bool,
                                      yy::parser::token::LE,
                                      yy::parser::token::LT,
                                      yy::parser::token::EQ,
                                      yy::parser::token::NE,
                                      yy::parser::token::GE,
                                      yy::parser::token::GT,
                                      yy::parser::token::AND,
                                      yy::parser::token::AND2,
                                      yy::parser::token::OR,
                                      yy::parser::token::OR2>(*t1, t2->getArray(), op);
      }
      break;
    }
    case val::vt_duration: {
      const auto& t2 = get<val::SpVADUR>(v2);
      if (mul_only.find(op) != mul_only.end()) {
        return evalbinop_array_array_<double, Global::duration, Global::duration,
                                      yy::parser::token::MUL
                                      >(*t1, *t2, op);
      }
      break;
    }
    case val::vt_period: {
      const auto& t2 = get<val::SpVAPRD>(v2);
      if (mul_only.find(op) != mul_only.end()) {
        return evalbinop_array_array_<double, tz::period, tz::period,
                                      yy::parser::token::MUL // DIV? LLL
                                      >(*t1, *t2, op);
      }
      break; }
    }
    break;
  }
    // Zts ----------------------- 
  case val::vt_zts: {
    const auto& t1 = get<val::SpZts>(v1);
    switch (v2.which()) {
    case val::vt_double: {
      const auto& t2 = get<val::SpVAD>(v2);
      if (isInplace(t1, t2) && arithmetic.find(op) != arithmetic.end()) {
        auto& t1 = get<val::SpZts>(v1);
        evalbinop_array_array_inplace_<double, double,
                                       yy::parser::token::PLUS,
                                       yy::parser::token::MINUS,
                                       yy::parser::token::MUL,
                                       yy::parser::token::DIV,
                                       yy::parser::token::MOD,
                                       yy::parser::token::POWER
                                       >(*t1->getArrayPtr(), *t2, op);
        return v1;
      }
      else if (arithmetic.find(op) != arithmetic.end()) {
        return evalbinop_zts<double, double>(t1->getIndex(), t1->getArray(), *t2, op);
      }
      else if (boolean.find(op) != boolean.end()) {
        return evalbinop_array_array_<double, double, bool,
                                      yy::parser::token::LE,
                                      yy::parser::token::LT,
                                      yy::parser::token::EQ,
                                      yy::parser::token::NE,
                                      yy::parser::token::GE,
                                      yy::parser::token::GT,
                                      yy::parser::token::AND,
                                      yy::parser::token::AND2,
                                      yy::parser::token::OR,
                                      yy::parser::token::OR2>(t1->getArray(), *t2, op);
      }
      break;
    }
    case val::vt_zts: {
      const auto& t2 = get<val::SpZts>(v2);
      if (t1->getIndex().getcol(0) != t2->getIndex().getcol(0)) {
        throw range_error("time indices of times series are not identical");
      }
      if (isInplace(t1, t2) && arithmetic.find(op) != arithmetic.end()) {
        auto& t1 = get<val::SpZts>(v1);
        evalbinop_array_array_inplace_<double, double,
                                       yy::parser::token::PLUS,
                                       yy::parser::token::MINUS,
                                       yy::parser::token::MUL,
                                       yy::parser::token::DIV,
                                       yy::parser::token::MOD,
                                       yy::parser::token::POWER
                                       >(*t1->getArrayPtr(), t2->getArray(), op);
        return v1;
      }
      else if (arithmetic.find(op) != arithmetic.end()) {
        return evalbinop_zts<double, double>(t1->getIndex(), t1->getArray(), t2->getArray(), op);
      }
      else if (boolean.find(op) != boolean.end()) {
        return evalbinop_array_array_<double, double, bool,
                                      yy::parser::token::LE,
                                      yy::parser::token::LT,
                                      yy::parser::token::EQ,
                                      yy::parser::token::NE,
                                      yy::parser::token::GE,
                                      yy::parser::token::GT,
                                      yy::parser::token::AND,
                                      yy::parser::token::AND2,
                                      yy::parser::token::OR,
                                      yy::parser::token::OR2>(t1->getArray(), t2->getArray(), op);
      }
      break; }
    }
    break;
  }


  // VArrayDT ----------------------- 
  case val::vt_time: {
    const auto& t1 = get<val::SpVADT>(v1);
    switch (v2.which()) {
    case val::vt_time: {
      const auto& t2 = get<val::SpVADT>(v2);
      if (comp.find(op) != comp.end()) {
        return evalbinop_array_array_<Global::dtime, Global::dtime, bool,
                                      yy::parser::token::LE,
                                      yy::parser::token::LT,
                                      yy::parser::token::EQ,
                                      yy::parser::token::NE,
                                      yy::parser::token::GE,
                                      yy::parser::token::GT
                                      >(*t1, *t2, op);
      }
      break;
    }
    case val::vt_duration: {
      const auto& t2 = get<val::SpVADUR>(v2);
      if (isInplace(t1, t2) && plus_minus.find(op) != plus_minus.end()) {
        auto& t1 = get<val::SpVADT>(v1);
        evalbinop_array_array_inplace_<Global::dtime, Global::duration,
                                       yy::parser::token::PLUS,
                                       yy::parser::token::MINUS
                                       >(*t1, *t2, op);
        return v1;
      }
      else if (plus_minus.find(op) != plus_minus.end()) {
        return evalbinop_array_array_<Global::dtime, Global::duration, Global::dtime,
                                      yy::parser::token::PLUS,
                                      yy::parser::token::MINUS
                                      >(*t1, *t2, op);
      }
      break;
    }
    case val::vt_period: {
      const auto& t2 = get<val::SpVAPRD>(v2);
      if (isInplace(t1, t2) && plus_minus.find(op) != plus_minus.end()) {
        auto& t1 = get<val::SpVADT>(v1);
        evalbinop_array_array_inplace_prd<Global::dtime, tz::period,
                                          yy::parser::token::PLUS,
                                          yy::parser::token::MINUS
                                          >(*t1, *t2, op, getTzone(attrib));
        return v1;
      }
      else if (plus_minus.find(op) != plus_minus.end()) {
        return evalbinop_array_array_prd<Global::dtime, tz::period, Global::dtime,
                                         yy::parser::token::PLUS,
                                         yy::parser::token::MINUS
                                         >(*t1, *t2, op, getTzone(attrib));
      }
      break; }
    }
    break;
  }

  // VArrayDUR ----------------------- 
  case val::vt_duration: {
    const auto& t1 = get<val::SpVADUR>(v1);
    switch (v2.which()) {
    case val::vt_duration: {
      const auto& t2 = get<val::SpVADUR>(v2);
      if (isInplace(t1, t2) && plus_minus.find(op) != plus_minus.end()) {
        auto& t1 = get<val::SpVADUR>(v1);
        evalbinop_array_array_inplace_<Global::duration, Global::duration,
                                       yy::parser::token::PLUS,
                                       yy::parser::token::MINUS
                                       >(*t1, *t2, op);
        return v1;
      }
      else if (arithmetic.find(op) != arithmetic.end())
        return evalbinop_array_array_<Global::duration, Global::duration, Global::duration,
                                      yy::parser::token::PLUS,
                                      yy::parser::token::MINUS
                                      >(*t1, *t2, op);
      else if (comp.find(op) != comp.end()) {
        return evalbinop_array_array_<Global::duration, Global::duration, bool,
                                      yy::parser::token::LE,
                                      yy::parser::token::LT,
                                      yy::parser::token::EQ,
                                      yy::parser::token::NE,
                                      yy::parser::token::GE,
                                      yy::parser::token::GT
                                      >(*t1, *t2, op);
      }
      break;
    }
    case val::vt_time: {
      const auto& t2 = get<val::SpVADT>(v2);
      if (plus_only.find(op) != plus_only.end()) {
        return evalbinop_array_array_<Global::duration, Global::dtime, Global::dtime,
                                      yy::parser::token::PLUS
                                      >(*t1, *t2, op);
      }
      break;
    }
    case val::vt_interval: {
      const auto& t2 = get<val::SpVAIVL>(v2);
      if (plus_only.find(op) != plus_only.end()) {
        return evalbinop_array_array_<Global::duration, tz::interval, tz::interval,
                                      yy::parser::token::PLUS
                                      >(*t1, *t2, op);
      }
      break;
    }
    case val::vt_double: {
      const auto& t2 = get<val::SpVAD>(v2);
      if (isInplace(t1, t2) && mul_div.find(op) != mul_div.end()) {
        auto& t1 = get<val::SpVADUR>(v1);
        evalbinop_array_array_inplace_<Global::duration, double,
                                       yy::parser::token::MUL,
                                       yy::parser::token::DIV
                                       >(*t1, *t2, op);
        return v1;
      }
      else if (mul_div.find(op) != mul_div.end()) {
        return evalbinop_array_array_<Global::duration, double, Global::duration,
                                      yy::parser::token::MUL,
                                      yy::parser::token::DIV
                                      >(*t1, *t2, op);
      }
      break; }
    }
    break;
  }

  // VArrayIVL ----------------------- 
  case val::vt_interval: {
    const auto& t1 = get<val::SpVAIVL>(v1);
    switch (v2.which()) {
    case val::vt_interval: {
      const auto& t2 = get<val::SpVAIVL>(v2);
      if (comp.find(op) != comp.end()) {
        return evalbinop_array_array_<tz::interval, tz::interval, bool,
                                      yy::parser::token::LE,
                                      yy::parser::token::LT,
                                      yy::parser::token::EQ,
                                      yy::parser::token::NE,
                                      yy::parser::token::GE,
                                      yy::parser::token::GT
                                      >(*t1, *t2, op);
      }
      break;
    }
    case val::vt_duration: {
      const auto& t2 = get<val::SpVADUR>(v2);
      if (isInplace(t1, t2) && plus_minus.find(op) != plus_minus.end()) {
        auto& t1 = get<val::SpVAIVL>(v1);
        evalbinop_array_array_inplace_<tz::interval, Global::duration,
                                       yy::parser::token::PLUS,
                                       yy::parser::token::MINUS
                                       >(*t1, *t2, op);
        return v1;
      }
      else if (plus_minus.find(op) != plus_minus.end()) {
        return evalbinop_array_array_<tz::interval, Global::duration, tz::interval,
                                      yy::parser::token::PLUS,
                                      yy::parser::token::MINUS
                                      >(*t1, *t2, op);
      }
      break;
    }    
    case val::vt_period: {
      const auto& t2 = get<val::SpVAPRD>(v2);
      if (isInplace(t1, t2) && plus_minus.find(op) != plus_minus.end()) {
        auto& t1 = get<val::SpVAIVL>(v1);
        evalbinop_array_array_inplace_prd<tz::interval, tz::period,
                                          yy::parser::token::PLUS,
                                          yy::parser::token::MINUS
                                          >(*t1, *t2, op, getTzone(attrib));
        return v1;
      }
      else if (plus_minus.find(op) != plus_minus.end()) {
        return evalbinop_array_array_prd<tz::interval, tz::period, tz::interval,
                                         yy::parser::token::PLUS,
                                         yy::parser::token::MINUS
                                         >(*t1, *t2, op, getTzone(attrib));
      }
      break; }
    }
    break;
  }

  // VArrayPRD ----------------------- 
  case val::vt_period: {
    const auto& t1 = get<val::SpVAPRD>(v1);
    switch (v2.which()) {
    case val::vt_period: {
      const auto& t2 = get<val::SpVAPRD>(v2);
      if (isInplace(t1, t2) && plus_minus.find(op) != plus_minus.end()) {
        auto& t1 = get<val::SpVAPRD>(v1);
        evalbinop_array_array_inplace_<tz::period, tz::period,
                                       yy::parser::token::PLUS,
                                       yy::parser::token::MINUS
                                       >(*t1, *t2, op);
        return v1;
      }
      else if (plus_minus.find(op) != plus_minus.end()) {
        return evalbinop_array_array_<tz::period, tz::period, tz::period,
                                      yy::parser::token::PLUS,
                                      yy::parser::token::MINUS
                                      >(*t1, *t2, op);
      }
      else if (eq_ne.find(op) != eq_ne.end()) {
        return evalbinop_array_array_<tz::period, tz::period, bool,
                                      yy::parser::token::EQ,
                                      yy::parser::token::NE
                                      >(*t1, *t2, op);
      }
      break;
    }
    case val::vt_time: {
      const auto& t2 = get<val::SpVADT>(v2);
      // not inplace
      if (plus_only.find(op) != plus_only.end()) {
        return evalbinop_array_array_prd<tz::period, Global::dtime, Global::dtime,
                                         yy::parser::token::PLUS
                                         >(*t1, *t2, op, getTzone(attrib));
      }
      break;
    }
    case val::vt_interval: {
      const auto& t2 = get<val::SpVAIVL>(v2);
      // not inplace
      if (plus_only.find(op) != plus_only.end()) {
        return evalbinop_array_array_prd<tz::period, tz::interval, tz::interval,
                                         yy::parser::token::PLUS
                                         >(*t1, *t2, op, getTzone(attrib));
      }
      break;
    }
    case val::vt_double: {
      const auto& t2 = get<val::SpVAD>(v2);
      if (mul_div.find(op) != mul_div.end()) {
        return evalbinop_array_array_<tz::period, double, tz::period,
                                      yy::parser::token::MUL,
                                      yy::parser::token::DIV
                                      >(*t1, *t2, op);
      }
      break; }
    }
    break;
  }

  // VArrayB ----------------------- 
  case val::vt_bool: {
    const auto& t1 = get<val::SpVAB>(v1);
    switch (v2.which()) {
    case val::vt_bool: {
      const auto& t2 = get<val::SpVAB>(v2);
      if (isInplace(t1, t2) && or_and.find(op) != or_and.end()) {
        auto& t1 = get<val::SpVAB>(v1);
        evalbinop_array_array_inplace_<bool, bool,
                                       yy::parser::token::AND,
                                       yy::parser::token::OR
                                       >(*t1, *t2, op);
        return v1;
      }
      else if (boolean.find(op) != boolean.end()) {
        return evalbinop_array_array_<bool, bool, bool,
                                      yy::parser::token::LE,
                                      yy::parser::token::LT,
                                      yy::parser::token::EQ,
                                      yy::parser::token::NE,
                                      yy::parser::token::GE,
                                      yy::parser::token::GT,
                                      yy::parser::token::AND,
                                      yy::parser::token::AND2,
                                      yy::parser::token::OR,
                                      yy::parser::token::OR2
                                      >(*t1, *t2, op);
      }
      break; }
    }
    break;
  }

  // VArrayS ----------------------- 
  case val::vt_string: {
    const auto& t1 = get<val::SpVAS>(v1);
    switch (v2.which()) {
    case val::vt_string: {
      const auto& t2 = get<val::SpVAS>(v2);
      if (comp.find(op) != comp.end()) {
        return evalbinop_array_array_<arr::zstring, arr::zstring, bool,
                                      yy::parser::token::LE,
                                      yy::parser::token::LT,
                                      yy::parser::token::EQ,
                                      yy::parser::token::NE,
                                      yy::parser::token::GE,
                                      yy::parser::token::GT
                                      >(*t1, *t2, op);
      }
      break; }
    case val::vt_error: {
      const auto& t2 = get<val::VError>(v2);
      const arr::Array<arr::zstring> t2_a({1}, arr::Vector<arr::zstring>{t2.what});
      return evalbinop_array_array_<arr::zstring, arr::zstring, bool,
                                    yy::parser::token::LE,
                                    yy::parser::token::LT,
                                    yy::parser::token::EQ,
                                    yy::parser::token::NE,
                                    yy::parser::token::GE,
                                    yy::parser::token::GT
                                    >(*t1, t2_a, op);
    }
    }
    break;
  }

  // VList ----------------------- 
  case val::vt_list: {
    const auto& t1 = get<val::SpVList>(v1);
    return evalbinop_array_scalar(t1->a, v2, op);
  }

  // VError
  case val::vt_error: {
    const auto& t1 = get<val::VError>(v1);
    const arr::Array<arr::zstring> t1_a(arr::Vector<idx_type>{1}, arr::Vector<arr::zstring>{t1.what});
    switch (v2.which()) {
    case val::vt_string: {
      const auto& t2 = get<val::SpVAS>(v2);
      return evalbinop_array_array_<arr::zstring, arr::zstring, bool,
                                    yy::parser::token::LE,
                                    yy::parser::token::LT,
                                    yy::parser::token::EQ,
                                    yy::parser::token::NE,
                                    yy::parser::token::GE,
                                    yy::parser::token::GT
                                    >(t1_a, *t2, op);
    }    
    break;
    }
  }
  }

  // in all other case, make the comp with the universal value/value
  // function:
  return evalbinop_value_value_comp(v1, v2, op);
}
