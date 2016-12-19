// -*- compile-command: "make -j -k -O test" -*-

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


#include <numeric>
#include <cmath>
#include <crpcut.hpp>
#include <ftw.h>
#include "array.hpp"
// #include "display.hpp"
#include "index.hpp"
#include "array_ops.hpp"
#include "../utils.hpp"
#include "timezone/ztime.hpp"
#include "pseudovector.hpp"
#include "pseudoarray.hpp"

// helper function for the creation of indices
namespace arr {

  template<typename T>
  static Index make_NullIndex(idx_type d, const Array<T>& t) {
    return NullIndex{t.dim[d]};
  }

  static Index make_IntIndex(const arr::Vector<size_t>& v) {
    return IntIndex(v);
  }

  static Index make_BoolIndex(const Vector<bool>& v) {
    return BoolIndex{v};
  }

  template<typename T>
  static Index make_NameIndex(idx_type d, const vector<string>& v, const Array<T>& t) {
    return NameIndex{v, *t.names[d]};
  }
}


// Dname tests
// -----------
TEST(Dmame_constructor_no_args) {
  arr::Dname dn;
  ASSERT_TRUE(dn.sz == 0U);
  ASSERT_THROW(dn[0], std::range_error, "subscript out of bounds");
  ASSERT_THROW(dn[""], std::range_error, "subscript out of bounds");
  ASSERT_THROW(dn["un"], std::range_error, "subscript out of bounds");
}
TEST(Dmame_constructor_no_names) {
  arr::Dname dn(5);
  ASSERT_TRUE(dn.sz == 5U);
  ASSERT_TRUE(dn[0] == ""); 
  ASSERT_TRUE(dn[1] == "");
  ASSERT_THROW(dn[5], std::range_error, "subscript out of bounds");
  ASSERT_THROW(dn[""], std::range_error, "subscript out of bounds");
  ASSERT_THROW(dn["un"], std::range_error, "subscript out of bounds");
}
TEST(Dmame_constructor) {
  arr::Dname dn(5, {"un","deux","trois","quatre","cinq"});
  ASSERT_TRUE(dn.sz == 5U);
  ASSERT_TRUE(dn[0] == "un");
  ASSERT_TRUE(dn[1] == "deux");
  ASSERT_TRUE(dn[2] == "trois");
  ASSERT_TRUE(dn[3] == "quatre");
  ASSERT_TRUE(dn[4] == "cinq");
  ASSERT_THROW(dn[5], std::range_error, "subscript out of bounds");
  ASSERT_TRUE(dn.sz == 5U);
  ASSERT_TRUE(dn["un"]     == 0U);
  ASSERT_TRUE(dn["deux"]   == 1U);
  ASSERT_TRUE(dn["trois"]  == 2U);
  ASSERT_TRUE(dn["quatre"] == 3U);
  ASSERT_TRUE(dn["cinq"]   == 4U);
  ASSERT_THROW(dn[""], std::range_error, "subscript out of bounds");
  ASSERT_THROW(dn["six"], std::range_error, "subscript out of bounds");
}
TEST(Dmame_constructor_length_names_mismatch) {
  ASSERT_THROW(arr::Dname dn(4, {"un","deux","trois","quatre","cinq"}),
               std::range_error,
               "length of 'dimnames' not equal to array extent");
}
TEST(Dmame_constructor_duplicate_names) {
  arr::Dname dn(5, {"un","deux","deux","un","un"});
  ASSERT_TRUE(dn.sz == 5U);
  ASSERT_TRUE(dn[0] == "un");
  ASSERT_TRUE(dn[1] == "deux");
  ASSERT_TRUE(dn[2] == "deux");
  ASSERT_TRUE(dn[3] == "un");
  ASSERT_TRUE(dn[4] == "un");
  ASSERT_THROW(dn[5], std::range_error, "subscript out of bounds");
  ASSERT_TRUE(dn.sz == 5U);
  ASSERT_TRUE(dn["un"]     == 0U);
  ASSERT_TRUE(dn["deux"]   == 1U);
  ASSERT_THROW(dn[""], std::range_error, "subscript out of bounds");
  ASSERT_THROW(dn["six"], std::range_error, "subscript out of bounds");
}
TEST(Dmame_addafter_string) {
  arr::Dname dn(3, {"un","deux","trois"});
  dn.addafter("quatre");
  dn.addafter("cinq");
  ASSERT_TRUE(dn.sz == 5U);
  ASSERT_TRUE(dn[0] == "un");
  ASSERT_TRUE(dn[1] == "deux");
  ASSERT_TRUE(dn[2] == "trois");
  ASSERT_TRUE(dn[3] == "quatre");
  ASSERT_TRUE(dn[4] == "cinq");
  ASSERT_THROW(dn[5], std::range_error, "subscript out of bounds");
  ASSERT_TRUE(dn.sz == 5U);
  ASSERT_TRUE(dn["un"]     == 0U);
  ASSERT_TRUE(dn["deux"]   == 1U);
  ASSERT_TRUE(dn["trois"]  == 2U);
  ASSERT_TRUE(dn["quatre"] == 3U);
  ASSERT_TRUE(dn["cinq"]   == 4U);
  ASSERT_THROW(dn[""], std::range_error, "subscript out of bounds");
  ASSERT_THROW(dn["six"], std::range_error, "subscript out of bounds");  
}
TEST(Dmame_addafter_string_missing_first) {
  arr::Dname dn(3);
  dn.addafter("quatre");
  dn.addafter("cinq");
  ASSERT_TRUE(dn.sz == 5U);
  ASSERT_TRUE(dn[0] == "");
  ASSERT_TRUE(dn[1] == "");
  ASSERT_TRUE(dn[2] == "");
  ASSERT_TRUE(dn[3] == "quatre");
  ASSERT_TRUE(dn[4] == "cinq");
  ASSERT_THROW(dn[5], std::range_error, "subscript out of bounds");
  ASSERT_TRUE(dn.sz == 5U);
  ASSERT_TRUE(dn["quatre"] == 3U);
  ASSERT_TRUE(dn["cinq"]   == 4U);
  ASSERT_THROW(dn[""], std::range_error, "subscript out of bounds");
  ASSERT_THROW(dn["six"], std::range_error, "subscript out of bounds");  
}
TEST(Dmame_addafter_dname) {
  arr::Dname dn(3, {"un","deux","trois"});
  arr::Dname dn2(2, {"quatre","cinq"});
  dn.addafter(dn2);
  ASSERT_TRUE(dn.sz == 5U);
  ASSERT_TRUE(dn[0] == "un");
  ASSERT_TRUE(dn[1] == "deux");
  ASSERT_TRUE(dn[2] == "trois");
  ASSERT_TRUE(dn[3] == "quatre");
  ASSERT_TRUE(dn[4] == "cinq");
  ASSERT_THROW(dn[5], std::range_error, "subscript out of bounds");
  ASSERT_TRUE(dn.sz == 5U);
  ASSERT_TRUE(dn["un"]     == 0U);
  ASSERT_TRUE(dn["deux"]   == 1U);
  ASSERT_TRUE(dn["trois"]  == 2U);
  ASSERT_TRUE(dn["quatre"] == 3U);
  ASSERT_TRUE(dn["cinq"]   == 4U);
  ASSERT_THROW(dn[""], std::range_error, "subscript out of bounds");
  ASSERT_THROW(dn["six"], std::range_error, "subscript out of bounds");  
}
TEST(Dmame_addafter_dname_missing_first) {
  arr::Dname dn(3);
  arr::Dname dn2(2, {"quatre","cinq"});
  dn.addafter(dn2);
  ASSERT_TRUE(dn.sz == 5U);
  ASSERT_TRUE(dn[0] == "");
  ASSERT_TRUE(dn[1] == "");
  ASSERT_TRUE(dn[2] == "");
  ASSERT_TRUE(dn[3] == "quatre");
  ASSERT_TRUE(dn[4] == "cinq");
  ASSERT_THROW(dn[5], std::range_error, "subscript out of bounds");
  ASSERT_TRUE(dn.sz == 5U);
  ASSERT_TRUE(dn["quatre"] == 3U);
  ASSERT_TRUE(dn["cinq"]   == 4U);
  ASSERT_THROW(dn[""], std::range_error, "subscript out of bounds");
  ASSERT_THROW(dn["six"], std::range_error, "subscript out of bounds");  
}
TEST(Dmame_addafter_dname_missing_second) {
  arr::Dname dn(3, {"un","deux","trois"});
  arr::Dname dn2(2);
  dn.addafter(dn2);
  ASSERT_TRUE(dn.sz == 5U);
  ASSERT_TRUE(dn[0] == "un");
  ASSERT_TRUE(dn[1] == "deux");
  ASSERT_TRUE(dn[2] == "trois");
  ASSERT_TRUE(dn[3] == "");
  ASSERT_TRUE(dn[4] == "");
  ASSERT_THROW(dn[5], std::range_error, "subscript out of bounds");
  ASSERT_TRUE(dn.sz == 5U);
  ASSERT_TRUE(dn["un"]     == 0U);
  ASSERT_TRUE(dn["deux"]   == 1U);
  ASSERT_TRUE(dn["trois"]  == 2U);
  ASSERT_THROW(dn[""], std::range_error, "subscript out of bounds");
  ASSERT_THROW(dn["six"], std::range_error, "subscript out of bounds");  
}
TEST(Dmame_remove_by_idx_first) {
  arr::Dname dn(5, {"un","deux","trois","quatre","cinq"});
  ASSERT_TRUE(dn.sz == 5U);
  dn.remove(0);
  ASSERT_TRUE(dn[0] == "deux");
  ASSERT_TRUE(dn[1] == "trois");
  ASSERT_TRUE(dn[2] == "quatre");
  ASSERT_TRUE(dn[3] == "cinq");
  ASSERT_THROW(dn[4], std::range_error, "subscript out of bounds");
  ASSERT_TRUE(dn.sz == 4U);
  ASSERT_TRUE(dn["deux"]   == 0U);
  ASSERT_TRUE(dn["trois"]  == 1U);
  ASSERT_TRUE(dn["quatre"] == 2U);
  ASSERT_TRUE(dn["cinq"]   == 3U);
  ASSERT_THROW(dn[""],   std::range_error, "subscript out of bounds");
  ASSERT_THROW(dn["un"], std::range_error, "subscript out of bounds");
}
TEST(Dmame_remove_by_idx_middle) {
  arr::Dname dn(5, {"un","deux","trois","quatre","cinq"});
  ASSERT_TRUE(dn.sz == 5U);
  dn.remove(2);
  ASSERT_TRUE(dn[0] == "un");
  ASSERT_TRUE(dn[1] == "deux");
  ASSERT_TRUE(dn[2] == "quatre");
  ASSERT_TRUE(dn[3] == "cinq");
  ASSERT_THROW(dn[4], std::range_error, "subscript out of bounds");
  ASSERT_TRUE(dn.sz == 4U);
  ASSERT_TRUE(dn["un"]     == 0U);
  ASSERT_TRUE(dn["deux"]   == 1U);
  ASSERT_TRUE(dn["quatre"] == 2U);
  ASSERT_TRUE(dn["cinq"]   == 3U);
  ASSERT_THROW(dn[""],      std::range_error, "subscript out of bounds");
  ASSERT_THROW(dn["trois"], std::range_error, "subscript out of bounds");
}
TEST(Dmame_remove_two_idx_end) {
  arr::Dname dn(5, {"un","deux","trois","quatre","cinq"});
  ASSERT_TRUE(dn.sz == 5U);
  dn.remove(4);
  dn.remove(3);
  ASSERT_TRUE(dn[0] == "un");
  ASSERT_TRUE(dn[1] == "deux");
  ASSERT_TRUE(dn[2] == "trois");
  ASSERT_THROW(dn[3], std::range_error, "subscript out of bounds");
  ASSERT_TRUE(dn.sz == 3U);
  ASSERT_TRUE(dn["un"]     == 0U);
  ASSERT_TRUE(dn["deux"]   == 1U);
  ASSERT_TRUE(dn["trois"]  == 2U);
  ASSERT_THROW(dn[""], std::range_error, "subscript out of bounds");
  ASSERT_THROW(dn["quatre"], std::range_error, "subscript out of bounds");
  ASSERT_THROW(dn["cinq"],   std::range_error, "subscript out of bounds");
}
TEST(Dmame_remove_by_name_first) {
  arr::Dname dn(5, {"un","deux","trois","quatre","cinq"});
  ASSERT_TRUE(dn.sz == 5U);
  dn.remove("un");
  ASSERT_TRUE(dn[0] == "deux");
  ASSERT_TRUE(dn[1] == "trois");
  ASSERT_TRUE(dn[2] == "quatre");
  ASSERT_TRUE(dn[3] == "cinq");
  ASSERT_THROW(dn[4], std::range_error, "subscript out of bounds");
  ASSERT_TRUE(dn.sz == 4U);
  ASSERT_TRUE(dn["deux"]   == 0U);
  ASSERT_TRUE(dn["trois"]  == 1U);
  ASSERT_TRUE(dn["quatre"] == 2U);
  ASSERT_TRUE(dn["cinq"]   == 3U);
  ASSERT_THROW(dn[""],   std::range_error, "subscript out of bounds");
  ASSERT_THROW(dn["un"], std::range_error, "subscript out of bounds");
}
TEST(Dmame_remove_by_name_middle) {
  arr::Dname dn(5, {"un","deux","trois","quatre","cinq"});
  ASSERT_TRUE(dn.sz == 5U);
  dn.remove("trois");
  ASSERT_TRUE(dn[0] == "un");
  ASSERT_TRUE(dn[1] == "deux");
  ASSERT_TRUE(dn[2] == "quatre");
  ASSERT_TRUE(dn[3] == "cinq");
  ASSERT_THROW(dn[4], std::range_error, "subscript out of bounds");
  ASSERT_TRUE(dn.sz == 4U);
  ASSERT_TRUE(dn["un"]     == 0U);
  ASSERT_TRUE(dn["deux"]   == 1U);
  ASSERT_TRUE(dn["quatre"] == 2U);
  ASSERT_TRUE(dn["cinq"]   == 3U);
  ASSERT_THROW(dn[""],      std::range_error, "subscript out of bounds");
  ASSERT_THROW(dn["trois"], std::range_error, "subscript out of bounds");
}
TEST(Dmame_remove_two_names_end) {
  arr::Dname dn(5, {"un","deux","trois","quatre","cinq"});
  ASSERT_TRUE(dn.sz == 5U);
  dn.remove("quatre");
  dn.remove("cinq");
  ASSERT_TRUE(dn[0] == "un");
  ASSERT_TRUE(dn[1] == "deux");
  ASSERT_TRUE(dn[2] == "trois");
  ASSERT_THROW(dn[3], std::range_error, "subscript out of bounds");
  ASSERT_TRUE(dn.sz == 3U);
  ASSERT_TRUE(dn["un"]     == 0U);
  ASSERT_TRUE(dn["deux"]   == 1U);
  ASSERT_TRUE(dn["trois"]  == 2U);
  ASSERT_THROW(dn[""], std::range_error, "subscript out of bounds");
  ASSERT_THROW(dn["quatre"], std::range_error, "subscript out of bounds");
  ASSERT_THROW(dn["cinq"],   std::range_error, "subscript out of bounds");
}
TEST(Dmame_remove_by_name_0_length) {
  arr::Dname dn(5, {"un","deux","trois","quatre","cinq"});
  ASSERT_THROW(dn.remove(""), std::range_error, "subscript out of bounds");
}
TEST(Dmame_equal_true) {
  arr::Dname dn1(5, {"un","deux","trois","quatre","cinq"});
  arr::Dname dn2(5, {"un","deux","trois","quatre","cinq"});
  ASSERT_TRUE(dn1 == dn2);
}
TEST(Dmame_equal_false) {
  arr::Dname dn1(5, {"un","deux","trois","quatre","cinq"});
  arr::Dname dn2(5, {"un","deux","trois","quatre","cin"});
  ASSERT_TRUE(!(dn1 == dn2));
}


