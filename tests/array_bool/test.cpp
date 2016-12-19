// -*- compile-command: "make -j -k test" -*-

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


// helper function for the creation of indices
namespace arr {

  template<typename T>
  static Index make_NullIndex(idx_type d, const Array<T>& t) {
    return NullIndex{t.dim[d]};
  }

  // static Index make_IntIndex(const Vector<size_t>& v) {
  //   return IntIndex(v);
  // }

  // static Index make_BoolIndex(const Vector<bool>& v) {
  //   return BoolIndex{v};
  // }

  template<typename T>
  static Index make_NameIndex(idx_type d, const vector<string>& v, const Array<T>& t) {
    return NameIndex{v, *t.names[d]};
  }

  template<typename T>
  static Index make_DtimeIndex(idx_type d, const vector<string>& v, const Array<T>& t) {
    return NameIndex{v, *t.names[d]};
  }

}


// Array tests
// -----------
TEST(array_make_4_scalar_init_bool_seq_to) {
  auto a   = arr::Array<bool>(arr::seq_to, false, true, true);
  auto res = arr::Array<bool>({2}, arr::Vector<bool>{false, true});
  cout << (a.getdim() == res.getdim()) << endl;
  cout << (a[0] == res[0]) << endl;
  cout << (a[1] == res[1]) << endl;
  cout << (a.isOrdered() == res.isOrdered()) << endl;
  ASSERT_TRUE(a==res);   
}
TEST(array_make_4_scalar_init_bool_seq_n) {
  auto a   = arr::Array<bool>(arr::seq_n, false, true, 4);
  auto res = arr::Array<bool>({4}, arr::Vector<bool>{false, true, true, true});
  ASSERT_TRUE(a==res);   
}
TEST(array_make_2x2_scalar_init) {
  auto a = arr::Array<bool>({2,2}, arr::Vector<bool>{true});
  for (arr::idx_type j=0; j<4; ++j) {
    ASSERT_TRUE(a[j] == true);
  }
  ASSERT_TRUE((a[{0,0}] == true));
  ASSERT_TRUE((a[{1,0}] == true));
  ASSERT_TRUE((a[{0,1}] == true));
  ASSERT_TRUE((a[{1,1}] == true));
}
TEST(array_make_4x1_scalar_init) {
  auto a = arr::Array<bool>({4,1}, arr::Vector<bool>{true});
  for (arr::idx_type j=0; j<4; ++j) {
    ASSERT_TRUE(a[j] == true);
  }
  ASSERT_TRUE((a[{0,0}] == true));
  ASSERT_TRUE((a[{1,0}] == true));
  ASSERT_TRUE((a[{2,0}] == true));
  ASSERT_TRUE((a[{3,0}] == true));
}
TEST(array_make_1x4_scalar_init) {
  auto a = arr::Array<bool>({1,4}, arr::Vector<bool>{true});
  for (arr::idx_type j=0; j<4; ++j) {
    ASSERT_TRUE(a[j] == true);
  }
  ASSERT_TRUE((a[{0,0}] == true));
  ASSERT_TRUE((a[{0,1}] == true));

  ASSERT_TRUE((a[{0,2}] == true));
  ASSERT_TRUE((a[{0,3}] == true));
}
TEST(array_make_2x2x2_scalar_init) {
  auto a = arr::Array<bool>({2,2,2}, arr::Vector<bool>{true});
  for (arr::idx_type j=0; j<8; ++j) {
    ASSERT_TRUE(a[j] == true);
  }
  ASSERT_TRUE((a[{0,0,0}] == true));
  ASSERT_TRUE((a[{0,0,1}] == true));
  ASSERT_TRUE((a[{0,1,0}] == true));
  ASSERT_TRUE((a[{0,1,1}] == true));
  ASSERT_TRUE((a[{1,0,0}] == true));
  ASSERT_TRUE((a[{1,0,1}] == true));
  ASSERT_TRUE((a[{1,1,0}] == true));
  ASSERT_TRUE((a[{1,1,1}] == true));
}
TEST(array_make_2x2_vector_init) {
  auto a = arr::Array<bool>({2,2}, arr::Vector<bool>{false,true,false,true});
  for (arr::idx_type j=0; j<4; ++j) {
    ASSERT_TRUE(a[j] == j%2);
  }
  ASSERT_TRUE((a[{0,0}] == false));
  ASSERT_TRUE((a[{1,0}] == true ));
  ASSERT_TRUE((a[{0,1}] == false));
  ASSERT_TRUE((a[{1,1}] == true ));
}
TEST(array_make_4x1_vector_init) {
  auto a = arr::Array<bool>({4,1}, arr::Vector<bool>{false,true,false,true});
  for (arr::idx_type j=0; j<4; ++j) {
    ASSERT_TRUE(a[j] == j%2);
  }
  ASSERT_TRUE((a[{0,0}] == false));
  ASSERT_TRUE((a[{1,0}] == true ));
  ASSERT_TRUE((a[{2,0}] == false));
  ASSERT_TRUE((a[{3,0}] == true ));
}
TEST(array_make_1x4_vector_init) {
  auto a = arr::Array<bool>({1,4}, arr::Vector<bool>{false,true,false,true});
  for (arr::idx_type j=0; j<4; ++j) {
    ASSERT_TRUE(a[j] == j%2);
  }
  ASSERT_TRUE((a[{0,0}] == false));
  ASSERT_TRUE((a[{0,1}] == true ));
  ASSERT_TRUE((a[{0,2}] == false));
  ASSERT_TRUE((a[{0,3}] == true ));
}
TEST(array_make_2x2x2_vector_init) {
  auto a = arr::Array<bool>({2,2,2}, arr::Vector<bool>{true,true,true,true,false,false,false,false});
  ASSERT_TRUE((a[{0,0,0}] == true));
  ASSERT_TRUE((a[{1,0,0}] == true));
  ASSERT_TRUE((a[{0,1,0}] == true));
  ASSERT_TRUE((a[{1,1,0}] == true));
  ASSERT_TRUE((a[{0,0,1}] == false));
  ASSERT_TRUE((a[{1,0,1}] == false));
  ASSERT_TRUE((a[{0,1,1}] == false));
  ASSERT_TRUE((a[{1,1,1}] == false));
}
TEST(array_make_3x2x4_vector_init) {
  auto v = arr::Vector<bool>(
    { false,true,false,true,false,true,false,true,
      false,true,false,true,false,true,false,true,
      false,true,false,true,false,true,false,true});

  auto a = arr::Array<bool>({3,2,4}, v);
  for (arr::idx_type j=0; j<24; ++j) {
    ASSERT_TRUE(a[j] == j%2);
  }
  ASSERT_TRUE((a[{0,0,0}] == false));
  ASSERT_TRUE((a[{1,0,0}] == true ));
  ASSERT_TRUE((a[{2,0,0}] == false));
  ASSERT_TRUE((a[{0,1,0}] == true ));
  ASSERT_TRUE((a[{1,1,0}] == false));
  ASSERT_TRUE((a[{2,1,0}] == true ));
  ASSERT_TRUE((a[{0,0,1}] == false));
  ASSERT_TRUE((a[{1,0,1}] == true ));
  ASSERT_TRUE((a[{2,0,1}] == false));
  ASSERT_TRUE((a[{0,1,1}] == true ));
  ASSERT_TRUE((a[{1,1,1}] == false));
  ASSERT_TRUE((a[{2,1,1}] == true ));
  ASSERT_TRUE((a[{0,0,2}] == false));
  ASSERT_TRUE((a[{1,0,2}] == true ));
  ASSERT_TRUE((a[{2,0,2}] == false));
  ASSERT_TRUE((a[{0,1,2}] == true ));
  ASSERT_TRUE((a[{1,1,2}] == false));
  ASSERT_TRUE((a[{2,1,2}] == true ));
  ASSERT_TRUE((a[{0,0,3}] == false));
  ASSERT_TRUE((a[{1,0,3}] == true ));
  ASSERT_TRUE((a[{2,0,3}] == false));
  ASSERT_TRUE((a[{0,1,3}] == true ));
  ASSERT_TRUE((a[{1,1,3}] == false));
  ASSERT_TRUE((a[{2,1,3}] == true ));
}
TEST(array_0_dim_0) {
  auto a = arr::Array<bool>(arr::rsv, arr::Vector<arr::idx_type>{0});
  ASSERT_THROW(a[0], std::range_error);
}
TEST(array_0x0_dim_0x0) {
  auto a = arr::Array<bool>(arr::rsv, arr::Vector<arr::idx_type>{0,0});
  ASSERT_THROW(a[0], std::range_error);
}
TEST(array_4x1_names) {
  auto a = arr::Array<bool>({4,1}, {false,true,false,true}, 
    {{"un","deux","trois","quatre"}, {"one"}});
  for (arr::idx_type j=0; j<4; ++j) {
    ASSERT_TRUE(a[j] == j%2);
  }
  ASSERT_TRUE((a[{0,0}] == false));
  ASSERT_TRUE((a[{1,0}] == true ));
  ASSERT_TRUE((a[{2,0}] == false));
  ASSERT_TRUE((a[{3,0}] == true ));
}
TEST(array_1x4_names) {
  auto a = arr::Array<bool>({1,4}, {false,true,false,true}, {{"un"}, {"one","two","three","four"}});
  for (arr::idx_type j=0; j<4; ++j) {
    ASSERT_TRUE(a[j] == j%2);
  }
  ASSERT_TRUE((a[{0,0}] == false));
  ASSERT_TRUE((a[{0,1}] == true ));
  ASSERT_TRUE((a[{0,2}] == false));
  ASSERT_TRUE((a[{0,3}] == true ));
}
TEST(array_2x2_names_vector) {
  auto a = arr::Array<bool>({2,2}, {false,true,false,true}, {{"un","deux"}, {"one", "two"}});
  for (arr::idx_type j=0; j<4; ++j) {
    ASSERT_TRUE(a[j] == j%2);
  }
  ASSERT_TRUE((a[{0,0}] == false));
  ASSERT_TRUE((a[{1,0}] == true ));
  ASSERT_TRUE((a[{0,1}] == false));
  ASSERT_TRUE((a[{1,1}] == true ));
}
// names only one dimension:
TEST(array_2x2_names_vector_init_names_only_y) {
  auto a = arr::Array<bool>({2,2}, {false,true,false,true}, {{}, {"one", "two"}});
  for (arr::idx_type j=0; j<4; ++j) {
    ASSERT_TRUE(a[j] == j%2);
  }
  ASSERT_TRUE((a[{0,0}] == false));
  ASSERT_TRUE((a[{1,0}] == true ));
  ASSERT_TRUE((a[{0,1}] == false));
  ASSERT_TRUE((a[{1,1}] == true ));
}
TEST(array_2x2_names_vector_init_names_only_x) {
  auto a = arr::Array<bool>({2,2}, {false,true,false,true}, vector<arr::Vector<arr::zstring>>{{"un","deux"}, {}});
  for (arr::idx_type j=0; j<4; ++j) {
    ASSERT_TRUE(a[j] == j%2);
  }
  ASSERT_TRUE((a[{0,0}] == false));
  ASSERT_TRUE((a[{1,0}] == true ));
  ASSERT_TRUE((a[{0,1}] == false));
  ASSERT_TRUE((a[{1,1}] == true ));
}
// incorrect name lengths:
TEST(array_2x2_names_init_x_names_too_long) {
  ASSERT_THROW(arr::Array<bool>({2,2}, {true,true,true,true}, {{"un","deux","trois"}, {"one", "two"}}),
               std::range_error);
} 
TEST(array_2x2_names_init_y_names_too_long) {
  ASSERT_THROW(arr::Array<bool>({2,2}, {true,true,true,true}, {{"un","deux"}, {"one","two","three"}}),
               std::range_error);
} 
TEST(array_2x2_names_init_x_names_too_short) {
  ASSERT_THROW(arr::Array<bool>({2,2}, {true,true,true,true}, {{"un"}, {"one", "two"}}),
               std::range_error);
} 
TEST(array_2x2_names_init_y_names_too_short) {
  ASSERT_THROW(arr::Array<bool>({2,2}, {true,true,true,true}, {{"un","deux"}, {"one"}}),
               std::range_error);
} 
TEST(array_2x2_names_init_x_names_missing_dim) {
  ASSERT_THROW(arr::Array<bool>(arr::Vector<arr::idx_type>{2,2}, 
                                arr::Vector<bool>{true,true,true,true}, 
                                  vector<arr::Vector<arr::zstring>>{{"un","deux"}}),
               std::range_error,
               "length of 'dimnames' [1] must match that of 'dims' [2]");
} 
TEST(array_2x2_names_init_x_names_additional_dim) {
  ASSERT_THROW(arr::Array<bool>({2,2}, {true,true,true,true}, 
    {{"un","deux"}, {"one", "two"}, {"1", "2"}}),
               std::range_error,
               "length of 'dimnames' [3] must match that of 'dims' [2]");
}
// incorrect data length
TEST(array_incorrect_datalen) {
  ASSERT_THROW(arr::Array<bool>({2,2}, {true,true,true}, {{"un"}, {"one", "two"}}),
               std::range_error, "data does not fit array dimensions");
  ASSERT_THROW(arr::Array<bool>({2,2}, {true,true,true,true,true}, {{"un"}, {"one", "two"}}),
               std::range_error, "data does not fit array dimensions");
  ASSERT_THROW(arr::Array<bool>({2}, {true,true,true}, {{"one", "two", "three"}}),
               std::range_error, "data does not fit array dimensions");
  ASSERT_THROW(arr::Array<bool>({0}, arr::Vector<bool>{true,true,true}),
               std::range_error, "data does not fit array dimensions");
}
// test equality operator:
TEST(array_1x4_equality) {
  auto a = arr::Array<bool>({1,4}, arr::Vector<bool>{true,false,true,false});
  auto b = arr::Array<bool>({1,4}, arr::Vector<bool>{true,false,true,false});
  ASSERT_TRUE(a==b);
}
TEST(array_3x2_to_2x3_equality) {
  auto a = arr::Array<bool>({2,3}, arr::Vector<bool>{true,false,true,false,true,true});
  auto b = arr::Array<bool>({3,2}, arr::Vector<bool>{true,false,true,false,true,true});
  ASSERT_FALSE(a==b);
}
TEST(array_1x4_equality_diff_content) {
  auto a = arr::Array<bool>({1,4}, arr::Vector<bool>{true,true,true,false});
  auto b = arr::Array<bool>({1,4}, arr::Vector<bool>{true,true,false,false});
  ASSERT_TRUE(!(a==b));
}
// TEST(array_4x1_equality) {
//   auto a = arr::Array<bool>({4,1}, arr::Vector<bool>{1,2,3,4});
//   auto b = arr::Array<bool>({4,1}, arr::Vector<bool>{1,2,3,4});
//   ASSERT_TRUE(a==b);
// }
// TEST(array_4x1_equality_diff_content) {
//   auto a = arr::Array<bool>({4,1}, arr::Vector<bool>{1,2,3,4});
//   auto b = arr::Array<bool>({4,1}, arr::Vector<bool>{1,2,3,0});
//   ASSERT_TRUE(!(a==b));
// }
// TEST(array_2x4_equality) {
//   auto a = arr::Array<bool>({2,2}, arr::Vector<bool>{1,2,3,4});
//   auto b = arr::Array<bool>({2,2}, arr::Vector<bool>{1,2,3,4});
//   ASSERT_TRUE(a==b);
// }
// TEST(array_2x4_equality_diff_content) {
//   auto a = arr::Array<bool>({2,2}, arr::Vector<bool>{1,1,3,4});
//   auto b = arr::Array<bool>({2,2}, arr::Vector<bool>{1,2,3,4});
//   ASSERT_TRUE(!(a==b));
// }
// TEST(array_2x4_equality_diff_size) {
//   auto a = arr::Array<bool>({1,4}, arr::Vector<bool>{1,2,3,4});
//   auto b = arr::Array<bool>({2,2}, arr::Vector<bool>{1,2,3,4});
//   ASSERT_TRUE(!(a==b));
// }
// TEST(array_2x4_equality_names) {
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   ASSERT_TRUE(a==b);
// }
// TEST(array_2x4_equality_diff_names) {
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "tw"}});
//   ASSERT_TRUE(!(a==b));
// }
// // constructor with Vector&&:
// TEST(array_mv_vector) {
//   auto a = arr::array_from_vector(arr::Vector<bool>{1,2,3,4});
//   auto b = arr::Array<bool>({4}, arr::Vector<bool>{1,2,3,4});
//   ASSERT_TRUE(a==b);
// }


