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


#ifndef VALUEVAR_HPP
#define VALUEVAR_HPP

#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <functional>
#include <memory>
#include <cassert>
#include <netinet/in.h>
#include <sys/timerfd.h>
#include "juice/variant.hpp"
#include "juice/variant_binary.hpp"
#include "ast.hpp"
#include "string.hpp"
#include "array.hpp"
#include "zts.hpp"
#include "globals.hpp"
#include "cow_ptr.hpp"
#include "timezone/interval.hpp"
#include "type_utils.hpp"
#include "main_parser/location.hpp"
#include "period.hpp"


using namespace Juice;
using namespace arr;
using namespace std;
namespace zcore {
  struct InterpCtx;
}


// forward declarations:
namespace interp {
  struct BaseFrame;               // to avoid dependency with env.hpp
  struct ArgFrame;                // to avoid dependency with env.hpp
  struct BuiltinFrame;            // to avoid dependency with env.hpp
  struct ClosureFrame;            // to avoid dependency with env.hpp
}

/// Contains the types that constitute the output of an evaluation by the interpreter.
namespace val {

  struct VList;
  struct VBuiltinG;
  struct VNamed;
  struct VClos;
  struct VCode;
  struct VFuture;
  struct VTimer;
  struct VPtr;
  struct VConn;

  struct VNull { };

  typedef Vector<size_t>      VI;
  typedef std::shared_ptr<VI> SpVI;
  
  typedef Array<double>             VArrayD;
  typedef Array<zstring>            VArrayS;
  typedef Array<bool>               VArrayB;
  typedef Array<Global::dtime>      VArrayDT;
  typedef Array<tz::interval>       VArrayIVL;
  typedef Array<Global::duration>   VArrayDUR;
  typedef Array<tz::period>         VArrayPRD;
  typedef cow_ptr<VArrayD>      SpVAD;
  typedef cow_ptr<VArrayS>      SpVAS;
  typedef cow_ptr<VArrayB>      SpVAB;
  typedef cow_ptr<VArrayDT>     SpVADT;
  typedef cow_ptr<VArrayDUR>    SpVADUR;
  typedef cow_ptr<VArrayIVL>    SpVAIVL;
  typedef cow_ptr<VArrayPRD>    SpVAPRD;
  typedef cow_ptr<zts>          SpZts; // zts, time series, not an Array, defined in its own header
  typedef cow_ptr<VList>        SpVList;
  typedef shared_ptr<VFuture>   SpFuture;
  typedef shared_ptr<VTimer>    SpTimer;
  typedef shared_ptr<VBuiltinG> SpBuiltin;
  typedef shared_ptr<VConn>     SpConn;

  struct VConn {
    VConn(const string& ip_p, int port_p, Global::conn_id_t id_p);
    
    void setId(Global::conn_id_t id_p); 

    string ip;                  // set this const? LLL
    int port;                   // set this const? LLL
    Global::conn_id_t id;

    ~VConn() { }                // on deletion has to close the connection!!!
  };

  struct VTimer {
    VTimer(const uint64_t nanosecs_p, 
           shared_ptr<E> loop_p, 
           shared_ptr<E> once_p, 
           size_t loop_max_p);
    void start();
    void stop();
    ~VTimer();

    uint64_t nanosecs;
    shared_ptr<E> loop;
    shared_ptr<E> once;
    size_t loop_max;

    size_t loop_n;
    size_t loop_failed_n;

    int fd;
    bool done_first;
  };

  struct VError {
    zstring what;
  }; 

  // these are the values that will be given back by Value::which:
  enum ValType {
    vt_std_string,
    vt_std_int,

    vt_double,
    vt_bool,
    vt_time,
    vt_duration,
    vt_interval,
    vt_period,
    vt_string,
    vt_zts,
    vt_list,
    vt_null,
    vt_code,
    vt_clos,
    vt_builting,
    vt_future,
    vt_connection,
    vt_timer,
    vt_named, // used exclusively by encode to transmit name/value pairs
    vt_error, // used exclusively to transmit an error over TCP
    vt_ptr    // used exclusively by interpreter to keep original address
  };


  const map<int, string> vt_to_string = { 
    {0,  "encode_std_string"},
    {1,  "encode_std_int"},
    {2,  "double"},
    {3,  "logical"},
    {4,  "datetime"},
    {5,  "duration"},
    {6,  "interval"},
    {7,  "period"},
    {8,  "string"},
    {9,  "zts"},
    {10, "list"},
    {11, "NULL"},
    {12, "expression"},
    {13, "closure"},
    {14, "builtin"},
    {15, "future"},
    {16, "connection"},
    {17, "timer"},
    {18, "named"},
    {19, "error"},
    {20, "vptr"}
  };


