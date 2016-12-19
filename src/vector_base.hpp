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


#ifndef VECTOR_BASE_HPP
#define VECTOR_BASE_HPP


#include <memory>
#include <string>
#include <iterator>
#include <new>          // std::bad_alloc
#include <vector>
#include <iostream>
#include <ratio>
#include <type_traits>
#include "globals.hpp"
#include "string.hpp"
#include "timezone/interval.hpp"
#include "period.hpp"
#include "type_utils.hpp"
#include "allocator.hpp"


/// New 'Vector' class rather than 'std::vector' because:
///
/// 1. We need a representation that can easily and efficiently be
///    memory mapped
///
/// 2. We want to be able to create uninitialized vectors so that we
///    don't have to go through twice some potentially very large
///    vectors (a specific use-case is on receipt of fragmented arrays
///    over TCP).


namespace arr { 

  const size_t VECTOR_INITIAL_ALLOC = 1024;
  using VECTOR_ALLOC_GROWTH = std::ratio<4,3>;

  template<typename T, typename O> struct vector_iterator;
  template<typename T, typename O> struct vector_const_iterator;

  // to disambiguate constructors:
  struct rsv_t { };
  constexpr rsv_t rsv{};
  struct noinit_t { };
  constexpr noinit_t noinit_tag{};

  // this is the information that must be mmapped.
  template<typename T>
  struct RawVector {
    size_t typenumber;
    size_t n;
    size_t ordered;
    T v[0];
  };

  template<typename T>
  inline T getInitValue() {
    return 0;
  }

  inline size_t growCapacity(size_t n) {
    static_assert(VECTOR_ALLOC_GROWTH::num > VECTOR_ALLOC_GROWTH::den, 
                  "vector alloc growth multiplier must be > 1.0");
    return n < VECTOR_INITIAL_ALLOC/2 ? VECTOR_INITIAL_ALLOC : 
      n * VECTOR_ALLOC_GROWTH::num / VECTOR_ALLOC_GROWTH::den;
  }

  template<typename T, typename O=std::less<T>>
  struct Vector {
    typedef T value_type;
    typedef O comparator;
    typedef vector_iterator<T,O> iterator;
    typedef vector_const_iterator<T,O> const_iterator;

    friend struct vector_iterator<T,O>;
    friend struct vector_const_iterator<T,O>;
    template<typename U, typename UO> 
    friend bool operator==(const Vector<U,UO>& v1, const Vector<U,UO>& v2);

    template <typename U, typename UO>
    friend void setv(Vector<U,UO>& v, size_t i, const U& t);
    template <typename U, typename UO>
    friend void setv_checkbefore(Vector<U,UO>& v, size_t i, const U& t);
    template <typename U, typename UO>
    friend void setv_nocheck(Vector<U,UO>& v, size_t i, const U& t);
    
    // constructors --------------------------------------------

    /// move constructor.
    Vector(Vector<T,O>&& v) : alloc(std::move(v.alloc)), c(v.c), capacity(v.capacity) {
      // std::cout << "Vector move constructor" << std::endl;
      // LLL use swap and then c can never be null, check that alloc will actually swap LLL
      v.c = nullptr;
    }
    
    // copy constructor.
    Vector(const Vector<T,O>& v, 
           std::unique_ptr<baseallocator>&& alloc_p=std::make_unique<memallocator>()) 
      : alloc(std::move(alloc_p)), capacity(v.capacity) 
    {
      // std::cout << "Vector copy constructor" << std::endl;
      if (!alloc) {
        throw std::invalid_argument("Vector<T>: null allocator");
      }
      c = new (alloc->allocate(capacity*sizeof(T) + sizeof(RawVector<T>))) RawVector<T>;
      memcpy(c, v.c, sizeof(RawVector<T>));
      for (size_t j=0; j<v.c->n; ++j) {
        c->v[j] = v.c->v[j];
      }
    }