// // 2D slices
// TEST(array_2x2_subset_null_null) {
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_NullIndex(1,a)});
//   ASSERT_TRUE(a==b);
// }
// TEST(array_2x2_subset_fullidx_fullidx) {
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = a(vector<arr::Index>{arr::make_IntIndex({0L,1L}), arr::make_IntIndex({0L,1L})});
//   ASSERT_TRUE(a==b);
// }
// TEST(array_2x2_subset_fullnames_fullnames) {
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = a(vector<arr::Index>{arr::make_NameIndex(0, {"un","deux"}, a), arr::make_NameIndex(1, {"one","two"}, a)});
//   ASSERT_TRUE(a==b);
// }
// TEST(array_2x2_subset_fullbool_fullbool) {
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = a(vector<arr::Index>{arr::make_BoolIndex({true,true}), arr::make_BoolIndex({true,true})});
//   ASSERT_TRUE(a==b);
// }
// TEST(array_2x2_subset_null_idx_col0) {
//   auto a = arr::Array<bool>({2,2}, arr::Vector<bool>{1,2,3,4});
//   auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_IntIndex({0})});
//   auto res = arr::Array<bool>({2}, arr::Vector<bool>{1,2});
//   ASSERT_TRUE(b==res);
// }
// TEST(array_2x2_subset_null_idx_col1) {
//   auto a = arr::Array<bool>({2,2}, arr::Vector<bool>{1,2,3,4});
//   auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_IntIndex({1})});
//   auto res = arr::Array<bool>({2}, arr::Vector<bool>{3,4});
//   ASSERT_TRUE(b==res);
// }
// TEST(array_2x2_subset_null_idx_col0_names) {
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_IntIndex({0})});
//   auto res = arr::Array<bool>({2}, arr::Vector<bool>{1,2}, {{"un","deux"}});
//   ASSERT_TRUE(b==res);
// }
// TEST(array_2x2_subset_null_string_col1_names) {
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_IntIndex({1})});
//   auto res = arr::Array<bool>({2}, arr::Vector<bool>{3,4}, {{"un","deux"}});
//   ASSERT_TRUE(b==res);
// }
// TEST(array_2x2_subset_null_string_col0_names) {
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_NameIndex(1, {"one"}, a)});
//   auto res = arr::Array<bool>({2}, arr::Vector<bool>{1,2}, {{"un","deux"}});
//   ASSERT_TRUE(b==res);
// }
// TEST(array_2x2_subset_null_bool_col0) {
//   auto a = arr::Array<bool>({2,2}, arr::Vector<bool>{1,2,3,4});
//   auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_BoolIndex({true,false})});
//   auto res = arr::Array<bool>({2}, arr::Vector<bool>{1,2});
//   ASSERT_TRUE(b==res);
// }
// TEST(array_2x2_subset_null_bool_col1) {
//   auto a = arr::Array<bool>({2,2}, arr::Vector<bool>{1,2,3,4});
//   auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_BoolIndex({false,true})});
//   auto res = arr::Array<bool>({2}, arr::Vector<bool>{3,4});
//   ASSERT_TRUE(b==res);
// }
// TEST(array_2x2_subset_null_bool_col0_names) {
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_BoolIndex({true,false})});
//   auto res = arr::Array<bool>({2}, arr::Vector<bool>{1,2}, {{"un","deux"}});
//   ASSERT_TRUE(b==res);
// }
// TEST(array_2x2_subset_null_bool_col1_names) {
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_BoolIndex({false,true})});
//   auto res = arr::Array<bool>({2}, arr::Vector<bool>{3,4}, {{"un","deux"}});
//   ASSERT_TRUE(b==res);
// }
// TEST(array_2x2_subset_bool_bool_names) {
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = a(vector<arr::Index>{arr::make_BoolIndex({false,true}), arr::make_BoolIndex({false,true})});
//   auto res = arr::Array<bool>({1}, arr::Vector<bool>{4}, {{"deux"}});
//   ASSERT_TRUE(b==res);
// }
// TEST(array_2x2_subset_null_time) {
//   auto a = arr::Array<Global::dtime>({2,2}, {Global::dtime(1ns),Global::dtime(2ns),
//         Global::dtime(3ns),Global::dtime(4ns)});  
//   auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_NullIndex(1,a)});
//   ASSERT_TRUE(a==b);
// }
// TEST(array_2x2_subset_time_time) {
//   // LLL
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = a(vector<arr::Index>{arr::make_IntIndex({0L,1L}), arr::make_IntIndex({0L,1L})});
//   ASSERT_TRUE(a==b);
// }
// // no drop cases:
// TEST(array_2x2_subset_null_idx_col0_nodrop) {
//   const auto a = arr::Array<bool>({2,2}, arr::Vector<bool>{1,2,3,4});
//   auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_IntIndex({0})}, false);
//   auto res = arr::Array<bool>({2,1}, arr::Vector<bool>{1,2});
//   ASSERT_TRUE(b==res);
// }
// TEST(array_2x2_subset_null_idx_col1_nodrop) {
//   const auto a = arr::Array<bool>({2,2}, arr::Vector<bool>{1,2,3,4});
//   auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_IntIndex({1})}, false);
//   auto res = arr::Array<bool>({2,1}, arr::Vector<bool>{3,4});
//   ASSERT_TRUE(b==res);
// }
// TEST(array_2x2_subset_null_idx_col0_names_nodrop) {
//   const auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_IntIndex({0})}, false);
//   auto res = arr::Array<bool>({2,1}, arr::Vector<bool>{1,2}, {{"un","deux"}, {"one"}});
//   ASSERT_TRUE(b==res);
// }
// TEST(array_2x2_subset_null_string_col1_names_nodrop) {
//   const auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_IntIndex({1})}, false);
//   auto res = arr::Array<bool>({2,1}, arr::Vector<bool>{3,4}, {{"un","deux"}, {"two"}});
//   ASSERT_TRUE(b==res);
// }
// TEST(array_2x2_subset_null_string_col0_names_nodrop) {
//   const auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_NameIndex(1, {"one"}, a)}, false);
//   auto res = arr::Array<bool>({2,1}, arr::Vector<bool>{1,2}, {{"un","deux"}, {"one"}});
//   ASSERT_TRUE(b==res);
// }
// TEST(array_2x2_subset_null_bool_col0_nodrop) {
//   const auto a = arr::Array<bool>({2,2}, arr::Vector<bool>{1,2,3,4});
//   auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_BoolIndex({true,false})}, false);
//   auto res = arr::Array<bool>({2,1}, arr::Vector<bool>{1,2});
//   ASSERT_TRUE(b==res);
// }
// TEST(array_2x2_subset_null_bool_col1_nodrop) {
//   const auto a = arr::Array<bool>({2,2}, arr::Vector<bool>{1,2,3,4});
//   auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_BoolIndex({false,true})}, false);
//   auto res = arr::Array<bool>({2,1}, arr::Vector<bool>{3,4});
//   ASSERT_TRUE(b==res);
// }
// TEST(array_2x2_subset_null_bool_col0_names_nodrop) {
//   const auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_BoolIndex({true,false})}, false);
//   auto res = arr::Array<bool>({2,1}, arr::Vector<bool>{1,2}, {{"un","deux"}, {"one"}});
//   ASSERT_TRUE(b==res);
// }
// TEST(array_2x2_subset_null_bool_col1_names_nodrop) {
//   const auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_BoolIndex({false,true})}, false);
//   auto res = arr::Array<bool>({2,1}, arr::Vector<bool>{3,4}, {{"un","deux"}, {"two"}});
//   ASSERT_TRUE(b==res);
// }
// TEST(array_2x2_subset_bool_bool_names_nodrop) {
//   const auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = a(vector<arr::Index>{arr::make_BoolIndex({false,true}), arr::make_BoolIndex({false,true})}, false);
//   auto res = arr::Array<bool>({1,1}, arr::Vector<bool>{4}, {{"deux"}, {"two"}});
//   ASSERT_TRUE(b==res);
// }
// // dim > 2 slices: LLL
// TEST(array_3x3x3_subset_null_null_null) {
//   auto v = arr::Vector<bool>(27);
//   std::iota(v.begin(), v.end(), 0);
//   auto a = arr::Array<bool>({3,3,3}, v, {{"1","2","3"}, {"i","ii","iii"}, {"I","II","III"}});
//   auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_NullIndex(1,a), arr::make_NullIndex(2,a)});
//   ASSERT_TRUE(a==b);
// }
// TEST(array_3x3x3x3_subset_3x3x1x1) {
//   auto v = arr::Vector<bool>(81);
//   std::iota(v.begin(), v.end(), 0);
//   auto a = arr::Array<bool>({3,3,3,3}, 
//                               arr::Vector<bool>(v.begin(), v.end()), 
//     {{"1","2","3"}, {"i","ii","iii"},{},{}});
//   auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), 
//               arr::make_NullIndex(1,a), 
//               arr::make_IntIndex({0L}), 
//               arr::make_IntIndex({0L})});
//   auto res = arr::Array<bool>({3,3}, 
//                                 arr::Vector<bool>(v.begin(), v.begin()+9), 
//     {{"1","2","3"}, {"i","ii","iii"}});
//   ASSERT_TRUE(b==res);
// }
// TEST(array_3x3x3x3_subset_3x3x1x1_nodrop) {
//   auto v = arr::Vector<bool>(81);
//   std::iota(v.begin(), v.end(), 0);
//   const auto a = arr::Array<bool>({3,3,3,3}, 
//                                     arr::Vector<bool>(v.begin(), v.end()), 
//     {{"1","2","3"}, {"i","ii","iii"}, {}, {}});
//   auto b = a(vector<arr::Index>{arr::make_NullIndex(0,a), 
//               arr::make_NullIndex(1,a), 
//               arr::make_IntIndex({0L}), 
//               arr::make_IntIndex({0L})},
//               false);
//   auto res = arr::Array<bool>({3,3,1,1}, 
//                               arr::Vector<bool>(v.begin(), v.begin()+9), 
//     {{"1","2","3"}, {"i","ii","iii"}, {}, {}});
//   ASSERT_TRUE(b==res);
// }

