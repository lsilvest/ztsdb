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


#ifndef ENV_HPP
#define ENV_HPP

#include <vector>
#include <map>
#include <string>
#include <iterator>
#include <memory>
#include <tuple>
#include "valuevar.hpp"
#include "display.hpp"


// #define ENV_HPP_DEBUG

using namespace Juice;

namespace interp {
  struct Kont;

  struct BaseFrame;
  typedef std::shared_ptr<BaseFrame> shpfrm;

  struct BaseFrame : std::enable_shared_from_this<BaseFrame> {

    typedef map<string,val::Value> map_type;
    typedef map<string,val::Value>::const_iterator const_map_iterator;


    //  BaseFrame(const string& name_p) : name(name_p), up(0), global(0), ec(nullptr), cc(nullptr) { }
    BaseFrame(const string& name_p,
              shpfrm g,           // pointer to global
              shpfrm u,           // pointer to upper frame
              shared_ptr<interp::Kont> bc_p = nullptr, 
              shared_ptr<interp::Kont> ec_p = shared_ptr<interp::Kont>(), 
              shared_ptr<interp::Kont> cc_p = nullptr) : 
      name(name_p), up(u), global(g), bc(bc_p), ec(ec_p), cc(cc_p), depth(u ? u->depth + 1 : 0) {
#ifdef ENV_HPP_DEBUG
      cout << name << " FRAME CREATED: " << this << endl; 
#endif

    }

    shared_ptr<interp::Kont> getec() { return ec; }
    void resetec() { ec = nullptr; }
    std::vector<BaseFrame*> getStack(std::vector<BaseFrame*> s = std::vector<BaseFrame*>()) {
      s.push_back(this);
      return up ? up->getStack(s) : s;
    }
    
  
    virtual val::Value  find(const string& s) const = 0; 
    virtual val::Value  findLocal(const string& s) const = 0; 
    virtual val::Value& findR(const string& s, bool funcall=false) = 0; 

    virtual val::SpVAS  getNames() = 0;

    virtual val::Value& add(string s, val::Value&& val) = 0;
    virtual val::Value& addSpecial(string s, val::Value&& val) = 0;
    virtual val::Value& addEllipsis(string s, val::Value&& val, const yy::location& loc) = 0;
    virtual val::Value& addArg(string s, val::Value&& val, const yy::location& loc) = 0;
    virtual bool remove(const string& symb) = 0;
    virtual bool removeSpecial(const string& symb) = 0;

    virtual shpfrm getTrueFrame() { return shared_from_this(); }

    virtual operator string() const = 0;
    virtual void clearTmp() = 0;
    virtual void clear() {
#ifdef ENV_HPP_DEBUG
      cout << "clearing: " << this << endl;
#endif
      m.clear();
      mtmp.clear();
      ec = bc = cc = nullptr;
    }
    virtual bool isFrame() { return false; }
    virtual bool isShadow() { return false; }
    virtual ~BaseFrame() {
      clear();
#ifdef ENV_HPP_DEBUG 
      cout << name << " FRAME DELETED: " << this << endl; 
#endif
    }

    unsigned getDepth() const { return depth; }
    
    string name;
    shpfrm up;
    shpfrm global;
    shared_ptr<interp::Kont> bc;  /// begin   continuation
    shared_ptr<interp::Kont> ec;  /// escape  continuation
    shared_ptr<interp::Kont> cc;  /// current continuation

  protected:
    map_type m;
    map_type mtmp;	// temporaries
    
    unsigned depth;     /// allows tracking of recursion
  }; 


  /// The default frame associated with an interpretation context.
  struct Frame : BaseFrame {
    Frame(const string& name_p,
          shpfrm g = nullptr, 
          shpfrm u = nullptr, 
          shared_ptr<interp::Kont> bc = nullptr, 
          shared_ptr<interp::Kont> ec = shared_ptr<interp::Kont>(), 
          shared_ptr<interp::Kont> cc = nullptr) : 
      BaseFrame(name_p, g, u, bc, ec, cc) 
    { }

    val::Value find(const string& s) const { 
      const map_type& ml = s[0] != '?' ? m : mtmp;
      auto elt = ml.find(s);
      if (elt != ml.end()) {
        return elt->second;
      } else {
        if (up) {
          return up->find(s);
        } else {
          throw std::out_of_range("object '" + s + "' not found");
        }
      }
    }
 