    /// basic constructor with initial value.
    Vector(size_t n=0,
           const T& value=getInitValue<T>(),
           std::unique_ptr<baseallocator>&& alloc_p=std::make_unique<memallocator>())
      : alloc(std::move(alloc_p))
    {
      if (!alloc) {
        throw std::invalid_argument("Vector<T>: null allocator");
      }
      capacity = growCapacity(n);
      c = new (alloc->allocate(capacity*sizeof(T) + sizeof(RawVector<T>))) RawVector<T>;
      c->typenumber = TypeNumber<T>::n;
      c->n = n;
      for (size_t i=0; i<n; ++i) {
        new (&c->v[i]) T(value);
      }
      c->ordered = n > 1 ? O()(value, value) : true;
    }

    /// basic constructor leaving the vector allocated but ininitialized.
    Vector(rsv_t, size_t n, 
           std::unique_ptr<baseallocator>&& alloc_p=std::make_unique<memallocator>()) 
      : alloc(std::move(alloc_p))
    {
      if (!alloc) {
        throw std::invalid_argument("Vector<T,O>: null allocator");
      }
      capacity = growCapacity(n);
      c = new (alloc->allocate(capacity*sizeof(T) + sizeof(RawVector<T>))) RawVector<T>;
      c->typenumber = TypeNumber<T>::n;
      c->n = 0;
      c->ordered = true;
    }

    /// basic constructor leaving the vector allocated with defined
    /// length, but elements uninitialized.
    Vector(noinit_t, size_t n, 
           std::unique_ptr<baseallocator>&& alloc_p=std::make_unique<memallocator>()) 
      : alloc(std::move(alloc_p))
    {
      if (!alloc) {
        throw std::invalid_argument("Vector<T,O>: null allocator");
      }
      capacity = growCapacity(n);
      c = new (alloc->allocate(capacity*sizeof(T) + sizeof(RawVector<T>))) RawVector<T>;
      c->typenumber = TypeNumber<T>::n;
      c->n = n;
      c->ordered = true;
    }

    /// std::vector iterator constructor.
    template <class InputIterator>
    Vector(const InputIterator& b, 
           const InputIterator& e, 
           std::unique_ptr<baseallocator>&& alloc_p=std::make_unique<memallocator>()) 
      : alloc(std::move(alloc_p))
    { 
      if (!alloc) {
        throw std::invalid_argument("Vector<T,O>: null allocator");
      }
      size_t n = e - b;
      capacity = growCapacity(n);
      c = new (alloc->allocate(capacity*sizeof(T) + sizeof(RawVector<T>))) RawVector<T>;    
      c->typenumber = TypeNumber<T>::n;
      c->n = n;
      size_t i = 0;
      c->ordered = true;
      if (n > 0) {
        c->v[i++] = *b;
        for (auto iter=b+1; iter!=e; ++iter) {
          c->v[i] = *iter;
          if (!O()(c->v[i-1], c->v[i])) {
            c->ordered = false;
          }
          ++i;
        }
      }     
    }

    Vector(std::initializer_list<T> l, 
           std::unique_ptr<baseallocator>&& alloc_p=std::make_unique<memallocator>()) 
      : alloc(std::move(alloc_p))
    {
      if (!alloc) {
        throw std::invalid_argument("Vector<T,O>: null allocator");
      }
      size_t n = l.size();
      capacity = growCapacity(n);
      c = new (alloc->allocate(capacity*sizeof(T) + sizeof(RawVector<T>))) RawVector<T>; 
      c->typenumber = TypeNumber<T>::n;
      c->n = n;
      size_t i = 0;
      c->ordered = true;
      if (n > 0) {
        c->v[i++] = *l.begin();
        for (auto iter=l.begin()+1; iter!=l.end(); ++iter) {
          c->v[i] = *iter;
          if (!O()(c->v[i-1], c->v[i])) c->ordered = false;
          ++i;
        }
      } 
    }