// Array tests
// -----------
TEST(array_make_4_scalar_init_integer_seq_to) {
  auto a   = arr::Array<int>(arr::seq_to, 1, 4, 1);
  auto res = arr::Array<int>({4}, arr::Vector<int>{1,2,3,4});
  ASSERT_TRUE(a==res);   
}
TEST(array_make_4_scalar_init_double_seq_to) {
  auto a   = arr::Array<double>(arr::seq_to, 1, 4, 1);
  auto res = arr::Array<double>({4}, arr::Vector<double>{1,2,3,4});
  ASSERT_TRUE(a==res);   
}
TEST(array_make_4_scalar_init_integer_seq_n) {
  auto a   = arr::Array<int>(arr::seq_n, 1, 1, 4);
  auto res = arr::Array<int>({4}, arr::Vector<int>{1,2,3,4});
  ASSERT_TRUE(a==res);   
}
TEST(array_make_4_scalar_init_double_seq_n) {
  auto a   = arr::Array<double>(arr::seq_n, 1, 1, 4);
  auto res = arr::Array<double>({4}, arr::Vector<double>{1,2,3,4});
  ASSERT_TRUE(a==res);   
}
TEST(array_make_2x2_scalar_init) {
  auto a = arr::Array<double>({2,2}, arr::Vector<double>{2});
  for (arr::idx_type j=0; j<4; ++j) {
    ASSERT_TRUE(a[j] == 2);
  }
  ASSERT_TRUE((a[{0,0}] == 2));
  ASSERT_TRUE((a[{1,0}] == 2));
  ASSERT_TRUE((a[{0,1}] == 2));
  ASSERT_TRUE((a[{1,1}] == 2));
}
TEST(array_make_4x1_scalar_init) {
  auto a = arr::Array<double>({4,1}, arr::Vector<double>{2});
  for (arr::idx_type j=0; j<4; ++j) {
    ASSERT_TRUE(a[j] == 2);
  }
  ASSERT_TRUE((a[{0,0}] == 2));
  ASSERT_TRUE((a[{1,0}] == 2));
  ASSERT_TRUE((a[{2,0}] == 2));
  ASSERT_TRUE((a[{3,0}] == 2));
}
TEST(array_make_1x4_scalar_init) {
  auto a = arr::Array<double>({1,4}, arr::Vector<double>{2});
  for (arr::idx_type j=0; j<4; ++j) {
    ASSERT_TRUE(a[j] == 2);
  }
  ASSERT_TRUE((a[{0,0}] == 2));
  ASSERT_TRUE((a[{0,1}] == 2));

  ASSERT_TRUE((a[{0,2}] == 2));
  ASSERT_TRUE((a[{0,3}] == 2));
}
TEST(array_make_2x2x2_scalar_init) {
  auto a = arr::Array<double>({2,2,2}, arr::Vector<double>{2});
  for (arr::idx_type j=0; j<8; ++j) {
    ASSERT_TRUE(a[j] == 2);
  }
  ASSERT_TRUE((a[{0,0,0}] == 2));
  ASSERT_TRUE((a[{0,0,1}] == 2));
  ASSERT_TRUE((a[{0,1,0}] == 2));
  ASSERT_TRUE((a[{0,1,1}] == 2));
  ASSERT_TRUE((a[{1,0,0}] == 2));
  ASSERT_TRUE((a[{1,0,1}] == 2));
  ASSERT_TRUE((a[{1,1,0}] == 2));
  ASSERT_TRUE((a[{1,1,1}] == 2));
}
TEST(array_make_2x2_vector_init) {
  auto a = arr::Array<double>({2,2}, arr::Vector<double>{1,2,3,4});
  for (arr::idx_type j=0; j<4; ++j) {
    ASSERT_TRUE(a[j] == j+1);
  }
  ASSERT_TRUE((a[{0,0}] == 1));
  ASSERT_TRUE((a[{1,0}] == 2));
  ASSERT_TRUE((a[{0,1}] == 3));
  ASSERT_TRUE((a[{1,1}] == 4));
}
TEST(array_make_4x1_vector_init) {
  auto a = arr::Array<double>({4,1}, arr::Vector<double>{1,2,3,4});
  for (arr::idx_type j=0; j<4; ++j) {
    ASSERT_TRUE(a[j] == j+1);
  }
  ASSERT_TRUE((a[{0,0}] == 1));
  ASSERT_TRUE((a[{1,0}] == 2));
  ASSERT_TRUE((a[{2,0}] == 3));
  ASSERT_TRUE((a[{3,0}] == 4));
}
TEST(array_make_1x4_vector_init) {
  auto a = arr::Array<double>({1,4}, arr::Vector<double>{1,2,3,4});
  for (arr::idx_type j=0; j<4; ++j) {
    ASSERT_TRUE(a[j] == j+1);
  }
  ASSERT_TRUE((a[{0,0}] == 1));
  ASSERT_TRUE((a[{0,1}] == 2));
  ASSERT_TRUE((a[{0,2}] == 3));
  ASSERT_TRUE((a[{0,3}] == 4));
}
TEST(array_make_2x2x2_vector_init) {
  auto a = arr::Array<double>({2,2,2}, arr::Vector<double>{1,2,3,4,5,6,7,8});
  for (arr::idx_type j=0; j<8; ++j) {
    ASSERT_TRUE(a[j] == j+1);
  }
  ASSERT_TRUE((a[{0,0,0}] == 1));
  ASSERT_TRUE((a[{1,0,0}] == 2));
  ASSERT_TRUE((a[{0,1,0}] == 3));
  ASSERT_TRUE((a[{1,1,0}] == 4));
  ASSERT_TRUE((a[{0,0,1}] == 5));
  ASSERT_TRUE((a[{1,0,1}] == 6));
  ASSERT_TRUE((a[{0,1,1}] == 7));
  ASSERT_TRUE((a[{1,1,1}] == 8));
}
TEST(array_make_3x2x4_vector_init) {
  auto v = arr::Vector<unsigned>(24);
  std::iota(v.begin(), v.end(), 0);
  auto a = arr::Array<unsigned>({3,2,4}, v);
  for (arr::idx_type j=0; j<24; ++j) {
    ASSERT_TRUE(a[j] == j);
  }
  ASSERT_TRUE((a[{0,0,0}] == 0));
  ASSERT_TRUE((a[{1,0,0}] == 1));
  ASSERT_TRUE((a[{2,0,0}] == 2));
  ASSERT_TRUE((a[{0,1,0}] == 3));
  ASSERT_TRUE((a[{1,1,0}] == 4));
  ASSERT_TRUE((a[{2,1,0}] == 5));
  ASSERT_TRUE((a[{0,0,1}] == 6));
  ASSERT_TRUE((a[{1,0,1}] == 7));
  ASSERT_TRUE((a[{2,0,1}] == 8));
  ASSERT_TRUE((a[{0,1,1}] == 9));
  ASSERT_TRUE((a[{1,1,1}] == 10));
  ASSERT_TRUE((a[{2,1,1}] == 11));
  ASSERT_TRUE((a[{0,0,2}] == 12));
  ASSERT_TRUE((a[{1,0,2}] == 13));
  ASSERT_TRUE((a[{2,0,2}] == 14));
  ASSERT_TRUE((a[{0,1,2}] == 15));
  ASSERT_TRUE((a[{1,1,2}] == 16));
  ASSERT_TRUE((a[{2,1,2}] == 17));
  ASSERT_TRUE((a[{0,0,3}] == 18));
  ASSERT_TRUE((a[{1,0,3}] == 19));
  ASSERT_TRUE((a[{2,0,3}] == 20));
  ASSERT_TRUE((a[{0,1,3}] == 21));
  ASSERT_TRUE((a[{1,1,3}] == 22));
  ASSERT_TRUE((a[{2,1,3}] == 23));
}
TEST(array_0_dim_0) {
  auto a = arr::Array<double>(arr::rsv, arr::Vector<arr::idx_type>{0});
  ASSERT_THROW(a[0], std::range_error);
}
TEST(array_0x0_dim_0x0) {
  auto a = arr::Array<double>(arr::rsv, arr::Vector<arr::idx_type>{0,0});
  ASSERT_THROW(a[0], std::range_error);
}
TEST(array_4x1_names) {
  auto a = arr::Array<double>({4,1}, {1,2,3,4}, {{"un","deux","trois","quatre"}, {"one"}});
  for (arr::idx_type j=0; j<4; ++j) {
    ASSERT_TRUE(a[j] == j+1);
  }
  ASSERT_TRUE((a[{0,0}] == 1));
  ASSERT_TRUE((a[{1,0}] == 2));
  ASSERT_TRUE((a[{2,0}] == 3));
  ASSERT_TRUE((a[{3,0}] == 4));
}
TEST(array_1x4_names) {
  auto a = arr::Array<double>({1,4}, {1,2,3,4}, {{"un"}, {"one","two","three","four"}});
  for (arr::idx_type j=0; j<4; ++j) {
    ASSERT_TRUE(a[j] == j+1);
  }
  ASSERT_TRUE((a[{0,0}] == 1));
  ASSERT_TRUE((a[{0,1}] == 2));
  ASSERT_TRUE((a[{0,2}] == 3));
  ASSERT_TRUE((a[{0,3}] == 4));
}
TEST(array_2x2_names_vector_init) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  for (arr::idx_type j=0; j<4; ++j) {
    ASSERT_TRUE(a[j] == j+1);
  }
  ASSERT_TRUE((a[{0,0}] == 1));
  ASSERT_TRUE((a[{1,0}] == 2));
  ASSERT_TRUE((a[{0,1}] == 3));
  ASSERT_TRUE((a[{1,1}] == 4));
}
// names only one dimension:
TEST(array_2x2_names_vector_init_names_only_y) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{}, {"one", "two"}});
  for (arr::idx_type j=0; j<4; ++j) {
    ASSERT_TRUE(a[j] == j+1);
  }
  ASSERT_TRUE((a[{0,0}] == 1));
  ASSERT_TRUE((a[{1,0}] == 2));
  ASSERT_TRUE((a[{0,1}] == 3));
  ASSERT_TRUE((a[{1,1}] == 4));
}
TEST(array_2x2_names_vector_init_names_only_x) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, vector<arr::Vector<arr::zstring>>{{"un","deux"}, {}});
  for (arr::idx_type j=0; j<4; ++j) {
    ASSERT_TRUE(a[j] == j+1);
  }
  ASSERT_TRUE((a[{0,0}] == 1));
  ASSERT_TRUE((a[{1,0}] == 2));
  ASSERT_TRUE((a[{0,1}] == 3));
  ASSERT_TRUE((a[{1,1}] == 4));
}
// incorrect name lengths:
TEST(array_2x2_names_init_x_names_too_long) {
  ASSERT_THROW(arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux","trois"}, {"one", "two"}}),
               std::range_error);
} 
TEST(array_2x2_names_init_y_names_too_long) {
  ASSERT_THROW(arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one","two","three"}}),
               std::range_error);
} 
TEST(array_2x2_names_init_x_names_too_short) {
  ASSERT_THROW(arr::Array<double>({2,2}, {1,2,3,4}, {{"un"}, {"one", "two"}}),
               std::range_error);
} 
TEST(array_2x2_names_init_y_names_too_short) {
  ASSERT_THROW(arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one"}}),
               std::range_error);
} 
TEST(array_2x2_names_init_x_names_missing_dim) {
  ASSERT_THROW(arr::Array<double>(arr::Vector<arr::idx_type>{2,2}, 
                                  arr::Vector<double>{1,2,3,4}, 
                                  vector<arr::Vector<arr::zstring>>{{"un","deux"}}),
               std::range_error,
               "length of 'dimnames' [1] must match that of 'dims' [2]");
} 
TEST(array_2x2_names_init_x_names_additional_dim) {
  ASSERT_THROW(arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}, {"1", "2"}}),
               std::range_error,
               "length of 'dimnames' [3] must match that of 'dims' [2]");
}
// incorrect data length
TEST(array_incorrect_datalen) {
  ASSERT_THROW(arr::Array<double>({2,2}, {1,2,3}, {{"un"}, {"one", "two"}}),
               std::range_error, "data does not fit array dimensions");
  ASSERT_THROW(arr::Array<double>({2,2}, {1,2,3,4,5}, {{"un"}, {"one", "two"}}),
               std::range_error, "data does not fit array dimensions");
  ASSERT_THROW(arr::Array<double>({2}, {1,2,3}, {{"one", "two", "three"}}),
               std::range_error, "data does not fit array dimensions");
  ASSERT_THROW(arr::Array<double>({0}, arr::Vector<double>{1,2,3}),
               std::range_error, "data does not fit array dimensions");
}
// test equality operator:
TEST(array_1x4_equality) {
  auto a = arr::Array<double>({1,4}, arr::Vector<double>{1,2,3,4});
  auto b = arr::Array<double>({1,4}, arr::Vector<double>{1,2,3,4});
  ASSERT_TRUE(a==b);
}
TEST(array_3x2_to_2x3_equality) {
  auto a = arr::Array<double>({2,3}, arr::Vector<double>{1,2,3,4,5,6});
  auto b = arr::Array<double>({3,2}, arr::Vector<double>{1,2,3,4,5,6});
  ASSERT_FALSE(a==b);
}
TEST(array_1x4_equality_diff_content) {
  auto a = arr::Array<double>({1,4}, arr::Vector<double>{1,2,3,4});
  auto b = arr::Array<double>({1,4}, arr::Vector<double>{1,2,3,0});
  ASSERT_TRUE(!(a==b));
}
TEST(array_4x1_equality) {
  auto a = arr::Array<double>({4,1}, arr::Vector<double>{1,2,3,4});
  auto b = arr::Array<double>({4,1}, arr::Vector<double>{1,2,3,4});
  ASSERT_TRUE(a==b);
}
TEST(array_4x1_equality_diff_content) {
  auto a = arr::Array<double>({4,1}, arr::Vector<double>{1,2,3,4});
  auto b = arr::Array<double>({4,1}, arr::Vector<double>{1,2,3,0});
  ASSERT_TRUE(!(a==b));
}
TEST(array_2x4_equality) {
  auto a = arr::Array<double>({2,2}, arr::Vector<double>{1,2,3,4});
  auto b = arr::Array<double>({2,2}, arr::Vector<double>{1,2,3,4});
  ASSERT_TRUE(a==b);
}
TEST(array_2x4_equality_diff_content) {
  auto a = arr::Array<double>({2,2}, arr::Vector<double>{1,1,3,4});
  auto b = arr::Array<double>({2,2}, arr::Vector<double>{1,2,3,4});
  ASSERT_TRUE(!(a==b));
}
TEST(array_2x4_equality_diff_size) {
  auto a = arr::Array<double>({1,4}, arr::Vector<double>{1,2,3,4});
  auto b = arr::Array<double>({2,2}, arr::Vector<double>{1,2,3,4});
  ASSERT_TRUE(!(a==b));
}
TEST(array_2x4_equality_names) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  ASSERT_TRUE(a==b);
}
TEST(array_2x4_equality_diff_names) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "tw"}});
  ASSERT_TRUE(!(a==b));
}
// constructor with Vector&&:
TEST(array_mv_vector) {
  auto a = arr::array_from_vector(arr::Vector<double>{1,2,3,4});
  auto b = arr::Array<double>({4}, arr::Vector<double>{1,2,3,4});
  ASSERT_TRUE(a==b);
}

