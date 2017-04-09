// -*- compile-command: "make -j -k -O test" -*-

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


#include <ftw.h>
#include "../interp_setup.hpp"


// helpers to wipe out a directory:
int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
  return remove(fpath);
}

int cleandir(const char* d) {
  return nftw(d, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}


TEST(mmap_array_double) {
  cleandir("./array_double");
  {
    auto eout = parse("a <- matrix(1:9, 3, 3, file=\"./array_double1\");"
                      "b <- matrix(1:9, 3, 3, file=\"./array_double2\");"
                      "cbind(--a, b);"
                      "a <- 2;"
                      "load(file=\"./array_double1\") \n");
    auto a = arr::Array<double>({3,6}, {1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9});
    const auto& res = eval(eout);
    ASSERT_TRUE(res == val::Value(make_cow<val::VArrayD>(false, a)));
  }
  cleandir("./array_double1");
  cleandir("./array_double2");
}
TEST(mmap_array_bool) {
  cleandir("./array_bool");
  {
    auto eout = parse("a <- matrix(TRUE, 3, 3, file=\"./array_bool1\");"
                      "b <- matrix(TRUE, 3, 3, file=\"./array_bool2\");"
                      "cbind(--a, b);"
                      "a <- 2;"
                      "load(file=\"./array_bool1\") \n");
    auto a = arr::Array<bool>({3,6}, {true,true,true,true,true,true,true,true,true,
          true,true,true,true,true,true,true,true,true});
    ASSERT_TRUE(eval(eout) == val::Value(make_cow<val::VArrayB>(false, a)));
  }
  cleandir("./array_bool1");
  cleandir("./array_bool2");
}
TEST(mmap_array_string) {
  cleandir("./array_string");
  {
    auto eout = parse("a <- matrix(\"a\", 3, 3, file=\"./array_string1\");"
                      "b <- matrix(\"a\", 3, 3, file=\"./array_string2\");"
                      "cbind(--a, b);"
                      "a <- 2;"
                      "load(file=\"./array_string1\") \n");
    auto a = arr::Array<arr::zstring>({3,6}, {"a","a","a","a","a","a","a","a","a",
          "a","a","a","a","a","a","a","a","a"});
    ASSERT_TRUE(eval(eout) == val::Value(make_cow<val::VArrayS>(false, a)));
  }
  cleandir("./array_string1");
  cleandir("./array_string2");
}
TEST(mmap_array_dtime) {
  cleandir("./array_dtime");
  {
    auto eout = parse("a <- matrix(|.2015-03-09 06:38:01 America/New_York.|, 3, 3, "
                      "file=\"./array_dtime1\");"
                      "b <- matrix(|.2015-03-09 06:38:01 America/New_York.|, 3, 3, "
                      "file=\"./array_dtime2\");"
                      "cbind(--a, b);"
                      "a <- 2;"
                      "load(file=\"./array_dtime1\") \n");
    using namespace std;
    using namespace std::chrono;
    auto v = arr::Vector<Global::dtime>(18, Global::dtime(1425897481L * 1s));
    auto a = arr::Array<Global::dtime>({3,6}, v);
    ASSERT_TRUE(eval(eout) == val::Value(make_cow<val::VArrayDT>(false, a)));
  }
  cleandir("./array_dtime1");
  cleandir("./array_dtime2");
}

int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