    /// constructor from a mmapallocator.
    Vector(std::unique_ptr<baseallocator>&& alloc_p) : alloc(std::move(alloc_p)) {
      if (!alloc) {
        throw std::invalid_argument("Vector<T,O>: null allocator");
      }
      c = static_cast<RawVector<T>*>(alloc->initialize()); // will throw!
      capacity = alloc->size();
    }

    void swap(Vector<T,O>& o) {
      std::swap(alloc, o.alloc);
      std::swap(c, o.c);
      std::swap(capacity, o.capacity);
    }

    Vector& operator=(Vector<T,O> other) {
      swap(other);
      return *this;
    }

    size_t getBufferSize() const {
      return sizeof(RawVector<T>) + size()*sizeof(T);
    }

    size_t to_buffer(char* buf) const {
      // header:
      memcpy(buf, c, sizeof(RawVector<T>));
      size_t offset = sizeof(RawVector<T>);
      // data:
      size_t copysz = size()*sizeof(T);
      memcpy(buf + offset, c->v, copysz);
      return offset + copysz; 
    }

    const T& operator[](size_t i) const { return c->v[i]; }

    void push_back(const T& value) {
      if (c->n + 1 > capacity) {
        if (!alloc) {
          throw std::range_error("vector::push_back: cannot reallocate with null allocator");
        }
        capacity = growCapacity(c->n);
        auto mem = alloc->reallocate(c, capacity*sizeof(T) + sizeof(RawVector<T>));
        c = static_cast<RawVector<T>*>(mem);
      }
      new (&c->v[c->n]) T(value);
      c->ordered = c->n > 0 ? c->ordered & O()(c->v[c->n-1], c->v[c->n]) : true;
      ++c->n;
    }


    template <class InputIterator>
    vector_iterator<T,O> insert(vector_iterator<T,O> position, 
                                const InputIterator first, 
                                const InputIterator last) {
      auto ordered = c->ordered; // remember this because the iterator
                                 // access lower will set c->ordered
                                 // to false no matter what
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
      *position = *first;
      if (position != begin()) {
        // if the insertion is not at the beginning we need to check the
        // next element for order:
        ordered &= O()(*(position-1), *position);      
      }
      ++position;
      for (auto iter = first+1; iter != last; ++iter) {
        *position = *iter;
        if (c->ordered && !O()(*(position-1), *position)) {
          ordered = false;
        }
        ++position;
      }
      if (position != end()) {           
        // if the insertion was not at the end we need to check the
        // next element for order:
        ordered &= O()(*(position-1), *position);
      }
      c->ordered = ordered;
      return position;
    }

    vector_iterator<T,O> erase(const vector_iterator<T,O>& position) {
      for (auto iter = position; iter != end()-1; ++iter) {
        *iter = *(iter + 1);
      }
      resize(c->n - 1);
      return position;  
    }

    vector_iterator<T,O> erase(const vector_iterator<T,O>& first, const vector_iterator<T,O>& last) {
      auto diff = last - first;
      for (auto iter = first; iter + diff != end(); ++iter) {
        *iter = *(iter + diff);
      }
      resize(c->n - diff);
      return first;  
    }

    explicit operator std::vector<T>() const {
      return std::vector<T>(&c->v[0], &c->v[c->n]);
    }

    const T& front() const {
      if (!c || !c->n) {
        throw std::range_error("front on empty Vector");
      }
      return c->v[0];
    }

    const T& back() const {
      if (!c || !c->n) {
        throw std::range_error("front on empty Vector");     
      }
      return c->v[c->n-1];    
    }

    size_t size() const { if (c) return c->n; else return 0; }
    bool isOrdered() const { if (c) return c->ordered; else return false; }
    void forceOrdered() { if (c) c->ordered = true; }
    void forceUnOrdered() { if (c) c->ordered = false; }
    void setOrdered(bool val) { if (c) c->ordered = val; }

    bool checkAndSetOrdered() {
      for (size_t j=1; j<size(); ++j) {
        if (!O()(c->v[j-1], c->v[j])) {
          c->ordered = false;
          return false;
        }
      }
      c->ordered = true;
      return true;
    }