    val::Value findLocal(const string& s) const { 
      const map_type& ml = s[0] != '?' ? m : mtmp;
      auto elt = ml.find(s);
      if (elt != ml.end()) {
        return elt->second;
      }
      throw std::out_of_range("object '" + s + "' not found");
    }

    val::Value& findR(const string& s, bool funcall=false) { 
      map_type& ml = s[0] != '?' ? m : mtmp;
      auto elt = ml.find(s);
      if (elt != ml.end() &&
          (!funcall || ml[s].which() == val::vt_clos || ml[s].which() == val::vt_builting)) {
        return ml[s];
      } else {
        if (up) {
          return up->findR(s, funcall);
        } else {
          throw std::out_of_range("object '" + s + "' not found");
        }
      }
    }

    val::SpVAS getNames() {
      auto a = arr::make_cow<arr::Array<arr::zstring>>(false, arr::rsv, Vector<idx_type>{0});
      for (auto elt=m.begin(); elt != m.end(); ++elt) {
        a->concat(arr::zstring(elt->first));
      }
      return a;
    }

    val::Value& add(string s, val::Value&& val) { 
#ifdef ENV_HPP_DEBUG
      cout << "add " << s << " to " << this << endl;
#endif    
      map_type& ml = s[0] != '?' ? m : mtmp;
      auto elt = ml.find(s);
      if (elt != ml.end()) {
        ml.erase(elt);
      }
      auto res = ml.emplace(s, std::move(val));
      return res.first->second;
    }


    val::Value& addSpecial(string s, val::Value&& val) { 
#ifdef ENV_HPP_DEBUG
      cout << "add special " << s << " to " << this << endl;
#endif
      auto elt = m.find(s);
      if (elt != m.end()) {
        m.erase(elt);
        auto res = m.emplace(s, std::move(val));
        return res.first->second;
      }
      else if (name == "global") { // not found, so just add it if this is global
        auto res = m.emplace(s, std::move(val));
        return res.first->second;
      }
      else {
        return up->addSpecial(s, std::move(val));
      }
    }


    val::Value& addArg(string s, val::Value&& val, const yy::location& loc) { 
      return add(s, std::move(val));
    }
  
    val::Value& addEllipsis(string s, val::Value&& val, const yy::location& loc) {
      assert(false);
      throw std::range_error("'addEllipsis' no meaningful on base frame");
    }

    virtual bool remove(const string& symb) {
      if (!symb.size()) return false;

      map_type& ml = symb[0] != '?' ? m : mtmp;
      return ml.erase(symb) == 1;
    }

    virtual bool removeSpecial(const string& symb) {
      if (!symb.size()) return false;

      map_type& ml = symb[0] != '?' ? m : mtmp;
      if (ml.erase(symb) == 0) {
        return up ? up->removeSpecial(symb) : false;
      }
      else {
        return true;
      }
    }

    operator string() const { 
      stringstream ss;
      ss << "frame(" << this->up << "<-" << this << ')' << endl;
      for (auto p : m) {
        ss << p.first << ":" << val::to_string(p.second) << " ";
      }
      for (auto p : mtmp) {
        ss << p.first << ":" << val::to_string(p.second) << " ";
      }
      return ss.str();
    }

    void clearTmp() {
      //    cout << "clearing: " << string(*this) << endl;
      mtmp.clear();
    }

    virtual bool isFrame() { return true; }

    ~Frame() { }
  }; // Frame


  /// Type of frame used when invoking functions defined in R (see 'val::VClos'). 
  struct ClosureFrame : Frame {
    ClosureFrame(shpfrm u) : Frame("closure", u->global, u) { }

    val::Value& addEllipsis(string s, val::Value&& val, const yy::location& loc) {
      mv.emplace_back(make_tuple(s, std::move(val), loc));
      return get<1>(mv.back());
    }

    std::vector<std::tuple<std::string, val::Value, yy::location>> mv;
  };


  /// Type of frame used when invoking builtin functions (see 'val::BuiltinG').
  struct BuiltinFrame : Frame {
    static const size_t MAX_ARGS = 5096;
    typedef std::tuple<string, val::Value, yy::location> arg_t;

    /// Construct a buitin frame. After construction, the vector of arguments has a size of n
    BuiltinFrame(shpfrm u, shared_ptr<interp::Kont> ec, int nargs) :
      Frame("native", u->global, u, nullptr, ec, nullptr), 
      currentPos(0), mv(nargs)
    {
      mv.reserve(MAX_ARGS);
    }

    val::Value& addArg(string s, val::Value&& val, const yy::location& loc) {
      mv[currentPos] = make_tuple(s, std::move(val), loc);
      return get<1>(mv[currentPos++]);
    }

