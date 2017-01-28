// -*- compile-command: "make -k -j test" -*-

// Copyright (C) 2015 Leonardo Silvestri
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


#include <cmath>
#include <crpcut.hpp>
#include <algorithm>
#include "allocator.hpp"
#include "misc.hpp"
#include "vector_base.hpp"


using namespace arr;

namespace arr {
  TYPE_NB(double, 0);
  TYPE_NB(bool, 1);
  TYPE_NAME(double, "double");
  TYPE_NAME(bool, "logical");
}


// -- no memory mapping tests ---------------
// constructors:
TEST(vector_constructor_basic_bool) {
  const size_t sz = 10;
  Vector<bool> v(sz,                    // n
                 0,                     // init value
                 std::make_unique<memallocator>());   // allocator
  for (size_t i=0; i<sz; ++i) {
    setv(v, i, true);
  }
  bool all3 = true;
  for (size_t i=0; i<sz; ++i) {
    if (v[i] != true) {
      all3 = false;
      break;
    }
  }
  ASSERT_TRUE(all3);
}
TEST(vector_constructor_init) {
  const size_t sz = 10;
  Vector<bool> v(sz,                    // n
                 true,                  // init value
                 std::make_unique<memallocator>());   // allocator
  bool all3 = true;
  for (size_t i=0; i<sz; ++i) {
    if (v[i] != true) {
      all3 = false;
      break;
    }
  }
  ASSERT_TRUE(all3);
}
TEST(vector_constructor_vector_iter) {
  std::vector<bool> sv{0,1,1,1,0,0,0,1,1,1};
  Vector<bool> v(sv.begin(), 
                 sv.end(),
                 std::make_unique<memallocator>()); // allocator
  bool allok = true;
  for (size_t i=0; i<v.size(); ++i) {
    std::cout << "v[" << i << "]:" << v[i] << std::endl;
    if (v[i] != sv[i]) {
      allok = false;
      break;
    }
  }
  ASSERT_TRUE(allok);
}
TEST(vector_constructor_Vector_iter) {
  std::vector<bool> sv{0,0,0,0,0,1,1,1,1,1};
  const size_t sz = 10;
  Vector<bool> v1(sv.begin(), 
                  sv.end(),
                  std::make_unique<memallocator>()); // allocator
  Vector<bool> v2(v1.begin(), 
                  v1.end(),
                  std::make_unique<memallocator>()); // allocator
  bool allok = true;
  for (size_t i=0; i<sz; ++i) {
    if (v2[i] != v1[i]) {
      allok = false;
      break;
    }
  }
  ASSERT_TRUE(allok);
}
TEST(vector_constructor_init_list) {
  std::vector<bool> sv{0,0,1,1,0,0,1,1,0,0};
  Vector<bool> v{0,0,1,1,0,0,1,1,0,0};
  bool allok = true;
  for (size_t i=0; i<sv.size(); ++i) {
    if (v[i] != sv[i]) {
      allok = false;
      break;
    }
  }
  ASSERT_TRUE(allok);
}
TEST(vector_copy_constructor) {
  Vector<bool> v1{0,0,1,1,0,0,1,1,0,0};
  Vector<bool> v2(v1);
  bool allok = true;
  for (size_t i=0; i<v1.size(); ++i) {
    if (v1[i] != v2[i]) {
      allok = false;
      break;
    }
  }
  ASSERT_TRUE(allok);
}
TEST(vector_move_constructor) {
  std::vector<bool> sv{0,0,1,1,0,0,1,1,0,0};
  auto v2(Vector<bool>{0,0,1,1,0,0,1,1,0,0});
  bool allok = true;
  for (size_t i=0; i<sv.size(); ++i) {
    if (sv[i] != v2[i]) {
      allok = false;
      break;
    }
  }
  ASSERT_TRUE(allok);
}
TEST(vector_constructor_huge_fail) {
  const size_t sz = 1000L * 1000000000L;
  ASSERT_THROW(Vector<bool>(sz,                    // n
                            0,                     // init value
                            std::make_unique<memallocator>()),   // allocator
               std::system_error
               );
}
TEST(vector_copy_assignment) {
  Vector<bool> v1{0,0,1,1,0,0,1,1,0,0};
  Vector<bool> v2{0,0,0};
  v1 = v2;
  ASSERT_TRUE(v1==v2);
}
TEST(vector_swap) {
  Vector<bool> v1{0,0,1,1,0,0,1,1,0,0};
  Vector<bool> v2{0,0,0};
  Vector<bool> v1_copy = v1;
  Vector<bool> v2_copy = v2;
  std::swap(v1_copy, v2_copy);
  ASSERT_TRUE(v1_copy == v2);
  ASSERT_TRUE(v2_copy == v1);
}