    Vector<T,O>& init(size_t count, const T& value) {
      resize(count);
      for (size_t j=0; j<count; ++j) {
        c->v[j] = value;
      }
      c->ordered = count > 1 ? O()(value, value) : true; // a single element is always ordered
      return *this;
    }

    // the elements after current end will be uninitialized
    Vector<T,O>& resize(size_t n, size_t from=0) { 
      if (from > c->n) {
        throw std::out_of_range("resize from out of bounds");
      }
      if (from || n != c->n) {          // only resize if needed
        if (!alloc) {
          throw std::range_error("vector::resize: cannot reallocate with null allocator");
        }
        capacity = growCapacity(n);
        RawVector<T> rv = *c;
        c = new (alloc->reallocate((char*)c + from*sizeof(T), memsize(capacity))) RawVector<T>;
        memcpy(c, &rv, sizeof(RawVector<T>));
        c->n = n;
      }
      return *this; 
    }

    Vector<T,O>& resize(size_t n, size_t from, const T& v) {
      if (from >= c->n) {
        throw std::out_of_range("resize from out of bounds");
      }
      size_t start_fill = c->n - from;
      resize(n, from);
      for (size_t i = start_fill; i < n; ++i) {
        c->v[i] = v;
      }
      return *this; 
    }


    // The problem is this will not work for 'T' that are not "flat"
    // and we don't know how to check for that. It's a pretty
    // dangerous trap set up for unaware users. LLL
    size_t append(const char* buf, const size_t len) { 
      // first check we have enough bytes to construct a 'RawVector':
      if (len < sizeof(RawVector<T>)) {
        throw std::out_of_range("invalid append buffer: too short");
      }
      auto appendvec = reinterpret_cast<const RawVector<T>*>(buf);
      // now we know the true size of the vector, check buf len again:
      const size_t totalsz = sizeof(RawVector<T>) + appendvec->n * sizeof(T);
      if (len < totalsz) {
        throw std::out_of_range("missing data");
      }
      
      // check the typenumber:
      if (TypeNumber<T>::n != appendvec->typenumber) {
        throw std::out_of_range("incorrect type");
      }

      auto old_n = c->n;
      resize(c->n + appendvec->n); // will also redimension
      memcpy(&c->v[old_n], appendvec->v, appendvec->n*sizeof(T));
              
      if (appendvec->n) {
        c->ordered &= appendvec->ordered;
        if (old_n) {
          c->ordered &= O()(c->v[old_n-1], c->v[old_n]);
        }
      }
      return totalsz;
    }

    template <typename AO=O>
    Vector& sort() {
      if (std::is_same<AO, O>::value && c->ordered) return *this;

      std::sort(begin(), end(), AO()); 
      
      // if the sort function is the same as the one defined for
      // 'Vector', then sorting will result in a vector that is
      // ordered by 'O', so we set the ordered flag.
      if (std::is_same<AO, O>::value) {
        c->ordered = true;
      }

      return *this;
    }

    template<typename U, typename AO=O>
    Vector<U> sort_idx(size_t base=0) const {
      Vector<U> idx(rsv, this->size());
      for (size_t j=0; j<size(); ++j) idx.push_back(j+base);
      std::sort(idx.begin(), idx.end(), 
                [this, base](size_t a, size_t b) { 
                  return AO()((*this)[a-base], (*this)[b-base]); });
      return idx;
    }

    template<typename F, typename ...U>
    Vector& apply(const U&... u) {
      c->ordered = true;        // we're doing the whole vector, so
                                // forget about the current status and
                                // calculate it with
                                // 'setv_checkbefore'
      for (size_t i=0; i<size(); ++i) {
        setv_checkbefore(*this, i, F()((*this)[i], u[i]...));
      }
      return *this;
    }

    template<typename F, typename U>
    Vector& apply_scalar_post(const U& u) {
      c->ordered = true;
      for (size_t i=0; i<size(); ++i) {
        setv_checkbefore(*this, i, F()((*this)[i], u));
      }
      return *this;
    }