// 1D slices

// 2D slices
TEST(array_2x2_subset_null_null) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_NullIndex(1,a)});
  ASSERT_TRUE(a==b);
}
TEST(array_2x2_subset_fullidx_fullidx) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = a(vector<arr::Index>{arr::make_IntIndex({0UL,1UL}), arr::make_IntIndex({0L,1L})});
  ASSERT_TRUE(a==b);
}
TEST(array_2x2_subset_fullnames_fullnames) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = a(vector<arr::Index>{arr::make_NameIndex(0, {"un","deux"}, a), arr::make_NameIndex(1, {"one","two"}, a)});
  ASSERT_TRUE(a==b);
}
TEST(array_2x2_subset_fullbool_fullbool) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = a(vector<arr::Index>{arr::make_BoolIndex({true,true}), arr::make_BoolIndex({true,true})});
  ASSERT_TRUE(a==b);
}
TEST(array_2x2_subset_null_idx_col0) {
  auto a = arr::Array<double>({2,2}, arr::Vector<double>{1,2,3,4});
  auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_IntIndex({0})});
  auto res = arr::Array<double>({2}, arr::Vector<double>{1,2});
  ASSERT_TRUE(b==res);
}
TEST(array_2x2_subset_null_idx_col1) {
  auto a = arr::Array<double>({2,2}, arr::Vector<double>{1,2,3,4});
  auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_IntIndex({1})});
  auto res = arr::Array<double>({2}, arr::Vector<double>{3,4});
  ASSERT_TRUE(b==res);
}
TEST(array_2x2_subset_null_idx_col0_names) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_IntIndex({0})});
  auto res = arr::Array<double>({2}, arr::Vector<double>{1,2}, {{"un","deux"}});
  ASSERT_TRUE(b==res);
}
TEST(array_2x2_subset_null_string_col1_names) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_IntIndex({1})});
  auto res = arr::Array<double>({2}, arr::Vector<double>{3,4}, {{"un","deux"}});
  ASSERT_TRUE(b==res);
}
TEST(array_2x2_subset_null_string_col0_names) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_NameIndex(1, {"one"}, a)});
  auto res = arr::Array<double>({2}, arr::Vector<double>{1,2}, {{"un","deux"}});
  ASSERT_TRUE(b==res);
}
TEST(array_2x2_subset_null_bool_col0) {
  auto a = arr::Array<double>({2,2}, arr::Vector<double>{1,2,3,4});
  auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_BoolIndex({true,false})});
  auto res = arr::Array<double>({2}, arr::Vector<double>{1,2});
  ASSERT_TRUE(b==res);
}
TEST(array_2x2_subset_null_bool_col1) {
  auto a = arr::Array<double>({2,2}, arr::Vector<double>{1,2,3,4});
  auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_BoolIndex({false,true})});
  auto res = arr::Array<double>({2}, arr::Vector<double>{3,4});
  ASSERT_TRUE(b==res);
}
TEST(array_2x2_subset_null_bool_col0_names) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_BoolIndex({true,false})});
  auto res = arr::Array<double>({2}, arr::Vector<double>{1,2}, {{"un","deux"}});
  ASSERT_TRUE(b==res);
}
TEST(array_2x2_subset_null_bool_col1_names) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_BoolIndex({false,true})});
  auto res = arr::Array<double>({2}, arr::Vector<double>{3,4}, {{"un","deux"}});
  ASSERT_TRUE(b==res);
}
TEST(array_2x2_subset_bool_bool_names) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = a(vector<arr::Index>{arr::make_BoolIndex({false,true}), arr::make_BoolIndex({false,true})});
  auto res = arr::Array<double>({1}, arr::Vector<double>{4}, {{"deux"}});
  ASSERT_TRUE(b==res);
}
TEST(array_2x2_subset_null_time) {
  auto a = arr::Array<Global::dtime>({2,2}, {Global::dtime(1ns),Global::dtime(2ns),
        Global::dtime(3ns),Global::dtime(4ns)});  
  auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_NullIndex(1,a)});
  ASSERT_TRUE(a==b);
}
TEST(array_2x2_subset_time_time) {
  // LLL
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = a(vector<arr::Index>{arr::make_IntIndex({0L,1L}), arr::make_IntIndex({0L,1L})});
  ASSERT_TRUE(a==b);
}

