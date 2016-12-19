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


#include <crpcut.hpp>
#include <system_error>
#include <stdio.h>
#include "globals.hpp"
#include "misc.hpp"
#include "timezone/ztime.hpp"
#include "timezone/zone.hpp"
#include "../utils.hpp"


using namespace std;


tz::Zones tzones("/usr/share/zoneinfo");


// The following are not very interesting tests, and we don't want to
// add a dependency on an R installation in our unit tests. If needed
// though, the following shows how to use the 'runR' to match unit
// test results with R:
DISABLED_TEST(dtime_constructor_string) {
  auto dt = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York");
  auto s = "as.numeric(as.POSIXct(\"2015-03-09 06:38:01\", tz=\"America/New_York\"))";
  auto r = stoll(runR(s));  
  ASSERT_TRUE((dt.time_since_epoch().count() / decltype(dt)::period::den) == r);   
}
DISABLED_TEST(dtime_constructor_string_origin) {
  auto dt = tz::dtime_from_string("1969-12-31 19:00:00 America/New_York");
  auto s = "as.numeric(as.POSIXct(\"1969-12-31 19:00:00\", tz=\"America/New_York\"))";
  auto r = stoll(runR(s));  
  ASSERT_TRUE((dt.time_since_epoch().count() / decltype(dt)::period::den) == r);   
}
DISABLED_TEST(dtime_constructor_string_neg) {
  auto dt = tz::dtime_from_string("1950-03-09 06:38:01 America/New_York");
  auto s = "as.numeric(as.POSIXct(\"1950-03-09 06:38:01\", tz=\"America/New_York\"))";
  auto r = stoll(runR(s));  
  ASSERT_TRUE((dt.time_since_epoch().count() / decltype(dt)::period::den) == r);   
}

TEST(dtime_constructor_nsec) {
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:01.11 America/New_York"); 
  auto dt2 = tz::dtime_from_string("2015-03-09 06:38:01.11 America/New_York");
  ASSERT_TRUE(dt1 == dt2);   
}
TEST(dtime_constructor_nsec_long) {
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:01.999999999 America/New_York"); 
  auto dt2 = tz::dtime_from_string("2015-03-09 06:38:01.999999998 America/New_York");
  ASSERT_TRUE(dt1 == dt2 + Global::duration(1));
}
TEST(dtime_constructor_format_sep) {
  auto dt1 = tz::dtime_from_string("2015/03/09 06.38.01 America/New_York", 
                                   "%Y/%m/%d %H.%M.%S[.%s] %Z");
  auto dt2 = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York");
  ASSERT_TRUE(dt1 == dt2);   
}
TEST(dtime_constructor_format_y) {
  auto dt1 = tz::dtime_from_string("03/09/15 06.38.01 America/New_York", 
                                   "%m/%d/%y %H.%M.%S[.%s] %Z");
  auto dt2 = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York");
  ASSERT_TRUE(dt1 == dt2);   
}
TEST(dtime_constructor_format_no_time) {
  auto dt1 = tz::dtime_from_string("03/09/15 America/New_York", 
                                   "%m/%d/%y %Z");
  auto dt2 = tz::dtime_from_string("2015-03-09 00:00:00 America/New_York");
  ASSERT_TRUE(dt1 == dt2);   
}
TEST(dtime_constructor_format_no_tz) {
  ASSERT_THROW(tz::dtime_from_string("2015-03-09 06:38:01"), 
               std::range_error, "timezone must be specified");   
}
TEST(dtime_constructor_format_nsec_not_optional) {
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:01.3333 America/New_York", 
                                   "%Y-%m-%d %H:%M:%S.%s %Z");
  auto dt2 = tz::dtime_from_string("2015-03-09 06:38:01.3333 America/New_York");
  ASSERT_TRUE(dt1 == dt2);   
}
TEST(dtime_constructor_format_no_sep) {
  // test that we read the correct number of chars:
  auto dt1 = tz::dtime_from_string("20150309063801.3333 America/New_York", 
                                   "%Y%m%d%H%M%S.%s %Z");
  auto dt2 = tz::dtime_from_string("2015-03-09 06:38:01.3333 America/New_York");
  ASSERT_TRUE(dt1 == dt2);   
}
TEST(dtime_constructor_format_no_leading0) {
  auto dt1 = tz::dtime_from_string("3/9/15 America/New_York", 
                                   "%m/%d/%y %Z");
  auto dt2 = tz::dtime_from_string("2015-03-09 00:00:00 America/New_York");
  ASSERT_TRUE(dt1 == dt2);   
}
TEST(dtime_constructor_throw) {
  ASSERT_THROW(tz::dtime_from_string("2015-030-09 06:38:01 America/New_York"), std::range_error);
  ASSERT_THROW(tz::dtime_from_string("|2015-030-09 06:38:01 America/New_York"), std::range_error);
  ASSERT_THROW(tz::dtime_from_string("2015-13-09 06:38:01 America/New_York"), std::range_error);
  ASSERT_THROW(tz::dtime_from_string("2015-11-32 06:38:01 America/New_York"), std::range_error);
  ASSERT_THROW(tz::dtime_from_string("2015-11-30 24:38:01 America/New_York"), std::range_error);
  ASSERT_THROW(tz::dtime_from_string("2015-11-30 06:60:01 America/New_York"), std::range_error);
  ASSERT_THROW(tz::dtime_from_string("2015-11-30 06:38:60 America/New_York"), std::range_error);
  ASSERT_THROW(tz::dtime_from_string("2015-11-09 06:38:01 Antarctica/New_York"), std::range_error);
  ASSERT_THROW(tz::dtime_from_string("2015-11-09 06:38:01 Antarctica/New_York&"), std::range_error);
  ASSERT_THROW(tz::dtime_from_string("2015-11-09 06:38:01 Antarctica/New_York",
                                     "%p"), std::range_error);
}
TEST(dtime_eq_true) {
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York");
  auto dt2 = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York");
  ASSERT_TRUE(dt1 == dt2);   
}
TEST(dtime_eq_false_0) {
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York");
  auto dt2 = tz::dtime_from_string("2015-03-09 06:38:02 America/New_York");
  ASSERT_TRUE(!(dt1 == dt2));   
}
TEST(dtime_eq_false_1) {
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:00 America/New_York");
  auto dt2 = tz::dtime_from_string("2015-03-09 06:38:00.1 America/New_York");
  ASSERT_TRUE(!(dt1 == dt2));   
}


