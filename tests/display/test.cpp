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
#include <crpcut.hpp>
//#include "cutest/cutest.h"
#include "ast.hpp"
#include "array.hpp"
#include "display.hpp"
#include "config.hpp"
#include "parser.hpp"
#include "timezone/zone.hpp"
#include "timezone/ztime.hpp"

cfg::CfgMap cfg::cfgmap;

tz::Zones tzones("/usr/share/zoneinfo");

void reinitOptions() {
  cfg::cfgmap.set("digits"s, 7L);
  cfg::cfgmap.set("scipen"s, 0L);
  cfg::cfgmap.set("width"s, 80L);
  cfg::cfgmap.set("max.print"s, 99999L);
  cfg::cfgmap.set("timezone"s, "UTC"s);
}


// scalars -------------------------------------------
TEST(display_NULL) {
  reinitOptions();
  ASSERT_TRUE(val::display(val::VNull()) == "NULL");
}
TEST(display_double) {
  reinitOptions();
  ASSERT_TRUE(val::display(val::make_array(10.0/3)) == "[1] 3.333333");
}
TEST(display_string) {
  reinitOptions();
  ASSERT_TRUE(val::display(val::make_array(arr::zstring("hello"))) == "[1] \"hello\"");
}
TEST(display_bool_true) {
  reinitOptions();
  ASSERT_TRUE(val::display(val::make_array(true)) == "[1] TRUE");
}
TEST(display_bool_false) {
  reinitOptions();
  ASSERT_TRUE(val::display(val::make_array(false)) == "[1] FALSE");
}
TEST(display_dtime) {
  auto dt = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones);
  ASSERT_TRUE(val::display(val::make_array(dt)) == "[1] 2015-03-09 10:38:01 UTC");
}
TEST(display_vclos) {
  auto i1 = new Double(1.0, yy::missing_loc());
  auto i2 = new Double(2.0, yy::missing_loc());
  auto body = new Binop(yy::parser::token::PLUS, i1, i2, yy::missing_loc());
  auto f = new Function(body, yy::missing_loc());
  val::Value vc = std::make_shared<val::VClos>(f);
  ASSERT_TRUE(val::display(vc) == "function() (1+2)");
  delete f;
}
// vectors -------------------------------------------
TEST(display_vector) {
  reinitOptions();
  auto a = arr::Array<double>({5}, {1.11,2.22,3.33,4,5.55});
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == "[1] 1.11 2.22 3.33 4    5.55");
}
TEST(display_vector_line_break_too_small) {
  // too small even for one col:
  reinitOptions();
  cfg::cfgmap.set("width"s, 3L);
  auto a = arr::Array<double>({4}, {1.11,2.22,3.33,4});
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) ==  
              "[1] 1.11\n"
              "[2] 2.22\n"
              "[3] 3.33\n"
              "[4] 4");
}
TEST(display_vector_line_break_1col) {
  reinitOptions();
  cfg::cfgmap.set("width"s, 12L);
  auto a = arr::Array<double>({4}, {1.11,2.22,3.33,4});
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) ==  
              "[1] 1.11\n"
              "[2] 2.22\n"
              "[3] 3.33\n"
              "[4] 4");
}
TEST(display_vector_line_break_2cols) {
  reinitOptions();
  cfg::cfgmap.set("width"s, 13L);
  auto a = arr::Array<double>({4}, {1.11,2.22,3.33,4});
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) ==  
              "[1] 1.11 2.22\n"
              "[3] 3.33 4");
}
TEST(display_vector_names) {
  reinitOptions();
  auto a = arr::Array<double>({4}, {1,2,3,4}, {{"1", "deux", "3", "4"}});
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == 
               "    1    deux 3    4\n"
               "[1] 1    2    3    4");
}
TEST(display_vector_names_width_10) {
  reinitOptions();
  cfg::cfgmap.set("width"s, 10L);
  auto a = arr::Array<double>({4}, {1,2,3,4}, {{"1", "deux", "3", "4"}});
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == 
              "    1   \n"    
              "[1] 1   \n"  
              "    deux\n"
              "[2] 2   \n"   
              "    3   \n"    
              "[3] 3   \n"   
              "    4\n"    
              "[4] 4");
}
TEST(display_vector_max_print_2) {
  reinitOptions();
  cfg::cfgmap.set("max.print"s, 2L);
  auto a = arr::Array<double>({4}, {1.11,2.22,3.33,4});
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == 
              "[1] 1.11 2.22\n"
              " [ reached getOption(max.print) -- omitted 2 entries ]");
}
TEST(display_vector_max_print_4) {
  reinitOptions();
  cfg::cfgmap.set("max.print"s, 4L);
  auto a = arr::Array<double>({4}, {1.11,2.22,3.33,4});
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == 
              "[1] 1.11 2.22 3.33 4");
}
// display arrays ----------------------------------
TEST(display_array_4_1) {
  reinitOptions();
  auto a = arr::Array<double>({4,1}, {1,2,3,4});  
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) ==
              "     [,1]\n"
              "[1,] 1   \n"
              "[2,] 2   \n"
              "[3,] 3   \n"
              "[4,] 4   ");
}
TEST(display_array_2x2) {
  reinitOptions();
  auto a = arr::Array<double>({2,2}, {1,2,3,4});  
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) ==
              "     [,1] [,2]\n"
              "[1,] 1    3   \n"
              "[2,] 2    4   ");
}
TEST(display_array_2x2_names) {
  reinitOptions();
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"one","two"}, {"un","doi"}});  
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) ==
              "    un doi\n"
              "one 1  3  \n"
              "two 2  4  ");
}
TEST(display_array_2x2_only_colnames) {
  reinitOptions();
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{}, {"un","doi"}});  
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) ==
              "     un doi\n"
              "[1,] 1  3  \n"
              "[2,] 2  4  ");
}
TEST(display_array_2x2_only_rownames) {
  reinitOptions();
  auto a = arr::Array<double>({2,2}, 
    {1,2,3,4}, 
    vector<arr::Vector<arr::zstring>>{{"un","deux"}, 
        {}});  
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) ==
              "     [,1] [,2]\n"
              "un   1    3   \n"
              "deux 2    4   ");
}
TEST(display_array_2x2_no_line_break) {
  // one more than what would trigger a line break:
  reinitOptions();
  cfg::cfgmap.set("width"s, 15L);
  auto a = arr::Array<double>({2,2}, {1,2,3,4});  
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) ==
              "     [,1] [,2]\n"
              "[1,] 1    3   \n"
              "[2,] 2    4   ");
}
TEST(display_array_2x2_line_break) {
  // exact trigger of a line break:
  reinitOptions();
  cfg::cfgmap.set("width"s, 13L);
  auto a = arr::Array<double>({2,2}, {1,2,3,4});  
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) ==
              "     [,1]\n"
              "[1,] 1   \n"
              "[2,] 2   \n"
              "     [,2]\n"
              "[1,] 3   \n"
              "[2,] 4   ");
}
TEST(display_array_3x3x3) {
  reinitOptions();
  auto v = arr::Vector<double>(27);
  std::iota(v.begin(), v.end(), 0);
  auto a = arr::Array<double>({3,3,3}, v);
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == 
              ", , 1\n"
              "\n"
              "     [,1] [,2] [,3]\n"
              "[1,] 0    3    6   \n"
              "[2,] 1    4    7   \n"
              "[3,] 2    5    8   \n"
              "\n"
              ", , 2\n"
              "\n"
              "     [,1] [,2] [,3]\n"
              "[1,]  9   12   15  \n"
              "[2,] 10   13   16  \n"
              "[3,] 11   14   17  \n"
              "\n"
              ", , 3\n"
              "\n"
              "     [,1] [,2] [,3]\n"
              "[1,] 18   21   24  \n"
              "[2,] 19   22   25  \n"
              "[3,] 20   23   26  \n");
}
TEST(display_array_2x2x2x2) {
  reinitOptions();
  auto v = arr::Vector<double>(16);
  std::iota(v.begin(), v.end(), 0);
  auto a = arr::Array<double>({2,2,2,2}, v);
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == 
              ", , 1, 1\n"
              "\n"
              "     [,1] [,2]\n"
              "[1,] 0    2   \n"
              "[2,] 1    3   \n"
              "\n"
              ", , 1, 2\n"
              "\n"
              "     [,1] [,2]\n"
              "[1,] 8    10  \n"
              "[2,] 9    11  \n"
              "\n"
              ", , 2, 1\n"
              "\n"
              "     [,1] [,2]\n"
              "[1,] 4    6   \n"
              "[2,] 5    7   \n"
              "\n"
              ", , 2, 2\n"
              "\n"
              "     [,1] [,2]\n"
              "[1,] 12   14  \n"
              "[2,] 13   15  \n");
}
TEST(display_array_3x3x3_names) {
  reinitOptions();
  auto v = arr::Vector<double>(27);
  std::iota(v.begin(), v.end(), 0);
  auto a = arr::Array<double>({3,3,3}, v, {{"1","2","3"}, {"i","ii","iii"}, {"I","II","III"}});
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == 
              ", , I\n"
              "\n"
              "  i ii iii\n"
              "1 0 3  6  \n"
              "2 1 4  7  \n"
              "3 2 5  8  \n"
              "\n"
              ", , II\n"
              "\n"
              "   i ii iii\n"
              "1  9 12 15 \n"
              "2 10 13 16 \n"
              "3 11 14 17 \n"
              "\n"
              ", , III\n"
              "\n"
              "   i ii iii\n"
              "1 18 21 24 \n"
              "2 19 22 25 \n"
              "3 20 23 26 \n");
}
TEST(display_array_3x3x3_names_line_break) {
  reinitOptions();
  cfg::cfgmap.set("width"s, 10L);
  auto v = arr::Vector<double>(27);
  std::iota(v.begin(), v.end(), 0);
  auto a = arr::Array<double>({3,3,3}, v, {{"1","2","3"}, {"i","ii","iii"}, {"I","II","III"}});
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == 
              ", , I\n"
              "\n"
              "  i ii iii\n"
              "1 0 3  6  \n"
              "2 1 4  7  \n"
              "3 2 5  8  \n"
              "\n"
              ", , II\n"
              "\n"
              "   i ii\n"
              "1  9 12\n"
              "2 10 13\n"
              "3 11 14\n"
              "  iii\n"
              "1 15 \n"
              "2 16 \n"
              "3 17 \n"
              "\n"
              ", , III\n"
              "\n"
              "   i ii\n"
              "1 18 21\n"
              "2 19 22\n"
              "3 20 23\n"
              "  iii\n"
              "1 24 \n"
              "2 25 \n"
              "3 26 \n");
}
TEST(display_array_names_max_print_2) {
  reinitOptions();
  cfg::cfgmap.set("max.print"s, 2L);
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"one","two"}, {"un","doi"}});  
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) ==
              "    un doi\n"
              "one 1  3  \n"
              " [ reached getOption(max.print) -- omitted 1 row(s) ]");
}
TEST(display_array_3x3x3_one_slice_ommitted) {
  reinitOptions();
  cfg::cfgmap.set("max.print"s, 18L);
  auto v = arr::Vector<double>(27);
  std::iota(v.begin(), v.end(), 0);
  auto a = arr::Array<double>({3,3,3}, v);
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == 
              ", , 1\n"
              "\n"
              "     [,1] [,2] [,3]\n"
              "[1,] 0    3    6   \n"
              "[2,] 1    4    7   \n"
              "[3,] 2    5    8   \n"
              "\n"
              ", , 2\n"
              "\n"
              "     [,1] [,2] [,3]\n"
              "[1,]  9   12   15  \n"
              "[2,] 10   13   16  \n"
              "[3,] 11   14   17  \n"
              "\n"
              " [ reached getOption(max.print) -- omitted 1 slice(s) ]");
}
TEST(display_array_3x3x3_1_slice_2_rows_ommitted) {
  reinitOptions();
  cfg::cfgmap.set("max.print"s, 12L);
  auto v = arr::Vector<double>(27);
  std::iota(v.begin(), v.end(), 0);
  auto a = arr::Array<double>({3,3,3}, v);
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == 
              ", , 1\n"
              "\n"
              "     [,1] [,2] [,3]\n"
              "[1,] 0    3    6   \n"
              "[2,] 1    4    7   \n"
              "[3,] 2    5    8   \n"
              "\n"
              ", , 2\n"
              "\n"
              "     [,1] [,2] [,3]\n"
              "[1,]  9   12   15  \n"
              "\n"
              " [ reached getOption(max.print) -- omitted 2 row(s) and 1 slice(s) ]");
}
TEST(display_array_3x3x3_2_slices_2_rows_ommitted) {
  reinitOptions();
  cfg::cfgmap.set("max.print"s, 3L);
  auto v = arr::Vector<double>(27);
  std::iota(v.begin(), v.end(), 0);
  auto a = arr::Array<double>({3,3,3}, v);
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == 
              ", , 1\n"
              "\n"
              "     [,1] [,2] [,3]\n"
              "[1,] 0    3    6   \n"
              "\n"
              " [ reached getOption(max.print) -- omitted 2 row(s) and 2 slice(s) ]");
}
TEST(display_array_3x3x3_2_slices_2_rows_ommitted_max_print_not_a_multiple) {
  reinitOptions();
  cfg::cfgmap.set("max.print"s, 5L);
  auto v = arr::Vector<double>(27);
  std::iota(v.begin(), v.end(), 0);
  auto a = arr::Array<double>({3,3,3}, v);
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == 
              ", , 1\n"
              "\n"
              "     [,1] [,2] [,3]\n"
              "[1,] 0    3    6   \n"
              "\n"
              " [ reached getOption(max.print) -- omitted 2 row(s) and 2 slice(s) ]");
}
TEST(display_array_3x3x3_less_than_a_row) {
  reinitOptions();
  cfg::cfgmap.set("max.print"s, 2L);
  auto v = arr::Vector<double>(27);
  std::iota(v.begin(), v.end(), 0);
  auto a = arr::Array<double>({3,3,3}, v);
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == 
              ", , 1\n"
              "\n"
              " [,1] [,2] ...\n"
              "\n"
              " [ reached getOption(max.print) -- omitted 3 row(s) and 2 slice(s) ]");
}
TEST(display_array_3x1x3) {
  reinitOptions();
  auto v = arr::Vector<double>(9);
  std::iota(v.begin(), v.end(), 1);
  auto a = arr::Array<double>({3,1,3}, v);
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == 
              ", , 1\n"
              "\n"
              "     [,1]\n"
              "[1,] 1   \n"
              "[2,] 2   \n"
              "[3,] 3   \n"
              "\n"
              ", , 2\n"
              "\n"
              "     [,1]\n"
              "[1,] 4   \n"
              "[2,] 5   \n"
              "[3,] 6   \n"
              "\n"
              ", , 3\n"
              "\n"
              "     [,1]\n"
              "[1,] 7   \n"
              "[2,] 8   \n"
              "[3,] 9   \n");
}