// no drop cases:
TEST(array_2x2_subset_null_idx_col0_nodrop) {
  const auto a = arr::Array<double>({2,2}, arr::Vector<double>{1,2,3,4});
  auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_IntIndex({0})}, false);
  auto res = arr::Array<double>({2,1}, arr::Vector<double>{1,2});
  ASSERT_TRUE(b==res);
}
TEST(array_2x2_subset_null_idx_col1_nodrop) {
  const auto a = arr::Array<double>({2,2}, arr::Vector<double>{1,2,3,4});
  auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_IntIndex({1})}, false);
  auto res = arr::Array<double>({2,1}, arr::Vector<double>{3,4});
  ASSERT_TRUE(b==res);
}
TEST(array_2x2_subset_null_idx_col0_names_nodrop) {
  const auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_IntIndex({0})}, false);
  auto res = arr::Array<double>({2,1}, arr::Vector<double>{1,2}, {{"un","deux"}, {"one"}});
  ASSERT_TRUE(b==res);
}
TEST(array_2x2_subset_null_string_col1_names_nodrop) {
  const auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_IntIndex({1})}, false);
  auto res = arr::Array<double>({2,1}, arr::Vector<double>{3,4}, {{"un","deux"}, {"two"}});
  ASSERT_TRUE(b==res);
}
TEST(array_2x2_subset_null_string_col0_names_nodrop) {
  const auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_NameIndex(1, {"one"}, a)}, false);
  auto res = arr::Array<double>({2,1}, arr::Vector<double>{1,2}, {{"un","deux"}, {"one"}});
  ASSERT_TRUE(b==res);
}
TEST(array_2x2_subset_null_bool_col0_nodrop) {
  const auto a = arr::Array<double>({2,2}, arr::Vector<double>{1,2,3,4});
  auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_BoolIndex({true,false})}, false);
  auto res = arr::Array<double>({2,1}, arr::Vector<double>{1,2});
  ASSERT_TRUE(b==res);
}
TEST(array_2x2_subset_null_bool_col1_nodrop) {
  const auto a = arr::Array<double>({2,2}, arr::Vector<double>{1,2,3,4});
  auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_BoolIndex({false,true})}, false);
  auto res = arr::Array<double>({2,1}, arr::Vector<double>{3,4});
  ASSERT_TRUE(b==res);
}
TEST(array_2x2_subset_null_bool_col0_names_nodrop) {
  const auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_BoolIndex({true,false})}, false);
  auto res = arr::Array<double>({2,1}, arr::Vector<double>{1,2}, {{"un","deux"}, {"one"}});
  ASSERT_TRUE(b==res);
}
TEST(array_2x2_subset_null_bool_col1_names_nodrop) {
  const auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_BoolIndex({false,true})}, false);
  auto res = arr::Array<double>({2,1}, arr::Vector<double>{3,4}, {{"un","deux"}, {"two"}});
  ASSERT_TRUE(b==res);
}
TEST(array_2x2_subset_bool_bool_names_nodrop) {
  const auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = a(vector<arr::Index>{arr::make_BoolIndex({false,true}), arr::make_BoolIndex({false,true})}, false);
  auto res = arr::Array<double>({1,1}, arr::Vector<double>{4}, {{"deux"}, {"two"}});
  ASSERT_TRUE(b==res);
}
// dim > 2 slices: LLL
TEST(array_3x3x3_subset_null_null_null) {
  auto v = arr::Vector<double>(27);
  std::iota(v.begin(), v.end(), 0);
  auto a = arr::Array<double>({3,3,3}, v, {{"1","2","3"}, {"i","ii","iii"}, {"I","II","III"}});
  auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_NullIndex(1,a), arr::make_NullIndex(2,a)});
  ASSERT_TRUE(a==b);
}
TEST(array_3x3x3x3_subset_3x3x1x1) {
  auto v = arr::Vector<double>(81);
  std::iota(v.begin(), v.end(), 0);
  auto a = arr::Array<double>({3,3,3,3}, 
                              arr::Vector<double>(v.begin(), v.end()), 
    {{"1","2","3"}, {"i","ii","iii"},{},{}});
  auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), 
              arr::make_NullIndex(1,a), 
              arr::make_IntIndex({0L}), 
              arr::make_IntIndex({0L})});
  auto res = arr::Array<double>({3,3}, 
                                arr::Vector<double>(v.begin(), v.begin()+9), 
    {{"1","2","3"}, {"i","ii","iii"}});
  ASSERT_TRUE(b==res);
}
TEST(array_3x3x3x3_subset_3x3x1x1_nodrop) {
  auto v = arr::Vector<double>(81);
  std::iota(v.begin(), v.end(), 0);
  const auto a = arr::Array<double>({3,3,3,3}, 
                                    arr::Vector<double>(v.begin(), v.end()), 
    {{"1","2","3"}, {"i","ii","iii"}, {}, {}});
  auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), 
              arr::make_NullIndex(1,a), 
              arr::make_IntIndex({0L}), 
              arr::make_IntIndex({0L})},
              false);
  auto res = arr::Array<double>({3,3,1,1}, 
                              arr::Vector<double>(v.begin(), v.begin()+9), 
    {{"1","2","3"}, {"i","ii","iii"}, {}, {}});
  ASSERT_TRUE(b==res);
}

// subset assign --------
// non-scalar
// 1D
TEST(array_4_subset_assign_int_index_unordered) {
  auto a = arr::Array<double>({4}, {1,2,3,4}, {{"un","deux","trois","quatre"}});
  auto b = arr::Array<double>({4}, {5,3,4,2}, {{"un","deux","trois","quatre"}});
  auto c = a(vector<arr::Index>{arr::make_IntIndex({3,1,2,0})}, b);
  auto exp = arr::Array<double>({4}, {2,3,4,5}, {{"un","deux","trois","quatre"}});
  for (auto e : c.getcol(0)) std::cout << e << std::endl;
  ASSERT_TRUE(c==exp);
}

TEST(array_2x2_subset_assign_null_null) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = arr::Array<double>({2,2}, {2,3,4,5}, {{"uu","dddd"}, {"ooo", "ttt"}});
  auto c = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_NullIndex(1,a)}, b);
  auto d = arr::Array<double>({2,2}, {2,3,4,5}, {{"un","deux"}, {"one", "two"}});
  ASSERT_TRUE(c==d);
}
TEST(array_2x2_subset_assign_fullidx_fullidx) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = arr::Array<double>({2,2}, {2,3,4,5}, {{"uu","dddd"}, {"ooo", "ttt"}});
  auto c = a(vector<arr::Index>{arr::make_IntIndex({0L,1L}), arr::make_IntIndex({0L,1L})}, b);
  auto d = arr::Array<double>({2,2}, {2,3,4,5}, {{"un","deux"}, {"one", "two"}});
  ASSERT_TRUE(c==d);
}
TEST(array_2x2_subset_assign_fullnames_fullnames) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = arr::Array<double>({2,2}, {2,3,4,5}, {{"uu","dddd"}, {"ooo", "ttt"}});
  auto c = a(vector<arr::Index>{arr::make_NameIndex(0, {"un","deux"}, a), 
              arr::make_NameIndex(1, {"one","two"}, a)}, b);
  auto d = arr::Array<double>({2,2}, {2,3,4,5}, {{"un","deux"}, {"one", "two"}});
  ASSERT_TRUE(c==d);
}
TEST(array_2x2_subset_assign_fullbool_fullbool) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = arr::Array<double>({2,2}, {2,3,4,5}, {{"uu","dddd"}, {"ooo", "ttt"}});
  auto c = a(vector<arr::Index>{arr::make_BoolIndex({true,true}), arr::make_BoolIndex({true,true})}, b);
  auto d = arr::Array<double>({2,2}, {2,3,4,5}, {{"un","deux"}, {"one", "two"}});
  for (unsigned i=0; i<c.size(); ++i) cout << "c[" << i << "]: " << c[i] << std::endl;
  ASSERT_TRUE(c==d);
}
TEST(array_2x2_subset_assign_null_idx_col0) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = arr::Array<double>({2,1}, arr::Vector<double>{2,3}, {{"uu","dddd"}, {"ooo"}});
  auto c = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_IntIndex({0})}, b);
  auto d = arr::Array<double>({2,2}, {2,3,3,4}, {{"un","deux"}, {"one", "two"}});
  ASSERT_TRUE(c==d);
}
TEST(array_2x2_subset_assign_null_idx_col1) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = arr::Array<double>({2,1}, arr::Vector<double>{4,5}, {{"uu","dddd"}, {"ttt"}});
  auto c = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_IntIndex({1})}, b);
  auto d = arr::Array<double>({2,2}, {1,2,4,5}, {{"un","deux"}, {"one", "two"}});
  ASSERT_TRUE(c==d);
}
TEST(array_2x2_subset_assign_null_string_col0_names) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = arr::Array<double>({2,1}, arr::Vector<double>{2,3}, {{"uu","dddd"}, {"ooo"}});
  auto c = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_NameIndex(1, {"one"}, a)}, b);
  auto d = arr::Array<double>({2,2}, {2,3,3,4}, {{"un","deux"}, {"one", "two"}});
  ASSERT_TRUE(c==d);
}
TEST(array_2x2_subset_assign_null_string_col1) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = arr::Array<double>({2,1}, arr::Vector<double>{4,5}, {{"uu","dddd"}, {"ttt"}});
  auto c = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_NameIndex(1, {"two"}, a)}, b);
  auto d = arr::Array<double>({2,2}, {1,2,4,5}, {{"un","deux"}, {"one", "two"}});
  ASSERT_TRUE(c==d);
}
TEST(array_2x2_subset_assign_null_bool_col0) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = arr::Array<double>({2,1}, arr::Vector<double>{2,3}, {{"uu","dddd"}, {"ooo"}});
  auto c = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_BoolIndex({true,false})}, b);
  auto d = arr::Array<double>({2,2}, {2,3,3,4}, {{"un","deux"}, {"one", "two"}});
  ASSERT_TRUE(c==d);
}
TEST(array_2x2_subset_assign_null_bool_col1) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = arr::Array<double>({2,1}, arr::Vector<double>{4,5}, {{"uu","dddd"}, {"ttt"}});
  auto c = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_BoolIndex({false,true})}, b);
  auto d = arr::Array<double>({2,2}, {1,2,4,5}, {{"un","deux"}, {"one", "two"}});
  ASSERT_TRUE(c==d);
}
TEST(array_2x2_subset_assign_bool_bool) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = arr::Array<double>({1,1}, arr::Vector<double>{5}, {{"dddd"}, {"ttt"}});
  auto c = a(vector<arr::Index>{arr::make_BoolIndex({false,true}), arr::make_BoolIndex({false,true})}, b);
  auto d = arr::Array<double>({2,2}, {1,2,3,5}, {{"un","deux"}, {"one", "two"}});
  ASSERT_TRUE(c==d);
}

