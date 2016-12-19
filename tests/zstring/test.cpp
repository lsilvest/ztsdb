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
#include "string.hpp"

// constructors ------------------
TEST(zstring_constructor_null) {
  ASSERT_TRUE(arr::zstring().length() == 0U);
  ASSERT_TRUE(arr::zstring() == "");
}
TEST(zstring_constructor_char) {
  ASSERT_TRUE(arr::zstring("hello").length() == 5U);
  ASSERT_TRUE(std::string(arr::zstring("hello")) == std::string("hello"));
}
TEST(zstring_constructor_string) {
  ASSERT_TRUE(arr::zstring(std::string("hello")).length() == 5U);
  ASSERT_TRUE(arr::zstring(std::string("hello")) == arr::zstring("hello"));
}
TEST(zstring_copy_constructor) {
  ASSERT_TRUE(arr::zstring(arr::zstring("hello")).length() == 5U);
  ASSERT_TRUE(arr::zstring(arr::zstring("hello")) == arr::zstring("hello"));
}
// tests smaller size and over limit init LLL

// eq, ne ------------------------
// ostream -----------------------
// operator+ ---------------------
TEST(zstring_operator_plus_z_z) {
  arr::zstring h("hello ");
  arr::zstring w("world");
  arr::zstring hw("hello world");
  arr::zstring n;
  ASSERT_TRUE(h + w == hw);
  ASSERT_TRUE(n + w == w);
  ASSERT_TRUE(h + n == h);
}
TEST(zstring_operator_plus_char_z) {
  const char* h = "hello ";
  arr::zstring w("world");
  arr::zstring hw("hello world");
  arr::zstring n;
  ASSERT_TRUE(h + w == hw);
  ASSERT_TRUE(n + w == w);
  ASSERT_TRUE(h + n == h);
}
TEST(zstring_operator_plus_string_z) {
  std::string h("hello ");
  arr::zstring w("world");
  arr::zstring hw("hello world");
  arr::zstring n;
  ASSERT_TRUE(h + w == hw);
  ASSERT_TRUE(n + w == w);
  ASSERT_TRUE(h + n == arr::zstring(h));
}
// test the edge cases LLL

int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