    void deallocate() {
      alloc->deallocate(c, c->n);
      c = nullptr;
      capacity = 0;
    }

    vector_iterator<T,O> begin() { return vector_iterator<T,O>(*this, 0); } 
    vector_iterator<T,O> end()   { return vector_iterator<T,O>(*this, c->n); }
    vector_const_iterator<T,O> begin() const { return vector_const_iterator<T,O>(*this, 0); } 
    vector_const_iterator<T,O> end()   const { return vector_const_iterator<T,O>(*this, c->n); }
    vector_const_iterator<T,O> cbegin() const { return vector_const_iterator<T,O>(*this, 0); } 
    vector_const_iterator<T,O> cend()   const { return vector_const_iterator<T,O>(*this, c->n); }

    ~Vector() { 
      // alloc will get destroyed automatically, which means freeing c
    }

    const RawVector<T>* getRawVectorPtr() const { return c; }

    /// buffer constructor
    Vector<T,O>(char* buf, size_t len) : alloc(nullptr)
    {
      capacity = len;
      // checks LLL
      c = new (buf) RawVector<T>;
    }

    T* c_ptr() { return c ? c->v : nullptr; }
    const T* c_ptr() const { return c ? c->v : nullptr; }
    const baseallocator* getAllocator() const { return alloc.get(); }
    
  private:
    std::unique_ptr<baseallocator> alloc;
    RawVector<T>* c;
    size_t capacity;

    T& at(size_t i) { return c->v[i]; }

    static inline size_t memsize(size_t n) { return n*sizeof(T) + sizeof(RawVector<T>); }
  };


  inline size_t getTypeNumber(const std::string& filename) {
    mmapallocator alloc(filename);
    size_t* tp = static_cast<size_t*>(alloc.initialize());
    return *tp;
  }


  /// Member equal.
  template<typename T, typename O>
  bool operator==(const Vector<T,O>& v1, const Vector<T,O>& v2) {
    if (v1.c->n != v2.c->n) return false;
    for (size_t i=0; i<v1.c->n; ++i) {
      if (!(v1.c->v[i] == v2.c->v[i])) {
        return false;
      }
    }
    return true;
  }


  /// Member equal. NaN correct, but will not work with strings for
  /// example.
  template<>
  inline bool operator==(const Vector<double>& v1, const Vector<double>& v2) { 
    if (v1.c->n != v2.c->n) return false;
    return memcmp(v1.c->v, v2.c->v, v1.c->n * sizeof(double)) == 0;
  }

  template<typename T, typename O>
  bool operator!=(const Vector<T,O>& v1, const Vector<T,O>& v2) {
    return !(v1 == v2);
  }

  template<typename T, typename O>
  struct vector_iterator { 
    typedef std::random_access_iterator_tag iterator_category;
    typedef vector_iterator<T,O> iterator;
    typedef std::ptrdiff_t difference_type;
    typedef size_t size_type;
    typedef T value_type;
    typedef T* pointer;
    typedef T& reference;

    vector_iterator(Vector<T,O>& v_p, size_t pos_p = 0) : v(v_p), pos(pos_p) { }
    vector_iterator(const vector_iterator& i) : v(i.v), pos(i.pos) { }
    ~vector_iterator() { }
    
    vector_iterator& operator=(const vector_iterator& i) { pos = i.pos; return *this; }
    bool operator==(const vector_iterator& i) const { return &v==&i.v && pos==i.pos; }
    bool operator!=(const vector_iterator& i) const { return &v!=&i.v || pos!=i.pos; }
    bool operator< (const vector_iterator& i) const { return pos <  i.pos; }
    bool operator<=(const vector_iterator& i) const { return pos <= i.pos; }
    bool operator> (const vector_iterator& i) const { return pos >  i.pos; }
    bool operator>=(const vector_iterator& i) const { return pos >= i.pos; }