// scalar subassign -------
TEST(array_2x2_subset_assign_scalar_null_null) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto c = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_NullIndex(1,a)}, 1.0);
  auto d = arr::Array<double>({2,2}, {1,1,1,1}, {{"un","deux"}, {"one", "two"}});
  ASSERT_TRUE(c==d);
}
TEST(array_2x2_subset_assign_scalar_fullidx_fullidx) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = arr::Array<double>({2,2}, {2,3,4,5}, {{"uu","dddd"}, {"ooo", "ttt"}});
  auto c = a(vector<arr::Index>{arr::make_IntIndex({0L,1L}), arr::make_IntIndex({0L,1L})}, 1.0);
  auto d = arr::Array<double>({2,2}, {1,1,1,1}, {{"un","deux"}, {"one", "two"}});
  ASSERT_TRUE(c==d);
}
TEST(array_2x2_subset_assign_scalar_fullnames_fullnames) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = arr::Array<double>({2,2}, {2,3,4,5}, {{"uu","dddd"}, {"ooo", "ttt"}});
  auto c = a(vector<arr::Index>{arr::make_NameIndex(0, {"un","deux"}, a), 
              arr::make_NameIndex(1, {"one","two"}, a)}, 1.0);
  auto d = arr::Array<double>({2,2}, {1,1,1,1}, {{"un","deux"}, {"one", "two"}});
  ASSERT_TRUE(c==d);
}
TEST(array_2x2_subset_assign_scalar_fullbool_fullbool) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = arr::Array<double>({2,2}, {2,3,4,5}, {{"uu","dddd"}, {"ooo", "ttt"}});
  auto c = a(vector<arr::Index>{arr::make_BoolIndex({true,true}), arr::make_BoolIndex({true,true})}, 
             1.0);
  auto d = arr::Array<double>({2,2}, {1,1,1,1}, {{"un","deux"}, {"one", "two"}});
  for (unsigned i=0; i<c.size(); ++i) cout << "c[" << i << "]: " << c[i] << std::endl;
  ASSERT_TRUE(c==d);
}
TEST(array_2x2_subset_assign_scalar_null_idx_col0) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = arr::Array<double>({2,1}, arr::Vector<double>{2,3}, {{"uu","dddd"}, {"ooo"}});
  auto c = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_IntIndex({0})}, 1.0);
  auto d = arr::Array<double>({2,2}, {1,1,3,4}, {{"un","deux"}, {"one", "two"}});
  ASSERT_TRUE(c==d);
}
TEST(array_2x2_subset_assign_scalar_null_idx_col1) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = arr::Array<double>({2,1}, arr::Vector<double>{4,5}, {{"uu","dddd"}, {"ttt"}});
  auto c = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_IntIndex({1})}, 1.0);
  auto d = arr::Array<double>({2,2}, {1,2,1,1}, {{"un","deux"}, {"one", "two"}});
  ASSERT_TRUE(c==d);
}
TEST(array_2x2_subset_assign_scalar_null_string_col0_names) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = arr::Array<double>({2,1}, arr::Vector<double>{2,3}, {{"uu","dddd"}, {"ooo"}});
  auto c = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_NameIndex(1, {"one"}, a)}, 1.0);
  auto d = arr::Array<double>({2,2}, {1,1,3,4}, {{"un","deux"}, {"one", "two"}});
  ASSERT_TRUE(c==d);
}
TEST(array_2x2_subset_assign_scalar_null_string_col1) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = arr::Array<double>({2,1}, arr::Vector<double>{4,5}, {{"uu","dddd"}, {"ttt"}});
  auto c = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_NameIndex(1, {"two"}, a)}, 1.0);
  auto d = arr::Array<double>({2,2}, {1,2,1,1}, {{"un","deux"}, {"one", "two"}});
  ASSERT_TRUE(c==d);
}
TEST(array_2x2_subset_assign_scalar_null_bool_col0) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = arr::Array<double>({2,1}, arr::Vector<double>{2,3}, {{"uu","dddd"}, {"ooo"}});
  auto c = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_BoolIndex({true,false})}, 1.0);
  auto d = arr::Array<double>({2,2}, {1,1,3,4}, {{"un","deux"}, {"one", "two"}});
  ASSERT_TRUE(c==d);
}
TEST(array_2x2_subset_assign_scalar_null_bool_col1) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = arr::Array<double>({2,1}, arr::Vector<double>{4,5}, {{"uu","dddd"}, {"ttt"}});
  auto c = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_BoolIndex({false,true})}, 1.0);
  auto d = arr::Array<double>({2,2}, {1,2,1,1}, {{"un","deux"}, {"one", "two"}});
  ASSERT_TRUE(c==d);
}
TEST(array_2x2_subset_assign_scalar_bool_bool) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = arr::Array<double>({1,1}, arr::Vector<double>{5}, {{"dddd"}, {"ttt"}});
  auto c = a(vector<arr::Index>{arr::make_BoolIndex({false,true}), 
        arr::make_BoolIndex({false,true})},
    1.0);
  auto d = arr::Array<double>({2,2}, {1,2,3,1}, {{"un","deux"}, {"one", "two"}});
  ASSERT_TRUE(c==d);
}

// 3D subset assign -------
arr::Array<double> make4x3x2() {
  return arr::Array<double>({4,3,2}, 
    {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24}, 
    {{"un","deux","trois","quatre"}, {"one","two","three"}, {"uno","due"}});
}
arr::Array<double> make4x3x2_p1() {
  return arr::Array<double>({4,3,2}, 
    {2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25}, 
    {{"un","deux","trois","quatre"}, {"one","two","three"}, {"uno","due"}});
}
arr::Array<double> make3x2() {
  return arr::Array<double>({3,2,2}, {1,2,3,4,5,6,7,8,9,10,11,12});
}
arr::Array<double> make4x3x2_exp() {
  return arr::Array<double>({4,3,2}, 
    {1,2,3,4,5,1,2,3,9,4,5,6,13,14,15,16,17,7,8,9,21,10,11,12}, 
    {{"un","deux","trois","quatre"}, {"one","two","three"}, {"uno","due"}});
}
// non-scalar
TEST(array_4x3x2_subset_assign_3d_null_null) {
  auto a = make4x3x2();
  auto b = make4x3x2_p1();
  auto c = a(vector<arr::Index>{
      arr::make_NullIndex(0,a), 
      arr::make_NullIndex(1,a),
      arr::make_NullIndex(2,a)}, b);
  ASSERT_TRUE(c==b);
}
TEST(array_4x3x2_subset_assign_3d_fullidx_fullidx) {
  auto a = make4x3x2();
  auto b = make4x3x2_p1();
  auto c = a(vector<arr::Index>{
      arr::make_IntIndex({0,1,2,3}), 
      arr::make_IntIndex({0,1,2}),
      arr::make_IntIndex({0,1})}, b);
  for (unsigned i=0;i<c.size();++i) {
    std::cout << "c[" << i << "]: " << c[i] << ", b[" << i << "]: " << b[i] << std::endl;
  }
  ASSERT_TRUE(c==b);
}
TEST(array_4x3x2_subset_assign_3d_fullnames_fullnames) {
  auto a = make4x3x2();
  auto b = make4x3x2_p1();
  auto c = a(vector<arr::Index>{
      arr::make_NameIndex(0, {"un","deux","trois","quatre"}, a), 
      arr::make_NameIndex(1, {"one","two","three"}, a),
      arr::make_NameIndex(2, {"uno","due"}, a)}, b);
  ASSERT_TRUE(c==b);
}
TEST(array_4x3x2_subset_assign_3d_fullbool_fullbool) {
  auto a = make4x3x2();
  auto b = make4x3x2_p1();
  auto c = a(vector<arr::Index>{
      arr::make_BoolIndex({true,true,true,true}), 
      arr::make_BoolIndex({true,true,true}),
      arr::make_BoolIndex({true,true})}, b);
  ASSERT_TRUE(c==b);
}
TEST(array_4x3x2_subset_assign_3d_int) {
  auto a = make4x3x2();
  auto b = make3x2();
  auto exp = make4x3x2_exp();
  auto c = a(vector<arr::Index>{
      arr::make_IntIndex({1,2,3}), 
      arr::make_IntIndex({1,2}),
      arr::make_IntIndex({0,1})}, b);
  for (unsigned i=0;i<c.size();++i) {
    std::cout << "c[" << i << "]: " << c[i] << ", exp[" << i << "]: " << exp[i] << std::endl;
  }
  ASSERT_TRUE(c==exp);
}
TEST(array_4x3x2_subset_assign_3d_negint) {
  auto a = make4x3x2();
  auto b = make3x2();
  auto exp = make4x3x2_exp();
  auto c = a(vector<arr::Index>{
      arr::IntIndexNeg({0}, 4), 
      arr::IntIndexNeg({0}, 3),
      arr::IntIndexNeg({}, 2)}, b);
  ASSERT_TRUE(c==exp);
}
TEST(array_4x3x2_subset_assign_3d_names) {
  auto a = make4x3x2();
  auto b = make3x2();
  auto exp = make4x3x2_exp();
  auto c = a(vector<arr::Index>{
      arr::make_NameIndex(0, {"deux","trois","quatre"}, a), 
      arr::make_NameIndex(1, {"two","three"}, a),
      arr::make_NameIndex(2, {"uno","due"}, a)}, b);
  ASSERT_TRUE(c==exp);
}
TEST(array_4x3x2_subset_assign_3d_bool) {
  auto a = make4x3x2();
  auto b = make3x2();
  auto exp = make4x3x2_exp();
  auto c = a(vector<arr::Index>{
      arr::make_BoolIndex({false,true,true,true}), 
      arr::make_BoolIndex({false,true,true}),
      arr::make_BoolIndex({true,true})}, b);
  ASSERT_TRUE(c==exp);
}

