// -*- compile-command: "make -k -j test" -*-

// Copyright (C) 2016 Leonardo Silvestri
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
#include "timezone/ztime.hpp"
#include "../utils_dtime.hpp"
#include "period.hpp"


using namespace std::string_literals;

tz::Zones tzones("/usr/share/zoneinfo");

TEST(period_parse) {
  ASSERT_TRUE(tz::period("00:00:00.987_654_321") == tz::period("/00:00:00.987654321"));
  ASSERT_TRUE(tz::period("00:00:00.987_654_321") == tz::period("/00:00:00.987_654321"));
  ASSERT_TRUE(tz::period("/00:00:00.987_654_321") == tz::period("/00:00:00.987654_321"));
  ASSERT_TRUE(tz::period("/00:00:00.987") == tz::period("/00:00:00.987_000"));
  ASSERT_TRUE(tz::period("/00:00:00.987") == tz::period("/00:00:00.987000"));
}
TEST(period_plus_1y) {
  auto dt = tz::dtime_from_string("2015-03-09 06:38:00 America/New_York"s);
  auto p = tz::period("12m");
  auto exp = tz::dtime_from_string("2016-03-09 06:38:00 America/New_York"s);
  auto& tz = tzones.find("America/New_York");
  ASSERT_TRUE(tz::plus(dt, p, tz) == exp);
  ASSERT_TRUE(tz::plus(p, dt, tz) == exp);
}
TEST(period_plus_1y_1m) {
  auto dt = tz::dtime_from_string("2015-03-09 06:38:00 America/New_York"s);
  auto p = tz::period("13m");
  auto exp = tz::dtime_from_string("2016-04-09 06:38:00 America/New_York"s);
  auto& tz = tzones.find("America/New_York");
  ASSERT_TRUE(tz::plus(dt, p, tz) == exp);
  ASSERT_TRUE(tz::plus(p, dt, tz) == exp);
}
TEST(period_plus_1y_1m_1d) {
  auto dt = tz::dtime_from_string("2015-03-09 06:38:00 America/New_York"s);
  auto p = tz::period("13m1d");
  auto exp = tz::dtime_from_string("2016-04-10 06:38:00 America/New_York"s);
  auto& tz = tzones.find("America/New_York");
  ASSERT_TRUE(tz::plus(dt, p, tz) == exp);
  ASSERT_TRUE(tz::plus(p, dt, tz) == exp);
}
TEST(period_minus_1y) {
  auto dt = tz::dtime_from_string("2015-03-09 06:38:00 America/New_York"s);
  auto p = tz::period("12m");
  auto exp = tz::dtime_from_string("2014-03-09 06:38:00 America/New_York"s);
  auto& tz = tzones.find("America/New_York");
  ASSERT_TRUE(tz::minus(dt, p, tz) == exp);
}
TEST(period_minus_1y_1m) {
  auto dt = tz::dtime_from_string("2015-03-09 06:38:00 America/New_York"s);
  auto p = tz::period("13m");
  auto exp = tz::dtime_from_string("2014-02-09 06:38:00 America/New_York"s);
  auto& tz = tzones.find("America/New_York");
  ASSERT_TRUE(tz::minus(dt, p, tz) == exp);
}
TEST(period_minus_1y_1m_1d) {
  auto dt = tz::dtime_from_string("2015-03-09 06:38:00 America/New_York"s);
  auto p = tz::period("1y1m1d");
  auto exp = tz::dtime_from_string("2014-02-08 06:38:00 America/New_York"s);
  auto& tz = tzones.find("America/New_York");
  ASSERT_TRUE(tz::minus(dt, p, tz) == exp);
}
TEST(period_pos_neg_during_ops) {
  tz::period p1("1m2d");
  tz::period p2("2m3d");
  ASSERT_TRUE(p1+p2 == tz::period(3,5,0s));
  ASSERT_TRUE(p2-p1 == tz::period(1,1,0s));
  tz::period p3("2m1d");
  ASSERT_TRUE(p1-p3 == tz::period(-1,1,0s));
}
TEST(period_mul) {
  ASSERT_TRUE(tz::period("1m1d/01:01:01.001")*3 == tz::period(3,3,3h+3min+3s+1ms));
  ASSERT_TRUE(tz::period(3,3,0s) == tz::period("1m1d")*3 );
}
TEST(period_div) {
  ASSERT_TRUE(tz::period("1m1d/01:01:01.001")/2 == tz::period(0,0,0s));
  ASSERT_TRUE(tz::period("5m10d")/5  == tz::period(1,2,0s));
  ASSERT_TRUE(tz::period("-1m1d/1")/-1  == tz::period(1,-1,-1s));
  ASSERT_THROW(tz::period("1y1m")/0, std::logic_error, "divide by zero"); 
}
TEST(period_comp) {
  ASSERT_TRUE(tz::period("1m1d")      == tz::period(1,1,0s));
  ASSERT_TRUE(tz::period("3m3d")      == tz::period(3,3,0s));
  ASSERT_TRUE(tz::period("100m100d")  == tz::period(100,100,0s));
  ASSERT_TRUE(tz::period("1y1d")      == tz::period(12,1,0s));
  ASSERT_TRUE(tz::period("/01:00:00") == tz::period(0,0,1h));
  ASSERT_TRUE(tz::period("/01:00:00.000_000_999") == tz::period(0,0,1h+999ns));
  ASSERT_TRUE(tz::period("0y")        == tz::period(0,0,0s));
  ASSERT_TRUE(tz::period("1m1d")      != tz::period(1,0,0s));
  ASSERT_TRUE(tz::period("1y1d")      != tz::period(11,1,0s));
  ASSERT_TRUE(tz::period("0y")        != tz::period(0,1,0s));
  ASSERT_TRUE(tz::period("/01:00:00") != tz::period(0,0,1s));
}
TEST(period_from_string) {
  auto dt = tz::dtime_from_string("2015-03-09 06:38:00 America/New_York"s);
  auto& tz = tzones.find("America/New_York");
  auto dt1 = tz::plus(dt, tz::period(2, 2, 0s), tz);
  auto dt2 = tz::plus(dt, tz::period("2m2d"), tz);
  ASSERT_TRUE(dt1 == dt2);
}


int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