    val::Value& addEllipsis(string s, val::Value&& val, const yy::location& loc) {
      mv.emplace_back(make_tuple(s, std::move(val), loc));
      return get<1>(mv.back());
    }
  
    val::Value find(const string& s) const {
      // this is not very efficient, but it will work for now:
      auto res = std::find_if(mv.begin(), mv.end(), [s](const arg_t& x) { return get<0>(x) == s; });
      if (res != mv.end()) {
        return get<1>(*res);
      }
      else {
        return Frame::find(s);
      }
    }

    val::Value findLocal(const string& s) const {
      // this is not very efficient, but it will work for now:
      auto res = std::find_if(mv.begin(), mv.end(), [s](const arg_t& x) { return get<0>(x) == s; });
      if (res != mv.end()) {
        return get<1>(*res);
      }
      else {
        return Frame::findLocal(s);
      }
    }

    val::Value& findR(const string& s, bool funcall=false) {
      // this is not very efficient, but it will work for now:
      auto res = std::find_if(mv.begin(), mv.end(), [s](const arg_t& x) { return get<0>(x) == s; });
      const auto res_t = get<1>(*res).which();
      if (res != mv.end() && (!funcall || res_t == val::vt_clos || res_t == val::vt_builting)) {
        return get<1>(*res);
      }
      else {
        return Frame::findR(s, funcall);
      }
    }

    operator string() const { 
      stringstream ss;
      for (const auto& t : mv) { 
        const auto& v = get<1>(t);
        if (v.which() != 0) {   // test if the argument was ever initialized
          ss << get<0>(t) << ":" << val::to_string(v) << " ";
        }
      }
      return ss.str();
    }

    void clearTmp() {
      assert(false);
    }

    void clear() {
      mv.clear();
      BaseFrame::clear();
    }

    bool hasFutures() {
      return std::any_of(mv.begin(), mv.end(), 
                         [](const std::tuple<string, val::Value, yy::location>& x) { 
                           return get<1>(x).which() == val::vt_future; });
    }

    int currentPos;
    vector<std::tuple<string, val::Value, yy::location>> mv;
  };


  // for cleanliness we could decide to do the temp allocs here and the
  // global ones in the upper env... LLL
  struct ShadowFrame : Frame {
    ShadowFrame(shpfrm u, 
                shared_ptr<interp::Kont> bc, 
                shared_ptr<interp::Kont> ec,
                shared_ptr<interp::Kont> cc) : Frame("shadow", u->global, u, bc, ec, cc) { }

    val::Value find(const string& s) const { 
      if (s[0] == '?') {
        return Frame::find(s);
      }
      else {
        return up->find(s);
      }
    }

    val::Value findLocal(const string& s) const { 
      if (s[0] == '?') {
        return Frame::findLocal(s);
      }
      else {
        return up->findLocal(s);
      }
    }

    val::Value& findR(const string& s, bool funcall=false) { 
      if (s[0] == '?') {
        return Frame::findR(s, funcall);
      }
      else {
        return up->findR(s, funcall);
      }
    }

    virtual val::SpVAS getNames() {
      return up->getNames();
    }

    val::Value& add(string s, val::Value&& val) { 
      if (s[0] == '?') {
#ifdef ENV_HPP_DEBUG
        cout << "add " << s << " to " << this << endl;
#endif
        return Frame::add(s, std::move(val));
      }
      else {
        return up->add(s, std::move(val));
      }    
    }

    val::Value& addSpecial(string s, val::Value&& val) { 
      return up->addSpecial(s, std::move(val));
    }

    val::Value& addArg(string s, val::Value&& val, const yy::location& loc) { 
      return up->add(s, std::move(val));
    }
  
    val::Value& addEllipsis(string s, val::Value&& val, const yy::location& loc) {
      assert(false);
      throw std::out_of_range("'addEllipsis' irrelevant for shadow frame");
    }

    bool remove(const string& symb) {
      return up->remove(symb);
    }

    bool removeSpecial(const string& symb) {
      return up->removeSpecial(symb);
    }

    operator string() const {
      if (up) {
        return "shadow(" + (string(*up)) + ")";
      }
      else {
        return "shadow()";
      }
    }

    void clear() {
      BaseFrame::clear();
    }

    virtual shpfrm getTrueFrame() { return up->getTrueFrame(); }

    ~ShadowFrame() { }
  };

} // end namespace interp

#endif