// scalar
arr::Array<double> make4x3x2_scalar_exp() {
  return arr::Array<double>({4,3,2}, 
    {1,2,3,4,5,1,1,1,9,1,1,1,13,14,15,16,17,1,1,1,21,1,1,1}, 
    {{"un","deux","trois","quatre"}, {"one","two","three"}, {"uno","due"}});
}
arr::Array<double> make4x3x2_scalar_exp_1() {
  return arr::Array<double>({4,3,2}, 
    {1,1,1,1,1,1, 1,1,1,1,1,1, 1,1,1,1,1,1, 1,1,1,1,1,1}, 
    {{"un","deux","trois","quatre"}, {"one","two","three"}, {"uno","due"}});
}
TEST(array_4x3x2_subset_assign_scalar_3d_null_null) {
  auto a = make4x3x2();
  auto b = make4x3x2_scalar_exp_1();
  auto c = a(vector<arr::Index>{
      arr::make_NullIndex(0,a), 
      arr::make_NullIndex(1,a),
      arr::make_NullIndex(2,a)}, 1.0);
  ASSERT_TRUE(c==b);
}
TEST(array_4x3x2_subset_assign_scalar_3d_fullidx_fullidx) {
  auto a = make4x3x2();
  auto b = make4x3x2_scalar_exp_1();
  auto c = a(vector<arr::Index>{
      arr::make_IntIndex({0,1,2,3}), 
      arr::make_IntIndex({0,1,2}),
      arr::make_IntIndex({0,1})}, 1.0);
  ASSERT_TRUE(c==b);
}
TEST(array_4x3x2_subset_assign_scalar_3d_fullnames_fullnames) {
  auto a = make4x3x2();
  auto b = make4x3x2_scalar_exp_1();
  auto c = a(vector<arr::Index>{
      arr::make_NameIndex(0, {"un","deux","trois","quatre"}, a), 
      arr::make_NameIndex(1, {"one","two","three"}, a),
      arr::make_NameIndex(2, {"uno","due"}, a)}, 1.0);
  ASSERT_TRUE(c==b);
}
TEST(array_4x3x2_subset_assign_scalar_3d_fullbool_fullbool) {
  auto a = make4x3x2();
  auto b = make4x3x2_scalar_exp_1();
  auto c = a(vector<arr::Index>{
      arr::make_BoolIndex({true,true,true,true}), 
      arr::make_BoolIndex({true,true,true}),
      arr::make_BoolIndex({true,true})}, 1.0);
  ASSERT_TRUE(c==b);
}
TEST(array_4x3x2_subset_assign_scalar_3d_int) {
  auto a = make4x3x2();
  auto b = make3x2();
  auto exp = make4x3x2_scalar_exp();
  auto c = a(vector<arr::Index>{
      arr::make_IntIndex({1,2,3}), 
      arr::make_IntIndex({1,2}),
      arr::make_IntIndex({0,1})}, 1.0);
  for (unsigned i=0;i<c.size();++i) {
    std::cout << "c[" << i << "]: " << c[i] << ", exp[" << i << "]: " << exp[i] << std::endl;
  }
  ASSERT_TRUE(c==exp);
}
TEST(array_4x3x2_subset_assign_scalar_3d_negint) {
  auto a = make4x3x2();
  auto b = make3x2();
  auto exp = make4x3x2_scalar_exp();
  auto c = a(vector<arr::Index>{
      arr::IntIndexNeg({0}, 4), 
      arr::IntIndexNeg({0}, 3),
      arr::IntIndexNeg({}, 2)}, 1.0);
  ASSERT_TRUE(c==exp);
}
TEST(array_4x3x2_subset_assign_scalar_3d_names) {
  auto a = make4x3x2();
  auto b = make3x2();
  auto exp = make4x3x2_scalar_exp();
  auto c = a(vector<arr::Index>{
      arr::make_NameIndex(0, {"deux","trois","quatre"}, a), 
      arr::make_NameIndex(1, {"two","three"}, a),
      arr::make_NameIndex(2, {"uno","due"}, a)}, 1.0);
  ASSERT_TRUE(c==exp);
}
TEST(array_4x3x2_subset_assign_scalar_3d_bool) {
  auto a = make4x3x2();
  auto b = make3x2();
  auto exp = make4x3x2_scalar_exp();
  auto c = a(vector<arr::Index>{
      arr::make_BoolIndex({false,true,true,true}), 
      arr::make_BoolIndex({false,true,true}),
      arr::make_BoolIndex({true,true})}, 1.0);
  ASSERT_TRUE(c==exp);
}


// wrong dims subset assign ------- LLL
// type conversions subset assign -------- LLL

// copy ---------------
TEST(array_3x3x3_copy_assignment) {
  auto v = arr::Vector<double>(27);
  std::iota(v.begin(), v.end(), 0);
  auto a = arr::Array<double>({3,3,3}, v, {{"1","2","3"}, {"i","ii","iii"}, {"I","II","III"}});
  auto b = arr::Array<double>(arr::rsv, arr::Vector<arr::idx_type>{3,3,3});
  b = a;
  ASSERT_TRUE(a==b);
}
TEST(array_3x3x3_copy_constructor) {
  auto v = arr::Vector<double>(27);
  std::iota(v.begin(), v.end(), 0);
  auto a = arr::Array<double>({3,3,3}, v, {{"1","2","3"}, {"i","ii","iii"}, {"I","II","III"}});
  auto b = a;
  ASSERT_TRUE(a==b);
}
TEST(array_3x3x3_copy_constructor_rvalue) {
  auto v = arr::Vector<double>(27);
  std::iota(v.begin(), v.end(), 0);
  auto a = arr::Array<double>({3,3,3}, v, {{"1","2","3"}, {"i","ii","iii"}, {"I","II","III"}});
  auto b = arr::Array<double>(arr::Array<double>({3,3,3}, v, 
    {{"1","2","3"}, {"i","ii","iii"}, {"I","II","III"}}));
  ASSERT_TRUE(a==b);
}

// // deletion LLL
// scalar arithmetic --------------
TEST(array_3x3x3_scalar_plus) {
  auto v = arr::Vector<double>(27);
  std::iota(v.begin(), v.end(), 0);
  auto a = arr::Array<double>({3,3,3}, v, {{"1","2","3"}, {"i","ii","iii"}, {"I","II","III"}});
  auto b = a;
  for (arr::idx_type i=0; i<27; ++i) {
    setv(b, i, b[i] + 3);
  }
  auto k = arr::apply_scalar<double, double, double, std::plus<double>>(3, a);
  std::cout << "b[0] = " << b[0] << std::endl;
  std::cout << "b[1] = " << b[1] << std::endl;
  std::cout << "b[2] = " << b[2] << std::endl;
  std::cout << "b[3] = " << b[3] << std::endl;
  std::cout << "b[9] = " << b[9] << std::endl;
  cout << "k.size(): " << k.size() << std::endl;

  ASSERT_TRUE((arr::apply_scalar<double, double, double, std::plus<double>>(3, a))==b);
}
TEST(array_3x3x3_scalar_minus) {
  auto v = arr::Vector<double>(27);
  std::iota(v.begin(), v.end(), 0);
  auto a = arr::Array<double>({3,3,3}, v, {{"1","2","3"}, {"i","ii","iii"}, {"I","II","III"}});
  auto b = a;
  for (arr::idx_type i=0; i<27; ++i) {
    setv(b, i, 3 - b[i]);
  }
  ASSERT_TRUE((arr::apply_scalar<double, double, double, std::minus<double>>(3, a))==b);
}
TEST(array_3x3x3_scalar_mult) {
  auto v = arr::Vector<double>(27);
  std::iota(v.begin(), v.end(), 0);
  auto a = arr::Array<double>({3,3,3}, v, {{"1","2","3"}, {"i","ii","iii"}, {"I","II","III"}});
  auto b = a;
  for (arr::idx_type i=0; i<27; ++i) {
    setv(b, i, b[i] * 3);
  }
  ASSERT_TRUE((arr::apply_scalar<double, double, double, std::multiplies<double>>(3, a))==b);
}
TEST(array_3x3x3_scalar_div) {
  auto v = arr::Vector<double>(27);
  std::iota(v.begin(), v.end(), 0);
  auto a = arr::Array<double>({3,3,3}, v, {{"1","2","3"}, {"i","ii","iii"}, {"I","II","III"}});
  auto b = a;
  for (arr::idx_type i=0; i<27; ++i) {
    setv(b, i, 3 / b[i]);
  }
  ASSERT_TRUE((arr::apply_scalar<double, double, double, std::divides<double>>(3, a))==b);
}


