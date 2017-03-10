// -*- compile-command: "make -k test" -*-

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


#include <crpcut.hpp>
#include "cow_ptr.hpp"

// struct used to check the functionality of 'cow_ptr'
struct A {
  A(int a_p, int& b_p) : a(a_p), b(b_p) { std::cout << "A(" << a << "," << b << ")" << std::endl; }
  ~A() { ++b; }                 // so we have an easy way to check the
                                // destructor was called
  int a;
  int& b;
};


// constructors ------------------
TEST(cow_ptr_constructor_and_deref) {
  int check_a = 0;
  {
    auto a = arr::make_cow<A>(arr::NOFLAGS, 2, check_a);
    ASSERT_TRUE(a->a == 2);     // const access
    ASSERT_TRUE((*a).a == 2);   // const access
  }
  ASSERT_TRUE(check_a == 1);
}
TEST(cow_ptr_deref_no_copy) {
  int check_a = 0;
  {
    auto pa = new A(3, check_a);
    auto a = arr::cow_ptr<A>(false, pa);
    (*a).a = 4;   // non-const access, but only one instance so no copy 
    ASSERT_TRUE(a->a == 4);
    ASSERT_TRUE((*a).a == 4);
    ASSERT_TRUE(pa->a == 4);    // no copy!
  }
  ASSERT_TRUE(check_a == 1);
}
TEST(cow_ptr_arrow_deref_no_copy) {
  int check_a = 0;
  {
    auto pa = new A(3, check_a);
    auto a = arr::cow_ptr<A>(arr::NOFLAGS, pa);
    a->a = 4;   // non-const access, but only one instance so no copy 
    ASSERT_TRUE(a->a == 4);
    ASSERT_TRUE((*a).a == 4);
    ASSERT_TRUE(pa->a == 4);    // no copy!
  }
  ASSERT_TRUE(check_a == 1);
}
TEST(cow_ptr_arrow_deref_copy) {
  int check_ab = 0;
  {
    auto b = arr::make_cow<A>(arr::NOFLAGS, 3, check_ab);
    auto a = b;
    a->a = 2;   // non-const access, with multiple instances, so copy
    a->a = 3;   // unique again now, so no copy
    a->a = 4;   // still unique, so no copy
    ASSERT_TRUE(a->a == 4);
    ASSERT_TRUE((*a).a == 4);
    ASSERT_TRUE(b->a == 3);
    ASSERT_TRUE((*b).a == 3);
  }
  ASSERT_TRUE(check_ab == 2);   // destroyed in initial instance and in copy!
}
TEST(cow_ptr_star_deref_copy) {
  int check_ab = 0;
  {
    auto b = arr::make_cow<A>(arr::NOFLAGS, 3, check_ab);
    auto a = b;
    (*a).a = 2;   // non-const access, with multiple instances, so copy
    (*a).a = 3;   // unique again now, so no copy
    (*a).a = 4;   // still unique, so no copy
    ASSERT_TRUE(a->a == 4);
    ASSERT_TRUE((*a).a == 4);
    ASSERT_TRUE(b->a == 3);
    ASSERT_TRUE((*b).a == 3);
  }
  ASSERT_TRUE(check_ab == 2);   // destroyed in initial instance and in copy!
}
TEST(cow_ptr_star_locked) {
  int check_ab = 0;
  auto a = arr::make_cow<A>(arr::LOCKED, 3, check_ab);
  // this is OK because no copy is made as we have only one reference:
  (*a).a = 4;
  ASSERT_TRUE(a->a == 4);
  ASSERT_TRUE((*a).a == 4);
}
TEST(cow_ptr_star_locked_copy_attempt) {
  int check_ab = 0;
  auto a = arr::make_cow<A>(arr::LOCKED, 3, check_ab);
  const auto b = a;
  ASSERT_TRUE(b->a == b->a);
  ASSERT_TRUE((*b).a == (*b).a);
  // with 'b', we now have two references so non-const dereference will throw
  ASSERT_THROW((*a).a = 2, std::range_error, "cannot copy locked object");
  ASSERT_THROW(a->a = 2, std::range_error, "cannot copy locked object");
}
TEST(cow_ptr_star_const) {
  int check_ab = 0;
  auto a = arr::make_cow<A>(arr::CONSTREF, 3, check_ab);
  // even without one reference we can't non const dereference const:
  ASSERT_THROW((*a).a = 2, std::range_error, "cannot modify const object");
  ASSERT_THROW(a->a = 2, std::range_error, "cannot modify const object");
}
TEST(cow_ptr_last) {
  // verify last does not increase use_count
  int check_ab = 0;
  auto a = arr::make_cow<A>(arr::CONSTREF, 3, check_ab);
  auto b = a;
  auto c = b;
  ASSERT_TRUE(a.use_count() == 3);
  ASSERT_TRUE(b.use_count() == 3);
  ASSERT_TRUE(c.use_count() == 3);
  ASSERT_FALSE(a.isLast());
  ASSERT_FALSE(b.isLast());
  ASSERT_FALSE(c.isLast());
  a.setLast();
  ASSERT_TRUE(a.isLast());
  ASSERT_FALSE(b.isLast());
  ASSERT_FALSE(c.isLast());
  ASSERT_TRUE(a.use_count() == 2);
  ASSERT_TRUE(b.use_count() == 2);
  ASSERT_TRUE(c.use_count() == 2);
}
TEST(cow_ptr_const) {
  int check_ab = 0;
  auto a = arr::make_cow<A>(arr::NOFLAGS, 3, check_ab);
  auto b = a;
  ASSERT_TRUE(a.use_count() == 2);
  ASSERT_TRUE(b.use_count() == 2);
  ASSERT_FALSE(a.isConst());
  ASSERT_FALSE(b.isConst());
  b.setConst();
  ASSERT_TRUE(a.use_count() == 2);
  ASSERT_TRUE(b.use_count() == 2);
  ASSERT_FALSE(a.isConst());
  ASSERT_TRUE(b.isConst());
}


int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
