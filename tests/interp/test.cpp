// -*- compile-command: "make -j -k test" -*-

// Copyright () 2015 Leonardo Silvestri
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


#include "../interp_setup.hpp"
#include "timezone/ztime.hpp"


// constructors ---------
TEST(interp_NULL) {
  auto eout = parse("NULL\n");
  ASSERT_TRUE(eval(eout) == val::VNull());
}
TEST(interp_double) {
  auto eout = parse("2.0\n");
  ASSERT_TRUE(eval(eout) == val::make_array(2.0));
}
TEST(interp_string) {
  auto eout = parse("\"string\"\n");
  ASSERT_TRUE(eval(eout) == val::make_array(zstring("string")));
}
TEST(interp_dtime) {
  auto eout = parse("|.2015-03-09 06:38:01 America/New_York.|\n");
  cout << "dtime: " << val::to_string(eval(eout)) << endl;
  ASSERT_TRUE(eval(eout) == 
              val::make_array(tz::dtime_from_string("2015-03-09 06:38:01 America/New_York")));
}
TEST(interp_interval) {
  auto eout = parse("|+2015-03-09 06:38:01 America/New_York -> 2015-03-10 06:38:01 America/New_York+|\n");
  cout << "dtime: " << val::to_string(eval(eout)) << endl;
  ASSERT_TRUE(eval(eout) == 
               val::make_array(tz::interval_from_string("|+2015-03-09 06:38:01 America/New_York -> 2015-03-10 06:38:01 America/New_York+|")));
}

// scalar double arithmetics:
TEST(interp_Unop_minus_double) {
  auto eout = parse("-2.0\n");
  ASSERT_TRUE(eval(eout) == val::make_array(-2.0));
}
TEST(interp_Binop_plus_double) {
  auto eout = parse("1.0 + 2.0\n");
  ASSERT_TRUE(eval(eout) == val::make_array(3.0));
}
TEST(interp_Binop_minus_double) {
  auto eout = parse("1.0 - 2.0\n");
  ASSERT_TRUE(eval(eout) == val::make_array(-1.0));
}
TEST(interp_Binop_mul_double) {
  auto eout = parse("3.0 * 2.0\n");
  ASSERT_TRUE(eval(eout) == val::make_array(6.0));
}
TEST(interp_Binop_div_double) {
  auto eout = parse("3.0 / 2.0\n");
  ASSERT_TRUE(eval(eout) == val::make_array(1.5));
}