// // subset assign --------
// TEST(array_2x2_subset_assign_null_null) {
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = arr::Array<bool>({2,2}, {2,3,4,5}, {{"uu","dddd"}, {"ooo", "ttt"}});
//   auto c = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_NullIndex(1,a)}, b);
//   auto d = arr::Array<bool>({2,2}, {2,3,4,5}, {{"un","deux"}, {"one", "two"}});
//   ASSERT_TRUE(c==d);
// }
// TEST(array_2x2_subset_assign_fullidx_fullidx) {
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = arr::Array<bool>({2,2}, {2,3,4,5}, {{"uu","dddd"}, {"ooo", "ttt"}});
//   auto c = a(vector<arr::Index>{arr::make_IntIndex({0L,1L}), arr::make_IntIndex({0L,1L})}, b);
//   auto d = arr::Array<bool>({2,2}, {2,3,4,5}, {{"un","deux"}, {"one", "two"}});
//   ASSERT_TRUE(c==d);
// }
// TEST(array_2x2_subset_assign_fullnames_fullnames) {
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = arr::Array<bool>({2,2}, {2,3,4,5}, {{"uu","dddd"}, {"ooo", "ttt"}});
//   auto c = a(vector<arr::Index>{arr::make_NameIndex(0, {"un","deux"}, a), 
//               arr::make_NameIndex(1, {"one","two"}, a)}, b);
//   auto d = arr::Array<bool>({2,2}, {2,3,4,5}, {{"un","deux"}, {"one", "two"}});
//   ASSERT_TRUE(c==d);
// }
// TEST(array_2x2_subset_assign_fullbool_fullbool) {
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = arr::Array<bool>({2,2}, {2,3,4,5}, {{"uu","dddd"}, {"ooo", "ttt"}});
//   auto c = a(vector<arr::Index>{arr::make_BoolIndex({true,true}), arr::make_BoolIndex({true,true})}, b);
//   auto d = arr::Array<bool>({2,2}, {2,3,4,5}, {{"un","deux"}, {"one", "two"}});
//   ASSERT_TRUE(c==d);
// }
// TEST(array_2x2_subset_assign_null_idx_col0) {
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = arr::Array<bool>({2,1}, arr::Vector<bool>{2,3}, {{"uu","dddd"}, {"ooo"}});
//   auto c = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_IntIndex({0})}, b);
//   auto d = arr::Array<bool>({2,2}, {2,3,3,4}, {{"un","deux"}, {"one", "two"}});
//   ASSERT_TRUE(c==d);
// }
// TEST(array_2x2_subset_assign_null_idx_col1) {
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = arr::Array<bool>({2,1}, arr::Vector<bool>{4,5}, {{"uu","dddd"}, {"ttt"}});
//   auto c = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_IntIndex({1})}, b);
//   auto d = arr::Array<bool>({2,2}, {1,2,4,5}, {{"un","deux"}, {"one", "two"}});
//   ASSERT_TRUE(c==d);
// }
// TEST(array_2x2_subset_assign_null_string_col0_names) {
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = arr::Array<bool>({2,1}, arr::Vector<bool>{2,3}, {{"uu","dddd"}, {"ooo"}});
//   auto c = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_NameIndex(1, {"one"}, a)}, b);
//   auto d = arr::Array<bool>({2,2}, {2,3,3,4}, {{"un","deux"}, {"one", "two"}});
//   ASSERT_TRUE(c==d);
// }
// TEST(array_2x2_subset_assign_null_string_col1) {
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = arr::Array<bool>({2,1}, arr::Vector<bool>{4,5}, {{"uu","dddd"}, {"ttt"}});
//   auto c = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_NameIndex(1, {"two"}, a)}, b);
//   auto d = arr::Array<bool>({2,2}, {1,2,4,5}, {{"un","deux"}, {"one", "two"}});
//   ASSERT_TRUE(c==d);
// }
// TEST(array_2x2_subset_assign_null_bool_col0) {
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = arr::Array<bool>({2,1}, arr::Vector<bool>{2,3}, {{"uu","dddd"}, {"ooo"}});
//   auto c = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_BoolIndex({true,false})}, b);
//   auto d = arr::Array<bool>({2,2}, {2,3,3,4}, {{"un","deux"}, {"one", "two"}});
//   ASSERT_TRUE(c==d);
// }
// TEST(array_2x2_subset_assign_null_bool_col1) {
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = arr::Array<bool>({2,1}, arr::Vector<bool>{4,5}, {{"uu","dddd"}, {"ttt"}});
//   auto c = a(vector<arr::Index>{arr::make_NullIndex(0,a), arr::make_BoolIndex({false,true})}, b);
//   auto d = arr::Array<bool>({2,2}, {1,2,4,5}, {{"un","deux"}, {"one", "two"}});
//   ASSERT_TRUE(c==d);
// }
// TEST(array_2x2_subset_assign_bool_bool) {
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = arr::Array<bool>({1,1}, arr::Vector<bool>{5}, {{"dddd"}, {"ttt"}});
//   auto c = a(vector<arr::Index>{arr::make_BoolIndex({false,true}), arr::make_BoolIndex({false,true})}, b);
//   auto d = arr::Array<bool>({2,2}, {1,2,3,5}, {{"un","deux"}, {"one", "two"}});
//   ASSERT_TRUE(c==d);
// }
// // 3D subset assign ------- LLL
// // wrong dims subset assign ------- LLL
// // type conversions subset assign -------- LLL