// test ambiguous and non-representable times LLL

// test it's not possible to construct intervals where the start is <= to the end LLL

TEST(interval_eq_true) {
  auto dt1 = tz::interval_from_string("|+2015-03-09 06:38:01 America/New_York "
                                      "-> 2015-03-10 06:38:01 America/New_York+|");
  auto dt2 = tz::interval_from_string("|+2015-03-09 06:38:01 America/New_York "
                                      "-> 2015-03-10 06:38:01 America/New_York+|");
  ASSERT_TRUE(dt1 == dt2);   
  dt1 = tz::interval_from_string("|-2015-03-09 06:38:01 America/New_York "
                                 "-> 2015-03-10 06:38:01 America/New_York-|");
  dt2 = tz::interval_from_string("|-2015-03-09 06:38:01 America/New_York "
                                 "-> 2015-03-10 06:38:01 America/New_York-|");
  ASSERT_TRUE(dt1 == dt2);   
  dt1 = tz::interval_from_string("|+2015-03-09 06:38:01 America/New_York "
                                 "-> 2015-03-10 06:38:01 America/New_York-|");
  dt2 = tz::interval_from_string("|+2015-03-09 06:38:01 America/New_York "
                                 "-> 2015-03-10 06:38:01 America/New_York-|");
  ASSERT_TRUE(dt1 == dt2);   
  dt1 = tz::interval_from_string("|-2015-03-09 06:38:01 America/New_York "
                                 "-> 2015-03-10 06:38:01 America/New_York+|");
  dt2 = tz::interval_from_string("|-2015-03-09 06:38:01 America/New_York "
                                 "-> 2015-03-10 06:38:01 America/New_York+|");
  ASSERT_TRUE(dt1 == dt2);   
}

TEST(interval_eq_false) {
  auto dt1 = tz::interval_from_string("|+2015-03-09 06:38:01 America/New_York "
                                      "-> 2015-03-10 06:38:01 America/New_York+|");
  auto dt2 = tz::interval_from_string("|+2015-03-09 06:38:01 America/New_York "
                                      "-> 2015-03-10 06:38:02 America/New_York+|");
  ASSERT_TRUE(!(dt1 == dt2));   
  dt1 = tz::interval_from_string("|+2015-03-09 06:38:01 America/New_York "
                                      "-> 2015-03-10 06:38:01 America/New_York-|");
  dt2 = tz::interval_from_string("|-2015-03-09 06:38:01 America/New_York "
                                      "-> 2015-03-10 06:38:01 America/New_York-|");
  ASSERT_TRUE(!(dt1 == dt2));   
  dt1 = tz::interval_from_string("|-2015-03-09 06:38:01 America/New_York "
                                      "-> 2015-03-10 06:38:01 America/New_York-|");
  dt2 = tz::interval_from_string("|+2015-03-09 06:38:01 America/New_York "
                                      "-> 2015-03-10 06:38:01 America/New_York-|");
  ASSERT_TRUE(!(dt1 == dt2));   
  dt1 = tz::interval_from_string("|-2015-03-09 06:38:01 America/New_York "
                                      "-> 2015-03-10 06:38:01 America/New_York+|");
  dt2 = tz::interval_from_string("|-2015-03-09 06:38:01.001 America/New_York "
                                      "-> 2015-03-10 06:38:01 America/New_York+|");
  ASSERT_TRUE(!(dt1 == dt2));   
}


int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