  /// Mapping from ValType to type.
  template<val::ValType V> struct gettype { }; // fails when trying to get TP
  template<> struct gettype<vt_double>   { typedef SpVAD   TP; };
  template<> struct gettype<vt_bool>     { typedef SpVAB   TP; };
  template<> struct gettype<vt_time>     { typedef SpVADT  TP; };
  template<> struct gettype<vt_duration> { typedef SpVADUR TP; };
  template<> struct gettype<vt_interval> { typedef SpVAIVL TP; };
  template<> struct gettype<vt_period>   { typedef SpVAPRD TP; };
  template<> struct gettype<vt_string>   { typedef SpVAS   TP; }; 
  template<> struct gettype<vt_zts>      { typedef SpZts   TP; }; 
  template<> struct gettype<vt_list>     { typedef SpVList TP; }; 
  template<> struct gettype<vt_null>     { typedef VNull   TP; }; 
 
  template<val::ValType V> struct getelttype { }; // fails when trying to get TP
  template<> struct getelttype<vt_double>   { typedef double            TP; };
  template<> struct getelttype<vt_bool>     { typedef bool              TP; };
  template<> struct getelttype<vt_time>     { typedef Global::dtime     TP; };
  template<> struct getelttype<vt_duration> { typedef Global::duration  TP; };
  template<> struct getelttype<vt_interval> { typedef tz::interval      TP; };
  template<> struct getelttype<vt_period>   { typedef tz::period        TP; };
  template<> struct getelttype<vt_string>   { typedef arr::zstring      TP; }; 

  template<typename T> struct rmptr { }; // fails when trying to get TP
  template<> struct rmptr<SpVAD>   { typedef VArrayD    TP; };
  template<> struct rmptr<SpVAB>   { typedef VArrayB    TP; };
  template<> struct rmptr<SpVADT>  { typedef VArrayDT   TP; };
  template<> struct rmptr<SpVADUR> { typedef VArrayDUR  TP; };
  template<> struct rmptr<SpVAIVL> { typedef VArrayIVL  TP; };
  template<> struct rmptr<SpVAPRD> { typedef VArrayPRD  TP; };
  template<> struct rmptr<SpVAS>   { typedef VArrayS    TP; }; 
  template<> struct rmptr<SpZts>   { typedef zts        TP; };  
  template<> struct rmptr<SpVList> { typedef VList      TP; }; 

  typedef int64_t integer_t;

  typedef Variant<std::string
                  ,SpVI
                  ,SpVAD
                  ,SpVAB
                  ,SpVADT
                  ,SpVADUR
                  ,SpVAIVL
                  ,SpVAPRD
                  ,SpVAS
                  ,SpZts
                  ,SpVList
                  ,VNull
                  ,recursive_wrapper<VCode> // probably does not need wrapper LLL
                  ,std::shared_ptr<VClos>
                  ,SpBuiltin
                  ,SpFuture
                  ,VConn
                  ,SpTimer
                  ,recursive_wrapper<VNamed>
                  ,VError
                  ,recursive_wrapper<VPtr>
                  > Value;


  /// Code value. This type contains code (as a result).
  struct VCode {
    VCode(const E* e) : expr(e->clone()) { }
    VCode(shared_ptr<E> e) : expr(e) { }
    shared_ptr<E> expr;
  };


  struct VPtr {
    VPtr(Value& val) {
      if (val.which() == vt_ptr) {
        auto& vp = get<VPtr>(val);
        p = vp.p;
      }
      else {
        p = &val;
      }
    }
    Value* p;
  };


#ifdef ZTSDB_CLIENT
  struct VBuiltinG { 
    VBuiltinG() : _signature(0), signature(&_signature) { }
    int _signature;
    int* signature;    // so display will work
  };
#else 
  #include "valuevar_ic.hpp"
#endif
  

  /// Closure value. This type is the result of the evaluation of a
  /// function definition.
  struct VClos {                // call it VFun now it's not a closure LLL
    VClos(const Function* f_a /*, BaseFrame* r_a */);

    VClos(const VClos& v) { 
      //cout << "calling VClos copy constructor" << endl;
      f = v.f;
      argMap = v.argMap;
      ellipsisPos = v.ellipsisPos;
      // r = v.r;
    }

    VClos(VClos&& v) { 
      //cout << "calling VClos move constructor" << endl;
      swap(v);
    }