// // cbind, rbind, abind
TEST(array_2x2_rbind) {
  auto v1 = arr::Vector<double>(12);
  std::iota(v1.begin(), v1.end(), 0);
  auto a = arr::Array<double>({2,3,2}, v1, {{"1","2"}, {"i","ii","iii"}, {"I","II"}});
  auto v2 = arr::Vector<double>{0,1,0,1,2,3,2,3,4,5,4,5,6,7,6,7,8,9,8,9,10,11,10,11};
  auto b = arr::Array<double>({4,3,2}, v2, {{"1","2","1","2"}, {"i","ii","iii"}, {"I","II"}});
  // cout << val::display(arr::make_cow<val::VArrayD>(false, b)) << endl; 
  ASSERT_TRUE(a.rbind(arr::Array<double>(a))==b);
}
TEST(array_2x2_rbind_first_no_names) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{},            {"one", "two"}});
  auto b = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto c = arr::Array<double>({4,2}, {1,2,1,2,3,4,3,4}, {{"","","un","deux"}, {"one","two"}});
  ASSERT_TRUE(a.rbind(b)==c);
}
TEST(array_2x2_rbind_second_no_names) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = arr::Array<double>({2,2}, {1,2,3,4}, {{},            {"one", "two"}});
  auto c = arr::Array<double>({4,2}, {1,2,1,2,3,4,3,4}, {{"un","deux","", ""}, {"one","two"}});
  ASSERT_TRUE(a.rbind(b)==c);
}
TEST(array_2x2_cbind) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = arr::Array<double>({2,4}, {1,2,3,4,1,2,3,4}, {{"un","deux"}, {"one","two","one","two"}});
  ASSERT_TRUE(a.cbind(arr::Array<double>(a))==b);
}
TEST(array_2x2_cbind_to_self) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  ASSERT_THROW(a.cbind(a), range_error, "cannot bind to self.");
}
TEST(array_2x2_cbind_diff_names) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = arr::Array<double>({2,2}, {1,2,3,4}, {{"1", "2"},    {"one", "two"}});
  auto c = arr::Array<double>({2,4}, {1,2,3,4,1,2,3,4}, {{"un","deux"}, {"one","two","one","two"}});
  ASSERT_TRUE(a.cbind(b)==c);
}
TEST(array_2x2_cbind_first_no_names) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{},            {"one", "two"}});
  auto b = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto c = arr::Array<double>({2,4}, {1,2,3,4,1,2,3,4}, {{"un","deux"}, {"one","two","one","two"}});
  ASSERT_TRUE(a.cbind(b)==c);
}
TEST(array_2x2_cbind_second_no_names) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  auto b = arr::Array<double>({2,2}, {1,2,3,4}, {{},            {"one", "two"}});
  auto c = arr::Array<double>({2,4}, {1,2,3,4,1,2,3,4}, {{"un","deux"}, {"one","two","one","two"}});
  ASSERT_TRUE(a.cbind(b)==c);
}
// test 0 dimensions LLL
TEST(array_rbind_null_0x0) {
  auto a = arr::Array<double>({}, arr::Vector<double>{}, vector<arr::Vector<arr::zstring>>{});
  auto b = arr::Array<double>({0,0}, arr::Vector<double>{}, vector<arr::Vector<arr::zstring>>{{}, {}});
  ASSERT_TRUE(a.rbind(b)==b);
}
TEST(array_rbind_null_1x0) {
  auto a = arr::Array<double>({}, arr::Vector<double>{}, vector<arr::Vector<arr::zstring>>{});
  auto b = arr::Array<double>({1,0}, {}, vector<arr::Vector<arr::zstring>>{{"one"}, {}});
  ASSERT_TRUE(a.rbind(b)==b);
}
TEST(array_rbind_null_1x1) {
  auto a = arr::Array<double>({}, arr::Vector<double>{}, vector<arr::Vector<arr::zstring>>{});
  auto b = arr::Array<double>({1,1}, arr::Vector<double>{1.1}, {{"one"}, {"1"}});
  ASSERT_TRUE(a.rbind(b)==b);
}
TEST(array_rbind_null_2x2) {
  auto a = arr::Array<double>({}, arr::Vector<double>{}, vector<arr::Vector<arr::zstring>>{});
  auto b = arr::Array<double>({2,2}, {1,2,3,4}, {{"one","two"}, {"1","2"}});
  ASSERT_TRUE(a.rbind(b)==b);
}
TEST(array_cbind_null_0x0) {
  auto a = arr::Array<double>({}, arr::Vector<double>{}, vector<arr::Vector<arr::zstring>>{});
  auto b = arr::Array<double>({0,0}, {}, vector<arr::Vector<arr::zstring>>{{}, {}});
  ASSERT_TRUE(a.cbind(b)==b);
}
TEST(array_cbind_null_1x0) {
  auto a = arr::Array<double>({}, arr::Vector<double>{}, vector<arr::Vector<arr::zstring>>{});
  auto b = arr::Array<double>({1,0}, {}, vector<arr::Vector<arr::zstring>>{{"one"}, {}});
  ASSERT_TRUE(a.cbind(b)==b);
}
TEST(array_cbind_null_1x1) {
  auto a = arr::Array<double>({}, arr::Vector<double>{}, vector<arr::Vector<arr::zstring>>{});
  auto b = arr::Array<double>({1,1}, arr::Vector<double>{1.1}, {{"one"}, {"1"}});
  ASSERT_TRUE(a.cbind(b)==b);
}
TEST(array_cbind_null_2x2) {
  auto a = arr::Array<double>({}, arr::Vector<double>{}, vector<arr::Vector<arr::zstring>>{});
  auto b = arr::Array<double>({2,2}, {1,2,3,4}, 
                              vector<arr::Vector<arr::zstring>>{{"one","two"}, {"1","2"}});
  ASSERT_TRUE(a.cbind(b)==b);
}
// test one or more dimensions == 0
TEST(array_rbind_0x0_0x0) {
  auto a = arr::Array<double>({0,0}, {}, vector<arr::Vector<arr::zstring>>{{}, {}});
  auto b = arr::Array<double>({0,0}, {}, vector<arr::Vector<arr::zstring>>{{}, {}});
  ASSERT_TRUE(a.rbind(b)==b);
}
TEST(array_rbind_0x0_1x0) {
  auto a = arr::Array<double>({0,0}, {}, vector<arr::Vector<arr::zstring>>{{}, {}});
  auto b = arr::Array<double>({1,0}, {}, vector<arr::Vector<arr::zstring>>{{"two"}, {}});
  auto c = arr::Array<double>({1,0}, {}, vector<arr::Vector<arr::zstring>>{{"two"}, {}});
  ASSERT_TRUE(a.rbind(b)==c);
}
TEST(array_rbind_1x0_1x0) {
  auto a = arr::Array<double>({1,0}, {}, vector<arr::Vector<arr::zstring>>{{"one"}, {}});
  auto b = arr::Array<double>({1,0}, {}, vector<arr::Vector<arr::zstring>>{{"two"}, {}});
  auto c = arr::Array<double>({2,0}, {}, vector<arr::Vector<arr::zstring>>{{"one", "two"}, {}});
  ASSERT_TRUE(a.rbind(b)==c);
}
TEST(array_rbind_1x0_1x1) {
  auto a = arr::Array<double>({0,1}, {}, vector<arr::Vector<arr::zstring>>{{}, {"one"}});
  auto b = arr::Array<double>({1,1}, arr::Vector<double>{1.1}, {{"1"}, {}});
  auto c = arr::Array<double>({1,1}, arr::Vector<double>{1.1}, {{"1"}, {"one"}});
  ASSERT_TRUE(a.rbind(b)==c);
}
TEST(array_rbind_1x0_2x0) {
  auto a = arr::Array<double>({1,0}, {}, vector<arr::Vector<arr::zstring>>{{"one"}, {}});
  auto b = arr::Array<double>({2,0}, arr::Vector<double>{}, {{"two", "three"}, {}});
  auto c = arr::Array<double>({3,0}, arr::Vector<double>{}, {{"one", "two", "three"}, {}});
  ASSERT_TRUE(a.rbind(b)==c);
}
TEST(array_cbind_0x0_0x1) {
  auto a = arr::Array<double>({0,0}, {}, vector<arr::Vector<arr::zstring>>{{}, {}});
  auto b = arr::Array<double>({0,1}, {}, vector<arr::Vector<arr::zstring>>{{}, {"two"}});
  auto c = arr::Array<double>({0,1}, {}, vector<arr::Vector<arr::zstring>>{{}, {"two"}});
  ASSERT_TRUE(a.cbind(b)==c);
}
TEST(array_cbind_0x1_0x1) {
  auto a = arr::Array<double>({0,1}, {}, vector<arr::Vector<arr::zstring>>{{}, {"one"}});
  auto b = arr::Array<double>({0,1}, {}, vector<arr::Vector<arr::zstring>>{{}, {"two"}});
  auto c = arr::Array<double>({0,2}, {}, vector<arr::Vector<arr::zstring>>{{}, {"one", "two"}});
  ASSERT_TRUE(a.cbind(b)==c);
}
TEST(array_cbind_0x1_0x2) {
  auto a = arr::Array<double>({0,1}, {}, vector<arr::Vector<arr::zstring>>{{}, {"one"}});
  auto b = arr::Array<double>({0,2}, {}, vector<arr::Vector<arr::zstring>>{{}, {"two", "three"}});
  auto c = arr::Array<double>({0,3}, {}, vector<arr::Vector<arr::zstring>>{{}, 
                                             {"one", "two", "three"}});
  ASSERT_TRUE(a.cbind(b)==c);
}
TEST(array_cbind_1x0_1x1) {
  auto a = arr::Array<double>({1,0}, {}, vector<arr::Vector<arr::zstring>>{{"one"}, {}});
  auto b = arr::Array<double>({1,1}, arr::Vector<double>{1.1}, 
    {{"won't be used as 'a' has a named row"}, {"1"}});
  auto c = arr::Array<double>({1,1}, arr::Vector<double>{1.1}, 
                              vector<arr::Vector<arr::zstring>>{{"one"}, {"1"}});
  ASSERT_TRUE(a.cbind(b)==c);
}
TEST(array_abind_0x0x0_0x0x1) {
  auto a = arr::Array<double>({0,0,0}, {}, vector<arr::Vector<arr::zstring>>{{},{},{}});
  auto b = arr::Array<double>({0,0,1}, {}, vector<arr::Vector<arr::zstring>>{{}, {}, {"two"}});
  auto c = arr::Array<double>({0,0,1}, {}, vector<arr::Vector<arr::zstring>>{{}, {}, {"two"}});
  ASSERT_TRUE(a.abind(b, 2)==c);
}
TEST(array_abind_0x0x1_0x0x1) {
  auto a = arr::Array<double>({0,0,1}, {}, vector<arr::Vector<arr::zstring>>{{}, {}, {"one"}});
  auto b = arr::Array<double>({0,0,1}, {}, vector<arr::Vector<arr::zstring>>{{}, {}, {"two"}});
  auto c = arr::Array<double>({0,0,2}, {}, vector<arr::Vector<arr::zstring>>{{}, {}, {"one", "two"}});
  ASSERT_TRUE(a.abind(b, 2)==c);
}
TEST(array_abind_0x0x1_0x0x2) {
  auto a = arr::Array<double>({0,0,1}, {}, vector<arr::Vector<arr::zstring>>{{}, {}, {"one"}});
  auto b = arr::Array<double>({0,0,2}, {}, 
                              vector<arr::Vector<arr::zstring>>{{}, {}, {"two", "three"}});
  auto c = arr::Array<double>({0,0,3}, {}, 
                              vector<arr::Vector<arr::zstring>>{{}, {}, {"one", "two", "three"}});
  ASSERT_TRUE(a.abind(b, 2)==c);
}
// test extension when dim > a.size() LLL

TEST(array_3x3x3_vectorize) {
  auto v = arr::Vector<double>(27);
  std::iota(v.begin(), v.end(), 0);
  auto a = arr::Array<double>({3,3,3}, v, {{"1","2","3"}, {"i","ii","iii"}, {"I","II","III"}});
  auto b = arr::Array<double>({27}, v);
  ASSERT_TRUE(vectorize(a)==b);
}

// test all the applies
TEST(array_apply_scalar_post) {
  auto a = arr::Array<double>({1,4}, arr::Vector<double>{1,2,3,4});  
  auto b = arr::Array<double>({1,4}, arr::Vector<double>{3,4,5,6});  
  auto res = arr::apply_scalar<double, double, double, std::plus<double>>(a, 2); 
  ASSERT_TRUE(res == b);
}
TEST(array_apply_scalar_pre) {
  auto a = arr::Array<double>({1,4}, arr::Vector<double>{1,2,3,4});  
  auto b = arr::Array<double>({1,4}, arr::Vector<double>{3,4,5,6});  
  auto res = arr::apply_scalar<double, double, double, std::plus<double>>(2, a); 
  ASSERT_TRUE(res == b);
}
TEST(array_apply) {
  auto a = arr::Array<double>({2,3}, arr::Vector<double>{1,2,3,4,5,6});  
  auto b = arr::Array<double>({2,3}, arr::Vector<double>{2,4,6,8,10,12});  
  ASSERT_TRUE((arr::apply<double,double,double,std::plus<double>>(a, a) == b));
}
TEST(array_apply_diff_size) {
  auto a = arr::Array<double>({2,3}, arr::Vector<double>{1,2,3,4,5,6});  
  auto b = arr::Array<double>({3,2}, arr::Vector<double>{1,2,3,4,5,6});  
  ASSERT_THROW((arr::apply<double,double,double,std::plus<double>>(a, b)),
               std::range_error, "incompatible array sizes");
}

// LLL test weird dimensions like 3 x 4 x 0, etc.
// but note that R isn't doing a great job here:
// > b = array(0, c(4, 2, 0, 1))
// > b
// > str(b)
//  num[1:4, 1:2, 0 , 1] 

// ----- test array ops:

TEST(array_rollmean) {
  auto a = arr::Array<double>({10}, arr::Vector<double>{1,2,3,4,5,6,7,8,9,10});  
  auto b = arr::Array<double>({10}, arr::Vector<double>{NAN,NAN,NAN,2.5,3.5,4.5,5.5,6.5,7.5,8.5});  
  auto res = arr::rollmean_inplace<double>(a, 4L, 4L); 
  ASSERT_TRUE(res == b);
}
TEST(array_rollmin) {
  auto a = arr::Array<double>({10}, arr::Vector<double>{1,2,3,4,5,6,7,8,9,10});  
  auto b = arr::Array<double>({10}, arr::Vector<double>{NAN, NAN, NAN, 1,2,3,4,5,6,7});  
  auto res = arr::rollmin_inplace<double>(a, 4L, 4L); 
  ASSERT_TRUE(res == b);
}
TEST(array_rollmax) {
  auto a = arr::Array<double>({10}, arr::Vector<double>{1,2,3,4,5,6,7,8,9,10});  
  auto b = arr::Array<double>({10}, arr::Vector<double>{NAN, NAN, NAN, 4,5,6,7,8,9,10});  
  auto res = arr::rollmax_inplace<double>(a, 4L, 4L); 
  ASSERT_TRUE(res == b);
}
TEST(array_rollvar) {
  auto a = arr::Array<double>({10}, arr::Vector<double>{1,2,3,4,5,6,7,8,9,10});
  auto v = 5.0/3;
  auto b = arr::Array<double>({10}, arr::Vector<double>{NAN, NAN, NAN, v,v,v,v,v,v,v});  
  auto res = arr::rollvar_inplace<double>(a, 4L, 4L); 
  ASSERT_TRUE(res == b);
}
TEST(array_rollcov) {
  auto a = arr::Array<double>({10, 2}, arr::Vector<double>{1,2,3,4,5,6,7,8,9,10,
                                        -1,-2,-3,-4,-5,-6,-7,-8,-9,-10});
  auto v = 5.0/3;
  auto b = arr::Array<double>({10,2,2}, arr::Vector<double>{NAN, NAN, NAN, v,v,v,v,v,v,v,
                                        NAN, NAN, NAN, -v,-v,-v,-v,-v,-v,-v,
                                        NAN, NAN, NAN, -v,-v,-v,-v,-v,-v,-v,
                                        NAN, NAN, NAN, v,v,v,v,v,v,v});  
  auto res = arr::rollcov<double>(a, a, 4L, 4L);
  ASSERT_TRUE(res == b);
}