// // ops & other member funcs
TEST(vector_push_back) {
  const size_t sz = arr::VECTOR_INITIAL_ALLOC + 2;
  Vector<bool> v;
  for (size_t i=0; i<sz; ++i) {
    v.push_back(static_cast<bool>(i % 2));
  }
  bool allok = true;
  for (size_t i=0; i<sz; ++i) {
    if (v[i] != static_cast<bool>(i % 2)) {
      allok = false;
      break;
    }
  }
  ASSERT_TRUE(allok);
}
TEST(vector_push_back_other_pattern) {
  const size_t sz = arr::VECTOR_INITIAL_ALLOC + 2;
  Vector<bool> v;
  for (size_t i=1; i<sz+1; ++i) {
    v.push_back(static_cast<bool>(i % 2));
  }
  bool allok = true;
  for (size_t i=1; i<sz+1; ++i) {
    if (v[i-1] != static_cast<bool>(i % 2)) {
      allok = false;
      break;
    }
  }
  ASSERT_TRUE(allok);
}
TEST(vector_front) {
  const size_t sz = 10;
  Vector<bool> v;
  for (size_t i=0; i<sz; ++i) {
    v.push_back(static_cast<bool>(i % 2));
    ASSERT_TRUE(v.front() == false);
  }
}
TEST(vector_back) {
  const size_t sz = 10;
  Vector<bool> v;
  for (size_t i=0; i<sz; ++i) {
    v.push_back(static_cast<bool>(i % 2));
    std::cout << static_cast<bool>(i % 2) << "==" << v.back() << std::endl;
    ASSERT_TRUE(v.back() == static_cast<bool>(i % 2));
  }
}
TEST(vector_size_0) {
  Vector<bool> v;
  ASSERT_TRUE(v.size() == 0UL);
}
TEST(vector_size_10) {
  Vector<bool> v{0,0,1,1,0,0,1,1,0,0};
  ASSERT_TRUE(v.size() == 10UL);
}
TEST(vector_eq) {
  ASSERT_TRUE ((Vector<bool>{0,0,1,1,0,0,1,1,0,0} == Vector<bool>{0,0,1,1,0,0,1,1,0,0}));
  ASSERT_FALSE((Vector<bool>{0,0,1,1,0,0,1,1,0}   == Vector<bool>{0,0,1,1,0,0,1,1,0,0}));
  ASSERT_FALSE((Vector<bool>{0,0,1,1,0,0,1,1,0,0} == Vector<bool>{0,0,1,1,0,0,1,1,0,1}));
}
TEST(vector_ne) {
  ASSERT_FALSE((Vector<bool>{0,0,1,1,0,0,1,1,0,0} != Vector<bool>{0,0,1,1,0,0,1,1,0,0}));
  ASSERT_TRUE ((Vector<bool>{0,0,1,1,0,0,1,1,0}   != Vector<bool>{0,0,1,1,0,0,1,1,0,0}));
  ASSERT_TRUE ((Vector<bool>{0,0,1,1,0,0,1,1,0,0} != Vector<bool>{0,0,1,1,0,0,1,1,0,1}));
}
TEST(vector_resize_larger_huge_fail) {
  const size_t n = 2 * arr::VECTOR_INITIAL_ALLOC + 2;
  const bool init = true;
  Vector<bool> v1(n, init);
  const size_t sz = 1000L * 1000000000L;
  ASSERT_THROW(v1.resize(sz), std::system_error);
  ASSERT_TRUE(v1.size() == n);
}
TEST(vector_resize_smaller) {
  const size_t n = 2 * arr::VECTOR_INITIAL_ALLOC + 2;
  const size_t n2 = n*2;
  const auto init = true;
  Vector<bool> v1(n2, init);
  v1.resize(n);
  Vector<bool> v2(n, init);
  ASSERT_TRUE(v1 == v2);
}
TEST(vector_deallocate) {
  const size_t n = 2 * arr::VECTOR_INITIAL_ALLOC + 2;
  const size_t n2 = n*2;
  const bool init = false;
  Vector<bool> v1(n2, init);
  v1.deallocate();
  ASSERT_TRUE(v1.size() == 0UL);
}
TEST(vector_assign_larger) {
  const size_t n = 2 * arr::VECTOR_INITIAL_ALLOC + 2;
  const bool init = true;
  Vector<bool> v1(n, false);
  const size_t n2 = n*2;
  v1.init(n2, init);
  Vector<bool> v2(n2, init);
  ASSERT_TRUE(v1 == v2);
}
TEST(vector_assign_smaller) {
  const size_t n = 2 * arr::VECTOR_INITIAL_ALLOC + 2;
  const size_t n2 = n*2;
  Vector<bool> v1(n2, false);
  const bool init = true;
  v1.init(n, init);
  Vector<bool> v2(n, init);
  ASSERT_TRUE(v1 == v2);
}
TEST(vector_insert_start) {
  Vector<bool> v1{0,0,1,1,0,0,1,1,0,0};
  std::vector<bool> v2{1,1,1};
  v1.insert(v1.begin(), v2.cbegin(), v2.cend());
  ASSERT_TRUE((v1 == Vector<bool>{1,1,1,0,0,1,1,0,0,1,1,0,0}));
}
TEST(vector_insert_middle) {
  Vector<bool> v1{0,0,0,0,0,0,0,0,0,0};
  std::vector<bool> v2{1,1,1};
  v1.insert(v1.begin()+4, v2.cbegin(), v2.cend());
  for (size_t i=0; i<v1.size(); ++i) 
    std::cout << v1[i] << ", " ; 
  std::cout << std::endl;
  ASSERT_TRUE((v1 == Vector<bool>{0,0,0,0,1,1,1,0,0,0,0,0,0}));
}
TEST(vector_insert_end) {
  Vector<bool> v1{0,0,1,1,0,0,1,1,0,0};
  std::vector<bool> v2{1,0,1};
  v1.insert(v1.end(), v2.cbegin(), v2.cend());
  ASSERT_TRUE((v1 == Vector<bool>{0,0,1,1,0,0,1,1,0,0,1,0,1}));
}
TEST(vector_erase_position) {
  // middle
  {
    Vector<bool> v{0,0,1,1,0,0,1,1,0,0};
    v.erase(v.begin() + 2);
    ASSERT_TRUE((v == Vector<bool>{0,0,1,0,0,1,1,0,0}));
  }
  // start
  {
    Vector<bool> v{0,0,1,1,0,0,1,1,0,0};
    v.erase(v.begin());
    ASSERT_TRUE((v == Vector<bool>{0,1,1,0,0,1,1,0,0}));
  }
  // end
  {
    Vector<bool> v{0,0,1,1,0,0,1,1,0,0};
    v.erase(v.end()-1);
    ASSERT_TRUE((v == Vector<bool>{0,0,1,1,0,0,1,1,0}));
  }
}
TEST(vector_erase_iterator) {
  // middle
  {
    Vector<bool> v{0,0,0,0,0,1,1,1,1,1};
    auto iter = v.erase(v.begin() + 2, v.begin() + 4);
    ASSERT_TRUE((v == Vector<bool>{0,0,0,1,1,1,1,1}));
    ASSERT_TRUE(*iter == v[2]);
  }
  // start
  {
    Vector<bool> v{0,0,0,0,0,1,1,1,1,1};
    auto iter = v.erase(v.begin() + 0, v.begin() + 2);
    ASSERT_TRUE((v == Vector<bool>{0,0,0,1,1,1,1,1}));
    ASSERT_TRUE(*iter == v[0]);
  }
  // end
  {
    Vector<bool> v{0,0,0,0,0,1,1,1,1,1};
    auto iter = v.erase(v.begin() + 8, v.end());
    ASSERT_TRUE((v == Vector<bool>{0,0,0,0,0,1,1,1}));
    ASSERT_TRUE(iter == v.end());
  }
}
// -- algo
TEST(vector_sort) {
  Vector<bool> v1{0,0,0,0,0,1,1,1,1,1};
  Vector<bool> v2{0,1,0,1,0,1,0,1,0,1};
  std::sort(v2.begin(), v2.end());
  ASSERT_TRUE(v1 == v2);
}