// // copy ---------------
// TEST(array_3x3x3_copy_assignment) {
//   auto v = arr::Vector<bool>(27);
//   std::iota(v.begin(), v.end(), 0);
//   auto a = arr::Array<bool>({3,3,3}, v, {{"1","2","3"}, {"i","ii","iii"}, {"I","II","III"}});
//   auto b = arr::Array<bool>(arr::rsv, arr::Vector<arr::idx_type>{3,3,3});
//   b = a;
//   ASSERT_TRUE(a==b);
// }
// TEST(array_3x3x3_copy_constructor) {
//   auto v = arr::Vector<bool>(27);
//   std::iota(v.begin(), v.end(), 0);
//   auto a = arr::Array<bool>({3,3,3}, v, {{"1","2","3"}, {"i","ii","iii"}, {"I","II","III"}});
//   auto b = a;
//   ASSERT_TRUE(a==b);
// }
// TEST(array_3x3x3_copy_constructor_rvalue) {
//   auto v = arr::Vector<bool>(27);
//   std::iota(v.begin(), v.end(), 0);
//   auto a = arr::Array<bool>({3,3,3}, v, {{"1","2","3"}, {"i","ii","iii"}, {"I","II","III"}});
//   auto b = arr::Array<bool>(std::move(arr::Array<bool>({3,3,3}, v, 
//     {{"1","2","3"}, {"i","ii","iii"}, {"I","II","III"}})));
//   ASSERT_TRUE(a==b);
// }

