// -*- compile-command: "make -k -j -O test" -*-

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
#include "vector_base.hpp"
#include "misc.hpp"

using namespace arr;

namespace arr {
  TYPE_NB(double, 0);
  TYPE_NB(bool, 1);
  TYPE_NAME(double, "double");
  TYPE_NAME(bool, "logical");
}

// -- non memory mapping tests ---------------
// constructors:
TEST(vector_constructor_basic) {
  const size_t sz = 10;
  Vector<double> v(sz,                    // n
                   0,                     // init value
                   std::make_unique<memallocator>());   // allocator
  for (size_t i=0; i<sz; ++i) {
    setv(v, i, 3.0);
  }
  bool all3 = true;
  for (size_t i=0; i<sz; ++i) {
    if (v[i] != 3) {
      all3 = false;
      break;
    }
  }
  ASSERT_TRUE(all3);
}
TEST(vector_constructor_init) {
  const size_t sz = 10;
  Vector<double> v(sz, 3, std::make_unique<memallocator>());
  bool all3 = true;
  for (size_t i=0; i<sz; ++i) {
    if (v[i] != 3) {
      all3 = false;
      break;
    }
  }
  ASSERT_TRUE(all3);
}
TEST(vector_constructor_vector_iter) {
  std::vector<double> sv{1,2,3,4,5,6,7,8,9,10};
  const size_t sz = 10;
  Vector<double> v(sv.begin(), sv.end());
  bool allok = true;
  for (size_t i=0; i<sz; ++i) {
    if (v[i] != sv[i]) {
      allok = false;
      break;
    }
  }
  ASSERT_TRUE(allok);
}
TEST(vector_constructor_Vector_iter) {
  std::vector<double> sv{1,2,3,4,5,6,7,8,9,10};
  const size_t sz = 10;
  Vector<double> v1(sv.begin(), 
                    sv.end(),
                    std::make_unique<memallocator>()); // allocator
  std::cout << "v1[2] is: " << v1[2] << std::endl; 
  Vector<double> v2(v1.cbegin(), 
                    v1.cend(),
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
  std::vector<double> sv{1,2,3,4,5,6,7,8,9,10};
  Vector<double> v{1,2,3,4,5,6,7,8,9,10};
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
  Vector<double> v1{1,2,3,4,5,6,7,8,9,10};
  Vector<double> v2(v1);
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
  std::vector<double> sv{1,2,3,4,5,6,7,8,9,10};
  Vector<double> v2(Vector<double>{1,2,3,4,5,6,7,8,9,10});
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
  ASSERT_THROW(Vector<double>(sz,                    // n
                              0,                     // init value
                              std::make_unique<memallocator>()),  // allocator
               std::system_error
               );
}
TEST(vector_copy_assignment) {
  Vector<double> v1{1,2,3,4,5,6,7,8,9,10};
  Vector<double> v2{8,9,10};
  v1 = v2;
  ASSERT_TRUE(v1==v2);
}
TEST(vector_swap) {
  Vector<double> v1{1,2,3,4,5,6,7,8,9,10};
  Vector<double> v2{8,9,10};
  Vector<double> v1_copy = v1;
  Vector<double> v2_copy = v2;
  std::swap(v1_copy, v2_copy);
  ASSERT_TRUE(v1_copy == v2);
  ASSERT_TRUE(v2_copy == v1);
}

// ordering semantics:
TEST(vector_constructor_basic_lt) {
  const size_t sz = 10;
  Vector<double> v(sz, 0);
  ASSERT_TRUE(!v.isOrdered());
}
TEST(vector_constructor_basic_le) {
  const size_t sz = 10;
  Vector<double, std::less_equal<double>> v(sz, 0);
  ASSERT_TRUE(v.isOrdered());
}
TEST(vector_constructor_vector_iter_lt) {
  std::vector<double> sv{1,2,3,4,5,6,7,8,9,10};
  Vector<double> v(sv.begin(), sv.end());
  ASSERT_TRUE(v.isOrdered());
}
TEST(vector_constructor_init_list_lt) {
  Vector<double> v{1,2,3,4,5,6,7,8,9,10};
  ASSERT_TRUE(v.isOrdered());
}
TEST(vector_constructor_init_list_lt_unordered) {
  Vector<double> v{1,2,3,4,5,2,7,8,9,10};
  ASSERT_TRUE(!v.isOrdered());
}
TEST(vector_constructor_Vector_iter_lt) {
  Vector<double> v1{1,2,3,4,5,6,7,8,9,10};
  Vector<double> v2(v1.cbegin(), v1.cend());
  ASSERT_TRUE(v2.isOrdered());
}
TEST(vector_copy_constructor_lt) {
  Vector<double> v1{1,2,3,4,5,6,7,8,9,10};
  Vector<double> v2(v1);
  ASSERT_TRUE(v2.isOrdered());
}
TEST(vector_move_constructor_lt) {
  Vector<double> v2(Vector<double>{1,2,3,4,5,6,7,8,9,10});
  ASSERT_TRUE(v2.isOrdered());
}

// ops & other member funcs
TEST(vector_push_back_ref) {
  const size_t sz = arr::VECTOR_INITIAL_ALLOC + 2;
  Vector<double> v;
  for (size_t i=0; i<sz; ++i) {
    v.push_back(i);
  }
  bool allok = true;
  for (size_t i=0; i<sz; ++i) {
    if (v[i] != i) {
      allok = false;
      break;
    }
  }
  ASSERT_TRUE(allok);
}
TEST(vector_size_0) {
  Vector<double> v;
  ASSERT_TRUE(v.size() == 0UL);
}
TEST(vector_size_10) {
  Vector<double> v{1,2,3,4,5,6,7,8,9,10};
  ASSERT_TRUE(v.size() == 10UL);
}
TEST(vector_eq) {
  ASSERT_TRUE((Vector<double>{1,2,3,4,5,6,7,8,9,10} == Vector<double>{1,2,3,4,5,6,7,8,9,10}));
  ASSERT_FALSE((Vector<double>{1,2,3,4,5,6,7,8,9} == Vector<double>{1,2,3,4,5,6,7,8,9,10}));
  ASSERT_FALSE((Vector<double>{1,2,3,4,5,6,7,8,9,10} == Vector<double>{1,1,3,4,5,6,7,8,9,10}));
}
TEST(vector_ne) {
  ASSERT_FALSE((Vector<double>{1,2,3,4,5,6,7,8,9,10} != Vector<double>{1,2,3,4,5,6,7,8,9,10}));
  ASSERT_TRUE((Vector<double>{1,2,3,4,5,6,7,8,9} != Vector<double>{1,2,3,4,5,6,7,8,9,10}));
  ASSERT_TRUE((Vector<double>{1,2,3,4,5,6,7,8,9,10} != Vector<double>{1,1,3,4,5,6,7,8,9,10}));
}
TEST(vector_eq_NaN) {
  ASSERT_TRUE((Vector<double>{Global::ZNAN,Global::ZNAN,3,4,5,6,7,8,9,10} == 
               Vector<double>{Global::ZNAN,Global::ZNAN,3,4,5,6,7,8,9,10}));
  ASSERT_FALSE((Vector<double>{Global::ZNAN,Global::ZNAN,3,4,5,6,7,8,9} == 
                Vector<double>{Global::ZNAN,Global::ZNAN,3,4,5,6,7,8,9,10}));
  ASSERT_FALSE((Vector<double>{Global::ZNAN,Global::ZNAN,3,4,5,6,7,8,9,10} == 
                Vector<double>{Global::ZNAN,Global::ZNAN,3,4,5,6,7,8,9,9}));
}
TEST(vector_ne_NaN) {
  ASSERT_FALSE((Vector<double>{Global::ZNAN,Global::ZNAN,3,4,5,6,7,8,9,10} != 
                Vector<double>{Global::ZNAN,Global::ZNAN,3,4,5,6,7,8,9,10}));
  ASSERT_TRUE((Vector<double>{Global::ZNAN,Global::ZNAN,3,4,5,6,7,8,9} != 
               Vector<double>{Global::ZNAN,Global::ZNAN,3,4,5,6,7,8,9,10}));
  ASSERT_TRUE((Vector<double>{Global::ZNAN,Global::ZNAN,3,4,5,6,7,8,9,10} != 
               Vector<double>{Global::ZNAN,Global::ZNAN,3,4,5,6,7,8,9,9}));
}
TEST(vector_eq_zstring) {
  ASSERT_TRUE((Vector<zstring>{"a","b","c1","c2","c3"} == 
               Vector<zstring>{"a","b","c1","c2","c3"}));
  ASSERT_FALSE((Vector<zstring>{"a","b","c1","c2","c3"} == 
               Vector<zstring>{"a","a","c1","c2","c3"}));
}
TEST(vector_ne_zstring) {
  ASSERT_FALSE((Vector<zstring>{"a","b","c1","c2","c3"} != 
               Vector<zstring>{"a","b","c1","c2","c3"}));
  ASSERT_TRUE((Vector<zstring>{"a","b","c1","c2","c3"} != 
               Vector<zstring>{"a","a","c1","c2","c3"}));
}
TEST(vector_resize_larger) {
  const size_t n = 2 * arr::VECTOR_INITIAL_ALLOC + 2;
  const double init = 10;
  Vector<double> v1(n, init);
  const size_t n2 = n*2;
  v1.resize(n2);
  for (size_t i=n; i<n2; ++i) {
    std::cout << "i: " << i << std::endl;
    setv(v1, i, init);
  }
  Vector<double> v2(n2, init);
  ASSERT_TRUE(v1 == v2);
}
TEST(vector_resize_larger_huge_fail) {
  const size_t n = 2 * arr::VECTOR_INITIAL_ALLOC + 2;
  const double init = 10;
  Vector<double> v1(n, init);
  const size_t sz = 1000L * 1000000000L;
  ASSERT_THROW(v1.resize(sz), std::system_error);
  ASSERT_TRUE(v1.size() == n);
}
TEST(vector_resize_smaller) {
  const size_t n = 2 * arr::VECTOR_INITIAL_ALLOC + 2;
  const size_t n2 = n*2;
  const double init = 10;
  Vector<double> v1(n2, init);
  v1.resize(n);
  Vector<double> v2(n, init);
  ASSERT_TRUE(v1 == v2);
}
TEST(vector_deallocate) {
  const size_t n = 2 * arr::VECTOR_INITIAL_ALLOC + 2;
  const size_t n2 = n*2;
  const double init = 10;
  Vector<double> v1(n2, init);
  v1.deallocate();
  ASSERT_TRUE(v1.size() == 0UL);
}
TEST(vector_init_larger) {
  const size_t n = 2 * arr::VECTOR_INITIAL_ALLOC + 2;
  const double init = 10;
  Vector<double> v1(rsv, n);
  const size_t n2 = n*2;
  v1.init(n2, init);
  Vector<double> v2(n2, init);
  ASSERT_TRUE(v1 == v2);
}
TEST(vector_init_smaller) {
  const size_t n = 2 * arr::VECTOR_INITIAL_ALLOC + 2;
  const size_t n2 = n*2;
  Vector<double> v1(rsv, n2);
  const double init = 10;
  v1.init(n, init);
  Vector<double> v2(n, init);
  ASSERT_TRUE(v1 == v2);
}
TEST(vector_insert_start) {
  Vector<double> v1{1,2,3,4,5,6,7,8,9,10};
  std::vector<double> v2{7,6,5};
  v1.insert(v1.begin(), v2.cbegin(), v2.cend());
  ASSERT_TRUE((v1 == Vector<double>{7,6,5,1,2,3,4,5,6,7,8,9,10}));
}
TEST(vector_insert_middle) {
  Vector<double> v1{1,2,3,4,5,6,7,8,9,10};
  std::vector<double> v2{7,6,5};
  v1.insert(v1.begin()+4, v2.cbegin(), v2.cend());
  for (size_t i=0; i<v1.size(); ++i) std::cout << v1[i] << ", " ; std::cout << std::endl;
  ASSERT_TRUE((v1 == Vector<double>{1,2,3,4,7,6,5,5,6,7,8,9,10}));
}
TEST(vector_insert_end) {
  Vector<double> v1{1,2,3,4,5,6,7,8,9,10};
  std::vector<double> v2{7,6,5};
  v1.insert(v1.end(), v2.cbegin(), v2.cend());
  ASSERT_TRUE((v1 == Vector<double>{1,2,3,4,5,6,7,8,9,10,7,6,5}));
}
TEST(vector_erase_position) {
  // middle
  {
    Vector<double> v{1,2,3,4,5,6,7,8,9,10};
    v.erase(v.begin() + 2);
    ASSERT_TRUE((v == Vector<double>{1,2,4,5,6,7,8,9,10}));
  }
  // start
  {
    Vector<double> v{1,2,3,4,5,6,7,8,9,10};
    v.erase(v.begin());
    ASSERT_TRUE((v == Vector<double>{2,3,4,5,6,7,8,9,10}));
  }
  // end
  {
    Vector<double> v{1,2,3,4,5,6,7,8,9,10};
    v.erase(v.end()-1);
    ASSERT_TRUE((v == Vector<double>{1,2,3,4,5,6,7,8,9}));
  }
}
TEST(vector_erase_iterator) {
  // middle
  {
    Vector<double> v{1,2,3,4,5,6,7,8,9,10};
    auto iter = v.erase(v.begin() + 2, v.begin() + 4);
    ASSERT_TRUE((v == Vector<double>{1,2,5,6,7,8,9,10}));
    ASSERT_TRUE(*iter == v[2]);
  }
  // start
  {
    Vector<double> v{1,2,3,4,5,6,7,8,9,10};
    auto iter = v.erase(v.begin() + 0, v.begin() + 2);
    ASSERT_TRUE((v == Vector<double>{3,4,5,6,7,8,9,10}));
    ASSERT_TRUE(*iter == v[0]);
  }
  // end
  {
    Vector<double> v{1,2,3,4,5,6,7,8,9,10};
    auto iter = v.erase(v.begin() + 8, v.end());
    ASSERT_TRUE((v == Vector<double>{1,2,3,4,5,6,7,8}));
    ASSERT_TRUE(iter == v.end());
  }
}
// -- algo
TEST(vector_sort) {
  Vector<double> v1{1,2,3,4,5,6,7,8,9,10};
  Vector<double> v2{2,1,3,10,5,8,7,6,9,4};
  std::sort(v2.begin(), v2.end());
  ASSERT_TRUE(v1 == v2);
}
// test the order semantics:
TEST(vector_order_lt_at) {
  Vector<double> v1{1,2,3,4,5,6,7,8,9,10};
  ASSERT_TRUE(v1.isOrdered());
  setv(v1, 3, 11.0);
  ASSERT_TRUE(!v1.isOrdered());
}
TEST(vector_order_lt_at_begin) {
  Vector<double> v1{1,2,3,4,5,6,7,8,9,10};
  ASSERT_TRUE(v1.isOrdered());
  setv(v1, 0, 2.0);
  ASSERT_TRUE(!v1.isOrdered());
}
TEST(vector_order_lt_at_end) {
  Vector<double> v1{1,2,3,4,5,6,7,8,9,10};
  ASSERT_TRUE(v1.isOrdered());
  setv(v1, v1.size()-1, 9.0);
  ASSERT_TRUE(!v1.isOrdered());
}
TEST(vector_order_lt_push_back_keep) {
  Vector<double> v1{1,2,3,4,5,6,7,8,9,10};
  ASSERT_TRUE(v1.isOrdered());
  v1.push_back(11);
  ASSERT_TRUE(v1.isOrdered());
}
TEST(vector_order_lt_push_back) {
  Vector<double> v1{1,2,3,4,5,6,7,8,9,10};
  ASSERT_TRUE(v1.isOrdered());
  v1.push_back(2);
  ASSERT_TRUE(!v1.isOrdered());
}
TEST(vector_order_lt_push_back_keep_rsv) {
  Vector<double> v1(rsv, 10);
  ASSERT_TRUE(v1.isOrdered());
  Vector<double> v2{1,2,3,4,5,6,7,8,9,10};
  for (auto e : v2) {
    v1.push_back(e);
  }
  ASSERT_TRUE(v1.isOrdered());
}
TEST(vector_order_lt_push_back_rsv) {
  Vector<double> v1(rsv, 10);
  ASSERT_TRUE(v1.isOrdered());
  Vector<double> v2{1,2,3,4,3,6,7,8,9,10};
  for (auto e : v2) {
    v1.push_back(e);
  }
  ASSERT_TRUE(!v1.isOrdered());
}
TEST(vector_order_ge_push_back_keep) {
  Vector<double, std::greater_equal<double>> v1{10,9,8,7,6,5,4,3,2,1};
  ASSERT_TRUE(v1.isOrdered());
  v1.push_back(1);
  ASSERT_TRUE(v1.isOrdered());
}
TEST(vector_order_ge_push_back) {
  Vector<double, std::greater_equal<double>> v1{10,9,8,7,6,5,4,3,2,1};
  ASSERT_TRUE(v1.isOrdered());
  v1.push_back(2);
  ASSERT_TRUE(!v1.isOrdered());
}
TEST(vector_sort_lt) {
  Vector<double> v1{2,1,3,10,5,8,7,6,9,4};
  std::sort(v1.begin(), v1.end());
  // even if it is actually ordered, we can't actually know it;
  // std::sort accesses and changes the 'Vector' through iterators
  // that invalidate the 'ordered' flag of the 'Vector' no matter
  // what.
  ASSERT_TRUE(!v1.isOrdered());
}
TEST(vector_checkset_ordered_unordered_lt) {
  Vector<double> v1{2,1,3,10,5,8,7,6,9,4};
  ASSERT_TRUE((!v1.checkAndSetOrdered() && !v1.isOrdered()));
}
TEST(vector_checkset_ordered_lt) {
  Vector<double> v1{2,1,3,10,5,8,7,6,9,4};
  std::sort(v1.begin(), v1.end());
  ASSERT_TRUE((v1.checkAndSetOrdered() && v1.isOrdered()));
}
TEST(vector_insert_start_ordered_lt) {
  Vector<double> v1{1,2,3,4,5,6,7,8,9,10};
  std::vector<double> v2{0.1,0.2,0.3};
  v1.insert(v1.begin(), v2.cbegin(), v2.cend());
  for (size_t i=0; i<v1.size(); ++i) std::cout << v1[i] << ", " ; std::cout << std::endl;
  ASSERT_TRUE(v1.isOrdered());
}
TEST(vector_insert_middle_ordered_lt) {
  Vector<double> v1{1,2,3,4,5,6,7,8,9,10};
  std::vector<double> v2{4.1,4.2,4.3};
  v1.insert(v1.begin()+4, v2.cbegin(), v2.cend());
  for (size_t i=0; i<v1.size(); ++i) std::cout << v1[i] << ", " ; std::cout << std::endl;
  ASSERT_TRUE(v1.isOrdered());
}
TEST(vector_insert_end_ordered_lt) {
  Vector<double> v1{1,2,3,4,5,6,7,8,9,10};
  std::vector<double> v2{10.1,10.2,10.3};
  v1.insert(v1.end(), v2.cbegin(), v2.cend());
  ASSERT_TRUE(v1.isOrdered());
}
TEST(vector_insert_start_unordered_lt) {
  Vector<double> v1{1,2,3,4,5,6,7,8,9,10};
  std::vector<double> v2{1,2,3};
  v1.insert(v1.begin(), v2.cbegin(), v2.cend());
  for (size_t i=0; i<v1.size(); ++i) std::cout << v1[i] << ", " ; std::cout << std::endl;
  ASSERT_TRUE(!v1.isOrdered());
}
TEST(vector_insert_middle_unordered_lt) {
  Vector<double> v1{1,2,3,4,5,6,7,8,9,10};
  std::vector<double> v2{4,4.1,4.2};
  v1.insert(v1.begin()+4, v2.cbegin(), v2.cend());
  for (size_t i=0; i<v1.size(); ++i) std::cout << v1[i] << ", " ; std::cout << std::endl;
  ASSERT_TRUE(!v1.isOrdered());
}
TEST(vector_insert_end_unordered_lt) {
  Vector<double> v1{1,2,3,4,5,6,7,8,9,10};
  std::vector<double> v2{7,6,5};
  v1.insert(v1.end(), v2.cbegin(), v2.cend());
  ASSERT_TRUE(!v1.isOrdered());
}

// -- memory mapping tests
TEST(vector_mmap_constructor_basic) {
  std::string filename = "temp1";
  {  
    const size_t sz = 10;
    Vector<double> v(sz,                    // n
                     0,                     // init value
                     std::make_unique<mmapallocator>(filename)); // allocator
    for (size_t i=0; i<sz; ++i) {
      setv(v, i, 3.0);
    }
    bool all3 = true;
    for (size_t i=0; i<sz; ++i) {
      if (v[i] != 3) {
        all3 = false;
        break;
      }
    }
    ASSERT_TRUE(all3);
  } // Vector (and consequently mmapallocator is being deleted here
  int res = remove(filename.c_str());
  ASSERT_TRUE(res==0);
}
// On some systems the mmap/fallocate allocation, even so it is well
// beyond the capacity of the system doesn't fail, so the following
// test becomes meaningless:
//
// TEST(vector_mmap_constructor_huge_fail) {
//   std::string filename = "temp1";
//   {  
//     const size_t sz = 1000000L * 1000000000L;
//     ASSERT_THROW(Vector<double>(sz,                    // n
//                                 0,                     // init value
//                                 std::make_unique<mmapallocator>(filename)),  // allocator
//                  std::system_error
//                  );
//   } // Vector (and consequently mmapallocator is being deleted here
//   int res = remove(filename.c_str());
//   ASSERT_TRUE(res==0);         // file should not be created? LLL
// }
TEST(vector_mmap_initialize) {
  std::string filename = "temp1";
  const size_t sz = 10;
  {  
    Vector<double> v(sz,                    // n
                     0,                     // init value
                     std::make_unique<mmapallocator>(filename)); // allocator
    for (size_t i=0; i<sz; ++i) {
      setv(v, i, double(i));
    }
  } // Vector (and consequently mmapallocator is being deleted here
  {
    Vector<double> v(std::make_unique<mmapallocator>(filename));
    ASSERT_TRUE(v.size() == sz);
    bool allok = true;
    for (size_t i=0; i<v.size(); ++i) {
      if (v[i] != i) {
        allok = false;
        break;
      }
    }
    ASSERT_TRUE(allok);
  } // Vector (and consequently mmapallocator is being deleted here
  int res = remove(filename.c_str());
  ASSERT_TRUE(res==0);
}
TEST(vector_mmap_initialize_zstring) {
  std::string filename = "temp1";
  const size_t sz = 10;
  {  
    Vector<zstring> v(sz,                    // n
                      "",                    // init value
                      std::make_unique<mmapallocator>(filename)); // allocator
    for (size_t i=0; i<sz; ++i) {
      setv(v, i, zstring(std::to_string(i)));
    }
  } // Vector (and consequently mmapallocator is being deleted here
  {
    Vector<zstring> v(std::make_unique<mmapallocator>(filename));
    ASSERT_TRUE(v.size() == sz);
    bool allok = true;
    for (size_t i=0; i<v.size(); ++i) {
      if (std::string(v[i]) != std::to_string(i)) {
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
    const double init = 10;
    Vector<double> v1(n, init, std::make_unique<mmapallocator>(filename));
    for (size_t i=0; i<n; ++i) {
      setv(v1, i, double(i));
    }
    const size_t n2 = n*2;
    v1.resize(n2);
    for (size_t i=n; i<n2; ++i) {
      setv(v1, i, double(i));
    }
    Vector<double> v2(n2, init);
    for (size_t i=0; i<n2; ++i) {
      setv(v2, i, double(i));
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
    const double init = 10;
    Vector<double> v1(n, init, std::make_unique<mmapallocator>(filename));
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
    const double init = 10;
    Vector<double> v1(n2, init, std::make_unique<mmapallocator>(filename));
    v1.resize(n);
    Vector<double> v2(n, init);
    ASSERT_TRUE(v1 == v2);
  } // Vector (and consequently mmapallocator is being deleted here
  int res = remove(filename.c_str());
  ASSERT_TRUE(res==0);
}
TEST(vector_mmap_deallocate) {
  std::string filename = "temp1";
  const size_t n = 2 * arr::VECTOR_INITIAL_ALLOC + 2;
  const double init = 10;
  Vector<double> v1(n, init, std::make_unique<mmapallocator>(filename));
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
TEST(vector_gettypenumber_double) {
  std::string filename = "temp1";
  const size_t sz = 10;
  {  
    Vector<double> v(sz,                    // n
                     0,                     // init value
                     std::make_unique<mmapallocator>(filename)); // allocator
  } // Vector (and consequently mmapallocator is being deleted here
  ASSERT_TRUE(getTypeNumber(filename) == TypeNumber<double>::n);
  ASSERT_TRUE(remove(filename.c_str())==0);
}
TEST(vector_gettypenumber_zstring) {
  std::string filename = "temp1";
  const size_t sz = 10;
  {  
    Vector<zstring> v(sz,                    // n
                      "",                    // init value
                      std::make_unique<mmapallocator>(filename)); // allocator
  } // Vector (and consequently mmapallocator is being deleted here
  ASSERT_TRUE(getTypeNumber(filename) == TypeNumber<zstring>::n);
  ASSERT_TRUE(remove(filename.c_str())==0);
}
// iterator:
TEST(vector_iterator_inc) {
  Vector<double> v1{1,2,3,4,5,6,7,8,9,10};
  Vector<double> v2{2,3,4,5,6,7,8,9,10,11};
  for (auto iter=v1.begin(); iter!=v1.end(); ++iter) {
    *iter += 1;
  }
  ASSERT_TRUE(v1 == v2);
}
TEST(vector_iterator_post_inc) {
  Vector<double> v1{1,2,3,4,5,6,7,8,9,10};
  Vector<double> v2{2,3,4,5,6,7,8,9,10,11};
  for (auto iter=v1.begin(); iter!=v1.end(); iter++) {
    *iter += 1;
  }
  ASSERT_TRUE(v1 == v2);
}
TEST(vector_const_iterator_inc) {
  Vector<double> v1{1,2,3,4,5,6,7,8,9,10};
  double sum = 0.0;
  auto iter=v1.cbegin();
  for (size_t i=0; i<v1.size()-1; ++i) {
    sum += *(++iter);
  }
  ASSERT_TRUE(iter == v1.cend()-1);
  ASSERT_TRUE(sum == 54.0);
}
TEST(vector_const_iterator_post_inc) {
  Vector<double> v1{1,2,3,4,5,6,7,8,9,10};
  double sum = 0.0;
  auto iter=v1.cbegin();
  for (size_t i=0; i<v1.size(); ++i) {
    sum += *(iter++);
  }
  ASSERT_TRUE(iter == v1.cend());
  ASSERT_TRUE(sum == 55.0);
}
TEST(vector_iterator_copy) {
  Vector<double> v1{1,2,3,4,5,6,7,8,9,10};
  Vector<double> v2;
  v2.resize(v1.size());
  std::copy(v1.begin(), v1.end(), v2.begin());
  ASSERT_TRUE(v1==v2);
}
TEST(vector_iterator_copy_backward) {
  Vector<double> v1{1,2,3,4,5,6,7,8,9,10};
  Vector<double> v2;
  v2.resize(v1.size());
  std::copy_backward(v1.begin(), v1.end(), v2.end());
  ASSERT_TRUE(v1==v2);
}
TEST(vector_erase_remove_if_something) {
  Vector<double> v{1,2};
  v.erase(std::remove_if(v.begin(), v.end(), [](size_t i) { return i == 1; }), v.end());
  ASSERT_TRUE((v==Vector<double>{2}));
}
TEST(vector_erase_remove_if_nothing) {
  Vector<double> v{2,2};
  v.erase(std::remove_if(v.begin(), v.end(), [](size_t i) { 
        std::cout << "returning: " << (i == 1) << std::endl;
        return i == 1; 
      }),
          v.end());
  ASSERT_TRUE((v==Vector<double>{2,2}));
}
TEST(vector_erase_remove_nothing) {
  Vector<double> v{2,2};
  v.erase(std::remove(v.begin(), v.end(), 1), v.end());
  ASSERT_TRUE((v==Vector<double>{2,2}));
}
TEST(vector_iterator_find) {
  Vector<double> v{2,2};
  auto elt = std::find(v.begin(), v.end(), 1);
  ASSERT_TRUE(elt == v.end());
}
TEST(vector_iterator_find_if) {
  Vector<double> v{2,2};
  auto elt = std::find_if(v.begin(), v.end(), [](size_t i) { 
        std::cout << "returning: " << (i == 1) << std::endl;
        return i == 1; 
      });
  ASSERT_TRUE(elt == v.end());
}

// test append:
template<unsigned N>
union U {
  uint64_t u[N];
  double d[N];
  bool b[N*sizeof(double)/sizeof(bool)];
  char s[N*sizeof(double)];
};
TEST(vector_append_ordered) {
  Vector<double> v{2,3};
  Vector<double> expected{2,3,4,5};
  U<5> buf;
  buf.u[0] = TypeNumber<double>::n;
  buf.u[1] = 2;
  buf.u[2] = 0;
  buf.d[3] = 4.0;
  buf.d[4] = 5.0;
  std::cout << printBuf(buf.s, 32) << std::endl;
  for (auto e : expected) {
    std::cout << e << std::endl;
  }
  v.append(buf.s, sizeof(buf));
  ASSERT_TRUE(v == expected);
}
TEST(vector_append_unordered) {
  Vector<double> v{2,2};
  Vector<double> expected{2,2,3,3};
  U<5> buf;
  buf.u[0] = TypeNumber<double>::n;
  buf.u[1] = 2;
  buf.u[2] = 0;
  buf.d[3] = 3.0;
  buf.d[4] = 3.0;
  std::cout << printBuf(buf.s, 32) << std::endl;
  for (auto e : expected) {
    std::cout << e << std::endl;
  }
  v.append(buf.s, sizeof(buf));
  ASSERT_TRUE(v == expected);
}
TEST(vector_append_bool) {
  Vector<bool> v{false, false};
  Vector<bool> expected{false, false, true, true};
  U<5> buf;
  buf.u[0] = TypeNumber<bool>::n;
  buf.u[1] = 2;
  buf.u[2] = 0;
  buf.b[3*sizeof(double)/sizeof(bool) + 0] = true;
  buf.b[3*sizeof(double)/sizeof(bool) + 1] = true;
  v.append(buf.s, 3*sizeof(double) + 2*sizeof(bool));
  ASSERT_TRUE(v == expected);
}
TEST(vector_append_incorrect_type) {
  Vector<double> v{2,2};
  U<5> buf;
  buf.u[0] = TypeNumber<bool>::n;
  buf.u[1] = 2;
  buf.b[3*sizeof(double)/sizeof(bool) + 0] = true;
  buf.b[3*sizeof(double)/sizeof(bool) + 1] = true;
  ASSERT_THROW(v.append(buf.s, sizeof(buf)), 
               std::out_of_range, "incorrect type");
}
TEST(vector_append_too_short) {
  Vector<double> v{2,2};
  const char buf[] = {1};
  ASSERT_THROW(v.append(buf, sizeof(buf)), std::out_of_range, 
               "invalid append buffer: too short");
}
TEST(vector_apply_1) {
  Vector<double> v1 {0,1,2,3,4,5,6,7,8,9};
  Vector<double> exp{std::negate<double>()(0),-1,-2,-3,-4,-5,-6,-7,-8,-9};
  v1.apply<std::negate<double>>();
  for (unsigned j=0; j<v1.size(); ++j) {
    std::cout << "v1[" << j  << "]: " << v1[j] << std::endl;;
    std::cout << (v1[j] == exp[j]) << std::endl;;
  }
  std::cout << std::hex << *((uint64_t*)(&v1[0])) << std::endl;
  std::cout << std::hex << *((uint64_t*)(&exp[0])) << std::endl;
  std::cout << "v1.isOrdered(): " << v1.isOrdered() << std::endl;
  std::cout << "exp.isOrdered(): " << exp.isOrdered() << std::endl;
  ASSERT_TRUE(v1 == exp);
}  
TEST(vector_apply_2) {
  Vector<double> v1 {0,1,2,3,4,5,6,7,8,9};
  Vector<double> v2 {9,8,7,6,5,4,3,2,1,0};
  Vector<double> exp{9,9,9,9,9,9,9,9,9,9};
  v1.apply<std::plus<double>, Vector<double>>(v2);
  for (unsigned j=0; j<v1.size(); ++j) {
    std::cout << "v1[" << j  << "]: " << v1[j] << std::endl;;
  }
  ASSERT_TRUE(v1 == exp);
}  
template<typename T, typename U, typename V>
struct MultipliesSums {
  T operator()(T t, U u, V v) { return t * u + v; }
};
TEST(vector_apply_3) {
  Vector<double> v1 {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  Vector<int>    v2 {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
  Vector<bool>   v3 {true, false, true, false, true, false, true, false, true, false};
  Vector<double> exp{1, 8,15,18,21,20,19,14, 9, 0};
  v1.apply<MultipliesSums<double,int,bool>, Vector<int>, Vector<bool>>(v2, v3);
  for (unsigned j=0; j<v1.size(); ++j) {
    std::cout << "v1[" << j  << "]: " << v1[j] << std::endl;;
  }
  ASSERT_TRUE(v1 == exp);
}  


// -- flexible allocator tests
TEST(vector_constructor_basic_flexalloc) {
  const size_t sz = 10;
  Vector<double> v(sz,                    // n
                   0,                     // init value
                   std::make_unique<flexallocator>());   // allocator
  for (size_t i=0; i<sz; ++i) {
    setv(v, i, 3.0);
  }
  bool all3 = true;
  for (size_t i=0; i<sz; ++i) {
    if (v[i] != 3) {
      all3 = false;
      break;
    }
  }
  ASSERT_TRUE(all3);
}
TEST(vector_constructor_init_flexalloc) {
  const size_t sz = 10;
  Vector<double> v(sz, 3, std::make_unique<flexallocator>());
  bool all3 = true;
  for (size_t i=0; i<sz; ++i) {
    if (v[i] != 3) {
      all3 = false;
      break;
    }
  }
  ASSERT_TRUE(all3);
}
TEST(vector_constructor_vector_iter_flexalloc) {
  std::vector<double> sv{1,2,3,4,5,6,7,8,9,10};
  const size_t sz = 10;
  Vector<double> v(sv.begin(), sv.end(), std::make_unique<flexallocator>());
  bool allok = true;
  for (size_t i=0; i<sz; ++i) {
    if (v[i] != sv[i]) {
      allok = false;
      break;
    }
  }
  ASSERT_TRUE(allok);
}
TEST(vector_constructor_Vector_iter_flexalloc) {
  std::vector<double> sv{1,2,3,4,5,6,7,8,9,10};
  const size_t sz = 10;
  Vector<double> v1(sv.begin(), 
                    sv.end(),
                    std::make_unique<flexallocator>()); // allocator
  std::cout << "v1[2] is: " << v1[2] << std::endl; 
  Vector<double> v2(v1.cbegin(), 
                    v1.cend(),
                    std::make_unique<flexallocator>()); // allocator
  bool allok = true;
  for (size_t i=0; i<sz; ++i) {
    if (v2[i] != v1[i]) {
      allok = false;
      break;
    }
  }
  ASSERT_TRUE(allok);
}
TEST(vector_move_constructor_flexalloc) {
  std::vector<double> sv{1,2,3,4,5,6,7,8,9,10};
  Vector<double> v2(Vector<double>{1,2,3,4,5,6,7,8,9,10});
  bool allok = true;
  for (size_t i=0; i<sv.size(); ++i) {
    if (sv[i] != v2[i]) {
      allok = false;
      break;
    }
  }
  ASSERT_TRUE(allok);
}
TEST(vector_constructor_huge_fail_flexalloc) {
  const size_t sz = 1000L * 1000000000L;
  ASSERT_THROW(Vector<double>(sz,                    // n
                              0,                     // init value
                              std::make_unique<flexallocator>()),  // allocator
               std::system_error
               );
}
TEST(vector_resize_larger_flexalloc) {
  const size_t n = 2 * arr::VECTOR_INITIAL_ALLOC + 2;
  const double init = 10;
  Vector<double> v1(n, init, std::make_unique<flexallocator>());
  const size_t n2 = n*2;
  v1.resize(n2);
  for (size_t i=n; i<n2; ++i) {
    std::cout << "i: " << i << std::endl;
    setv(v1, i, init);
  }
  Vector<double> v2(n2, init);
  ASSERT_TRUE(v1 == v2);
}
TEST(vector_resize_larger_huge_fail_flexalloc) {
  const size_t n = 2 * arr::VECTOR_INITIAL_ALLOC + 2;
  const double init = 10;
  Vector<double> v1(n, init, std::make_unique<flexallocator>());
  const size_t sz = 1000L * 1000000000L;
  ASSERT_THROW(v1.resize(sz), std::system_error);
  ASSERT_TRUE(v1.size() == n);
}
TEST(vector_resize_smaller_flexalloc) {
  const size_t n = 2 * arr::VECTOR_INITIAL_ALLOC + 2;
  const size_t n2 = n*2;
  const double init = 10;
  Vector<double> v1(n2, init, std::make_unique<flexallocator>());
  v1.resize(n);
  Vector<double> v2(n, init);
  ASSERT_TRUE(v1 == v2);
}
TEST(vector_resize_smaller_at_offset_flexalloc) {
  const size_t n = 2 * arr::VECTOR_INITIAL_ALLOC + 2;
  const size_t n2 = n*2;
  const double init = 10;
  Vector<double> v1(n2, init, std::make_unique<flexallocator>());
  v1.resize(n, n);
  Vector<double> v2(n, init);
  ASSERT_TRUE(v1 == v2);
}
TEST(vector_resize_0_smaller_at_offset_flexalloc) {
  const size_t n = 2 * arr::VECTOR_INITIAL_ALLOC + 2;
  const size_t n2 = n*2;
  const double init = 10;
  Vector<double> v1(n2, init, std::make_unique<flexallocator>());
  v1.resize(0, n);

  Vector<double> v2(0, init);
  ASSERT_TRUE(v1 == v2);
}

int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
