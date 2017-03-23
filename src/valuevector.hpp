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


#ifndef VALUE_VECTOR_HPP
#define VALUE_VECTOR_HPP


#include <cassert>
#include "valuevar.hpp"
#include "vector.hpp"
#include "index.hpp"


namespace arr { 

  template<typename O>
  struct Vector<val::Value, O> {
    typedef O comparator;
    typedef val::Value value_type;

    friend struct vector_iterator<val::Value, O>;
    friend struct vector_const_iterator<val::Value, O>;
    //    friend bool operator== <>(const Vector<val::Value>& v1, const Vector<val::Value>& v2);

    template <typename U, typename UO>
    friend void setv(Vector<U,UO>& v, size_t i, const U& t);
    template <typename U, typename UO>
    friend void setv_checkbefore(Vector<U,UO>& v, size_t i, const U& t);
    
    // constructors --------------------------------------------

    /// move constructor.
    Vector(Vector<val::Value>&& v) : c(std::move(v.c)) { }
    
    // copy constructor.
    Vector(const Vector<val::Value>& v, 
           std::unique_ptr<baseallocator>&& alloc_p = nullptr) : c(v.c) { }

    /// basic constructor with conditional initial value.
    Vector(size_t n=0,
           const val::Value& value=getInitValue<val::Value>(),
           std::unique_ptr<baseallocator>&& alloc_p = nullptr) : c(n, value) { }

    /// basic constructor leaving the vector allocated but unininitialized.
    Vector(rsv_t, size_t n, std::unique_ptr<baseallocator>&& alloc_p = nullptr) {
      c.reserve(n);
    }

    /// vector_iterator constructor.
    Vector(const vector_const_iterator<val::Value,O>& b, 
             const vector_const_iterator<val::Value,O>& e, 
             std::unique_ptr<baseallocator>&& alloc_p = nullptr) : c(b, e) { }

    /// std::vector iterator constructor.
    template <class InputIterator>
    Vector(const InputIterator& b, 
           const InputIterator& e, 
           std::unique_ptr<baseallocator>&& alloc_p = nullptr) : c(b, e) { }

    Vector(std::initializer_list<val::Value> l, 
           std::unique_ptr<baseallocator>&& alloc_p = nullptr) : c(l) { }

    /// constructor from a mmapallocator.
    Vector(std::unique_ptr<baseallocator>&& alloc_p) {
      assert(false);
    }

    void swap(Vector<val::Value>& o) {
      std::swap(c, o.c);
    }

    Vector& operator=(Vector<val::Value> other) {
      swap(other);
      return *this;
    }

    bool isOrdered() { return false; }

    void at(arr::idx_type i, const val::Value& v) {
      if (i > size() - 1) {
        throw std::out_of_range("subscript out of bounds");
      }
      c[i] = v;
    }
    val::Value& operator[](size_t i) {
      if (i > size() - 1) {
        throw std::out_of_range("subscript out of bounds");
      }
      return c[i];
    }
    const val::Value& operator[](size_t i) const {
      if (i > size() - 1) {
        throw std::out_of_range("subscript out of bounds");
      }
      return c[i];
    }

    void push_back(const val::Value& value) {
      c.push_back(value);
    }

    template <class InputIterator>
    vector_iterator<val::Value,O> insert(vector_iterator<val::Value,O> position, 
                                         InputIterator first, 
                                         InputIterator last) {
      auto diff = last - first;
      if (diff <= 0) {
        return position;
      }
      auto oldbegin = begin();
      auto oldend = end();
      resize(size() + static_cast<size_t>(diff));
      // move down everything that is at and beyond position: 
      if (oldbegin != oldend && oldend - 1 - position > 0) {
        for (auto iter = end(); iter != position; --iter) {
          *(iter) = *(iter - diff);
        }
      }
      // insert the elements
      for (auto iter = first; iter != last; ++iter) {
        *position = *iter;
        ++position;
      }
      return position;
    }

    vector_iterator<val::Value,O> erase(const vector_iterator<val::Value,O>& position) {
      c.erase(c.begin() + size_t(position));
      return position;
    }

    vector_iterator<val::Value,O> erase(const vector_iterator<val::Value,O>& first, 
                                      const vector_iterator<val::Value,O>& last) {
      c.erase(c.begin() + size_t(first), c.begin() + size_t(last));
      return first;
    }

    explicit operator std::vector<val::Value>() const {
      return c;
    }

    const val::Value& front() const {
      if (c.empty()) {
        throw std::range_error("front on empty Vector");
      }
      return c[0];
    }

    const val::Value& back() const {
      if (c.empty()) {
        throw std::range_error("back on empty Vector");     
      }
      return c[c.size()-1];    
    }

    size_t size() const { return c.size(); }
    void forceUnOrdered() { ; } // only here for template compatibility reasons

    Vector<val::Value>& assign(size_t count, const val::Value& value) {
      c.assign(count, value);
      return *this;
    }

    Vector<val::Value>& resize(size_t n) { 
      c.resize(n);
      return *this; 
    }

    Vector<val::Value>& resize(size_t n, const value_type& v) { 
      c.resize(n, v);
      return *this; 
    }
    
    bool operator==(const Vector<val::Value,O>& v1) const {
      return c == v1.c;
    }

    template <typename AO=O>
    Vector& sort() { return *this; } // only for template reasons

    template<typename U, typename AO=O>
    Vector<U> sort_idx(size_t base) { return Vector<U>(); } // only for template reasons

    void deallocate() { }

    vector_iterator<val::Value,O> begin() { return vector_iterator<val::Value,O>(*this, 0); } 
    vector_iterator<val::Value,O> end()   { return vector_iterator<val::Value,O>(*this, c.size()); }
    vector_const_iterator<val::Value,O> begin() const { 
      return vector_const_iterator<val::Value,O>(*this, 0); } 
    vector_const_iterator<val::Value,O> end()   const { 
      return vector_const_iterator<val::Value,O>(*this, c.size()); }
    vector_const_iterator<val::Value,O> cbegin() const { 
      return vector_const_iterator<val::Value,O>(*this, 0); } 
    vector_const_iterator<val::Value,O> cend()   const { 
      return vector_const_iterator<val::Value,O>(*this, c.size()); }

    ~Vector() { }

    private:
    val::Value& at(size_t i) { return c[i]; }
    vector<val::Value> c;
  };


  // we redefine these so as not to take into account ordering:
  template <>
  inline void setv(Vector<val::Value>& v, size_t i, const val::Value& t) {
    if (i >= v.size()) throw std::range_error("subscript out of bounds");        
    v.at(i, t);
  }
  template <>
  inline void setv_checkbefore(Vector<val::Value>& v, size_t i, const val::Value& t) {
    if (i >= v.size()) throw std::range_error("subscript out of bounds");        
    v.at(i, t);
  }

}



#endif