// // // deletion LLL
// // scalar arithmetic --------------
// TEST(array_3x3x3_scalar_plus) {
//   auto v = arr::Vector<bool>(27);
//   std::iota(v.begin(), v.end(), 0);
//   auto a = arr::Array<bool>({3,3,3}, v, {{"1","2","3"}, {"i","ii","iii"}, {"I","II","III"}});
//   auto b = a;
//   for (arr::idx_type i=0; i<27; ++i) {
//     b.at(i, b[i] + 3);
//   }
//   auto k = arr::apply_scalar<bool, bool, bool, std::plus<bool>>(3, a);
//   std::cout << "b[0] = " << b[0] << std::endl;
//   std::cout << "b[1] = " << b[1] << std::endl;
//   std::cout << "b[2] = " << b[2] << std::endl;
//   std::cout << "b[3] = " << b[3] << std::endl;
//   std::cout << "b[9] = " << b[9] << std::endl;
//   cout << "k.size(): " << k.size() << std::endl;

//   ASSERT_TRUE((arr::apply_scalar<bool, bool, bool, std::plus<bool>>(3, a))==b);
// }
// TEST(array_3x3x3_scalar_minus) {
//   auto v = arr::Vector<bool>(27);
//   std::iota(v.begin(), v.end(), 0);
//   auto a = arr::Array<bool>({3,3,3}, v, {{"1","2","3"}, {"i","ii","iii"}, {"I","II","III"}});
//   auto b = a;
//   for (arr::idx_type i=0; i<27; ++i) {
//     b.at(i, 3 - b[i]);
//   }
//   ASSERT_TRUE((arr::apply_scalar<bool, bool, bool, std::minus<bool>>(3, a))==b);
// }
// TEST(array_3x3x3_scalar_mult) {
//   auto v = arr::Vector<bool>(27);
//   std::iota(v.begin(), v.end(), 0);
//   auto a = arr::Array<bool>({3,3,3}, v, {{"1","2","3"}, {"i","ii","iii"}, {"I","II","III"}});
//   auto b = a;
//   for (arr::idx_type i=0; i<27; ++i) {
//     b.at(i, b[i] * 3);
//   }
//   ASSERT_TRUE((arr::apply_scalar<bool, bool, bool, std::multiplies<bool>>(3, a))==b);
// }
// TEST(array_3x3x3_scalar_div) {
//   auto v = arr::Vector<bool>(27);
//   std::iota(v.begin(), v.end(), 0);
//   auto a = arr::Array<bool>({3,3,3}, v, {{"1","2","3"}, {"i","ii","iii"}, {"I","II","III"}});
//   auto b = a;
//   for (arr::idx_type i=0; i<27; ++i) {
//     b.at(i, 3 / b[i]);
//   }
//   ASSERT_TRUE((arr::apply_scalar<bool, bool, bool, std::divides<bool>>(3, a))==b);
// }