// scalar bool ----------
TEST(interp_bool_true) {
  auto eout = parse("TRUE\n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_bool_false) {
  auto eout = parse("FALSE\n");
  ASSERT_TRUE(eval(eout) == val::make_array(false));
}
TEST(interp_bool_and_00) {
  auto eout = parse("FALSE && FALSE\n");
  ASSERT_TRUE(eval(eout) == val::make_array(false));
}
TEST(interp_bool_and_01) {
  auto eout = parse("FALSE && TRUE\n");
  ASSERT_TRUE(eval(eout) == val::make_array(false));
}
TEST(interp_bool_and_10) {
  auto eout = parse("TRUE && FALSE\n");
  ASSERT_TRUE(eval(eout) == val::make_array(false));
}
TEST(interp_bool_and_11) {
  auto eout = parse("TRUE && TRUE\n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_bool_or_00) {
  auto eout = parse("FALSE || FALSE\n");
  ASSERT_TRUE(eval(eout) == val::make_array(false));
}
TEST(interp_bool_or_01) {
  auto eout = parse("FALSE || TRUE\n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_bool_or_10) {
  auto eout = parse("TRUE || FALSE\n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_bool_or_11) {
  auto eout = parse("TRUE || TRUE\n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_bool_not_true) {
  auto eout = parse("!TRUE\n");
  ASSERT_TRUE(eval(eout) == val::make_array(false));
}
TEST(interp_bool_not_false) {
  auto eout = parse("!FALSE\n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}

// scalar strings -------
TEST(interp_string_0length) {
  auto eout = parse("\"\"\n");
  ASSERT_TRUE(eval(eout) == val::make_array(zstring("")));
}

// equality -------------
TEST(interp_eq_double_true) {
  auto eout = parse("1.0==1.0\n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_eq_double_false) {
  auto eout = parse("1.001==1.0\n");
  ASSERT_TRUE(eval(eout) == val::make_array(false));
}
TEST(interp_eq_bool_true) {
  auto eout = parse("TRUE==TRUE\n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_eq_bool_false) {
  auto eout = parse("FALSE==FALSE\n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_eq_bool_ne) {
  auto eout = parse("FALSE==TRUE\n");
  ASSERT_TRUE(eval(eout) == val::make_array(false));
}
TEST(interp_eq_dtime_true) {
  auto eout = parse("|.2015-03-09 06:38:01 America/New_York.| == "
        "|.2015-03-09 06:38:01 America/New_York.|\n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_eq_dtime_false) {
  auto eout = parse("|.2015-03-09 06:38:01 America/New_York.| == "
        "|.2015-03-09 06:38:02 America/New_York.|\n");
  ASSERT_TRUE(eval(eout) == val::make_array(false));
}
TEST(interp_eq_interval_true) {
  auto eout = parse("|+2015-03-09 06:38:01 America/New_York -> 2015-03-10 06:38:01 America/New_York+| == "
        "|+2015-03-09 06:38:01 America/New_York -> 2015-03-10 06:38:01 America/New_York+| \n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_eq_interval_false) {
  auto eout = parse("|+2015-03-09 06:38:01 America/New_York -> 2015-03-10 06:38:01 America/New_York+| == "
        "|+2015-03-09 06:38:01 America/New_York -> 2015-03-10 06:38:02 America/New_York+| \n");
  ASSERT_TRUE(eval(eout) == val::make_array(false));
}

// inequality -----------
TEST(interp_neq_double_false) {
  auto eout = parse("1.0!=1.0\n");
  ASSERT_TRUE(eval(eout) == val::make_array(false));
}
TEST(interp_neq_double_true) {
  auto eout = parse("1.0!=1.001\n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_neq_bool_true) {
  auto eout = parse("TRUE!=TRUE\n");
  ASSERT_TRUE(eval(eout) == val::make_array(false));
}
TEST(interp_neq_bool_false) {
  auto eout = parse("FALSE!=FALSE\n");
  ASSERT_TRUE(eval(eout) == val::make_array(false));
}
TEST(interp_neq_bool_ne) {
  auto eout = parse("TRUE!=FALSE\n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_neq_dtime_false) {
  auto eout = parse("|.2015-03-09 06:38:01 America/New_York.| != "
        "|.2015-03-09 06:38:01 America/New_York.|\n");
  ASSERT_TRUE(eval(eout) == val::make_array(false));
}
TEST(interp_neq_dtime_true) {
  auto eout = parse("|.2015-03-09 06:38:01 America/New_York.| == "
        "|.2015-03-09 06:38:01 America/New_York.|\n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_neq_interval_true) {
  auto eout = parse("|+2015-03-09 06:38:01 America/New_York -> 2015-03-10 06:38:01 America/New_York+| != "
        "|+2015-03-09 06:38:01 America/New_York -> 2015-03-10 06:38:02 America/New_York+| \n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_neq_interval_false) {
  auto eout = parse("|+2015-03-09 06:38:01 America/New_York -> 2015-03-10 06:38:01 America/New_York+| != "
        "|+2015-03-09 06:38:01 America/New_York -> 2015-03-10 06:38:01 America/New_York+| \n");
  ASSERT_TRUE(eval(eout) == val::make_array(false));
}

// LT -------------------
TEST(interp_lt_double_false) {
  auto eout = parse("1.0<1.0\n");
  ASSERT_TRUE(eval(eout) == val::make_array(false));
}
TEST(interp_lt_double_true) {
  auto eout = parse("1.0<1.001\n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_lt_bool_false_true) {
  auto eout = parse("TRUE<TRUE\n");
  ASSERT_TRUE(eval(eout) == val::make_array(false));
}
TEST(interp_lt_bool_false_false) {
  auto eout = parse("FALSE<FALSE\n");
  ASSERT_TRUE(eval(eout) == val::make_array(false));
}
TEST(interp_lt_bool_ne) {
  auto eout = parse("FALSE<TRUE\n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_lt_dtime_true) {
  auto eout = parse("|.2015-03-09 06:38:00 America/New_York.| < "
        "|.2015-03-09 06:38:01 America/New_York.|\n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_lt_dtime_equal) {
  auto eout = parse("|.2015-03-09 06:38:00 America/New_York.| < "
        "|.2015-03-09 06:38:00 America/New_York.|\n");
  ASSERT_TRUE(eval(eout) == val::make_array(false));
}
TEST(interp_lt_dtime_false) {
  auto eout = parse("|.2016-03-09 06:38:01 America/New_York.| < "
        "|.2015-03-09 06:38:01 America/New_York.|\n");
  ASSERT_TRUE(eval(eout) == val::make_array(false));
}
TEST(interp_lt_interval_equal) {
  auto eout = parse("|+2015-03-09 06:38:01 America/New_York -> 2015-03-10 06:38:01 America/New_York+| < "
        "|+2015-03-09 06:38:01 America/New_York -> 2015-03-10 06:38:01 America/New_York+| \n");
  ASSERT_TRUE(eval(eout) == val::make_array(false));
}
TEST(interp_lt_interval_non_overlapping_true) {
  auto eout = parse("|+2015-03-09 06:38:01 America/New_York -> 2015-03-10 06:38:01 America/New_York+| < "
        "|+2016-03-09 06:38:01 America/New_York -> 2016-03-10 06:38:01 America/New_York+| \n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_lt_interval_overlapping_not_included_true) {
  auto eout = parse("|+2015-03-09 06:38:01 America/New_York -> 2016-03-10 06:38:01 America/New_York+| < "
        "|+2015-03-10 06:38:01 America/New_York -> 2016-03-10 06:38:02 America/New_York+| \n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_lt_interval_overlapping_included_true) {
  auto eout = parse("|+2015-03-09 06:38:01 America/New_York -> 2016-03-10 06:38:01 America/New_York+| < "
        "|+2015-03-10 06:38:01 America/New_York -> 2015-03-10 06:38:02 America/New_York+| \n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_lt_interval_sopen_true) {
  auto eout = parse("|+2015-03-09 06:38:01 America/New_York -> 2016-03-10 06:38:01 America/New_York+| < "
        "|-2015-03-09 06:38:01 America/New_York -> 2015-03-10 06:38:02 America/New_York+| \n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_lt_interval_eopen_true) {
  auto eout = parse("|+2015-03-09 06:38:01 America/New_York -> 2015-03-10 06:38:02 America/New_York-| < "
        "|+2015-03-09 06:38:01 America/New_York -> 2015-03-10 06:38:02 America/New_York+| \n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_lt_interval_false) {
  auto eout = parse("|+2015-03-09 06:38:01.000001 America/New_York -> 2015-03-10 06:38:01 America/New_York+| < "
        "|+2015-03-09 06:38:01 America/New_York -> 2015-03-10 06:38:01 America/New_York+| \n");
  ASSERT_TRUE(eval(eout) == val::make_array(false));
}

// LE -------------------
TEST(interp_le_double_false) {
  auto eout = parse("1.1<=1.0\n");
  ASSERT_TRUE(eval(eout) == val::make_array(false));
}
TEST(interp_le_double_true) {
  auto eout = parse("1.001<=1.001\n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_le_bool_true_true) {
  auto eout = parse("TRUE<=TRUE\n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_le_bool_true_false) {
  auto eout = parse("FALSE<=FALSE\n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_le_bool_ne) {
  auto eout = parse("FALSE<=TRUE\n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_le_dtime_true) {
  auto eout = parse("|.2015-03-09 06:38:00 America/New_York.| <= "
        "|.2015-03-09 06:38:01 America/New_York.|\n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_le_dtime_equal) {
  auto eout = parse("|.2015-03-09 06:38:00 America/New_York.| <= "
        "|.2015-03-09 06:38:00 America/New_York.|\n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_le_dtime_false) {
  auto eout = parse("|.2016-03-09 06:38:01 America/New_York.| <= "
        "|.2015-03-09 06:38:01 America/New_York.|\n");
  ASSERT_TRUE(eval(eout) == val::make_array(false));
}
TEST(interp_le_interval_equal) {
  auto eout = parse("|+2015-03-09 06:38:01 America/New_York -> 2015-03-10 06:38:01 America/New_York+| <= "
        "|+2015-03-09 06:38:01 America/New_York -> 2015-03-10 06:38:01 America/New_York+| \n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_le_interval_non_overlapping_true) {
  auto eout = parse("|+2015-03-09 06:38:01 America/New_York -> 2015-03-10 06:38:01 America/New_York+| <= "
        "|+2016-03-09 06:38:01 America/New_York -> 2016-03-10 06:38:01 America/New_York+| \n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_le_interval_overlapping_not_included_true) {
  auto eout = parse("|+2015-03-09 06:38:01 America/New_York -> 2016-03-10 06:38:01 America/New_York+| <= "
        "|+2015-03-10 06:38:01 America/New_York -> 2016-03-10 06:38:02 America/New_York+| \n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_le_interval_overlapping_included_true) {
  auto eout = parse("|+2015-03-09 06:38:01 America/New_York -> 2016-03-10 06:38:01 America/New_York+| <= "
        "|+2015-03-10 06:38:01 America/New_York -> 2015-03-10 06:38:02 America/New_York+| \n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_le_interval_sopen_true) {
  auto eout = parse("|+2015-03-09 06:38:01 America/New_York -> 2016-03-10 06:38:01 America/New_York+| <= "
        "|-2015-03-09 06:38:01 America/New_York -> 2015-03-10 06:38:02 America/New_York+| \n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_le_interval_eopen_true) {
  auto eout = parse("|+2015-03-09 06:38:01 America/New_York -> 2015-03-10 06:38:02 America/New_York-| <= "
        "|+2015-03-09 06:38:01 America/New_York -> 2015-03-10 06:38:02 America/New_York+| \n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));
}
TEST(interp_le_interval_false) {
  auto eout = parse("|+2015-03-09 06:38:01.000001 America/New_York -> 2015-03-10 06:38:01 America/New_York+| <= "
        "|+2015-03-09 06:38:01 America/New_York -> 2015-03-10 06:38:01 America/New_York+| \n");
  ASSERT_TRUE(eval(eout) == val::make_array(false));
}

// GT LLL

// GE LLL


// simple assignments:
TEST(interp_assign_one) {
  auto eout = parse("a <- 0.0\n");
  ASSERT_TRUE(eval(eout) == val::make_array(0.0));
}


// time
// test strings with complete definition of what's really allowed
// variables with complete definition of what's really allowed
// conversions int/bool
// conversions double/bool
// conversions int/double
// test assignment LLL
// lists ----------------
TEST(interp_empty_list) {
  auto eout = parse("list()\n");
  auto exp = vector<pair<string, val::Value>>();
  ASSERT_TRUE(eval(eout) == make_cow<val::VList>(false, exp));
}
TEST(interp_list_1_double) {
  auto eout = parse("list(1.0)\n");
  auto exp = vector<pair<string, val::Value>>{make_pair("", val::make_array(1.0))};
  ASSERT_TRUE(eval(eout) == make_cow<val::VList>(false, exp));
}
TEST(interp_list_3_double) {
  auto eout = parse("list(1,2,3)\n");
  auto exp = vector<pair<string,val::Value>>{
    make_pair("", val::make_array(1.0)),
    make_pair("", val::make_array(2.0)),
    make_pair("", val::make_array(3.0))};
  ASSERT_TRUE(eval(eout) == make_cow<val::VList>(false, exp));
}
//   list(c("1","2"), c("one", "two"), c("un", "deux"))
// vectors
// matrices
// arrays
TEST(interp_matrix_double_3_4) {
  auto eout = parse("matrix(1.0:12, 3, 4) \n");
  auto a = arr::Array<double>({3,4}, arr::Vector<double>{1,2,3,4,5,6,7,8,9,10,11,12});
  cout << "array: " << val::to_string(make_cow<val::VArrayD>(false, a)) << endl;
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayD>(false, a));
}
TEST(interp_matrix_bool_2_3) {
  auto eout = parse("matrix(c(TRUE, FALSE, TRUE, FALSE, TRUE, FALSE), 2, 3) \n");
  auto a = arr::Array<bool>({2,3}, Vector<bool>{1,0,1,0,1,0});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayB>(false, a));
}
TEST(interp_matrix_dtime_2_3) {
  auto eout = parse("a <- |.1969-12-31 19:00:00 America/New_York.|;"
        "matrix(c(a,a,a,a,a,a), 2, 3) \n");
  auto a = arr::Array<Global::dtime>({2,3}, {Global::dtime{0s},Global::dtime{0s},
    Global::dtime{0s},Global::dtime{0s},Global::dtime{0s},Global::dtime{0s}});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayDT>(false, a));
}
TEST(interp_matrix_duration_2_3) {
  auto eout = parse("a <- as.duration(1);"
        "matrix(c(a,a,a,a,a,a), 2, 3) \n");
  auto a = arr::Array<Global::duration>({2,3}, {1ns, 1ns, 1ns, 1ns, 1ns, 1ns});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayDUR>(false, a));
}
TEST(interp_matrix_interval_2_3) {
  auto eout = parse("a <-  |+1969-12-31 19:00:00 America/New_York -> 1969-12-31 19:00:01 America/New_York+|;"
        "b <-  |+1969-12-31 19:00:01 America/New_York -> 1969-12-31 19:00:02 America/New_York+|;"
        "cc <- |+1969-12-31 19:00:02 America/New_York -> 1969-12-31 19:00:03 America/New_York+|;"
        "d <-  |+1969-12-31 19:00:03 America/New_York -> 1969-12-31 19:00:04 America/New_York+|;"
        "e <-  |+1969-12-31 19:00:04 America/New_York -> 1969-12-31 19:00:05 America/New_York+|;"
        "f <-  |+1969-12-31 19:00:05 America/New_York -> 1969-12-31 19:00:06 America/New_York+|;"
        "matrix(c(a,b,cc,d,e,f), 2, 3) \n");
  using namespace std::chrono;
  using namespace std::literals;
  constexpr auto z = Global::dtime(0s);
  auto a = arr::Array<tz::interval>({2,3}, 
        {tz::interval(z+0s,z+1s,0,0), tz::interval(z+1s,z+2s,0,0),
         tz::interval(z+2s,z+3s,0,0), tz::interval(z+3s,z+4s,0,0), 
         tz::interval(z+4s,z+5s,0,0), tz::interval(z+5s,z+6s,0,0)});
  cout << "dtime: " << val::to_string(make_cow<val::VArrayIVL>(false, a)) << endl;
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayIVL>(false, a));
}
// LLL and the other types
// time series
TEST(interp_zts_3_2) {
  auto eout = parse("idx <- c(|.2015-03-09 06:38:01 America/New_York.|, "
        "|.2015-03-09 06:38:02 America/New_York.|, "
        "|.2015-03-09 06:38:03 America/New_York.|);"
        "zts(idx, 1.0:6, dim=c(3, 2), dimnames=list(NULL, c(\"one\", \"two\"))) \n");
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York");
  auto dt2 = tz::dtime_from_string("2015-03-09 06:38:02 America/New_York");
  auto dt3 = tz::dtime_from_string("2015-03-09 06:38:03 America/New_York");
  auto z = arr::zts({3,2}, {dt1, dt2, dt3}, {1,2,3,4,5,6}, {{}, {"one", "two"}});
  cout << "array: " << val::to_string(make_cow<arr::zts>(false, z)) << endl;
  cout << "array: " << val::to_string(eval(eout)) << endl;
  ASSERT_TRUE(eval(eout) == make_cow<arr::zts>(false, z));
}
// this should get fixed and then we can uncomment this test LLL
// TEST(interp_zts_3_2_automatic_dim) {
//   auto eout = parse("idx <- c(|.2015-03-09 06:38:01 America/New_York.|, "
//         "|.2015-03-09 06:38:02 America/New_York.|, "
//         "|.2015-03-09 06:38:03 America/New_York.|);"
//         "zts(idx=idx, data=1.0:6, dimnames=list(NULL, c(\"one\", \"two\"))) \n");
//   auto dt1 = tz::dtime_from_string("|.2015-03-09 06:38:01 America/New_York.|");
//   auto dt2 = tz::dtime_from_string("|.2015-03-09 06:38:02 America/New_York.|");
//   auto dt3 = tz::dtime_from_string("|.2015-03-09 06:38:03 America/New_York.|");
//   auto z = arr::zts({3,2}, {dt1, dt2, dt3}, {1,2,3,4,5,6}, {{}, {"one", "two"}});
//   ASSERT_TRUE(eval(eout) == make_cow<arr::zts>(false, z));
// }
TEST(interp_zts_2_2_2) {
  auto eout = parse("idx <- c(|.2015-03-09 06:38:01 America/New_York.|, "
        "|.2015-03-09 06:38:02 America/New_York.|);"
        "zts(idx, 1.0:8, dim=c(2,2,2), "
        "dimnames=list(NULL, c(\"one\", \"two\"), c(\"1\",\"2\"))) \n");
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York");
  auto dt2 = tz::dtime_from_string("2015-03-09 06:38:02 America/New_York");
  auto z = arr::zts({2,2,2}, {dt1, dt2}, {1,2,3,4,5,6,7,8}, {{}, {"one", "two"}, {"1","2"}});
  ASSERT_TRUE(eval(eout) == make_cow<arr::zts>(false, z));
}


// operator :
// maybe test here for the builtin functions? or maybe in another file with same setup
// expression lists: {} , { 1.0 } { 1.0; 2.0 }
TEST(interp_el_0) {
  auto eout = parse("{ }\n");
  ASSERT_TRUE(eval(eout) == val::VNull());
}
TEST(interp_el_1) {
  auto eout = parse("{ 1.0 } \n");
  ASSERT_TRUE(eval(eout) == val::make_array(1.0));
}
TEST(interp_el_2) {
  auto eout = parse("{ 1.0; 2.0 }\n");
  ASSERT_TRUE(eval(eout) == val::make_array(2.0));
}

// function calls and argument evaluation
TEST(interp_funcall_no_args) {
  auto eout = parse("f <- function() 3; f()\n");
  ASSERT_TRUE(eval(eout) == val::make_array(3.0));
}  
TEST(interp_funcall_no_default) {
  auto eout = parse("f <- function(a,b,c) { a + b + c }; f(1,2,3) \n");
  ASSERT_TRUE(eval(eout) == val::make_array(6.0));
}
TEST(interp_funcall_all_default) {
  auto eout = parse("f <- function(a=1,b=2,c=3) { a + b + c }; f() \n");
  ASSERT_TRUE(eval(eout) == val::make_array(6.0));
}
TEST(interp_funcall_all_default_override_b) {
  auto eout = parse("f <- function(a=1,b=2,c=3) { a + b + c }; f(b=4) \n");
  ASSERT_TRUE(eval(eout) == val::make_array(8.0));
}
TEST(interp_funcall_all_default_override_all) {
  auto eout = parse("f <- function(a=1,b=2,c=3) { a + b + c }; f(a=-1,b=-2,c=-3) \n");
  ASSERT_TRUE(eval(eout) == val::make_array(-6.0));
}
TEST(interp_funcall_b_default) {
  auto eout = parse("f <- function(a,b=2,c) { a + b + c }; f(1,3) \n");
  ASSERT_THROW(eval(eout), interp::EvalException);
}
TEST(interp_funcall_ellipsis_only) {
  auto eout = parse("f <- function(...) { }; f(1,2,3,4) \n");
  ASSERT_TRUE(eval(eout) == val::VNull());
}
TEST(interp_funcall_ellipsis_only_call_with_no_arg) {
  auto eout = parse("f <- function(...) { }; f() \n");
  ASSERT_TRUE(eval(eout) == val::VNull());
}
TEST(interp_funcall_ellipsis_only_named_args) {
  auto eout = parse("f <- function(...) { }; f(a=1,b=2,c=3,d=4) \n");
  ASSERT_TRUE(eval(eout) == val::VNull());
}
TEST(interp_funcall_repeated_formal_arg) {
  auto eout = parse("f <- function(a, b, c, a) { } \n");
  ASSERT_THROW(eval(eout), std::range_error, "repeated formal argument 'a'");
}
TEST(interp_funcall_repeated_formal_arg_with_default) {
  auto eout = parse("f <- function(a=1, b, c, a) { } \n");
  ASSERT_THROW(eval(eout), std::range_error, "repeated formal argument 'a'");
}
TEST(interp_funcall_repeated_given_arg) {
  auto eout = parse("f <- function(a=1, b=2) a+b; f(a=1, b=2, a=2) \n");
  ASSERT_THROW(eval(eout), interp::EvalException, 
               "formal argument \"a\" matched by multiple actual arguments");
}
TEST(interp_funcall_repeated_ellipsis) {
  auto eout = parse("f <- function(..., ...) { } \n");
  ASSERT_THROW(eval(eout), std::range_error, "repeated formal argument '...'");
}

// subsetting -----------
TEST(interp_vector_subset_noargs) {
  auto eout = parse("a <- 1:10.0; a[] \n");
  auto a = arr::Array<double>({10}, arr::Vector<double>{1,2,3,4,5,6,7,8,9,10});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayD>(false, a));
}
TEST(interp_vector_subset_1arg) {
  auto eout = parse("a <- 1:10.0; a[1] \n");
  ASSERT_TRUE(eval(eout) == val::make_array(1.0));
}
TEST(interp_vector_subset_range) {
  auto eout = parse("a <- 1:10.0; a[1:2] \n");
  auto r = arr::Array<double>({2}, arr::Vector<double>{1,2});  
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayD>(false, r));
}
TEST(interp_array_subset_noargs) {
  auto eout = parse("a <- matrix(1:9.0, 3, 3); a[] \n");
  auto r = arr::Array<double>({3,3}, arr::Vector<double>{1,2,3,4,5,6,7,8,9});  
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayD>(false, r));
}
TEST(interp_array_subset_1elt) {
  auto eout = parse("a <- matrix(1:9.0, 3, 3); a[2, 2] \n");
  ASSERT_TRUE(eval(eout) == val::make_array(5.0));
}
TEST(interp_array_subset_1row) {
  auto eout = parse("a <- matrix(1:9.0, 3, 3); a[1, ] \n");
  auto a = arr::Array<double>({3}, arr::Vector<double>{1,4,7});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayD>(false, a));
}
TEST(interp_array_subset_1col) {
  auto eout = parse("a <- matrix(1:9.0, 3, 3); a[, 1] \n");
  auto a = arr::Array<double>({3}, arr::Vector<double>{1,2,3});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayD>(false, a));
}
TEST(interp_named_array_subset_noargs) {
  auto eout = parse("a <- matrix(1:9.0, 3, 3, "
                    "dimnames=list(c(\"1\",\"2\",\"3\"), c(\"a\",\"b\",\"c\"))); a[] \n");
  auto r = arr::Array<double>({3,3}, arr::Vector<double>{1,2,3,4,5,6,7,8,9}, 
    {{"1","2","3"},{"a","b","c"}});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayD>(false, r));
}
TEST(interp_named_array_subset_1elt) {
  auto eout = parse("a <- matrix(1:9.0, 3, 3, dimnames=list(c(\"1\",\"2\",\"3\"), c(\"a\",\"b\",\"c\"))); "
        "a[3, 3] \n");
  auto a = arr::Array<double>({1}, arr::Vector<double>{9}, {{"3"}});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayD>(false, a));
}
TEST(interp_named_array_subset_1row) {
  auto eout = parse("a <- matrix(1:9.0, 3, 3, dimnames=list(c(\"1\",\"2\",\"3\"), c(\"a\",\"b\",\"c\"))); "
        "a[1, ] \n");
  auto a = arr::Array<double>({3}, {1,4,7}, {{"a","b","c"}});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayD>(false, a));
}
TEST(interp_named_array_subset_1col) {
  auto eout = parse("a <- matrix(1:9.0, 3, 3, dimnames=list(c(\"1\",\"2\",\"3\"), c(\"a\",\"b\",\"c\"))); "
        "a[, 1] \n");
  auto a = arr::Array<double>({3}, {1,2,3}, {{"1","2","3"}});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayD>(false, a));
}
TEST(interp_named_array_named_subset_1elt) {
  auto eout = parse("a <- matrix(1:9.0, 3, 3, dimnames=list(c(\"1\",\"2\",\"3\"), c(\"a\",\"b\",\"c\"))); "
        "a[\"3\", \"c\"] \n");
  auto a = arr::Array<double>({1}, arr::Vector<double>{9}, {{"3"}});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayD>(false, a));
}
TEST(interp_named_array_named_subset_1row) {
  auto eout = parse("a <- matrix(1:9.0, 3, 3, dimnames=list(c(\"1\",\"2\",\"3\"), c(\"a\",\"b\",\"c\"))); "
        "a[\"1\", ] \n");
  auto a = arr::Array<double>({3}, {1,4,7}, {{"a","b","c"}});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayD>(false, a));
}
TEST(interp_named_array_named_subset_1col) {
  auto eout = parse("a <- matrix(1:9.0, 3, 3, dimnames=list(c(\"1\",\"2\",\"3\"), c(\"a\",\"b\",\"c\"))); "
        "a[, \"a\"] \n");
  auto a = arr::Array<double>({3}, {1,2,3}, {{"1","2","3"}});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayD>(false, a));
}

// subset various types:
TEST(interp_vector_subset_named_slice_bool) {
  auto eout = parse("a <- matrix(c(TRUE,FALSE,TRUE,FALSE), 2, 2, "
        "dimnames=list(c(\"1\",\"2\"), c(\"a\",\"b\"))); a[2, , drop=FALSE] \n");
  auto a = arr::Array<bool>({1,2}, arr::Vector<bool>{false,false}, {{"2"}, {"a","b"}});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayB>(false, a));
}
TEST(interp_vector_subset_named_slice_double) {
  auto eout = parse("a <- matrix(1:4, 2, 2, "
        "dimnames=list(c(\"1\",\"2\"), c(\"a\",\"b\"))); a[2, , drop=FALSE] \n");
  auto a = arr::Array<double>({1,2}, arr::Vector<double>{2,4}, {{"2"}, {"a","b"}});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayD>(false, a));
}
TEST(interp_vector_subset_named_slice_string) {
  auto eout = parse("a <- matrix(c(\"1\", 2:4), 2, 2, "
        "dimnames=list(c(\"1\",\"2\"), c(\"a\",\"b\"))); a[2, , drop=FALSE] \n");
  auto a = arr::Array<arr::zstring>({1,2}, {"2","4"}, {{"2"}, {"a","b"}});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayS>(false, a));
}
TEST(interp_vector_subset_named_slice_dtime) {
  auto eout = parse("a1 <- |.2015-03-09 06:38:01 America/New_York.|;"
        "a2 <- |.2015-03-09 06:38:02 America/New_York.|;"
        "a3 <- |.2015-03-09 06:38:03 America/New_York.|;"
        "a4 <- |.2015-03-09 06:38:04 America/New_York.|;"
        "a <- matrix(c(a1,a2,a3,a4), 2, 2, "
        "dimnames=list(c(\"1\",\"2\"), c(\"a\",\"b\"))); a[2, , drop=FALSE] \n");
  auto a = arr::Array<Global::dtime>({1,2}, 
    { tz::dtime_from_string("2015-03-09 06:38:02 America/New_York"),
      tz::dtime_from_string("2015-03-09 06:38:04 America/New_York")
    }, {{"2"}, {"a","b"}});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayDT>(false, a));
}
TEST(interp_vector_subset_named_slice_duration) {
  auto eout = parse("a1 <- as.duration(1);"
        "a2 <- as.duration(2);"
        "a3 <- as.duration(3);"
        "a4 <- as.duration(4);"
        "a <- matrix(c(a1,a2,a3,a4), 2, 2, "
        "dimnames=list(c(\"1\",\"2\"), c(\"a\",\"b\"))); a[2, , drop=FALSE] \n");
  using namespace std::chrono;
  using namespace std::literals;
  auto a = arr::Array<Global::duration>({1,2}, 
    { Global::duration(2ns), Global::duration(4ns) }, {{"2"}, {"a","b"}});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayDUR>(false, a));
}
TEST(interp_vector_subset_named_slice_interval) {
  auto eout = parse("a1 <- |+1969-12-31 19:00:00 America/New_York -> 1969-12-31 19:00:01 America/New_York+|;"
        "a2 <- |+1969-12-31 19:00:01 America/New_York -> 1969-12-31 19:00:02 America/New_York+|;"
        "a3 <- |+1969-12-31 19:00:02 America/New_York -> 1969-12-31 19:00:03 America/New_York+|;"
        "a4 <- |+1969-12-31 19:00:03 America/New_York -> 1969-12-31 19:00:04 America/New_York+|;"
        "a <- matrix(c(a1,a2,a3,a4), 2, 2, "
        "dimnames=list(c(\"1\",\"2\"), c(\"a\",\"b\"))); a[2, , drop=FALSE] \n");
  auto a = arr::Array<tz::interval>({1,2}, 
    { tz::interval_from_string("|+1969-12-31 19:00:01 America/New_York "
                               "-> 1969-12-31 19:00:02 America/New_York+|"),
      tz::interval_from_string("|+1969-12-31 19:00:03 America/New_York "
                               "-> 1969-12-31 19:00:04 America/New_York+|")
    }, {{"2"}, {"a","b"}});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayIVL>(false, a));
}
// subset out of bound
// null idx can't be out of bound
// int idx
TEST(interp_subset_array_out_of_bound_up) {
  auto eout = parse("a <- matrix(1:9.0, 3, 3); a[4,3] \n");
  ASSERT_THROW(eval(eout), std::range_error, "subscript out of bounds");
}
TEST(interp_subset_array_out_of_bound_low) {
  auto eout = parse("a <- matrix(1:9.0, 3, 3); a[0,3] \n");
  ASSERT_THROW(eval(eout), std::range_error, "subscript out of bounds");
}
TEST(interp_subset_array_out_of_bound_neg_low) {
  auto eout = parse("a <- matrix(1:9.0, 3, 3); a[c(-1,0), ] \n");
  ASSERT_THROW(eval(eout), std::range_error, "subscript out of bounds");
}
TEST(interp_subset_array_out_of_bound_neg_mixed1) {
  auto eout = parse("a <- matrix(1:9.0, 3, 3); a[c(-1,1),] \n");
  ASSERT_THROW(eval(eout), std::range_error, "mixed positive and negative subscripts");
}
TEST(interp_subset_array_out_of_bound_neg_mixed2) {
  auto eout = parse("a <- matrix(1:9.0, 3, 3); a[c(1,-1),] \n");
  ASSERT_THROW(eval(eout), std::range_error, "mixed positive and negative subscripts");
}
// bool idx
TEST(interp_subset_vector_bool_too_short) {
  auto eout = parse("(1:10.0)[TRUE] \n");
  ASSERT_THROW(eval(eout), interp::EvalException, "boolean index not equal to array extent");
}
TEST(interp_subset_vector_bool_too_long) {
  auto eout = parse("(1:2.0)[c(TRUE,TRUE,TRUE)] \n");
  ASSERT_THROW(eval(eout), interp::EvalException, "boolean index not equal to array extent");
}
// name idx
TEST(interp_subset_vector_incorrect_name) {
  auto eout = parse("(1:10.0)[\"a\"] \n");
  ASSERT_THROW(eval(eout), std::range_error, "subscript out of bounds");
}
TEST(interp_subset_array_incorrect_name) {
  auto eout = parse("a <- matrix(1:9.0, 3, 3, dimnames=list(c(\"1\",\"2\",\"3\"), c(\"a\",\"b\",\"c\"))); "
        "a[\"4\",] \n");
  ASSERT_THROW(eval(eout), std::range_error, "subscript out of bounds");
}
// subset incorrect nb of dims:
TEST(interp_subset_vector_wrong_dim) {
  auto eout = parse("(1:10.0)[,] \n");
  ASSERT_THROW(eval(eout), std::range_error, "incorrect number of dimensions");
}
TEST(interp_subset_array_wrong_dim_lo2) {
  auto eout = parse("(array(1,c(2,2,2)))[,] \n");
  ASSERT_THROW(eval(eout), std::range_error, "incorrect number of dimensions");
}
TEST(interp_subset_array_wrong_dim_hi2) {
  auto eout = parse("(array(1,c(2,2,2)))[,,,,] \n");
  ASSERT_THROW(eval(eout), std::range_error, "incorrect number of dimensions");
}


// subassign ------------
TEST(interp_subassign_vector_noargs) {
  auto eout = parse("(1:10.0)[] <- 2:11.0 \n");
  auto a = arr::Array<double>(arr::Vector<double>{2,3,4,5,6,7,8,9,10,11});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayD>(false, a));
}
TEST(interp_subassign_vector_1arg) {
  auto eout = parse("(1:10.0)[1] <- 2.0 \n");
  auto a = arr::Array<double>({10}, arr::Vector<double>{2,2,3,4,5,6,7,8,9,10});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayD>(false, a));
}
TEST(interp_subassign_vector_range) {
  auto eout = parse("a <- 1:10.0; a[8:10] <- 1:3.0 \n");
  auto a = arr::Array<double>({10}, arr::Vector<double>{1,2,3,4,5,6,7,1,2,3});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayD>(false, a));
}
TEST(interp_subassign_array_noargs) {
  auto eout = parse("a <- matrix(1:9.0, 3, 3); a[] <- 1 \n");
  auto a = arr::Array<double>({3,3}, arr::Vector<double>{1,2,3,4,5,6,7,8,9});
  auto r = arr::Array<double>({3,3}, arr::Vector<double>{1});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayD>(false, r));
}
TEST(interp_subassign_array_1elt) {
  auto eout = parse("a <- matrix(1:9.0, 3, 3); a[2, 2] <- 10.0 \n");
  auto a = arr::Array<double>({3,3}, arr::Vector<double>{1,2,3,4,10,6,7,8,9});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayD>(false, a));
}
TEST(interp_subassign_array_1row) {
  auto eout = parse("a <- matrix(1:9.0, 3, 3); a[1, ] <- 10:8\n");
  auto a = arr::Array<double>({3,3}, arr::Vector<double>{10,2,3,9,5,6,8,8,9});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayD>(false, a));
}
TEST(interp_subassign_array_1col) {
  auto eout = parse("a <- matrix(1:9.0, 3, 3); a[, 1] <- 11:13\n");
  auto a = arr::Array<double>({3,3}, arr::Vector<double>{11,12,13,4,5,6,7,8,9});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayD>(false, a));
}
TEST(interp_subassign_array_subarray) {
  auto eout = parse("a <- matrix(1:9.0, 3, 3); a[1:2, 1:2] <- matrix(11:14, 2, 2)\n");
  auto a = arr::Array<double>({3,3}, arr::Vector<double>{11,12,3,13,14,6,7,8,9});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayD>(false, a));
}
// subassign type of a == type of b
TEST(interp_subassign_array_bool_bool) {
  auto eout = parse("matrix(FALSE, 2, 2)[1,1] <- TRUE \n");
  auto a = arr::Array<bool>({2,2}, arr::Vector<bool>{true,false,false,false});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayB>(false, a));
}
TEST(interp_subassign_array_string_string) {
  auto eout = parse("matrix(\"ho\", 2, 2)[1,1] <- c(\"hi\") \n");
  auto a = arr::Array<arr::zstring>({2,2}, {"hi","ho","ho","ho"});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayS>(false, a));
}
TEST(interp_subassign_array_dtime_dtime) {
  auto eout = parse("a1 <- |.2015-03-09 06:38:01 America/New_York.|;"
        "a2 <- |.2015-03-09 06:38:02 America/New_York.|;"
        "a3 <- |.2015-03-09 06:38:03 America/New_York.|;"
        "a4 <- |.2015-03-09 06:38:04 America/New_York.|;"
        "a <- matrix(c(a1,a2,a3,a4), 2, 2, dimnames=list(c(\"1\",\"2\"), c(\"a\",\"b\")));"
        "a[2,2] <- |.2015-03-09 06:38:07 America/New_York.| \n"
        );
  auto a = arr::Array<Global::dtime>({2,2}, 
    { tz::dtime_from_string("2015-03-09 06:38:01 America/New_York"),
      tz::dtime_from_string("2015-03-09 06:38:02 America/New_York"),
      tz::dtime_from_string("2015-03-09 06:38:03 America/New_York"),
      tz::dtime_from_string("2015-03-09 06:38:07 America/New_York")
    }, {{"1","2"}, {"a","b"}});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayDT>(false, a));
}
TEST(interp_subassign_vector_double_to_bool) {
  auto eout = parse("c(FALSE,FALSE,TRUE,FALSE,TRUE,FALSE)[1] <- 1.0  \n");
  auto a = arr::Array<bool>({6}, arr::Vector<bool>{true,false,true,false,true,false});
  auto aa =  make_cow<val::VArrayB>(false, a);
  // const auto& bb = get<val::SpVAB>(eval(eout));
  // cout << bb->size() << endl;
  // cout << (*bb)[0] << endl;
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayB>(false, a));
}
TEST(interp_subassign_array_string_to_double) {
  auto eout = parse("matrix(1.1, 2, 2)[1,1] <- \"hi\" \n");
  auto a = arr::Array<arr::zstring>({2,2}, {"hi","1.1","1.1","1.1"});
  //  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayS>(false, a));
  ASSERT_THROW(eval(eout), range_error, "conversion not defined for string to double");
}
TEST(interp_subassign_array_string_to_bool) {
  auto eout = parse("matrix(TRUE, 2, 2)[1,1] <- c(\"hi\") \n");
  auto a = arr::Array<arr::zstring>({2,2}, {"hi","TRUE","TRUE","TRUE"});
  //  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayS>(false, a));
  ASSERT_THROW(eval(eout), range_error, "conversion not defined for string to logical");
}
// subassign type of a > type of b
TEST(interp_subassign_vector_bool_to_double) {
  auto eout = parse("(1:10.0)[3:4] <- FALSE  \n");
  auto a = arr::Array<double>({10}, arr::Vector<double>{1,2,0,0,5,6,7,8,9,10});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayD>(false, a));
}
TEST(interp_subassign_vector_double_to_string) {
  auto eout = parse("c(\"1\",\"2\",\"3\",\"4\",\"5\")[1:3] <- c(2,3,4) \n");
  auto a = arr::Array<arr::zstring>({5}, {"2","3","4","4","5"});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayS>(false, a));
}
TEST(interp_subassign_array_bool_to_string) {
  auto eout = parse("matrix(\"1.1\", 2, 2)[1,] <- TRUE  \n");
  auto a = arr::Array<arr::zstring>({2,2}, {"TRUE","1.1","TRUE","1.1"});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayS>(false, a));
}
// subassign 3D
TEST(interp_subassign_array_scalar) {
  auto eout = parse("a <- array(1:24.0, c(2,3,4), dimnames=list(c(\"1\",\"2\"), c(\"i\",\"ii\",\"iii\"), "
        "c(\"I\",\"II\",\"III\",\"IV\"))); a[2,2,3] <- 100.0\n");
  auto v = arr::Vector<double>{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,100,17,18,19,20,21,22,23,24};
  auto a = arr::Array<double>({2,3,4}, v, {{"1","2"}, {"i","ii","iii"}, {"I","II","III","IV"}});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayD>(false, a));
}
TEST(interp_subassign_array_matrix) {
  auto eout = parse("a <- array(1:24.0, c(2,3,4), dimnames=list(c(\"1\",\"2\"), c(\"i\",\"ii\",\"iii\"), "
        "c(\"I\",\"II\",\"III\",\"IV\"))); a[,,3] <- matrix(11:16, 2, 3)\n");
  auto v = arr::Vector<double>{1,2,3,4,5,6,7,8,9,10,11,12,11,12,13,14,15,16,19,20,21,22,23,24};
  auto a = arr::Array<double>({2,3,4}, v, {{"1","2"}, {"i","ii","iii"}, {"I","II","III","IV"}});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayD>(false, a));
}
// subassign incorrect replacement length
TEST(interp_subassign_vector_datalen_lo) {
  auto eout = parse("(1:10)[1:3] <- c(1,2) \n");
  ASSERT_THROW(eval(eout), std::range_error, 
               "number of items to replace is not equal to the replacement length");
}
TEST(interp_subassign_vector_datalen_hi) {
  auto eout = parse("(1:10)[1:3] <-1:4 \n");
  ASSERT_THROW(eval(eout), std::range_error, 
               "number of items to replace is not equal to the replacement length");
}
TEST(interp_subassign_array_datalen_hi) {
  auto eout = parse("(array(1,c(2,2,2)))[,,] <- matrix(1,3,3) \n");
  ASSERT_THROW(eval(eout), std::range_error, 
               "number of items to replace is not equal to the replacement length");
}
// subassign out of bound
TEST(interp_subassign_vector_out_of_bound_up) {
  auto eout = parse("(1:10)[0] <- c(1) \n");
  ASSERT_THROW(eval(eout), std::range_error, "subscript out of bounds");
}
TEST(interp_subassign_vector_out_of_bound_low) {
  auto eout = parse("(1:10)[11] <- c(1) \n");
  ASSERT_THROW(eval(eout), std::range_error, "subscript out of bounds");
}
TEST(interp_subassign_array_out_of_bound_up) {
  auto eout = parse("a <- matrix(1:9.0, 3, 3); a[4,3] <- 1 \n");
  ASSERT_THROW(eval(eout), std::range_error, "subscript out of bounds");
}
TEST(interp_subassign_array_out_of_bound_low) {
  auto eout = parse("a <- matrix(1:9.0, 3, 3); a[0,3] <- 1 \n");
  ASSERT_THROW(eval(eout), std::range_error, "subscript out of bounds");
}
// subassign incorrect nb of dims:
TEST(interp_subassign_vector_wrong_dim) {
  auto eout = parse("(1:10.0)[,] <- 1.0\n");
  ASSERT_THROW(eval(eout), std::range_error, "incorrect number of dimensions");
}
TEST(interp_subassign_array_wrong_dim_lo2) {
  auto eout = parse("(array(1,c(2,2,2)))[,] <- matrix(1,3,3) \n");
  ASSERT_THROW(eval(eout), std::range_error, "incorrect number of dimensions");
}
TEST(interp_subassign_array_wrong_dim_hi2) {
  auto eout = parse("(array(1.0,c(2,2,2)))[,,,,] <- 2.0 \n");
  ASSERT_THROW(eval(eout), std::range_error, "incorrect number of dimensions");
}

