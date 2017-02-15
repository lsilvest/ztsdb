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
#include <random>
#include <fstream>
#include <crpcut.hpp>
#include "csv.hpp"
#include <sys/time.h>
#include <sys/times.h>
#include "display.hpp"
#include "timezone/ztime.hpp"
#include "timezone/zone.hpp"


#include "../utils.hpp"


tz::Zones tzones("/usr/share/zoneinfo");
cfg::CfgMap cfg::cfgmap;


// double:
TEST(csv_array_double) {
  string dir = "./";
  string file = dir + "array.csv";
  {
    auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{}, {"one", "two"}});
    arr::writecsv_array(a, file, true, ',');
  }
  {
    auto a = arr::readcsv_array<double>(file, true, ',', "");
    auto b = arr::Array<double>({2,2}, {1,2,3,4}, {{}, {"one", "two"}});
    ASSERT_TRUE(*a == b);
  }
  ASSERT_TRUE(remove((dir + file).c_str())==0);
}
TEST(csv_double_quoted) {
  string dir = "./";
  string file = dir + "array.csv";
  ofstream f;
  f.open(file);
  f << "a,\"b\",c\n";
  f << "1,\"4\",7\n";
  f << "2,5,\"8\"\n";
  f << "3,6,9\n";
  f.close();
  auto a = arr::readcsv_array<double>(file, true, ',', "");
  auto b = arr::Array<double>({3,3}, {1,2,3,4,5,6,7,8,9}, {{}, {"a","b","c"}});
  std::cout << display(val::Value(a)) << std::endl;
  ASSERT_TRUE(*a == b);
  ASSERT_TRUE(remove(file.c_str())==0);
}
TEST(csv_double_no_return_at_end) {
  string dir = "./";
  string file = dir + "array.csv";
  ofstream f;
  f.open(file);
  f << "a,b,c\n";
  f << "1,4,7\n";
  f << "2,5,8\n";
  f << "3,6,9";
  f.close();
  auto a = arr::readcsv_array<double>(file, true, ',', "");
  auto b = arr::Array<double>({3,3}, {1,2,3,4,5,6,7,8,9}, {{}, {"a","b","c"}});
  std::cout << display(val::Value(a)) << std::endl;
  ASSERT_TRUE(*a == b);
  ASSERT_TRUE(remove(file.c_str())==0);
}
TEST(csv_double_no_return_at_end_last_elt_quoted) {
  string dir = "./";
  string file = dir + "array.csv";
  ofstream f;
  f.open(file);
  f << "a,b,c\n";
  f << "1,4,7\n";
  f << "2,5,8\n";
  f << "3,6,\"9\"";
  f.close();
  auto a = arr::readcsv_array<double>(file, true, ',', "");
  auto b = arr::Array<double>({3,3}, {1,2,3,4,5,6,7,8,9}, {{}, {"a","b","c"}});
  std::cout << display(val::Value(a)) << std::endl;
  ASSERT_TRUE(*a == b);
  ASSERT_TRUE(remove(file.c_str())==0);
}
TEST(csv_array_double_mmap) {
  string dir = "./";
  string file = dir + "array.csv";
  string mmapdir = dir + "array";
  {
    auto a = arr::Array<double>({2,2}, {1,2,-3,4}, {{}, {"one", "two"}});
    arr::writecsv_array(a, file, true, ',');
  }
  {
    // reading the csv file into a memory mapped array:
    auto a = arr::readcsv_array<double>(file, true, ',', mmapdir);
    auto b = arr::Array<double>({2,2}, {1,2,-3,4}, {{}, {"one", "two"}});
    ASSERT_TRUE(*a == b);
  }
  {
    // checking the memory mapped array was correctly written to file:
    auto a = arr::Array<double>(std::make_unique<MmapAllocFactory>(mmapdir, true));
    auto b = arr::Array<double>({2,2}, {1,2,-3,4}, {{}, {"one", "two"}});
    ASSERT_TRUE(a == b);    
  }
  ASSERT_TRUE(remove((dir + file).c_str())==0);
  cleandir(mmapdir.c_str());
}
TEST(csv_vector_double) {
  string dir = "./";
  string file = dir + "array.csv";
  {
    auto a = arr::Array<double>({4}, {1,2,3,4}, {{}});
    arr::writecsv_array(a, file, true, ',');
  }
  {
    auto a = arr::readcsv_array<double>(file, true, ',', "");
    auto b = arr::Array<double>({4,1}, {1,2,3,4}, {{}, {""}});
    std::cout << display(val::Value(a)) << std::endl;
    ASSERT_TRUE(*a == b);
  }
  ASSERT_TRUE(remove((dir + file).c_str())==0);
}