// // // cbind, rbind, abind
// TEST(array_2x3x2_rbind) {
//   auto v1 = arr::Vector<bool>(12);
//   std::iota(v1.begin(), v1.end(), 0);
//   auto a = arr::Array<bool>({2,3,2}, v1, {{"1","2"}, {"i","ii","iii"}, {"I","II"}});
//   auto v2 = arr::Vector<bool>{0,1,0,1,2,3,2,3,4,5,4,5,6,7,6,7,8,9,8,9,10,11,10,11};
//   auto b = arr::Array<bool>({4,3,2}, v2, {{"1","2","1","2"}, {"i","ii","iii"}, {"I","II"}});
//   // cout << val::display(arr::make_cow<val::VArrayD>(false, b)) << endl; 
//   ASSERT_TRUE(a.rbind(arr::Array<bool>(a))==b);
// }
// TEST(array_2x2_rbind_first_no_names) {
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{},            {"one", "two"}});
//   auto b = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto c = arr::Array<bool>({4,2}, {1,2,1,2,3,4,3,4}, {{"","","un","deux"}, {"one","two"}});
//   ASSERT_TRUE(a.rbind(b)==c);
// }
// TEST(array_2x2_rbind_second_no_names) {
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = arr::Array<bool>({2,2}, {1,2,3,4}, {{},            {"one", "two"}});
//   auto c = arr::Array<bool>({4,2}, {1,2,1,2,3,4,3,4}, {{"un","deux","", ""}, {"one","two"}});
//   ASSERT_TRUE(a.rbind(b)==c);
// }
// TEST(array_2x2_cbind) {
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = arr::Array<bool>({2,4}, {1,2,3,4,1,2,3,4}, {{"un","deux"}, {"one","two","one","two"}});
//   ASSERT_TRUE(a.cbind(arr::Array<bool>(a))==b);
// }
// TEST(array_2x2_cbind_to_self) {
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   ASSERT_THROW(a.cbind(a), range_error, "cannot bind to self.");
// }
// TEST(array_2x2_cbind_diff_names) {
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = arr::Array<bool>({2,2}, {1,2,3,4}, {{"1", "2"},    {"one", "two"}});
//   auto c = arr::Array<bool>({2,4}, {1,2,3,4,1,2,3,4}, {{"un","deux"}, {"one","two","one","two"}});
//   ASSERT_TRUE(a.cbind(b)==c);
// }
// TEST(array_2x2_cbind_first_no_names) {
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{},            {"one", "two"}});
//   auto b = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto c = arr::Array<bool>({2,4}, {1,2,3,4,1,2,3,4}, {{"un","deux"}, {"one","two","one","two"}});
//   ASSERT_TRUE(a.cbind(b)==c);
// }
// TEST(array_2x2_cbind_second_no_names) {
//   auto a = arr::Array<bool>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
//   auto b = arr::Array<bool>({2,2}, {1,2,3,4}, {{},            {"one", "two"}});
//   auto c = arr::Array<bool>({2,4}, {1,2,3,4,1,2,3,4}, {{"un","deux"}, {"one","two","one","two"}});
//   ASSERT_TRUE(a.cbind(b)==c);
// }
// // test 0 dimensions LLL
// TEST(array_rbind_null_0x0) {
//   auto a = arr::Array<bool>({}, arr::Vector<bool>{}, vector<arr::Vector<arr::zstring>>{});
//   auto b = arr::Array<bool>({0,0}, arr::Vector<bool>{}, vector<arr::Vector<arr::zstring>>{{}, {}});
//   ASSERT_TRUE(a.rbind(b)==b);
// }
// TEST(array_rbind_null_1x0) {
//   auto a = arr::Array<bool>({}, arr::Vector<bool>{}, vector<arr::Vector<arr::zstring>>{});
//   auto b = arr::Array<bool>({1,0}, {}, vector<arr::Vector<arr::zstring>>{{"one"}, {}});
//   ASSERT_TRUE(a.rbind(b)==b);
// }
// TEST(array_rbind_null_1x1) {
//   auto a = arr::Array<bool>({}, arr::Vector<bool>{}, vector<arr::Vector<arr::zstring>>{});
//   auto b = arr::Array<bool>({1,1}, arr::Vector<bool>{1.1}, {{"one"}, {"1"}});
//   ASSERT_TRUE(a.rbind(b)==b);
// }
// TEST(array_rbind_null_2x2) {
//   auto a = arr::Array<bool>({}, arr::Vector<bool>{}, vector<arr::Vector<arr::zstring>>{});
//   auto b = arr::Array<bool>({2,2}, {1,2,3,4}, {{"one","two"}, {"1","2"}});
//   ASSERT_TRUE(a.rbind(b)==b);
// }
// TEST(array_cbind_null_0x0) {
//   auto a = arr::Array<bool>({}, arr::Vector<bool>{}, vector<arr::Vector<arr::zstring>>{});
//   auto b = arr::Array<bool>({0,0}, {}, vector<arr::Vector<arr::zstring>>{{}, {}});
//   ASSERT_TRUE(a.cbind(b)==b);
// }
// TEST(array_cbind_null_1x0) {
//   auto a = arr::Array<bool>({}, arr::Vector<bool>{}, vector<arr::Vector<arr::zstring>>{});
//   auto b = arr::Array<bool>({1,0}, {}, vector<arr::Vector<arr::zstring>>{{"one"}, {}});
//   ASSERT_TRUE(a.cbind(b)==b);
// }
// TEST(array_cbind_null_1x1) {
//   auto a = arr::Array<bool>({}, arr::Vector<bool>{}, vector<arr::Vector<arr::zstring>>{});
//   auto b = arr::Array<bool>({1,1}, arr::Vector<bool>{1.1}, {{"one"}, {"1"}});
//   ASSERT_TRUE(a.cbind(b)==b);
// }
// TEST(array_cbind_null_2x2) {
//   auto a = arr::Array<bool>({}, arr::Vector<bool>{}, vector<arr::Vector<arr::zstring>>{});
//   auto b = arr::Array<bool>({2,2}, {1,2,3,4}, 
//                               vector<arr::Vector<arr::zstring>>{{"one","two"}, {"1","2"}});
//   ASSERT_TRUE(a.cbind(b)==b);
// }
// // test one or more dimensions == 0
// TEST(array_rbind_0x0_0x0) {
//   auto a = arr::Array<bool>({0,0}, {}, vector<arr::Vector<arr::zstring>>{{}, {}});
//   auto b = arr::Array<bool>({0,0}, {}, vector<arr::Vector<arr::zstring>>{{}, {}});
//   ASSERT_TRUE(a.rbind(b)==b);
// }
// TEST(array_rbind_0x0_1x0) {
//   auto a = arr::Array<bool>({0,0}, {}, vector<arr::Vector<arr::zstring>>{{}, {}});
//   auto b = arr::Array<bool>({1,0}, {}, vector<arr::Vector<arr::zstring>>{{"two"}, {}});
//   auto c = arr::Array<bool>({1,0}, {}, vector<arr::Vector<arr::zstring>>{{"two"}, {}});
//   ASSERT_TRUE(a.rbind(b)==c);
// }
// TEST(array_rbind_1x0_1x0) {
//   auto a = arr::Array<bool>({1,0}, {}, vector<arr::Vector<arr::zstring>>{{"one"}, {}});
//   auto b = arr::Array<bool>({1,0}, {}, vector<arr::Vector<arr::zstring>>{{"two"}, {}});
//   auto c = arr::Array<bool>({2,0}, {}, vector<arr::Vector<arr::zstring>>{{"one", "two"}, {}});
//   ASSERT_TRUE(a.rbind(b)==c);
// }
// TEST(array_rbind_1x0_1x1) {
//   auto a = arr::Array<bool>({0,1}, {}, vector<arr::Vector<arr::zstring>>{{}, {"one"}});
//   auto b = arr::Array<bool>({1,1}, arr::Vector<bool>{1.1}, {{"1"}, {}});
//   auto c = arr::Array<bool>({1,1}, arr::Vector<bool>{1.1}, {{"1"}, {"one"}});
//   ASSERT_TRUE(a.rbind(b)==c);
// }
// TEST(array_rbind_1x0_2x0) {
//   auto a = arr::Array<bool>({1,0}, {}, vector<arr::Vector<arr::zstring>>{{"one"}, {}});
//   auto b = arr::Array<bool>({2,0}, arr::Vector<bool>{}, {{"two", "three"}, {}});
//   auto c = arr::Array<bool>({3,0}, arr::Vector<bool>{}, {{"one", "two", "three"}, {}});
//   ASSERT_TRUE(a.rbind(b)==c);
// }
// TEST(array_cbind_0x0_0x1) {
//   auto a = arr::Array<bool>({0,0}, {}, vector<arr::Vector<arr::zstring>>{{}, {}});
//   auto b = arr::Array<bool>({0,1}, {}, vector<arr::Vector<arr::zstring>>{{}, {"two"}});
//   auto c = arr::Array<bool>({0,1}, {}, vector<arr::Vector<arr::zstring>>{{}, {"two"}});
//   ASSERT_TRUE(a.cbind(b)==c);
// }
// TEST(array_cbind_0x1_0x1) {
//   auto a = arr::Array<bool>({0,1}, {}, vector<arr::Vector<arr::zstring>>{{}, {"one"}});
//   auto b = arr::Array<bool>({0,1}, {}, vector<arr::Vector<arr::zstring>>{{}, {"two"}});
//   auto c = arr::Array<bool>({0,2}, {}, vector<arr::Vector<arr::zstring>>{{}, {"one", "two"}});
//   ASSERT_TRUE(a.cbind(b)==c);
// }
// TEST(array_cbind_0x1_0x2) {
//   auto a = arr::Array<bool>({0,1}, {}, vector<arr::Vector<arr::zstring>>{{}, {"one"}});
//   auto b = arr::Array<bool>({0,2}, {}, vector<arr::Vector<arr::zstring>>{{}, {"two", "three"}});
//   auto c = arr::Array<bool>({0,3}, {}, vector<arr::Vector<arr::zstring>>{{}, 
//                                              {"one", "two", "three"}});
//   ASSERT_TRUE(a.cbind(b)==c);
// }
// TEST(array_cbind_1x0_1x1) {
//   auto a = arr::Array<bool>({1,0}, {}, vector<arr::Vector<arr::zstring>>{{"one"}, {}});
//   auto b = arr::Array<bool>({1,1}, arr::Vector<bool>{1.1}, 
//     {{"won't be used as 'a' has a named row"}, {"1"}});
//   auto c = arr::Array<bool>({1,1}, arr::Vector<bool>{1.1}, 
//                               vector<arr::Vector<arr::zstring>>{{"one"}, {"1"}});
//   ASSERT_TRUE(a.cbind(b)==c);
// }
// TEST(array_abind_0x0x0_0x0x1) {
//   auto a = arr::Array<bool>({0,0,0}, {}, vector<arr::Vector<arr::zstring>>{{},{},{}});
//   auto b = arr::Array<bool>({0,0,1}, {}, vector<arr::Vector<arr::zstring>>{{}, {}, {"two"}});
//   auto c = arr::Array<bool>({0,0,1}, {}, vector<arr::Vector<arr::zstring>>{{}, {}, {"two"}});
//   ASSERT_TRUE(a.abind(b, 2)==c);
// }
// TEST(array_abind_0x0x1_0x0x1) {
//   auto a = arr::Array<bool>({0,0,1}, {}, vector<arr::Vector<arr::zstring>>{{}, {}, {"one"}});
//   auto b = arr::Array<bool>({0,0,1}, {}, vector<arr::Vector<arr::zstring>>{{}, {}, {"two"}});
//   auto c = arr::Array<bool>({0,0,2}, {}, vector<arr::Vector<arr::zstring>>{{}, {}, {"one", "two"}});
//   ASSERT_TRUE(a.abind(b, 2)==c);
// }
// TEST(array_abind_0x0x1_0x0x2) {
//   auto a = arr::Array<bool>({0,0,1}, {}, vector<arr::Vector<arr::zstring>>{{}, {}, {"one"}});
//   auto b = arr::Array<bool>({0,0,2}, {}, 
//                               vector<arr::Vector<arr::zstring>>{{}, {}, {"two", "three"}});
//   auto c = arr::Array<bool>({0,0,3}, {}, 
//                               vector<arr::Vector<arr::zstring>>{{}, {}, {"one", "two", "three"}});
//   ASSERT_TRUE(a.abind(b, 2)==c);
// }
// // test extension when dim > a.size() LLL