    VClos& operator=(const VClos& v) {
      //cout << "calling VClos copy assignment" << endl;
      f = v.f;
      argMap = v.argMap;
      ellipsisPos = v.ellipsisPos;
      // r = v.r;
      return *this;
    }

    VClos& operator=(VClos&& v) {
      //cout << "calling VClos move assignment" << endl;
      return swap(v);
    }

    VClos& swap(VClos& v) {
      f = v.f;
      v.f = nullptr;
      std::swap(argMap, v.argMap);
      std::swap(ellipsisPos, v.ellipsisPos);
      // std::swap(r, v.r);
      return *this;
    }
      
    std::shared_ptr<Function> f;
    map<string, int> argMap; 
    int ellipsisPos;

    // For the time being at least, no closures. It complicates memory
    // management very significantly and in the context of a database
    // manipulation language I don't see any big advantage:

    // BaseFrame* r;
  };

  
  // Note that a future is never sent over (a request is, but not a future!)
  struct VFuture {
    /// Constructs a generic future ready to be populated. Since it's
    /// difficult to know the exact Value location of a future in
    /// advance, a future is created in two steps. The memory position
    /// is created here and then the Value location will be updated
    /// with 'updateValuePtr'.
    VFuture() : val(nullptr) { } 

    void setvalptr(val::Value& val_p, const std::shared_ptr<interp::BaseFrame>& frame_p);

    Value* getvalptr();

    std::string to_string() const;

    //  private:
    Value* val;

    // it is important to not have shared_ptr here because else a
    // circular reference can be introduced when a future is created
    // in a nested frame:
    std::weak_ptr<interp::BaseFrame> frame;
  };


  struct VList {
    VList();
    VList(const Array<Value>& a_p);
    VList(const VList& l);

    inline void push_back(pair<string, Value> p) { a.concat(p.second, p.first); }
    inline size_t size() const { return a.size(); }

    /// These allow 'VList' to be used in a template the same way as 'Array<T>':
    inline VList subsetRows(idx_type from, idx_type to, bool addrownums=false) const {
      return VList(a.subsetRows(from, to, addrownums));
    }
    inline const Vector<idx_type>& getdim() const { return a.getdim(); }
    inline const idx_type getdim(idx_type d) const { return a.getdim(d); }
    inline Vector<zstring> getNamesVector(idx_type d) const { return a.getNamesVector(d); } 


    void at(arr::idx_type i, const val::Value& v);
    void at(arr::idx_type i, val::Value&& v);
    inline Value operator[](arr::idx_type i) const { 
      return a[i];
    }

    Array<Value> a;
  };


  struct VNamed {
    Value name;
    Value val;
  };


  // end of type definitions
  
  template<class UnaryPredicate>
  bool any_of(const val::Value& v, UnaryPredicate p) {
    if (v.which() == vt_list) {
      const auto& vl = get<val::SpVList>(v);
      for (size_t i=0; i<vl->size(); ++i) {
        auto res = any_of((vl->a)[i], p);
        if (res) {
          return true;
        }
      }
    }
    else {
      return p(v);
    }
    return false;
  }

  // ------------------------------------------------

  struct Typeof {
    typedef string result_type;
    string operator()(const SpVList&)                 const { return "list"; }
    string operator()(const VNull&)                   const { return "NULL"; }
    string operator()(const VCode&)                   const { return "expression"; }
    string operator()(const std::shared_ptr<VClos>&)  const { return "function"; }
    string operator()(const VConn&)                   const { return "connection"; }
    string operator()(const VTimer&)                  const { return "timer"; }
    string operator()(const VBuiltinG&)               const { return "builtin"; }
    string operator()(const SpVAD&)                   const { return "double"; }
    string operator()(const SpVAS&)                   const { return "character"; }
    string operator()(const SpVAB&)                   const { return "logical"; }
    string operator()(const SpVADT&)                  const { return "time"; }
    string operator()(const SpVADUR&)                 const { return "duration"; }
    string operator()(const SpVAIVL&)                 const { return "interval"; }
    string operator()(const SpVAPRD&)                 const { return "period"; }
    string operator()(const SpZts&)                   const { return "zts"; }

    string operator()(const VArrayD&)                 const { return "double"; }
    string operator()(const VArrayS&)                 const { return "character"; }
    string operator()(const VArrayB&)                 const { return "logical"; }
    string operator()(const VArrayDT&)                const { return "time"; }
    string operator()(const VArrayDUR&)               const { return "duration"; }
    string operator()(const VArrayIVL&)               const { return "interval"; }
    string operator()(const VArrayPRD&)               const { return "period"; }
    string operator()(const arr::zts&)                const { return "zts"; }
    string operator()(const VList&)                   const { return "list"; }
    string operator()(const VClos&)                   const { return "function"; }
    string operator()(const VError&)                  const { return "error"; }