// bool:
TEST(csv_array_bool) {
  string dir = "./";
  string file = dir + "array.csv";
  {
    auto a = arr::Array<bool>({2,2}, {1,0,1,0}, {{}, {"one", "two"}});
    arr::writecsv_array(a, file, true, ',');
  }
  {
    auto a = arr::readcsv_array<bool>(file, true, ',', "");
    auto b = arr::Array<bool>({2,2}, {1,0,1,0}, {{}, {"one", "two"}});
    ASSERT_TRUE(*a == b);
  }
  ASSERT_TRUE(remove(file.c_str())==0);
}
TEST(csv_array_bool_mmap) {
  string dir = "./";
  string file = dir + "array.csv";
  string mmapdir = dir + "array";
  {
    auto a = arr::Array<bool>({2,2}, {1,1,0,0}, {{}, {"one", "two"}});
    arr::writecsv_array(a, file, true, ',');
  }
  {
    // reading the csv file into a memory mapped array:
    auto a = arr::readcsv_array<bool>(file, true, ',', mmapdir);
    auto b = arr::Array<bool>({2,2}, {1,1,0,0}, {{}, {"one", "two"}});
    ASSERT_TRUE(*a == b);
  }
  {
    // checking the memory mapped array was correctly written to file:
    auto a = arr::Array<bool>(std::make_unique<MmapAllocFactory>(mmapdir, true));
    auto b = arr::Array<bool>({2,2}, {1,1,0,0}, {{}, {"one", "two"}});
    ASSERT_TRUE(a == b);    
  }
  ASSERT_TRUE(remove((dir + file).c_str())==0);
  cleandir(mmapdir.c_str());
}