TEST(array_3x3x3_vectorize) {
  auto v = arr::Vector<bool>(
    { false,true,false,true,false,true,false,true,false,
      true, false,true,false,true,false,true,false,true,
      false,true,false,true,false,true,false,true,false,});
  auto a = arr::Array<bool>({3,3,3}, v, {{"1","2","3"}, {"i","ii","iii"}, {"I","II","III"}});
  auto b = arr::Array<bool>({27}, v);
  ASSERT_TRUE(vectorize(a)==b);
}
// // array of dtime:
// // TEST(array_2x2_subset_dtime) {
// //   auto a = arr::Array<zdt::dtime>({2,2}, {1L,2L,3L,4L}, {{"un","deux"}, {"one", "two"}});
// //   auto b = arr::Array<zdt::dtime>({2,2}, {1L,2L,3L,4L}, {{"un","deux"}, {"one", "two"}});
// //   ASSERT_TRUE((a({vector<zdt::dtime>{1L}, arr::null()})==b));
// // }

// // test all the applies
// TEST(array_apply_scalar_post) {
//   auto a = arr::Array<bool>({1,4}, arr::Vector<bool>{1,2,3,4});  
//   auto b = arr::Array<bool>({1,4}, arr::Vector<bool>{3,4,5,6});  
//   auto res = arr::apply_scalar<bool, bool, bool, std::plus<bool>>(a, 2); 
//   ASSERT_TRUE(res == b);
// }
// TEST(array_apply_scalar_pre) {
//   auto a = arr::Array<bool>({1,4}, arr::Vector<bool>{1,2,3,4});  
//   auto b = arr::Array<bool>({1,4}, arr::Vector<bool>{3,4,5,6});  
//   auto res = arr::apply_scalar<bool, bool, bool, std::plus<bool>>(2, a); 
//   ASSERT_TRUE(res == b);
// }
// TEST(array_apply) {
//   auto a = arr::Array<bool>({2,3}, arr::Vector<bool>{1,2,3,4,5,6});  
//   auto b = arr::Array<bool>({2,3}, arr::Vector<bool>{2,4,6,8,10,12});  
//   ASSERT_TRUE((arr::apply<bool,bool,bool,std::plus<bool>>(a, a) == b));
// }
// TEST(array_apply_diff_size) {
//   auto a = arr::Array<bool>({2,3}, arr::Vector<bool>{1,2,3,4,5,6});  
//   auto b = arr::Array<bool>({3,2}, arr::Vector<bool>{1,2,3,4,5,6});  
//   ASSERT_THROW((arr::apply<bool,bool,bool,std::plus<bool>>(a, b)),
//                std::range_error, "incompatible array sizes");
// }

// // LLL test weird dimensions like 3 x 4 x 0, etc.
// // but note that R isn't doing a great job here:
// // > b = array(0, c(4, 2, 0, 1))
// // > b
// // > str(b)
// //  num[1:4, 1:2, 0 , 1] 

// // ----- test array ops:

// TEST(array_rollmean) {
//   auto a = arr::Array<bool>({10}, arr::Vector<bool>{1,2,3,4,5,6,7,8,9,10});  
//   auto b = arr::Array<bool>({10}, arr::Vector<bool>{NAN,NAN,NAN,2.5,3.5,4.5,5.5,6.5,7.5,8.5});  
//   auto res = arr::rollmean_inplace<bool>(a, 4L, 4L); 
//   ASSERT_TRUE(res == b);
// }
// TEST(array_rollmin) {
//   auto a = arr::Array<bool>({10}, arr::Vector<bool>{1,2,3,4,5,6,7,8,9,10});  
//   auto b = arr::Array<bool>({10}, arr::Vector<bool>{NAN, NAN, NAN, 1,2,3,4,5,6,7});  
//   auto res = arr::rollmin_inplace<bool>(a, 4L, 4L); 
//   ASSERT_TRUE(res == b);
// }
// TEST(array_rollmax) {
//   auto a = arr::Array<bool>({10}, arr::Vector<bool>{1,2,3,4,5,6,7,8,9,10});  
//   auto b = arr::Array<bool>({10}, arr::Vector<bool>{NAN, NAN, NAN, 4,5,6,7,8,9,10});  
//   auto res = arr::rollmax_inplace<bool>(a, 4L, 4L); 
//   ASSERT_TRUE(res == b);
// }
// TEST(array_rollvar) {
//   auto a = arr::Array<bool>({10}, arr::Vector<bool>{1,2,3,4,5,6,7,8,9,10});
//   auto v = 5.0/3;
//   auto b = arr::Array<bool>({10}, arr::Vector<bool>{NAN, NAN, NAN, v,v,v,v,v,v,v});  
//   auto res = arr::rollvar_inplace<bool>(a, 4L, 4L); 
//   ASSERT_TRUE(res == b);
// }