    vector_iterator operator+(size_t n) const { 
      return vector_iterator<T,O>(v, pos + n);
    }
    vector_iterator& operator+=(size_t n) { 
      pos += n;
      return *this;
    }
    vector_iterator operator-(size_t n) const { 
      return vector_iterator<T,O>(v, pos - n);
    }

    difference_type operator-(const vector_iterator& i) const { 
      return pos - i.pos;
    }
    
    vector_iterator& operator++() { ++pos; return *this; }
    vector_iterator operator++(int)
    {
      return vector_iterator<T,O>(v, pos++);
    }
    vector_iterator& operator--() { --pos; return *this; }
    vector_iterator operator--(int)
    {
      return vector_iterator<T,O>(v, pos--);
    }

    reference operator*() const {
      v.c->ordered = false;
      return v.at(pos); 
    }
    pointer operator->() const { 
      v.c->ordered = false;
      return &v.at(pos); 
    }

    explicit operator size_t() const { 
      return pos; 
    }

  private:
    Vector<T,O>& v;
    size_t pos;
  };

  template<typename T, typename O>
  struct vector_const_iterator { 
    typedef std::random_access_iterator_tag iterator_category;
    typedef vector_const_iterator<T,O> iterator;
    typedef std::ptrdiff_t difference_type;
    typedef size_t size_type;
    typedef T value_type;
    typedef const T* pointer;
    typedef const T& reference;

    vector_const_iterator(const Vector<T,O>& v_p, size_t pos_p = 0) : v(v_p), pos(pos_p) { }
    vector_const_iterator(const vector_const_iterator& i) : v(i.v), pos(i.pos) { }
    ~vector_const_iterator() { }
    
    vector_const_iterator& operator=(const vector_const_iterator& i) { pos = i.pos; return *this; }
    bool operator==(const vector_const_iterator& i) const { return &v==&i.v && pos==i.pos; }
    bool operator!=(const vector_const_iterator& i) const { return &v!=&i.v || pos!=i.pos; }
    vector_const_iterator operator+(size_t n) const { 
      return vector_const_iterator<T,O>(v, pos + n);
    }
    vector_const_iterator& operator+=(size_t n) { 
      pos += n;
      return *this;
    }
    vector_const_iterator operator-(size_t n) const { 
      return vector_const_iterator<T,O>(v, pos - n);
    }
    difference_type operator-(const vector_const_iterator& i) const { 
      return pos - i.pos;
    }
    
    vector_const_iterator& operator++() { ++pos; return *this; }
    vector_const_iterator operator++(int)
    {
      return vector_const_iterator<T,O>(v, pos++);
    }
    vector_const_iterator& operator--() { --pos; return *this; }
    vector_const_iterator operator--(int)
    {
      return vector_const_iterator<T,O>(v, pos--);
    }

    reference operator*()  const { return v[pos]; }
    pointer   operator->() const { return &v[pos]; }

    size_t getpos() const { return pos; } // mostly for debug purposes

  private:
    const Vector<T,O>& v;
    size_t pos;
  };


  template <typename T, typename O>
  void setv(Vector<T,O>& v, size_t i, const T& t) {
    if (i >= v.size()) throw std::range_error("subscript out of bounds");
    if (v.isOrdered()) {
      if (i > 0)           v.c->ordered = O()(v[i-1], t);
      if (i < v.size()-1)  v.c->ordered = v.c->ordered && O()(t, v[i+1]);
    }
    v.c->v[i] = t;    
  }
  template <typename T, typename O>
  void setv_checkbefore(Vector<T,O>& v, size_t i, const T& t) {
    if (i >= v.size()) throw std::range_error("subscript out of bounds");
    v.c->v[i] = t;    
    if (v.isOrdered()) {
      if (i > 0)           v.c->ordered = O()(v.c->v[i-1], t);
    }
  }

  template <typename T, typename O>
  void setv_nocheck(Vector<T,O>& v, size_t i, const T& t) {
    v.c->v[i] = t;    
  }

} // end namespace arr


#endif
