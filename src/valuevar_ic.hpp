// (C) 2015-2017 Leonardo Silvestri
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

  /// Builtin function value. This type contains the necessary
  /// information to make a C++ call. A flag controls if the
  /// parameters are executed or if a VCode is returned instead for
  /// each parameter.

  struct VBuiltinG {

    typedef std::tuple<std::string, Value, yy::location> arg_t;
    
    struct ArgInfo {
      const set<val::ValType> typeset;
      const bool doEval;
    };

    static const unsigned MAXNAME = 128;
    // get R out of here if possible LLL
    VBuiltinG(interp::BaseFrame* r, /// environment in which this builtin is added
              string name, 
              string signature_s, 
              function<Value(vector<arg_t>&, zcore::InterpCtx&)> f_p,
              bool evalEllipsis_p=true,
              map<string, ArgInfo> argInfo_p = {});

    function<Value(vector<arg_t>&, zcore::InterpCtx& ic)> f;
    shared_ptr<Invoke>   invoke;
    shared_ptr<Function> signature;
    bool evalEllipsis;
    map<string, int> argMap;    // maps each argument name to a position
    map<string, ArgInfo> argInfo;
    int ellipsisPos;

    Value operator()(interp::BuiltinFrame& a, zcore::InterpCtx& ic) const;
    void checkArgs(const interp::BuiltinFrame& r) const;
  };

  inline Value& gval(Value& val) {
    if (val.which()!=vt_ptr) {
      return val;
    }
    else {
      auto& ptr = get<VPtr>(val);
      if (ptr.p == nullptr)
        throw std::out_of_range("gval: null ptr!");
      return *ptr.p;
    }
  }

  inline const Value& gval(const Value& val) {
    if (val.which()!=vt_ptr)
      return val;
    else {
      const auto& ptr = get<VPtr>(val);
      if (ptr.p == nullptr)
        throw std::out_of_range("gval: null ptr!");
      return *ptr.p;
    }
  }
  
  // access the tuples of 'val::VBuiltinG::arg_t' clearer.
  inline const std::string& getName(const VBuiltinG::arg_t& x) { return get<0>(x); }
  inline Value& getVal(VBuiltinG::arg_t& x) { return gval(get<1>(x)); } 
  inline const Value& getVal(const VBuiltinG::arg_t& x) { return gval(get<1>(x)); } 
  inline const yy::location& getLoc(const VBuiltinG::arg_t& x) { return get<2>(x); }