// dim ------------------
TEST(interp_dim_scalar) {
  auto eout = parse("dim(0) \n");
  ASSERT_TRUE(eval(eout) == val::make_array(1.0));  
}
TEST(interp_dim_vector) {
  auto eout = parse("dim(c(1,2,3)) \n");
  auto a = arr::Array<double>({1}, arr::Vector<double>{3});
  ASSERT_TRUE(eval(eout) == val::Value(make_cow<val::VArrayD>(false, a)));  
}
TEST(interp_dim_vector_one_elt) {
  auto eout = parse("dim(c(1)) \n");
  auto a = arr::Array<double>({1}, arr::Vector<double>{1});
  ASSERT_TRUE(eval(eout) == val::Value(make_cow<val::VArrayD>(false, a)));  
}
TEST(interp_dim_matrix) {
  auto eout = parse("dim(matrix(1,2,2)) \n");
  auto a = arr::Array<double>({2}, arr::Vector<double>{2,2});
  ASSERT_TRUE(eval(eout) == val::Value(make_cow<val::VArrayD>(false, a)));  
}
TEST(interp_dim_array) {
  auto eout = parse("dim(array(1,c(3,3,3,3))) \n");
  auto a = arr::Array<double>({4}, arr::Vector<double>{3,3,3,3});
  ASSERT_TRUE(eval(eout) == val::Value(make_cow<val::VArrayD>(false, a)));  
}
TEST(interp_dim_null_array) {
  auto eout = parse("dim(array(0,c(0,0,0,0))) \n");
  auto a = arr::Array<double>({4}, arr::Vector<double>{0,0,0,0});
  ASSERT_TRUE(eval(eout) == val::Value(make_cow<val::VArrayD>(false, a)));  
}

// do time conversions LLL

int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