// dtime 
TEST(csv_array_dtime) {
  string dir = "./";
  string file = dir + "array.csv";
  {
    auto a = arr::Array<Global::dtime>({2,2}, 
      { tz::dtime_from_string("2015-03-09 06:38:00 America/New_York", tzones),
        tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones),
        tz::dtime_from_string("2015-03-09 06:38:02 America/New_York", tzones),
        tz::dtime_from_string("2015-03-09 06:38:03 America/New_York", tzones) }, 
      {{}, {"one", "two"}});
    arr::writecsv_array(a, file, true, ',');
  }
  {
    auto a = arr::readcsv_array<Global::dtime>(file, true, ',', "");
    auto b = arr::Array<Global::dtime>({2,2}, 
      { tz::dtime_from_string("2015-03-09 06:38:00 America/New_York", tzones),
        tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones),
        tz::dtime_from_string("2015-03-09 06:38:02 America/New_York", tzones),
        tz::dtime_from_string("2015-03-09 06:38:03 America/New_York", tzones) }, 
      {{}, {"one", "two"}});
    ASSERT_TRUE(*a == b);
  }
  ASSERT_TRUE(remove((dir + file).c_str())==0);
}
TEST(csv_array_dtime_mmap) {
  string dir = "./";
  string file = dir + "array.csv";
  string mmapdir = dir + "array";
  {
    auto a = arr::Array<Global::dtime>({2,2}, 
      { tz::dtime_from_string("2015-03-09 06:38:00 America/New_York", tzones),
        tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones),
        tz::dtime_from_string("2015-03-09 06:38:02 America/New_York", tzones),
        tz::dtime_from_string("2015-03-09 06:38:03 America/New_York", tzones) }, 
      {{}, {"one", "two"}});
    arr::writecsv_array(a, file, true, ',');
  }
  {
    // reading the csv file into a memory mapped array:
    auto a = arr::readcsv_array<Global::dtime>(file, true, ',', mmapdir);
    auto b = arr::Array<Global::dtime>({2,2}, 
      { tz::dtime_from_string("2015-03-09 06:38:00 America/New_York", tzones),
        tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones),
        tz::dtime_from_string("2015-03-09 06:38:02 America/New_York", tzones),
        tz::dtime_from_string("2015-03-09 06:38:03 America/New_York", tzones) }, 
      {{}, {"one", "two"}});
    ASSERT_TRUE(*a == b);
  }
  {
    // checking the memory mapped array was correctly written to file:
    auto a = arr::Array<Global::dtime>(std::make_unique<MmapAllocFactory>(mmapdir, true));
    auto b = arr::Array<Global::dtime>({2,2}, 
      { tz::dtime_from_string("2015-03-09 06:38:00 America/New_York", tzones),
        tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones),
        tz::dtime_from_string("2015-03-09 06:38:02 America/New_York", tzones),
        tz::dtime_from_string("2015-03-09 06:38:03 America/New_York", tzones) }, 
      {{}, {"one", "two"}});
    ASSERT_TRUE(a == b);    
  }
  ASSERT_TRUE(remove(file.c_str())==0);
  cleandir(mmapdir.c_str());
}


// zts:
TEST(csv_array_zts) {
  const string dir = "./";
  const string file = dir + "array.csv";
  auto idx = arr::Array<Global::dtime>({2}, 
      { tz::dtime_from_string("2015-03-09 06:38:00 America/New_York", tzones),
        tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones) });
  auto data = arr::Array<double>({2,2}, {1,2,3,4}, {{}, {"one", "two"}});
  auto z1 = arr::zts(idx, data);
  arr::writecsv_zts(z1, file, true, ',');
  // system(("cat " + file).c_str());
  auto z2 = arr::readcsv_zts(file, true, ',', "");
  std::cout << display(val::Value(z2)) << std::endl;
  ASSERT_TRUE(z1 == *z2);
  ASSERT_TRUE(remove((dir + file).c_str())==0);
}
TEST(csv_array_zts_mmap) {
  const string dir = "./";
  const string file = dir + "array.csv";
  string mmapdir = dir + "zts";
  {
    auto idx = arr::Array<Global::dtime>({2}, 
      { tz::dtime_from_string("2015-03-09 06:38:00 America/New_York", tzones),
        tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones) });
    auto data = arr::Array<double>({2,2}, {1,2,3,4}, {{}, {"one", "two"}});
    auto z1 = arr::zts(idx, data);
    arr::writecsv_zts(z1, file, true, ',');
    // system(("cat " + file).c_str());
  }
  {
    auto idx = arr::Array<Global::dtime>({2}, 
      { tz::dtime_from_string("2015-03-09 06:38:00 America/New_York", tzones),
        tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones) });
    auto data = arr::Array<double>({2,2}, {1,2,3,4}, {{}, {"one", "two"}});
    auto z1 = arr::zts(idx, data);
    auto z2 = arr::readcsv_zts(file, true, ',', mmapdir);
    std::cout << display(val::Value(z2)) << std::endl;
    ASSERT_TRUE(z1 == *z2);
  }
  ASSERT_TRUE(remove((dir + file).c_str())==0);
  cleandir(mmapdir.c_str());
}

