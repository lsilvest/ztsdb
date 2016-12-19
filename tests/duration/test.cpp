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
#include "timezone/ztime.hpp"


using namespace std::chrono_literals;

tz::Zones tzones("/usr/share/zoneinfo");

TEST(to_string_no_frac) {
  Global::duration d = 1h + 1min + 1s;
  ASSERT_TRUE(tz::to_string(d) == "01:01:01");
}
TEST(to_string_ms) {
  Global::duration d = 1h + 1min + 1s + 1ms;
  ASSERT_TRUE(tz::to_string(d) == "01:01:01.001");
}
TEST(to_string_us) {
  Global::duration d = 1h + 1min + 1s + 1ms + 1us;
  ASSERT_TRUE(tz::to_string(d) == "01:01:01.001_001");
}
TEST(to_string_ns) {
  Global::duration d = 1h + 1min + 1s + 1ms + 1us + 1ns;
  ASSERT_TRUE(tz::to_string(d) == "01:01:01.001_001_001");
}
TEST(to_string_many_hours) {
  Global::duration d = 1000h + 1000s + 1000ms + 1000us + 1001ns;
  ASSERT_TRUE(tz::to_string(d) == "1000:16:41.001_001_001");
}
TEST(to_string_many_hours_neg) {
  Global::duration d = -1000h - 1000s - 1000ms - 1000us - 1001ns;
  ASSERT_TRUE(tz::to_string(d) == "-1000:16:41.001_001_001");
}
TEST(from_string_seconds) {
  auto d = tz::duration_from_string2("1001");
  Global::duration exp = 1001s;
  ASSERT_TRUE(d == exp);
}
TEST(from_string_seconds_fraction) {
  auto d = tz::duration_from_string2("1001.001_001_001");
  Global::duration exp = 1000s + 1000ms + 1000us + 1001ns;
  ASSERT_TRUE(d == exp);
}
TEST(from_string_all) {
  auto d = tz::duration_from_string2("1000:00:01.001_001_001");
  Global::duration exp = 1000h + 1000ms + 1000us + 1001ns;
  ASSERT_TRUE(d == exp);
}
TEST(from_string_all_neg) {
  auto d = tz::duration_from_string2("-1000:00:01.001_001_001");
  Global::duration exp = -(1000h + 1000ms + 1000us + 1001ns);
  ASSERT_TRUE(d == exp);
}

int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
