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
//#include "cutest/cutest.h"
#include <system_error>
#include <stdio.h>
#include "zts.hpp"
#include "display.hpp"
#include "misc.hpp"
#include "timezone/ztime.hpp"
#include "timezone/zone.hpp"
#include "../utils.hpp"


using namespace std;
using namespace Global;


tz::Zones tzones("/usr/share/zoneinfo");
cfg::CfgMap cfg::cfgmap;


TEST(zts_constructor) {
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones);
  auto dt2 = tz::dtime_from_string("2015-03-10 06:38:01 America/New_York", tzones);
  auto dt3 = tz::dtime_from_string("2015-03-11 06:38:01 America/New_York", tzones);
  const arr::zts a({3,3}, {dt1, dt2, dt3}, {1,2,3,4,5,6,7,8,9}, {{}, {"one", "two", "three"}});
  // no ASSERT, just checking the constructor runs correctly
}
TEST(zts_constructor_from_array) {
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones);
  auto dt2 = tz::dtime_from_string("2015-03-10 06:38:01 America/New_York", tzones);
  auto dt3 = tz::dtime_from_string("2015-03-11 06:38:01 America/New_York", tzones);
  arr::Array<double> a({3,3}, {1,2,3,4,5,6,7,8,9}, {{}, {"one", "two", "three"}});
  const arr::zts z(Vector<Global::dtime>({dt1, dt2, dt3}), std::move(a));
  // no ASSERT, just checking the constructor runs correctly
}
TEST(zts_to_string) {
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones);
  auto dt2 = tz::dtime_from_string("2015-03-10 06:38:01 America/New_York", tzones);
  auto dt3 = tz::dtime_from_string("2015-03-11 06:38:01 America/New_York", tzones);
  auto a = arr::Array<double>({3,3}, {1,2,3,4,5,6,7,8,9}, {{}, {"one", "two", "three"}});
  const auto z = arr::zts(Vector<Global::dtime>({dt1, dt2, dt3}), std::move(a));
  // cout << val::to_string(make_shared<arr::zts>(z)) << endl;
  // no ASSERT, just checking the function runs correctly
}
TEST(zts_constructor_from_file) {
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones);
  auto dt2 = tz::dtime_from_string("2015-03-10 06:38:01 America/New_York", tzones);
  auto dt3 = tz::dtime_from_string("2015-03-11 06:38:01 America/New_York", tzones);
  {
    std::unique_ptr<arr::AllocFactory> mmap_core_dir =
                    std::make_unique<arr::MmapAllocFactory>("./zts_mmap_constructor/"s, false);
    std::unique_ptr<arr::AllocFactory> mmap_idx_dir  =
                    std::make_unique<arr::MmapAllocFactory>("./zts_mmap_constructor/idx"s, false);
    const arr::zts a({3,3}, 
                     {dt1, dt2, dt3}, 
                     {1,2,3,4,5,6,7,8,9}, 
                     {{}, {"one", "two", "three"}},
                     std::move(mmap_core_dir),
                     std::move(mmap_idx_dir));
    ASSERT_TRUE((a.getArray()[{0,0}] == 1));
    ASSERT_TRUE((a.getArray()[{1,0}] == 2));
    ASSERT_TRUE((a.getArray()[{2,0}] == 3));
    ASSERT_TRUE((a.getArray()[{0,1}] == 4));
    ASSERT_TRUE((a.getArray()[{1,1}] == 5));
    ASSERT_TRUE((a.getArray()[{2,1}] == 6));
    ASSERT_TRUE((a.getArray()[{0,2}] == 7));
    ASSERT_TRUE((a.getArray()[{1,2}] == 8));
    ASSERT_TRUE((a.getArray()[{2,2}] == 9));
    ASSERT_TRUE((a.getIndex()[{0}] == dt1));
    ASSERT_TRUE((a.getIndex()[{1}] == dt2));
    ASSERT_TRUE((a.getIndex()[{2}] == dt3));
  } // a is being deleted here
  // read the mmapped zts:
  {
    const arr::zts a(std::make_unique<arr::MmapAllocFactory>("./zts_mmap_constructor/"s, true), 
                     std::make_unique<arr::MmapAllocFactory>("./zts_mmap_constructor/idx"s, true));
    // construct the same array as above, but without mmap:
    const arr::zts b({3,3}, 
                     {dt1, dt2, dt3}, 
                     {1,2,3,4,5,6,7,8,9}, 
                     {{}, {"one", "two", "three"}});
    ASSERT_TRUE(a == b);    
  }
  ASSERT_TRUE(remove("./zts_mmap_constructor/0")==0);      
  ASSERT_TRUE(remove("./zts_mmap_constructor/1")==0);      
  ASSERT_TRUE(remove("./zts_mmap_constructor/2")==0);      
  ASSERT_TRUE(remove("./zts_mmap_constructor/dim")==0);    
  ASSERT_TRUE(remove("./zts_mmap_constructor/names0")==0); 
  ASSERT_TRUE(remove("./zts_mmap_constructor/names1")==0); 
  ASSERT_TRUE(remove("./zts_mmap_constructor/idx/0")==0); 
  ASSERT_TRUE(remove("./zts_mmap_constructor/idx/dim")==0); 
  ASSERT_TRUE(remove("./zts_mmap_constructor/idx/names0")==0); 
  ASSERT_TRUE(rmdir("./zts_mmap_constructor/idx") == 0);        
  ASSERT_TRUE(rmdir("./zts_mmap_constructor/") == 0);        
}

// slicing LLL
// equality, etc.


int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
