// modified from:
//-----------------------------------------------------------------------------
// boost variant/detail/apply_visitor_binary.hpp header file
// See http://www.boost.org for updates, documentation, and revision history.
//-----------------------------------------------------------------------------
//
// Copyright (c) 2002-2003
// Eric Friedman
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef VARIANT_BINARY_HPP_INCLUDED
#define VARIANT_BINARY HPP_INCLUDED

#include "variant.hpp"

template <typename Visitor, typename Value1, typename... Args>
class apply_visitor_binary_invoke {
public: // visitor typedefs
  typedef typename Visitor::result_type result_type;
  
private: // representation
  Visitor& visitor_;
  Value1& value1_;

public: // structors
  apply_visitor_binary_invoke(Visitor& visitor, Value1& value1) :
    visitor_(visitor), value1_(value1)  { }

public: // visitor interfaces
  template <typename Value2>
  result_type operator()(Value2& value2, Args&&... args) {
    return visitor_(value1_, value2, std::forward<Args>(args)...);
  }

private:
    apply_visitor_binary_invoke& operator=(const apply_visitor_binary_invoke&);
};


template <typename Visitor, typename Visitable2, typename... Args>
class apply_visitor_binary_unwrap {
public: // visitor typedefs
  typedef typename Visitor::result_type result_type;

private: // representation
    Visitor& visitor_;
    Visitable2& visitable2_;

public: // structors
  apply_visitor_binary_unwrap(Visitor& visitor, Visitable2& visitable2) :
    visitor_(visitor), visitable2_(visitable2) { }

public: // visitor interfaces
  template <typename Value1>
  result_type operator()(Value1& value1, Args&&... args) {
    apply_visitor_binary_invoke<Visitor, Value1, Args...> invoker(visitor_, value1);
    
    return apply_visitor(invoker, visitable2_, std::forward<Args>(args)...);
  }

private:
  apply_visitor_binary_unwrap& operator=(const apply_visitor_binary_unwrap&);
};


template <typename Visitor, typename Visitable1, typename Visitable2, typename... Args>
inline typename Visitor::result_type
apply_visitor_binary2(Visitor& visitor, Visitable1& visitable1, Visitable2& visitable2, Args&&... args)
{
  apply_visitor_binary_unwrap<Visitor, Visitable2, Args...> unwrapper(visitor, visitable2);
  return apply_visitor(unwrapper, visitable1, std::forward<Args>(args)...);
}

//const version:
template <typename Visitor, typename Visitable1, typename Visitable2, typename... Args>
inline typename Visitor::result_type
apply_visitor_binary2(const Visitor& visitor, Visitable1& visitable1, Visitable2& visitable2, Args&&... args)
{
  apply_visitor_binary_unwrap<const Visitor, Visitable2> unwrapper(visitor, visitable2);
  return apply_visitor(unwrapper, visitable1, std::forward<Args>(args)...);
}

#endif