// // -- memory mapping tests
TEST(vector_mmap_constructor_basic) {
  std::string filename = "temp1";
  {  
    const size_t sz = 10;
    Vector<bool> v(sz,                    // n
                     true,                // init value
                     std::make_unique<mmapallocator>(filename)); // allocator
    for (size_t i=0; i<sz; ++i) {
      setv(v, i, false);
    }
    bool all3 = true;
    for (size_t i=0; i<sz; ++i) {
      if (v[i] != false) {
        all3 = false;
        break;
      }
    }
    ASSERT_TRUE(all3);
  } // Vector (and consequently mmapallocator is being deleted here
  int res = remove(filename.c_str());
  ASSERT_TRUE(res==0);
}
TEST(vector_mmap_initialize) {
  std::string filename = "temp1";
  const size_t sz = 10;
  {  
    Vector<bool> v(rsv,
                   sz,                    // n
                   std::make_unique<mmapallocator>(filename)); // allocator
    for (size_t i=0; i<sz; ++i) {
      v.push_back(static_cast<bool>(i % 2));
    }
  } // Vector (and consequently mmapallocator is being deleted here
  {
    Vector<bool> v(std::make_unique<mmapallocator>(filename));
    ASSERT_TRUE(v.size() == sz);
    bool allok = true;
    for (size_t i=0; i<v.size(); ++i) {
      if (v[i] != static_cast<bool>(i % 2)) {
        allok = false;
        break;
      }
    }
    ASSERT_TRUE(allok);
  } // Vector (and consequently mmapallocator is being deleted here
  int res = remove(filename.c_str());
  ASSERT_TRUE(res==0);
}
TEST(vector_mmap_resize_larger) {
  std::string filename = "temp1";
  {
    const size_t n = 2 * arr::VECTOR_INITIAL_ALLOC + 2;
    const bool init = true;
    Vector<bool> v1(n, init, std::make_unique<mmapallocator>(filename));
    for (size_t i=0; i<n; ++i) {
      setv(v1, i, static_cast<bool>(i % 2));
    }
    const size_t n2 = n*2;
    v1.resize(n2);
    for (size_t i=n; i<n2; ++i) {
      setv(v1, i, static_cast<bool>(i % 2));
    }
    Vector<bool> v2(n2, init);
    for (size_t i=0; i<n2; ++i) {
      setv(v2, i, static_cast<bool>(i % 2));
    }
    ASSERT_TRUE(v1 == v2);
  } // Vector (and consequently mmapallocator is being deleted here
  int res = remove(filename.c_str());
  ASSERT_TRUE(res==0);
}
TEST(vector_mmap_resize_larger_huge_fail) {
  std::string filename = "temp1";
  {
    const size_t n = 2 * arr::VECTOR_INITIAL_ALLOC + 2;
    const bool init = true;
    Vector<bool> v1(n, init, std::make_unique<mmapallocator>(filename));
    const size_t sz = 1000000L * 1000000000L;
    ASSERT_THROW(v1.resize(sz), std::system_error);
    ASSERT_TRUE(v1.size() == n);
  } // Vector (and consequently mmapallocator is being deleted here
  int res = remove(filename.c_str());
  ASSERT_TRUE(res==0);
}
TEST(vector_mmap_resize_smaller) {
  std::string filename = "temp1";
  {
    const size_t n = 2 * arr::VECTOR_INITIAL_ALLOC + 2;
    const size_t n2 = n*2;
    const bool init = true;
    Vector<bool> v1(n2, init, std::make_unique<mmapallocator>(filename));
    v1.resize(n);
    Vector<bool> v2(n, init);
    ASSERT_TRUE(v1 == v2);
  } // Vector (and consequently mmapallocator is being deleted here
  int res = remove(filename.c_str());
  ASSERT_TRUE(res==0);
}
TEST(vector_mmap_deallocate) {
  std::string filename = "temp1";
  const size_t n = 2 * arr::VECTOR_INITIAL_ALLOC + 2;
  const bool init = true;
  Vector<bool> v1(n, init, std::make_unique<mmapallocator>(filename));
  const size_t n2 = n*2;
  v1.resize(n2);
  std::cout << "v1.size(): " << v1.size() << std::endl;
  for (size_t i=n; i<n2; ++i) {
    std::cout << i << std::endl;
    setv(v1, i, init);
  }
  v1.deallocate();
  ASSERT_TRUE(v1.size()==0UL);
  int res = remove(filename.c_str());
  ASSERT_TRUE(res==-1);         // file was already removed in deallocate
}
TEST(vector_gettypenumber_bool) {
  std::string filename = "temp1";
  const size_t sz = 10;
  {  
    Vector<bool> v(sz,                    // n
                   true,                  // init value
                   std::make_unique<mmapallocator>(filename)); // allocator
  } // Vector (and consequently mmapallocator is being deleted here
  ASSERT_TRUE(getTypeNumber(filename) == TypeNumber<bool>::n);
  ASSERT_TRUE(remove(filename.c_str())==0);
}
// // iterator:
TEST(vector_iterator_inc) {
  Vector<bool> v1{0,0,0,0,0,1,1,1,1,1};
  Vector<bool> v2{1,1,1,1,1,0,0,0,0,0};
  for (auto iter=v1.begin(); iter!=v1.end(); ++iter) {
    *iter = !*iter;
  }
  ASSERT_TRUE(v1 == v2);
}
TEST(vector_iterator_post_inc) {
  Vector<bool> v1{0,0,0,0,0,1,1,1,1,1};
  Vector<bool> v2{1,1,1,1,1,0,0,0,0,0};
  for (auto iter=v1.begin(); iter!=v1.end(); iter++) {
    *iter = !*iter;
  }
  ASSERT_TRUE(v1 == v2);
}
TEST(vector_const_iterator_inc) {
  Vector<bool> v1{0,1,0,1,0,1,0,1,0,1};
  double sum = 0.0;
  auto iter=v1.cbegin();
  for (size_t i=0; i<v1.size()-1; ++i) {
    sum += *(++iter) ? 1.0 : 0.0;
  }
  ASSERT_TRUE(iter == v1.cend()-1);
  ASSERT_TRUE(sum == 5.0);
}
TEST(vector_const_iterator_post_inc) {
  Vector<bool> v1{0,1,0,1,0,1,0,1,0,1};
  double sum = 0.0;
  auto iter=v1.cbegin();
  for (size_t i=0; i<v1.size(); ++i) {
    sum += *(iter++) ? 1.0 : 0.0;
  }
  ASSERT_TRUE(sum == 5.0);
}
TEST(vector_iterator_copy) {
  Vector<bool> v1{0,1,0,1,0,1,0,1,0,1};
  Vector<bool> v2;
  v2.resize(v1.size());
  std::copy(v1.begin(), v1.end(), v2.begin());
  ASSERT_TRUE(v1==v2);
}
TEST(vector_iterator_copy_backward) {
  Vector<bool> v1{0,0,0,0,0,1,1,1,1,1};
  Vector<bool> v2;
  v2.resize(v1.size());
  std::copy_backward(v1.begin(), v1.end(), v2.end());
  ASSERT_TRUE(v1==v2);
}
TEST(vector_erase_remove_if_something) {
  Vector<bool> v{1,1,1,1,0,0,0,0};
  v.erase(std::remove_if(v.begin(), v.end(), [](size_t i) { return i; }), v.end());
  ASSERT_TRUE((v==Vector<bool>{0,0,0,0}));
}
TEST(vector_erase_remove_if_nothing) {
  Vector<bool> v{0,0,0,0};
  v.erase(std::remove_if(v.begin(), v.end(), [](size_t i) { 
        return i; 
      }),
          v.end());
  ASSERT_TRUE((v==Vector<bool>{0,0,0,0}));
}
TEST(vector_erase_remove_nothing) {
  Vector<bool> v{1,1,1,1,1};
  v.erase(std::remove(v.begin(), v.end(), false), v.end());
  ASSERT_TRUE((v==Vector<bool>{1,1,1,1,1}));
}
TEST(vector_iterator_find) {
  Vector<bool> v{0,0,0,0,0};
  auto elt = std::find(v.begin(), v.end(), true);
  ASSERT_TRUE(elt == v.end());
}
TEST(vector_iterator_find_if) {
  Vector<bool> v{0,0,0,0,0};
  auto elt = std::find_if(v.begin(), v.end(), [](bool i) { 
      std::cout << "i: " << i << std::endl;
      return i; 
    });
  ASSERT_TRUE(elt == v.end());
}

// // test append:
template<unsigned N>
union U {
  uint64_t u[N];
  double d[N];
  bool b[N*sizeof(double)/sizeof(bool)];
  char s[N*sizeof(double)];
};
TEST(vector_append_bool) {
  Vector<bool> v{false, false};
  Vector<bool> expected{false, false, true, true};
  U<1> data;
  data.b[0] = true;
  data.b[1] = true;
  U<4> buf;
  buf.u[0] = TypeNumber<bool>::n;
  buf.u[1] = 2;
  buf.u[2] = 0;
  buf.u[3] = data.u[0];
  
  v.append(buf.s, sizeof(buf));
  std::cout << v.size() << std::endl;
  std::cout << v[0] << ", "  << v[1] << ", " << v[2] << ", " << v[3] << std::endl;
  std::cout << expected[0] << ", "  << expected[1] << ", " << expected[2] << ", " << expected[3] << std::endl;
  ASSERT_TRUE(v == expected);
}


int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
