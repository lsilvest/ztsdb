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


#ifndef COW_PTR_HPP
#define COW_PTR_HPP


#include <memory>

namespace arr {

  const unsigned NOFLAGS  = 0x00;
  const unsigned REF      = 0x01;
  const unsigned LOCKED   = 0x02;
  const unsigned CONSTREF = 0x04;
  const unsigned TMP      = 0x08;
  const unsigned LAST     = 0x10;

  /// This is a typical copy on write pointer, with the addition that
  /// if the nevercopy flag is set, it acts as a regular shared_ptr
  /// and no copy is ever made. This allows the implementation of the
  /// pass by reference and pass by value semantics of the
  /// interpreter. In the end, as this class is just a wrapper for a
  /// shared_ptr, all the usual shared pointer semantics apply and
  /// memory will be freed as with a shared_ptr.
  template<typename T>
  struct cow_ptr {

    cow_ptr(unsigned flags_p, T* t) 
      : flags(flags_p), p(t), count(std::make_shared<long>(1)) { }
    cow_ptr(unsigned flags_p, std::shared_ptr<T> tp) 
      : flags(flags_p),  p(tp), count(std::make_shared<long>(1)) { }

    // LLL: to be complete we need a constructor that can construct
    // from a class derived from T

    // const dereferencing
    const T* operator->() const {
      return p.get();
    }
    const T& operator*() const {
      return *p.get();
    }
  
    // non-const dereferencing
    T* operator->() {
      if (isConst()) {
        throw std::range_error("cannot modify const object");        
      }
      //std::cout << "->deref: use_count(): " << use_count() << std::endl;
      if (*count > 1) {
        if (flags & LOCKED) {
          throw std::range_error("cannot copy locked object");
        } 
        //std::cout << "->deref: making copy" << std::endl;
        p = std::make_shared<T>(*p); // call T's copy constructor
        --*count;
        count = std::make_shared<long>(1);
      }
      return p.get();
    }
    T& operator*() {
      //std::cout << "*deref: use_count(): " << use_count() << std::endl;
      if (isConst()) {
        throw std::range_error("cannot modify const object");        
      }
      if (*count > 1) {
        if (flags & LOCKED) {
          throw std::range_error("cannot copy locked object");
        } 
        //std::cout << "*deref: making copy" << std::endl;
        p = std::make_shared<T>(*p); // call T's copy constructor
        --*count;
        count = std::make_shared<long>(1);
      }
      return *p.get();
    }

    /// Get a regular pointer to the object without increasing the
    /// count of p.
    T* get() { return p.get(); }
    /// Get a regular const pointer to the object without increasing
    /// the count of p.
    const T* get() const { return p.get(); }

    cow_ptr& operator=(const cow_ptr<T>& o) {
      // should not allow copying of CONST ? and LAST? LLL
      p = o.p;
      flags = o.flags & ~LAST;
      count = o.count;
      if (!(flags & CONSTREF))
        ++*count;
      return *this;
    }

    cow_ptr(const cow_ptr& o)
      : flags(o.flags & ~LAST), p(o.p), count(o.count) { ++*count; }

    bool isRef()       const { return flags & REF; }
    bool isLocked()    const { return flags & LOCKED; }
    bool isConst()     const { return flags & CONSTREF; }
    bool isTmp()       const { return flags & TMP; }
    bool isLast()      const { return flags & LAST; }

    void setTmp()   { flags |=  TMP; --*count; }
    void setRef()   { flags |=  REF; }
    void setLock()  { flags |=  LOCKED; }
    void setConst() { flags |=  CONSTREF; }
    void setLast()  {
      flags |=  LAST; --*count;
    }

    void resetTmp()  { flags &= ~TMP; }
    void resetLock() { flags &= ~LOCKED; }
    void resetRef()  { flags &= ~REF; }

    unsigned getFlags() const { return flags; }

    long use_count() const { return *count; }

    ~cow_ptr() {
      if (!isLast() && !isTmp()) {
        --*count;
      }
    }

  private:
    unsigned flags;
    std::shared_ptr<T> p;
    std::shared_ptr<long> count; 
  };


  template <typename T, typename... Args>
  inline cow_ptr<T> make_cow(unsigned flags, Args&&... args)
  {
    return cow_ptr<T>(flags, new T(std::forward<Args>(args)...));
  }

}


#endif