    template <typename T>
    string operator()(const T& t) const { return "unknown"; }
  };

  struct SizeOf {
    typedef size_t result_type;
    size_t operator()(const VNull& x)                   const { return 0; }
    size_t operator()(const SpVList& x)                 const { return x->size(); }
    size_t operator()(const SpVAD& x)                   const { return x->size(); }
    size_t operator()(const SpVAS& x)                   const { return x->size(); }
    size_t operator()(const SpVAB& x)                   const { return x->size(); }
    size_t operator()(const SpVADT& x)                  const { return x->size(); }
    size_t operator()(const SpVADUR& x)                 const { return x->size(); }
    size_t operator()(const SpVAIVL& x)                 const { return x->size(); }
    size_t operator()(const SpVAPRD& x)                 const { return x->size(); }
    size_t operator()(const SpZts& x)                   const { return x->size(); }

    template <typename T>
    size_t operator()(const T&) const { return 1; }
  };

  inline bool operator==(const VNamed& p1, const VNamed& p2) { 
    return p1.name == p2.name && p1.val == p2.val; 
  }
  inline bool operator==(const VNull&   p1, const VNull&   p2) { return true; }
  inline bool operator==(const VFuture& p1, const VFuture& p2) { return false; }
  inline bool operator==(const SpVList& p1, const SpVList& p2) { return p1->a == p2->a; }
  inline bool operator==(const VError&  p1, const VError&  p2) { return p1.what == p2.what; }

  // for these it's preferable to throw "not implemented"
  inline bool operator==(const VCode&   p1, const VCode&   p2) { return false; }
  inline bool operator==(const VClos&   p1, const VClos&   p2) { return false; }
  inline bool operator==(const VConn&   p1, const VConn&   p2) { return false; }
  inline bool operator==(const VTimer&  p1, const VTimer&  p2) { return false; }
  inline bool operator==(const VBuiltinG&  p1, const VBuiltinG&  p2) { return false; }
  inline bool operator==(const VPtr&  p1, const VPtr&  p2)     { return false; }

  inline size_t size(const val::Value& v) { return apply_visitor(val::SizeOf(), v); }

  /// Build a one dimensional 'Array' with one element and no names. (put it in one location LLL)
  template<typename T>
  inline arr::cow_ptr<arr::Array<T>> make_array(const T& t) {
    return arr::make_cow<arr::Array<T>>(false, 
                                        arr::Vector<arr::idx_type>{1}, 
                                        arr::Vector<T>{t}, 
                                        vector<arr::Vector<arr::zstring>>());
  }
    
  template<typename T>
  inline T get_scalar(const Value& v) {
    const auto& a = get<arr::cow_ptr<arr::Array<T>>>(v);
    if (a->size() != 1) {
      throw std::out_of_range("expecting scalar value");
    }
    return (*a)[0];
  }

  void setTmp(Value& v);
  void resetTmp(Value& v);
  void setConst(Value& v);
  void setLast(Value& v);
  void setLock(Value& v);
  void resetLock(Value& v);
  void setRef(Value& v);
  void resetRef(Value& v);
  bool isLocked(const Value& v);
  bool isRef(const Value& v);
  bool isTmp(const Value& v);
  bool isConst(const Value& v);

} // end namespace val


namespace arr {
  template<>
  inline val::Value getInitValue() {
    return val::VNull();
  }

  TYPE_NB(val::Value, 7);
  TYPE_NAME(val::Value, "Value");

  template<>
  inline val::Value convert(const val::Value& u) {
    return u;
  }

  template<typename T>
  inline bool operator==(const cow_ptr<Array<T>>& a1, const cow_ptr<Array<T>>& a2) { 
    return *a1 == *a2; 
  }

  inline bool operator==(const val::SpZts& a1, const val::SpZts& a2) { return *a1 == *a2; }

  inline bool operator==(const val::SpVI& a1, const val::SpVI& a2) { return *a1 == *a2; }

  inline bool operator!=(const val::Value& v1, const val::Value& v2) { return !(v1 == v2); }

} // end namespace arr

// In the same spirit as the above definition, we include here the
// specialization of 'Vector<T>' for T = 'Value'. Indeed, 'Value' has
// members that allocate memory (e.g. through smart pointers), so
// memory freeing would not work properly without this specialization.
#include "valuevector.hpp"

#endif