// arrays with 0 value as dimension:
TEST(display_array_0) {
  reinitOptions();
  auto a = arr::Array<double>(arr::Vector<idx_type>{0}, arr::Vector<double>()); 
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == "double(0)");
}
TEST(display_array_0x0) {
  reinitOptions();
  auto a = arr::Array<double>(Vector<idx_type>{0,0}, arr::Vector<double>());  
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == "double(0x0)");
}
TEST(display_array_0x0x0) {
  reinitOptions();
  auto a = arr::Array<double>(Vector<idx_type>{0,0,0}, arr::Vector<double>());
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == "double(0x0x0)");
}
TEST(display_array_4x0) {
  reinitOptions();
  auto a = arr::Array<double>({4,0}, arr::Vector<double>());  
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) ==
              "[1,]\n"
              "[2,]\n"
              "[3,]\n"
              "[4,]");
}
TEST(display_array_0x4) {
  reinitOptions();
  auto a = arr::Array<double>({0,4}, arr::Vector<double>());  
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == 
              " [,1] [,2] [,3] [,4]");
}
TEST(display_array_2x4x0) {
  reinitOptions();
  auto a = arr::Array<double>(Vector<idx_type>{2,4,0}, arr::Vector<double>());  
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == 
              "     [,1] [,2] [,3] [,4]\n"
              "[1,]\n"
              "[2,]");
}
TEST(display_array_2x0x3) {
  reinitOptions();
  auto a = arr::Array<double>(Vector<idx_type>{2,0,3}, arr::Vector<double>());  
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == 
              ", , 1\n"
              "\n"
              "[1,]\n"
              "[2,]\n"
              "\n"
              ", , 2\n"
              "\n"
              "[1,]\n"
              "[2,]\n"
              "\n"
              ", , 3\n"
              "\n"
              "[1,]\n"
              "[2,]\n");
}
// note that R isn't doing a great job with 0 dimensions with dim > 2:
// > b = array(0, c(4, 2, 0, 1))
// > b
// > str(b)
//  num[1:4, 1:2, 0 , 1] 
TEST(display_array_2x4x0x5) {
  reinitOptions();
  auto a = arr::Array<double>({2,4,0,5}, arr::Vector<double>());  
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == 
              ", , 0, 1\n"
              "\n"
              "     [,1] [,2] [,3] [,4]\n"
              "[1,]\n"
              "[2,]\n"
              "\n"
              ", , 0, 2\n"
              "\n"
              "     [,1] [,2] [,3] [,4]\n"
              "[1,]\n"
              "[2,]\n"
              "\n"
              ", , 0, 3\n"
              "\n"
              "     [,1] [,2] [,3] [,4]\n"
              "[1,]\n"
              "[2,]\n"
              "\n"
              ", , 0, 4\n"
              "\n"
              "     [,1] [,2] [,3] [,4]\n"
              "[1,]\n"
              "[2,]\n"
              "\n"
              ", , 0, 5\n"
              "\n"
              "     [,1] [,2] [,3] [,4]\n"
              "[1,]\n"
              "[2,]\n");
}
TEST(display_array_4x0_names) {
  reinitOptions();
  auto a = arr::Array<double>({4,0}, arr::Vector<double>(), {{"a","b","c","d"}, {}});  
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) ==
              "a\n"
              "b\n"
              "c\n"
              "d");
}
TEST(display_array_0x4_names) {
  reinitOptions();
  auto a = arr::Array<double>({0,4}, arr::Vector<double>(), {{}, {"a","b","c","d"}});  
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == " a b c d");
}
TEST(display_array_2x4x0_names) {
  reinitOptions();
  auto a = arr::Array<double>({2,4,0}, arr::Vector<double>(), {{"un","deux"}, {"a","b","c","d"}, {}});  
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == 
              "     a b c d\n"
              "un  \n"
              "deux");
}
TEST(display_array_2x0x3_names) {
  reinitOptions();
  auto a = arr::Array<double>({2,0,3}, arr::Vector<double>(), {{"un","deux"}, {}, {"a","b","c"}});  
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == 
              ", , a\n"
              "\n"
              "un  \n"
              "deux\n"
              "\n"
              ", , b\n"
              "\n"
              "un  \n"
              "deux\n"
              "\n"
              ", , c\n"
              "\n"
              "un  \n"
              "deux\n");
}
TEST(display_array_2x4x0x5_names) {
  reinitOptions();
  auto a = arr::Array<double>({2,4,0,5}, arr::Vector<double>(), 
    {{"un","deux"}, {"a","b","c","d"}, {}, {"1","2","3","4","5"}});  
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == 
              ", , 0, 1\n"
              "\n"
              "     a b c d\n"
              "un  \n"
              "deux\n"
              "\n"
              ", , 0, 2\n"
              "\n"
              "     a b c d\n"
              "un  \n"
              "deux\n"
              "\n"
              ", , 0, 3\n"
              "\n"
              "     a b c d\n"
              "un  \n"
              "deux\n"
              "\n"
              ", , 0, 4\n"
              "\n"
              "     a b c d\n"
              "un  \n"
              "deux\n"
              "\n"
              ", , 0, 5\n"
              "\n"
              "     a b c d\n"
              "un  \n"
              "deux\n");
}
// now test arrays with a 0 dimension with line breaks and or max.print set:
TEST(display_array_0x3x3_names_line_break) {
  reinitOptions();
  cfg::cfgmap.set("width"s, 7L);
  auto a = arr::Array<double>({0,3,3},
                              arr::Vector<double>{}, {{}, {"i","ii","iii"}, {"I","II","III"}});
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == 
              ", , I\n"
              "\n"
              " i ii\n"
              " iii\n"
              "\n"
              ", , II\n"
              "\n"
              " i ii\n"
              " iii\n"
              "\n"
              ", , III\n"
              "\n"
              " i ii\n"
              " iii\n");
}
TEST(display_array_2x0_names_max_print_0) {
  reinitOptions();
  cfg::cfgmap.set("max.print"s, 0L);
  auto a = arr::Array<double>({2,0}, arr::Vector<double>{}, {{"one","two"}, {}});  
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) ==
              "\n [ reached getOption(max.print) -- omitted 2 row(s) ]");
}
TEST(display_array_2x0_names_max_print_1) {
  reinitOptions();
  cfg::cfgmap.set("max.print"s, 1L);
  auto a = arr::Array<double>({2,0}, arr::Vector<double>{1}, {{"one","two"}, {}});  
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) ==
              "one\n"
              " [ reached getOption(max.print) -- omitted 1 row(s) ]");
}
TEST(display_array_3x3x0x3_one_slice_ommitted) {
  reinitOptions();
  cfg::cfgmap.set("max.print"s, 18L);
  auto a = arr::Array<double>({3,3,0,3}, arr::Vector<double>{0});
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == 
              ", , 0, 1\n"
              "\n"
              "     [,1] [,2] [,3]\n"
              "[1,]\n"
              "[2,]\n"
              "[3,]\n"
              "\n"
              ", , 0, 2\n"
              "\n"
              "     [,1] [,2] [,3]\n"
              "[1,]\n"
              "[2,]\n"
              "[3,]\n"
              "\n"
              " [ reached getOption(max.print) -- omitted 1 slice(s) ]");
}
TEST(display_array_3x3x0x3_1_slice_2_rows_ommitted) {
  reinitOptions();
  cfg::cfgmap.set("max.print"s, 12L);
  auto a = arr::Array<double>({3,3,0,3}, arr::Vector<double>());
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == 
              ", , 0, 1\n"
              "\n"
              "     [,1] [,2] [,3]\n"
              "[1,]\n"
              "[2,]\n"
              "[3,]\n"
              "\n"
              ", , 0, 2\n"
              "\n"
              "     [,1] [,2] [,3]\n"
              "[1,]\n"
              "\n"
              " [ reached getOption(max.print) -- omitted 2 row(s) and 1 slice(s) ]");
}
TEST(display_array_3x3x0x3_2_slices_2_rows_ommitted) {
  reinitOptions();
  cfg::cfgmap.set("max.print"s, 3L);
  auto a = arr::Array<double>({3,3,0,3}, arr::Vector<double>{0});
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == 
              ", , 0, 1\n"
              "\n"
              "     [,1] [,2] [,3]\n"
              "[1,]\n"
              "\n"
              " [ reached getOption(max.print) -- omitted 2 row(s) and 2 slice(s) ]");
}
TEST(display_array_3x3x0x3_2_slices_2_rows_ommitted_max_print_not_a_multiple) {
  reinitOptions();
  cfg::cfgmap.set("max.print"s, 5L);
  auto a = arr::Array<double>({3,3,0,3}, arr::Vector<double>{});
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == 
              ", , 0, 1\n"
              "\n"
              "     [,1] [,2] [,3]\n"
              "[1,]\n"
              "\n"
              " [ reached getOption(max.print) -- omitted 2 row(s) and 2 slice(s) ]");
}
TEST(display_array_3x3x0x3_less_than_a_row) {
  reinitOptions();
  cfg::cfgmap.set("max.print"s, 2L);
  auto a = arr::Array<double>({3,3,0,3}, arr::Vector<double>{1});
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == 
              ", , 0, 1\n"
              "\n"
              " [,1] [,2] ...\n"
              "\n"
              " [ reached getOption(max.print) -- omitted 3 row(s) and 2 slice(s) ]");
}
// other types of array --------------------
TEST(display_array_string_3x3) {
  reinitOptions();
  auto a = arr::Array<arr::zstring>({3,3}, {"a","b","c","d","e","f","g","h","i"});
  ASSERT_TRUE(val::display(make_cow<val::VArrayS>(false, a)) == 
              "     [,1] [,2] [,3]\n"
              "[1,] \"a\"  \"d\"  \"g\" \n"
              "[2,] \"b\"  \"e\"  \"h\" \n"
              "[3,] \"c\"  \"f\"  \"i\" ");
}
TEST(display_array_string_4x2) {
  reinitOptions();
  auto a = arr::Array<arr::zstring>({4,2}, {"a","b","c","d","e","f","g","h"});
  ASSERT_TRUE(val::display(make_cow<val::VArrayS>(false, a)) == 
              "     [,1] [,2]\n"
              "[1,] \"a\"  \"e\" \n"
              "[2,] \"b\"  \"f\" \n"
              "[3,] \"c\"  \"g\" \n"
              "[4,] \"d\"  \"h\" ");
}
TEST(display_array_string_2x2x3) {
  reinitOptions();
  auto a = arr::Array<arr::zstring>({2,2,3}, {"a","b","c","d","e","f","g","h","i","j","k","l"});
  ASSERT_TRUE(val::display(make_cow<val::VArrayS>(false, a)) == 
              ", , 1\n"
              "\n"
              "     [,1] [,2]\n"
              "[1,] \"a\"  \"c\" \n"
              "[2,] \"b\"  \"d\" \n"
              "\n"
              ", , 2\n"
              "\n"
              "     [,1] [,2]\n"
              "[1,] \"e\"  \"g\" \n"
              "[2,] \"f\"  \"h\" \n"
              "\n"
              ", , 3\n"
              "\n"
              "     [,1] [,2]\n"
              "[1,] \"i\"  \"k\" \n"
              "[2,] \"j\"  \"l\" \n");
}
TEST(display_array_string_2x2x3_names) {
  reinitOptions();
  auto a = arr::Array<arr::zstring>({2,2,3}, 
    {"a","b","c","d","e","f","g","h","i","j","k","l"},
    {{"1","2"}, {"i","ii"}, {"I","II","III"}});
  ASSERT_TRUE(val::display(make_cow<val::VArrayS>(false, a)) == 
              ", , I\n"
              "\n"
              "    i  ii\n"
              "1 \"a\" \"c\"\n"
              "2 \"b\" \"d\"\n"
              "\n"
              ", , II\n"
              "\n"
              "    i  ii\n"
              "1 \"e\" \"g\"\n"
              "2 \"f\" \"h\"\n"
              "\n"
              ", , III\n"
              "\n"
              "    i  ii\n"
              "1 \"i\" \"k\"\n"
              "2 \"j\" \"l\"\n");
}
TEST(display_array_0x0x0_string) {
  reinitOptions();
  auto a = arr::Array<zstring>(Vector<idx_type>{0,0,0}, arr::Vector<zstring>());
  ASSERT_TRUE(val::display(make_cow<val::VArrayS>(false, a)) == "character(0x0x0)");
}
TEST(display_array_4x0_string) {
  reinitOptions();
  auto a = arr::Array<zstring>({4,0}, arr::Vector<zstring>());  
  ASSERT_TRUE(val::display(make_cow<val::VArrayS>(false, a)) ==
              "[1,]\n"
              "[2,]\n"
              "[3,]\n"
              "[4,]");
}
TEST(display_array_4x1_bool) {
  reinitOptions();
  auto a = arr::Array<bool>({4,1}, {true, false, true, false});  
  ASSERT_TRUE(val::display(make_cow<val::VArrayB>(false, a)) ==
              "      [,1]\n"
              "[1,] TRUE \n"
              "[2,] FALSE\n"
              "[3,] TRUE \n"
              "[4,] FALSE");
}
TEST(display_array_2x2_bool) {
  reinitOptions();
  auto a = arr::Array<bool>({2,2}, {true, false, true, false});  
  ASSERT_TRUE(val::display(make_cow<val::VArrayB>(false, a)) ==
              "      [,1]  [,2]\n"
              "[1,] TRUE  TRUE \n"
              "[2,] FALSE FALSE");
}
TEST(display_array_2x2_names_bool) {
  reinitOptions();
  auto a = arr::Array<bool>({2,2}, {true, false, true, false}, 
    {{"one","two"}, {"un","doi"}});  
  ASSERT_TRUE(val::display(make_cow<val::VArrayB>(false, a)) ==
              "       un   doi\n"
              "one TRUE  TRUE \n"
              "two FALSE FALSE");
}
TEST(display_array_0x0x0_bool) {
  reinitOptions();
  auto a = arr::Array<bool>(Vector<idx_type>{0,0,0}, arr::Vector<bool>());
  ASSERT_TRUE(val::display(make_cow<val::VArrayB>(false, a)) == "logical(0x0x0)");
}
TEST(display_array_4x0_bool) {
  reinitOptions();
  auto a = arr::Array<bool>({4,0}, arr::Vector<bool>());  
  ASSERT_TRUE(val::display(make_cow<val::VArrayB>(false, a)) ==
              "[1,]\n"
              "[2,]\n"
              "[3,]\n"
              "[4,]");
}
TEST(display_array_4x1_dtime) {
  reinitOptions();
  cfg::cfgmap.set("timezone"s, "America/New_York"s);
  auto a = arr::Array<Global::dtime>({4,1}, {Global::dtime(1s),Global::dtime(2s),
        Global::dtime(3s),Global::dtime(4s)});  
  ASSERT_TRUE(val::display(make_cow<val::VArrayDT>(false, a)) ==
              "                        [,1]\n"
              "[1,] 1969-12-31 19:00:01 EST\n"
              "[2,] 1969-12-31 19:00:02 EST\n"
              "[3,] 1969-12-31 19:00:03 EST\n"
              "[4,] 1969-12-31 19:00:04 EST");
}
TEST(display_array_2x2_dtime) {
  reinitOptions();
  auto a = arr::Array<Global::dtime>({2,2}, {Global::dtime(1ns),Global::dtime(2ns),
        Global::dtime(3ns),Global::dtime(4ns)});  
  ASSERT_TRUE(val::display(make_cow<val::VArrayDT>(false, a)) ==
              "                                  [,1]                              [,2]\n"
              "[1,] 1970-01-01 00:00:00.000000001 UTC 1970-01-01 00:00:00.000000003 UTC\n"
              "[2,] 1970-01-01 00:00:00.000000002 UTC 1970-01-01 00:00:00.000000004 UTC");
}
TEST(display_array_2x2_names_dtime) {
  reinitOptions();
  cfg::cfgmap.set("timezone"s, "America/New_York"s);
  auto a = arr::Array<Global::dtime>({2,2}, {Global::dtime(1ns),Global::dtime(0ns),
        Global::dtime(1ns),Global::dtime(0ns)}, 
    {{"one","two"}, {"un","doi"}});  
  ASSERT_TRUE(val::display(make_cow<val::VArrayDT>(false, a)) ==
              "                                   un                               doi\n"
              "one 1969-12-31 19:00:00.000000001 EST 1969-12-31 19:00:00.000000001 EST\n"
              "two 1969-12-31 19:00:00.000000000 EST 1969-12-31 19:00:00.000000000 EST");
}
TEST(display_array_0x0x0_dtime) {
  reinitOptions();
  auto a = arr::Array<Global::dtime>(Vector<idx_type>{0,0,0}, Vector<Global::dtime>());
  ASSERT_TRUE(val::display(make_cow<val::VArrayDT>(false, a)) == "time(0x0x0)");
}
TEST(display_array_4x0_dtime) {
  reinitOptions();
  auto a = arr::Array<Global::dtime>({4,0}, Vector<Global::dtime>());  
  ASSERT_TRUE(val::display(make_cow<val::VArrayDT>(false, a)) ==
              "[1,]\n"
              "[2,]\n"
              "[3,]\n"
              "[4,]");
}
// list ------------------------
TEST(display_list_empty) {
  reinitOptions();
  val::Value l = make_cow<val::VList>(false, vector<pair<string, val::Value>>());
  ASSERT_TRUE(val::display(l) == "list()");
}
TEST(display_list_double_one) {
  reinitOptions();
  val::Value l = make_cow<val::VList>(false, 
                                      vector<pair<string, 
                                      val::Value>>{make_pair("name", val::make_array(1.0))});
  ASSERT_TRUE(val::display(l) == 
              "$name\n"
              "[1] 1\n");
}
TEST(display_list_double_two) {
  reinitOptions();
  val::Value l = make_cow<val::VList>(false, 
                                      vector<pair<string, 
                                      val::Value>>{make_pair("name", val::make_array(1.0)), 
                                          make_pair("", val::make_array(2.0))});
  ASSERT_TRUE(val::display(l) == 
              "$name\n"
              "[1] 1\n"
              "\n"
              "[[2]]\n"
              "[1] 2\n");
}
TEST(display_list_of_list) {
  reinitOptions();
  val::Value l = make_cow<val::VList>(false, vector<pair<string, val::Value>>{
      make_pair("name", val::make_array(1.0))
      ,make_pair("", val::make_array(2.0))
      ,make_pair("", make_cow<val::VList>(false, vector<pair<string, val::Value>>{
      make_pair("sublist", val::make_array(1.1))
      ,make_pair("", val::make_array(2.1))}))
      ,make_pair("", val::make_array(3.0))});
  ASSERT_TRUE(val::display(l) == 
              "$name\n"
              "[1] 1\n"
              "\n"
              "[[2]]\n"
              "[1] 2\n"
              "\n"
              "[[3]]\n"
              "[[3]]$sublist\n"
              "[1] 1.1\n"
              "\n"
              "[[3]][[2]]\n"
              "[1] 2.1\n"
              "\n"
              "\n"
              "[[4]]\n"
              "[1] 3\n");
}
TEST(display_list_array) {
  reinitOptions();
  val::Value l = make_cow<val::VList>(false, vector<pair<string, val::Value>>{
      make_pair("name", val::make_array(1.0))
        ,make_pair("", val::make_array(2.0))
        ,make_pair("an_array", make_cow<val::VArrayD>(false, val::VArrayD({2,2}, {1,2,3,4})))
        ,make_pair("", val::make_array(3.0))});
  ASSERT_TRUE(val::display(l) ==     
              "$name\n"
              "[1] 1\n"
              "\n"
              "[[2]]\n"
              "[1] 2\n"
              "\n"
              "$an_array\n"
              "     [,1] [,2]\n"
              "[1,] 1    3   \n"
              "[2,] 2    4   \n"
              "\n"
              "[[4]]\n"
              "[1] 3\n");
}
TEST(display_vector_large) {
  reinitOptions();
  cfg::cfgmap.set("max.print"s, 3L);
  const unsigned len = 10*1000*1000;
  auto data = arr::Vector<double>(len);
  std::iota(data.begin(), data.end(), 1);
  auto a = arr::Array<double>({len}, data);
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == 
              "[1] 1 2 3\n"
              " [ reached getOption(max.print) -- omitted 9999997 entries ]");
}
TEST(display_matrix_large) {
  reinitOptions();
  cfg::cfgmap.set("max.print"s, 4L);
  const unsigned rows = 5*1000*1000;
  const unsigned cols = 2;
  auto data = arr::Vector<double>(rows*cols);
  std::iota(data.begin(), data.end(), 1);
  auto a = arr::Array<double>({rows, cols}, data);
  ASSERT_TRUE(val::display(make_cow<val::VArrayD>(false, a)) == 
              "     [,1]    [,2]\n"
              "[1,] 1    5000001\n"
              "[2,] 2    5000002\n"
              " [ reached getOption(max.print) -- omitted 4999998 row(s) ]");
}


int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