// errors:
TEST(csv_array_3dim) {
  string dir = "./";
  string file = dir + "array_3dim.csv";
  auto a = arr::Array<double>({2,2,2}, {1,2,3,4,5,6,7,8}, {{}, {"one", "two"}, {}});
  arr::writecsv_array(a, file, true, ',');
  // system(("cat " + file).c_str());
  auto b = arr::readcsv_array<double>(file, true, ',', "");
  auto exp = arr::Array<double>({2,4}, {1,2,3,4,5,6,7,8}, {{}, {"0","1","2","3"}});
  ASSERT_TRUE(exp == *b);
  ASSERT_TRUE(remove((dir + file).c_str())==0);
}
TEST(csv_read_too_few_cols) {
  string dir = "./";
  string file = dir + "array.csv";
  ofstream f;
  f.open(file);
  f << "a,b,c\n";
  f << "1,2,3\n";
  f << "4,5,6\n";
  f << "7,8\n";
  f << "1,1,1\n";
  f.close();
  ASSERT_THROW(arr::readcsv_array<bool>(file, true, ',', ""),
               std::out_of_range, 
               "incorrect number of elements on row 4");
  ASSERT_TRUE(remove(file.c_str())==0);
}
TEST(csv_read_too_many_cols) {
  string dir = "./";
  string file = dir + "array.csv";
  ofstream f;
  f.open(file);
  f << "a,b,c\n";
  f << "1,2,3\n";
  f << "4,5,6\n";
  f << "7,8,9,10\n";
  f << "1,1,1\n";
  f.close();
  ASSERT_THROW(arr::readcsv_array<bool>(file, true, ',', ""),
               std::out_of_range, 
               "incorrect number of elements on row 4");
  ASSERT_TRUE(remove(file.c_str())==0);
}
TEST(csv_read_parse_error) {
  string dir = "./";
  string file = dir + "array.csv";
  ofstream f;
  f.open(file);
  f << "a,b,c\n";
  f << "1,2,3\n";
  f << "4,5,6\n";
  f << "7,8,9.asd\n";
  f << "1,1,1\n";
  f.close();
  ASSERT_THROW(arr::readcsv_array<bool>(file, true, ',', ""),
               std::out_of_range, 
               "can't parse '9.asd', col 3 on row 4");
  ASSERT_TRUE(remove(file.c_str())==0);
}
TEST(csv_double_unclosed_quote) {
  string dir = "./";
  string file = dir + "array.csv";
  ofstream f;
  f.open(file);
  f << "a,\"b\",c\n";
  f << "1,\"4,7\n";
  f << "2,5,\"8\"\n";
  f << "3,6,9\n";
  f.close();
  ASSERT_THROW(arr::readcsv_array<double>(file, true, ',', ""),
               std::out_of_range,
               "quote does not terminate token on row 2");
  ASSERT_TRUE(remove(file.c_str())==0);
}

DISABLED_TEST(csv_perf_write) {
  const arr::idx_type ROWS = 100000000L;
  const arr::idx_type COLS = 3;  
  auto a = arr::Array<double>({ROWS,COLS}, {}, {{}, {"one", "two", "three"}});

  std::uniform_real_distribution<double> unif(-1e300,1e300);
  std::default_random_engine re;
  for (arr::idx_type i = 0; i<ROWS*COLS; ++i) {
    setv(a, i, unif(re));
  }

  double elapsed;
  struct itimerval e1, e2;
  itimerval init{{1000,0},{1000,0}};
  setitimer(ITIMER_REAL, &init, NULL);
  getitimer(ITIMER_REAL, &e1);

  arr::writecsv_array(a, "/tmp/a4.csv", true, ',');

  getitimer(ITIMER_REAL, &e2);
  elapsed = (e1.it_value.tv_sec - e2.it_value.tv_sec)
    + ((e1.it_value.tv_usec - e2.it_value.tv_usec) / 1000000.0);
  cout << "elapsed: " << elapsed << endl;
  ASSERT_TRUE(false);    
}
DISABLED_TEST(perf) {
  auto a = arr::readcsv_array<double>("/tmp/a2.csv", true, ',', "");
}



int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