// test array append (and array::to_buffer):
TEST(array_append) {
  auto v = arr::Vector<double>(27);
  std::iota(v.begin(), v.end(), 0);
  auto a1 = arr::Array<double>({3,3,3}, v, {{"1","2","3"}, {"i","ii","iii"}, {"I","II","III"}});
  auto a2 = arr::Array<double>({3,3,3}, v, {{"1","2","3"}, {"i","ii","iii"}, {"I","II","III"}});
  auto b1 = arr::Array<double>({1,3,3}, arr::Vector<double>{100,101,102,103,104,105,106,107,108});
  char buffer[1024];
  auto sz = b1.to_buffer(buffer);
  auto expected = a1.rbind(b1);
  size_t offset = 0;
  auto res = a2.append(buffer, sz, offset);
  ASSERT_TRUE(expected == res);
}
TEST(array_append_multiple_rows) {
  auto v = arr::Vector<double>(27);
  std::iota(v.begin(), v.end(), 0);
  auto a1 = arr::Array<double>({3,3,3}, v, {{"1","2","3"}, {"i","ii","iii"}, {"I","II","III"}});
  auto a2 = a1;
  auto b1 = arr::Array<double>({3,3,3}, v); // because append doesn't have names
  char buffer[1024];
  auto sz = b1.to_buffer(buffer);
  auto expected = a1.rbind(b1);
  size_t offset = 0;
  auto res = a2.append(buffer, sz, offset);
  ASSERT_TRUE(expected == res);
}
DISABLED_TEST(array_append_to_null_array) {
  auto a1 = arr::Array<double>({}, arr::Vector<double>{});
  auto v = arr::Vector<double>(27);
  std::iota(v.begin(), v.end(), 0);
  auto b1 = arr::Array<double>({3,3,3}, v);
  auto a2 = a1;
  char buffer[1024];
  auto sz = b1.to_buffer(buffer);
  auto expected = a1.rbind(b1);
  size_t offset = 0;
  auto res = a2.append(buffer, sz, offset);
  ASSERT_TRUE(expected == res);
}
TEST(array_append_to_0x3x3_array) {
  auto a1 = arr::Array<double>({0,3,3}, arr::Vector<double>{});
  auto v = arr::Vector<double>(27);
  std::iota(v.begin(), v.end(), 0);
  auto b1 = arr::Array<double>({3,3,3}, v);
  auto a2 = a1;
  char buffer[1024];
  auto sz = b1.to_buffer(buffer);
  auto expected = a1.rbind(b1);
  size_t offset = 0;
  auto res = a2.append(buffer, sz, offset);
  ASSERT_TRUE(expected == res);
}


// ----- mmaped arrays:

TEST(array_constructor_from_file) {
  cleandir("./array_mmap_constructor/");
  {
    auto a = arr::Array<double>(
      {2,2}, 
      {1,2,3,4},
      {{"un","deux"}, {"one", "two"}},
      std::make_unique<arr::MmapAllocFactory>("./array_mmap_constructor"s, false));
    std::cout << "a[{0,0}] = " << a[{0,0}] << std::endl;
    std::cout << "a[{1,0}] = " << a[{1,0}] << std::endl;
    std::cout << "a[{0,1}] = " << a[{0,1}] << std::endl;
    std::cout << "a[{1,1}] = " << a[{1,1}] << std::endl;
    ASSERT_TRUE((a[{0,0}] == 1));
    ASSERT_TRUE((a[{1,0}] == 2));
    ASSERT_TRUE((a[{0,1}] == 3));
    ASSERT_TRUE((a[{1,1}] == 4));
  } // a is being deleted here
  ASSERT_TRUE(remove("./array_mmap_constructor/0")==0);      
  ASSERT_TRUE(remove("./array_mmap_constructor/1")==0);      
  ASSERT_TRUE(remove("./array_mmap_constructor/dim")==0);    
  ASSERT_TRUE(remove("./array_mmap_constructor/names0")==0); 
  ASSERT_TRUE(remove("./array_mmap_constructor/names1")==0); 
  ASSERT_TRUE(rmdir("./array_mmap_constructor/") == 0);        
}
TEST(array_mmap_constructor) {
  cleandir("./array_mmap_constructor/");
  // create a mmapped array:
  {
    auto a = arr::Array<double>(
      {2,2}, 
      {1,2,3,4},
      {{"un","deux"}, {"one", "two"}},
      std::make_unique<arr::MmapAllocFactory>("./array_mmap_constructor"s, false));
  } // a is being deleted here
  // read the mmapped array:
  {
    auto a = arr::Array<double>(std::make_unique<arr::MmapAllocFactory>
                                ("./array_mmap_constructor"s, true));
    auto b = arr::Array<double>(
      {2,2}, 
      {1,2,3,4},
      {{"un","deux"}, {"one", "two"}});
    ASSERT_TRUE(a == b);
  } // a is being deleted here
  ASSERT_TRUE(remove("./array_mmap_constructor/0")==0);      
  ASSERT_TRUE(remove("./array_mmap_constructor/1")==0);      
  ASSERT_TRUE(remove("./array_mmap_constructor/dim")==0);    
  ASSERT_TRUE(remove("./array_mmap_constructor/names0")==0); 
  ASSERT_TRUE(remove("./array_mmap_constructor/names1")==0); 
  ASSERT_TRUE(rmdir("./array_mmap_constructor/") == 0);        
}
TEST(array_mmap_mod) {
  cleandir("./array_mmap_mod/");
  // create a mmapped array:
  {
    arr::Array<double> a(
      {3,3}, 
      {1,2,3,4,5,6,7,8,9},
      {{"un","deux","trois"}, {"one", "two", "three"}},
      std::make_unique<arr::MmapAllocFactory>("./array_mmap_mod", false));
    setv(a, 0, a[0] * 3);
    setv(a, 1, a[1] * 3);
    setv(a, 2, a[2] * 3);
    setv(a, 3, a[3] * 3);
    setv(a, 4, a[4] * 3);
    setv(a, 5, a[5] * 3);
    setv(a, 6, a[6] * 3);
    setv(a, 7, a[7] * 3);
    setv(a, 8, a[8] * 3);
  } // a is being deleted here
  // read the mmapped array:
  {
    auto a = arr::Array<double>(std::make_unique<arr::MmapAllocFactory>("./array_mmap_mod", true));
    auto b = arr::Array<double>(
      {3,3}, 
      {3,6,9,12,15,18,21,24,27},
      {{"un","deux","trois"}, {"one", "two", "three"}});
    ASSERT_TRUE(a == b);
  } // a is being deleted here
  ASSERT_TRUE(remove("./array_mmap_mod/0")==0);      
  ASSERT_TRUE(remove("./array_mmap_mod/1")==0);      
  ASSERT_TRUE(remove("./array_mmap_mod/2")==0);      
  ASSERT_TRUE(remove("./array_mmap_mod/dim")==0);    
  ASSERT_TRUE(remove("./array_mmap_mod/names0")==0); 
  ASSERT_TRUE(remove("./array_mmap_mod/names1")==0); 
  ASSERT_TRUE(rmdir("./array_mmap_mod/") == 0);  
}
// test mmap array append:
// LLL
// member apply:
TEST(array_member_apply_1) {
  auto a = arr::Array<double>({4,1}, arr::Vector<double>{1,2,3,4});
  auto exp = arr::Array<double>({4,1}, arr::Vector<double>{-1,-2,-3,-4});
  a.apply<std::negate<double>>();
  ASSERT_TRUE(a == exp);
}
TEST(array_member_apply_2) {
  auto a = arr::Array<double>({4,2}, arr::Vector<double>{1,2,3,4,5,6,7,8});
  auto b = arr::Array<double>({4,2}, arr::Vector<double>{1,2,3,4,5,6,7,8});
  auto exp = arr::Array<double>({4,2}, arr::Vector<double>{2,4,6,8,10,12,14,16});
  a.apply<std::plus<double>>(b);
  ASSERT_TRUE(a == exp);
}
// external apply:
TEST(array_apply_1) {
  auto a = arr::Array<double>({4,1}, arr::Vector<double>{1,2,3,4});
  auto exp = arr::Array<double>({4,1}, arr::Vector<double>{-1,-2,-3,-4});
  auto b = arr::apply<std::negate<double>, 
                      double, 
                      arr::Array<double>::comparator, 
                      arr::Array<double>>(a);
  for (unsigned i=0; i<b.size(); ++i) cout << "b[" << i << "]: " << b[i] << std::endl;
  ASSERT_TRUE(b == exp);
}
TEST(array_apply_2) {
  auto a = arr::Array<double>({4,2}, arr::Vector<double>{1,2,3,4,5,6,7,8},
    {{"un","deux","trois","quatre"}, {"one", "two"}});
  auto b = arr::Array<int>({8,1}, arr::Vector<int>{8,7,6,5,4,3,2,1});
  auto exp = arr::Array<double>({4,2}, arr::Vector<double>{9,9,9,9,9,9,9,9},
    {{"un","deux","trois","quatre"}, {"one", "two"}});
  auto res = arr::apply<ztsdb::plus<double,int,double>, 
                        double, 
                        arr::Array<double>::comparator, 
                        arr::Array<double>,
                        arr::Array<int>>(a, b);
  ASSERT_TRUE(res == exp);
}
template<typename T, typename U, typename V>
struct MultipliesSums {
  T operator()(T t, U u, V v) { return t * u + v; }
};
TEST(array_apply_3) {
  auto a = arr::Array<double>({4,2}, arr::Vector<double>{1,2,3,4,5,6,7,8},
    {{"un","deux","trois","quatre"}, {"one", "two"}});
  auto b = arr::Array<int>({8,1}, arr::Vector<int>{8,7,6,5,4,3,2,1});
  auto data = arr::Vector<unsigned>{11,12,13,14,15,16,17,18};
  auto c = arr::PseudoVector<unsigned>(data); // just keeps a reference to data
  auto exp = arr::Array<double>({4,2}, arr::Vector<double>{19,26,31,34,35,34,31,26},
    {{"un","deux","trois","quatre"}, {"one", "two"}});
  auto res = arr::apply<MultipliesSums<double, int, unsigned>, 
                        double, 
                        arr::Array<double>::comparator, 
                        arr::Array<double>,
                        arr::Array<int>,
                        arr::PseudoVector<unsigned>
                        >(a, b, c);
  for (unsigned i=0; i<res.size(); ++i) cout << "res[" << i << "]: " << res[i] << std::endl;
  ASSERT_TRUE(res == exp);
}
TEST(array_apply_3_pseudoarray) {
  auto a = arr::Array<double>({4,2}, arr::Vector<double>{1,2,3,4,5,6,7,8},
    {{"un","deux","trois","quatre"}, {"one", "two"}});
  auto b = arr::Array<int>({8,1}, arr::Vector<int>{8,7,6,5,4,3,2,1});
  auto data = arr::Array<unsigned>{{2}};
  auto c = arr::PseudoArray<unsigned>{data, a.ncols(), a.size()};
  auto exp = arr::Array<double>({4,2}, arr::Vector<double>{10,16,20,22,22,20,16,10},
    {{"un","deux","trois","quatre"}, {"one", "two"}});
  auto res = arr::apply<MultipliesSums<double, int, unsigned>, 
                        double, 
                        arr::Array<double>::comparator, 
                        arr::Array<double>,
                        arr::Array<int>,
                        arr::PseudoArray<unsigned>
                        >(a, b, c);
  for (unsigned i=0; i<res.size(); ++i) cout << "res[" << i << "]: " << res[i] << std::endl;
  ASSERT_TRUE(res == exp);
}

// test ordering for scalar subassign with an IntIndex when O=le rather that lt! LLL

int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