// // test array append (and array::to_buffer):
// TEST(array_append) {
//   auto v = arr::Vector<bool>(27);
//   std::iota(v.begin(), v.end(), 0);
//   auto a1 = arr::Array<bool>({3,3,3}, v, {{"1","2","3"}, {"i","ii","iii"}, {"I","II","III"}});
//   auto a2 = arr::Array<bool>({3,3,3}, v, {{"1","2","3"}, {"i","ii","iii"}, {"I","II","III"}});
//   auto b1 = arr::Array<bool>({1,3,3}, arr::Vector<bool>{100,101,102,103,104,105,106,107,108});
//   char buffer[1024];
//   auto sz = b1.to_buffer(buffer);
//   auto expected = a1.rbind(b1);
//   size_t offset = 0;
//   auto res = a2.append(buffer, sz, offset);
//   ASSERT_TRUE(expected == res);
// }
// TEST(array_append_multiple_rows) {
//   auto v = arr::Vector<bool>(27);
//   std::iota(v.begin(), v.end(), 0);
//   auto a1 = arr::Array<bool>({3,3,3}, v, {{"1","2","3"}, {"i","ii","iii"}, {"I","II","III"}});
//   auto a2 = a1;
//   auto b1 = arr::Array<bool>({3,3,3}, v); // because append doesn't have names
//   char buffer[1024];
//   auto sz = b1.to_buffer(buffer);
//   auto expected = a1.rbind(b1);
//   size_t offset = 0;
//   auto res = a2.append(buffer, sz, offset);
//   ASSERT_TRUE(expected == res);
// }
// DISABLED_TEST(array_append_to_null_array) {
//   auto a1 = arr::Array<bool>({}, arr::Vector<bool>{});
//   auto v = arr::Vector<bool>(27);
//   std::iota(v.begin(), v.end(), 0);
//   auto b1 = arr::Array<bool>({3,3,3}, v);
//   auto a2 = a1;
//   char buffer[1024];
//   auto sz = b1.to_buffer(buffer);
//   auto expected = a1.rbind(b1);
//   size_t offset = 0;
//   auto res = a2.append(buffer, sz, offset);
//   ASSERT_TRUE(expected == res);
// }
// TEST(array_append_to_0x3x3_array) {
//   auto a1 = arr::Array<bool>({0,3,3}, arr::Vector<bool>{});
//   auto v = arr::Vector<bool>(27);
//   std::iota(v.begin(), v.end(), 0);
//   auto b1 = arr::Array<bool>({3,3,3}, v);
//   auto a2 = a1;
//   char buffer[1024];
//   auto sz = b1.to_buffer(buffer);
//   auto expected = a1.rbind(b1);
//   size_t offset = 0;
//   auto res = a2.append(buffer, sz, offset);
//   ASSERT_TRUE(expected == res);
// }


// // ----- mmaped arrays:

// TEST(array_constructor_from_file) {
//   cleandir("./array_mmap_constructor/");
//   {
//     auto a = arr::Array<bool>(
//       {2,2}, 
//       {1,2,3,4},
//       {{"un","deux"}, {"one", "two"}},
//       "./array_mmap_constructor/");
//     std::cout << "a[{0,0}] = " << a[{0,0}] << std::endl;
//     std::cout << "a[{1,0}] = " << a[{1,0}] << std::endl;
//     std::cout << "a[{0,1}] = " << a[{0,1}] << std::endl;
//     std::cout << "a[{1,1}] = " << a[{1,1}] << std::endl;
//     ASSERT_TRUE((a[{0,0}] == 1));
//     ASSERT_TRUE((a[{1,0}] == 2));
//     ASSERT_TRUE((a[{0,1}] == 3));
//     ASSERT_TRUE((a[{1,1}] == 4));
//   } // a is being deleted here
//   ASSERT_TRUE(remove("./array_mmap_constructor/0")==0);      
//   ASSERT_TRUE(remove("./array_mmap_constructor/1")==0);      
//   ASSERT_TRUE(remove("./array_mmap_constructor/dim")==0);    
//   ASSERT_TRUE(remove("./array_mmap_constructor/names0")==0); 
//   ASSERT_TRUE(remove("./array_mmap_constructor/names1")==0); 
//   ASSERT_TRUE(rmdir("./array_mmap_constructor/") == 0);        
// }
// TEST(array_mmap_constructor) {
//   cleandir("./array_mmap_constructor/");
//   // create a mmapped array:
//   {
//     auto a = arr::Array<bool>(
//       {2,2}, 
//       {1,2,3,4},
//       {{"un","deux"}, {"one", "two"}},
//       "./array_mmap_constructor/");
//   } // a is being deleted here
//   // read the mmapped array:
//   {
//     auto a = arr::Array<bool>("./array_mmap_constructor/");
//     auto b = arr::Array<bool>(
//       {2,2}, 
//       {1,2,3,4},
//       {{"un","deux"}, {"one", "two"}});
//     ASSERT_TRUE(a == b);
//   } // a is being deleted here
//   ASSERT_TRUE(remove("./array_mmap_constructor/0")==0);      
//   ASSERT_TRUE(remove("./array_mmap_constructor/1")==0);      
//   ASSERT_TRUE(remove("./array_mmap_constructor/dim")==0);    
//   ASSERT_TRUE(remove("./array_mmap_constructor/names0")==0); 
//   ASSERT_TRUE(remove("./array_mmap_constructor/names1")==0); 
//   ASSERT_TRUE(rmdir("./array_mmap_constructor/") == 0);        
// }
// TEST(array_mmap_mod) {
//   cleandir("./array_mmap_mod/");
//   // create a mmapped array:
//   {
//     arr::Array<bool> a(
//       {3,3}, 
//       {1,2,3,4,5,6,7,8,9},
//       {{"un","deux","trois"}, {"one", "two", "three"}},
//       "./array_mmap_mod/");
//     a.at(0, a[0] * 3);
//     a.at(1, a[1] * 3);
//     a.at(2, a[2] * 3);
//     a.at(3, a[3] * 3);
//     a.at(4, a[4] * 3);
//     a.at(5, a[5] * 3);
//     a.at(6, a[6] * 3);
//     a.at(7, a[7] * 3);
//     a.at(8, a[8] * 3);
//   } // a is being deleted here
//   // read the mmapped array:
//   {
//     auto a = arr::Array<bool>("./array_mmap_mod/");
//     auto b = arr::Array<bool>(
//       {3,3}, 
//       {3,6,9,12,15,18,21,24,27},
//       {{"un","deux","trois"}, {"one", "two", "three"}});
//     ASSERT_TRUE(a == b);
//   } // a is being deleted here
//   ASSERT_TRUE(remove("./array_mmap_mod/0")==0);      
//   ASSERT_TRUE(remove("./array_mmap_mod/1")==0);      
//   ASSERT_TRUE(remove("./array_mmap_mod/2")==0);      
//   ASSERT_TRUE(remove("./array_mmap_mod/dim")==0);    
//   ASSERT_TRUE(remove("./array_mmap_mod/names0")==0); 
//   ASSERT_TRUE(remove("./array_mmap_mod/names1")==0); 
//   ASSERT_TRUE(rmdir("./array_mmap_mod/") == 0);  
// }
// test mmap array append:

int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
