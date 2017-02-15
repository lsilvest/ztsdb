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


// c ----- concatenation function
TEST(binds_c_noargs) {
  auto eout = parse("is.null(c()) \n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));  
}
TEST(binds_c_NULL) {
  auto eout = parse("is.null(c(NULL)) \n");
  ASSERT_TRUE(eval(eout) == val::make_array(true));  
}
TEST(binds_c_int) {
  auto eout = parse("c(1,2,3)==1:3 \n");
  auto a = arr::Array<bool>({3}, {true,true,true});
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayB>(false, a));  
}
TEST(binds_c_int_null) {
  auto eout = parse("c(1,NULL,2,3)==1:3 \n");
  auto a = arr::Array<bool>({3}, {true,true,true});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayB>(false, a));  
}
TEST(binds_c_double) {
  auto eout = parse("c(1.0,2,3)==1.0:3 \n");
  auto a = arr::Array<bool>({3}, {true,true,true});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayB>(false, a));  
}
TEST(binds_c_double_bool) {
  auto eout = parse("c(1.0,2,FALSE) \n");
  auto a = arr::Array<double>({3}, {1,2,0});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_c_string) {
  auto eout = parse("c(1,2.0,TRUE,\"3\") \n");
  auto a = arr::Array<arr::zstring>({4}, {"1","2","TRUE","3"});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayS>(false, a));  
}
TEST(binds_c_string_matrix) {
  auto eout = parse("c(1, TRUE,matrix(\"a\",2,2)) \n");
  auto a = arr::Array<arr::zstring>({6}, {"1","TRUE","a","a","a","a"});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayS>(false, a));  
}
TEST(binds_c_double_named) {
  auto eout = parse("c(a=1.0,2,3) \n");
  auto a = arr::Array<double>({3}, {1,2,3}, {{"a","",""}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_c_double_partial_named) {
  auto eout = parse("c(1,b=2,c=3) \n");
  auto a = arr::Array<double>({3}, {1,2,3}, {{"","b","c"}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_c_double_bool_named) {
  auto eout = parse("c(1.0,x=2,FALSE) \n");
  auto a = arr::Array<double>({3}, {1,2,0}, {{"","x",""}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_c_string_named) {
  auto eout = parse("c(a=1,2.0,TRUE,\"3\") \n");
  auto a = arr::Array<arr::zstring>({4}, {"1","2","TRUE","3"}, {{"a","","",""}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayS>(false, a));  
}
TEST(binds_c_string_matrix_named) {
  auto eout = parse("c(a=1, TRUE, m=matrix(\"a\",2,2)) \n");
  auto a = arr::Array<arr::zstring>({6}, {"1","TRUE","a","a","a","a"},{{"a","","m1","m2","m3","m4"}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayS>(false, a));  
}

TEST(binds_c_list) {
  auto eout = parse("c(list(1,2,3)) \n");
  auto exp = vector<pair<string,val::Value>>{
    make_pair("", val::make_array(1.0)),
    make_pair("", val::make_array(2.0)),
    make_pair("", val::make_array(3.0))};
  const auto& a = eval(eout);
  const auto& b = make_cow<val::VList>(false, exp);
  ASSERT_TRUE(a == b);  
}  

TEST(binds_c_list_global_named) {
  auto eout = parse("c(a=list(1,2,3)) \n");
  auto exp = vector<pair<string,val::Value>>{
    make_pair("a1", val::make_array(1.0)),
    make_pair("a2", val::make_array(2.0)),
    make_pair("a3", val::make_array(3.0))};
  ASSERT_TRUE(eval(eout) ==make_cow<val::VList>(false, exp));  
}  
TEST(binds_c_list_name_and_global_name) {
  auto eout = parse("c(a=list(1,b=2,3)) \n");
  auto exp = vector<pair<string,val::Value>>{
    make_pair("a1",  val::make_array(1.0)),
    make_pair("a.b", val::make_array(2.0)),
    make_pair("a3",  val::make_array(3.0))};
  ASSERT_TRUE(eval(eout) ==make_cow<val::VList>(false, exp));  
}  
TEST(binds_c_list_name_and_global_name_and_vector) {
  auto eout = parse("c(a=list(1,b=2,3,c(1,2,3))) \n");
  auto a = arr::Array<double>({3}, {1,2,3});
  auto exp = vector<pair<string,val::Value>>{
    make_pair("a1",  val::make_array(1.0)),
    make_pair("a.b", val::make_array(2.0)),
    make_pair("a3",  val::make_array(3.0)),
    make_pair("a4",  val::Value(make_cow<val::VArrayD>(false, a)))};
  ASSERT_TRUE(eval(eout) ==make_cow<val::VList>(false, exp));  
}  
// rbind, cbind, make_cow<val::VArrayD>(false, a)bind ----- bind functions
TEST(binds_rbind_0x0_0x0) {
  auto eout = parse("rbind(matrix(0,0,0), matrix(0,0,0)) \n");
  auto a = arr::Array<double>({0,0}, Vector<double>{});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_rbind_2x0_2x0) {
  auto eout = parse("rbind(matrix(0,2,0), matrix(0,2,0)) \n");
  auto a = arr::Array<double>({4,0}, Vector<double>{});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_rbind_0x3_1x3) {
  auto eout = parse("rbind(matrix(0,0,3), matrix(1:3,1,3)) \n");
  auto a = arr::Array<double>({1,3}, {1,2,3});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_rbind_3_3) {
  auto eout = parse("rbind(c(1,2,3), c(4,5,6)) \n");
  auto a = arr::Array<double>({2,3}, {1,4,2,5,3,6});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_rbind_1x3_3) {
  auto eout = parse("rbind(matrix(1:3,1,3), c(4,5,6)) \n");
  auto a = arr::Array<double>({2,3}, {1,4,2,5,3,6});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
// -- r bind incorrect dimensions
TEST(binds_rbind_0x2_1x3) {
  auto eout = parse("rbind(matrix(0,0,2), matrix(0,1,3)) \n");
  auto a = arr::Array<double>({2,3}, {1,2,3,4,5,6});
  ASSERT_THROW(eval(eout), std::range_error, "incorrect dimensions for abind");  
}
// -- cbind
TEST(binds_cbind_0x0_0x0) {
  auto eout = parse("cbind(matrix(0,0,0), matrix(0,0,0)) \n");
  auto a = arr::Array<double>({0,0}, Vector<double>{});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_cbind_2x0_2x0) {
  auto eout = parse("cbind(matrix(0,2,0), matrix(0,2,0)) \n");
  auto a = arr::Array<double>({2,0}, Vector<double>{});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_cbind_0x2_0x2) {
  auto eout = parse("cbind(matrix(0,0,2), matrix(0,0,2)) \n");
  auto a = arr::Array<double>({0,4}, Vector<double>{});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_cbind_3_3) {
  auto eout = parse("cbind(c(1,2,3), c(4,5,6)) \n");
  auto a = arr::Array<double>({3,2}, {1,2,3,4,5,6});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
// -- abind
TEST(binds_abind_0x0x0_0x0x0) {
  auto eout = parse("abind(array(0,c(0,0,0)), array(0,c(0,0,0)), along=3) \n");
  auto a = arr::Array<double>({0,0,0}, Vector<double>{});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_abind_2x0x0_2x0x0) {
  auto eout = parse("abind(array(0,c(2,0,0)), array(0,c(2,0,0)), along=3) \n");
  auto a = arr::Array<double>({2,0,0}, Vector<double>{});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_abind_0x0x2_0x0x2) {
  auto eout = parse("abind(array(0,c(0,0,2)), array(0,c(0,0,2)), along=3)\n");
  auto a = arr::Array<double>({0,0,4}, {});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_abind_3_3) {
  auto eout = parse("abind(1:3, 4:6, along=3) \n");
  auto a = arr::Array<double>({3,1,2}, {1,2,3,4,5,6});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_abind_3_3_3) {
  auto eout = parse("abind(1:3, 4:6, 7:9, along=3) \n");
  auto a = arr::Array<double>({3,1,3}, {1,2,3,4,5,6,7,8,9});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_abind_3_3_3_along4) {
  auto eout = parse("abind(1:3, 4:6, 7:9, along=4) \n");
  auto a = arr::Array<double>({3,1,1,3}, {1,2,3,4,5,6,7,8,9});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
// --- incorrect dimensions
TEST(binds_cbind_1x3_3) {
  auto eout = parse("cbind(matrix(1:3,1,3), c(4,5,6)) \n"); // debatable LLL
  auto a = arr::Array<double>({4,0}, Vector<double>{});
  ASSERT_THROW(eval(eout), std::range_error, "incorrect dimensions for abind");  
}
// -- bind type conversions handling
TEST(binds_rbind_int_string) {
  auto eout = parse("rbind(matrix(1:3,1,3), matrix(\"a\",1,3)) \n");
  auto a = arr::Array<arr::zstring>({2,3}, {"1","a","2","a","3","a"});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayS>(false, a));  
}
TEST(binds_cbind_int_string) {
  auto eout = parse("cbind(matrix(1,3,1), matrix(\"a\",3,1)) \n");
  auto a = arr::Array<arr::zstring>({3,2}, {"1","1","1","a","a","a"});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayS>(false, a));  
}
TEST(binds_cbind_bool_double) {
  auto eout = parse("cbind(matrix(TRUE,3,1), matrix(1.0,3,1)) \n");
  auto a = arr::Array<double>({3,2}, {1,1,1,1,1,1});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_abind_double_string) {
  auto eout = parse("abind(matrix(1.1,3,1), matrix(\"a\",3,1),along=3) \n");
  auto a = arr::Array<arr::zstring>({3,1,2}, {"1.1","1.1","1.1","a","a","a"});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayS>(false, a));  
}
// rbind with names ------
TEST(binds_rbind_named_0x0_0x0) {
  auto eout = parse("rbind(matrix(0,0,0, dimnames=NULL), matrix(0,0,0, dimnames=NULL)) \n");
  auto a = arr::Array<double>({0,0}, Vector<double>{});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_rbind_named_2x0_2x0) {
  auto eout = parse("rbind(matrix(0,2,0, dimnames=list(c(\"1\",\"2\"), NULL)), "
        "matrix(0,2,0, dimnames=list(c(\"3\",\"4\"), NULL))) \n");
  auto a = arr::Array<double>({4,0}, {}, {{"1","2","3","4"}, {}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_rbind_named_0x3_1x3) {
  auto eout = parse("rbind(matrix(0,0,3), matrix(1:3,1,3, "
                    "dimnames=list(c(\"1\"), c(\"a\",\"b\",\"c\")))) \n");
  auto a = arr::Array<double>({1,3}, {1,2,3}, {{"1"}, {"a","b","c"}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_rbind_named_3_3) {
  auto eout = parse("rbind(c(a=1,b=2,c=3), c(d=4,e=5,f=6)) \n");
  auto a = arr::Array<double>({2,3}, {1,4,2,5,3,6}, {{}, {"a","b","c"}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_rbind_named_1x3_3) {
  auto eout = parse("rbind(matrix(1:3,1,3, dimnames=list(c(\"1\"), c(\"a\",\"b\",\"c\"))), c(4,5,6)) \n");
  auto a = arr::Array<double>({2,3}, {1,4,2,5,3,6}, {{"1",""}, {"a","b","c"}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_rbind_named_3_3_3) {
  auto eout = parse("rbind(a=1:3,b=4:6,c=7:9) \n");
  auto a = arr::Array<double>({3,3}, {1,4,7,2,5,8,3,6,9}, {{"a","b","c"}, {}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
// cbind with names ------
TEST(binds_cbind_named_2x0_2x0) {
  auto eout = parse("cbind(matrix(0,2,0), matrix(0,2,0,dimnames=list(c(\"1\",\"2\"), NULL))) \n");
  auto a = arr::Array<double>({2,0}, arr::Vector<double>{}, {{"1","2"}, {}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_cbind_named_0x2_0x2) {
  auto eout = parse("cbind(matrix(0,0,2,dimnames=list(NULL, c(\"a\",\"b\"))), matrix(0,0,2)) \n");
  auto a = arr::Array<double>({0,4}, {}, {{}, {"a","b","",""}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_cbind_named_3_3) {
  auto eout = parse("cbind(a=c(i=1,ii=2,iii=3), b=c(k=4,l=5,m=6)) \n");
  auto a = arr::Array<double>({3,2}, {1,2,3,4,5,6}, {{"i","ii","iii"}, {"a","b"}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_cbind_named_3_3_3) {
  auto eout = parse("cbind(a=1:3,b=4:6,c=7:9) \n");
  auto a = arr::Array<double>({3,3}, {1,2,3,4,5,6,7,8,9}, {{}, {"a","b","c"}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
// -- named abind
TEST(binds_abind_named_0x0x0_0x0x0) {
  auto eout = parse("abind(array(0,c(0,0,0), dimnames=NULL), "
        "array(0,c(0,0,0), dimnames=NULL), along=3) \n");
  auto a = arr::Array<double>({0,0,0}, {});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_abind_named_2x0x0_2x0x0) {
  auto eout = parse("abind(array(0,c(2,0,0), dimnames=list(c(\"1\",\"2\"), NULL, NULL)), "
        "array(0,c(2,0,0), dimnames=list(c(\"1\",\"2\"), NULL, NULL)), along=3) \n");
  auto a = arr::Array<double>({2,0,0}, {}, {{"1","2"}, {}, {}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_abind_named_0x0x2_0x0x2) {
  auto eout = parse("abind(array(0,c(0,0,2), dimnames=list(NULL, NULL, c(\"a\",\"b\"))), "
        "array(0,c(0,0,2), dimnames=list(NULL, NULL, c(\"c\",\"d\"))), along=3)\n");
  auto a = arr::Array<double>({0,0,4}, {}, {{}, {}, {"a","b","c","d"}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_abind_named_3_3) {
  auto eout = parse("abind(a=1:3, b=4:6, along=3) \n");
  auto a = arr::Array<double>({3,1,2}, {1,2,3,4,5,6}, {{},{},{"a","b"}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_abind_named_3_3_3) {
  auto eout = parse("abind(a=c(i=1,ii=2,iii=3), b=4:6, c=7:9, along=3) \n");
  auto a = arr::Array<double>({3,1,3}, {1,2,3,4,5,6,7,8,9}, 
    {{"i","ii","iii"}, {}, {"a","b","c"}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_abind_named_3_3_3_along4) {
  auto eout = parse("abind(a=1:3, b=4:6, c=7:9, along=4) \n");
  auto a = arr::Array<double>({3,1,1,3}, {1,2,3,4,5,6,7,8,9},
    {{},{},{},{"a","b","c"}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
// -- abind types
TEST(binds_abind_named_3_3_3_bool) {
  auto eout = parse("abind(a=c(i=TRUE,ii=TRUE,iii=TRUE), b=c(FALSE,FALSE,FALSE), "
        "c=c(TRUE,FALSE,TRUE), along=3) \n");
  auto a = arr::Array<bool>({3,1,3}, {true,true,true,false,false,false,true,false,true}, 
    {{"i","ii","iii"}, {}, {"a","b","c"}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayB>(false, a));  
}
TEST(binds_abind_named_3_3_3_double) {
  auto eout = parse("abind(a=c(i=1.0,ii=2,iii=3), b=4.0:6, c=7.0:9, along=3) \n");
  auto a = arr::Array<double>({3,1,3}, {1,2,3,4,5,6,7,8,9},
    {{"i","ii","iii"}, {}, {"a","b","c"}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_abind_named_3_3_3_string) {
  auto eout = parse("abind(a=c(i=\"1\",ii=\"2\",iii=\"3\"), b=c(\"4\",\"5\",\"6\"), "
        "c=c(\"7\",\"8\",\"9\"), along=3) \n");
  auto a = arr::Array<arr::zstring>({3,1,3}, {"1","2","3","4","5","6","7","8","9"},
    {{"i","ii","iii"}, {}, {"a","b","c"}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayS>(false, a));  
}
TEST(binds_abind_named_3_3_3_dtime) {
  auto eout = parse("a1 <- |.2015-03-09 06:38:01 America/New_York.|;"
        "a2 <- |.2015-03-09 06:38:02 America/New_York.|;"
        "a3 <- |.2015-03-09 06:38:03 America/New_York.|;"
        "b1 <- |.2015-03-09 06:38:04 America/New_York.|;"
        "b2 <- |.2015-03-09 06:38:05 America/New_York.|;"
        "b3 <- |.2015-03-09 06:38:06 America/New_York.|;"
        "c1 <- |.2015-03-09 06:38:07 America/New_York.|;"
        "c2 <- |.2015-03-09 06:38:08 America/New_York.|;"
        "c3 <- |.2015-03-09 06:38:09 America/New_York.|;"
        "abind(a=c(i=a1,ii=a2,iii=a3), b=c(b1,b2,b3), "
        "c=c(c1,c2,c3), along=3) \n");
  auto a = arr::Array<Global::dtime>({3,1,3}, 
    { tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones),
      tz::dtime_from_string("2015-03-09 06:38:02 America/New_York", tzones),
      tz::dtime_from_string("2015-03-09 06:38:03 America/New_York", tzones),
      tz::dtime_from_string("2015-03-09 06:38:04 America/New_York", tzones),
      tz::dtime_from_string("2015-03-09 06:38:05 America/New_York", tzones),
      tz::dtime_from_string("2015-03-09 06:38:06 America/New_York", tzones),
      tz::dtime_from_string("2015-03-09 06:38:07 America/New_York", tzones),
      tz::dtime_from_string("2015-03-09 06:38:08 America/New_York", tzones),
      tz::dtime_from_string("2015-03-09 06:38:09 America/New_York", tzones)
    },
    {{"i","ii","iii"}, {}, {"a","b","c"}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayDT>(false, a));  
}
// -- abind scalar
TEST(binds_abind_named_double_double) {
  auto eout = parse("abind(a=1.0, b=2.0, along=3) \n");
  auto a = arr::Array<double>({1,1,2}, arr::Vector<double>{1,2},
    {{}, {}, {"a","b"}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_abind_named_bool_bool) {
  auto eout = parse("abind(a=TRUE, b=FALSE, along=3) \n");
  auto a = arr::Array<bool>({1,1,2}, arr::Vector<bool>{true,false},
    {{}, {}, {"a","b"}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayB>(false, a));  
}
TEST(binds_abind_named_string_string) {
  auto eout = parse("abind(a=\"s1\", b=\"s2\", along=3) \n");
  auto a = arr::Array<arr::zstring>({1,1,2}, {"s1","s2"},
    {{}, {}, {"a","b"}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayS>(false, a));  
}
TEST(binds_abind_named_dtime_dtime) {
  auto eout = parse("a1 <- |.2015-03-09 06:38:01 America/New_York.|;"
        "a2 <- |.2015-03-09 06:38:02 America/New_York.|;"
        "abind(a=a1, b=a2, along=3) \n");
  auto a = arr::Array<Global::dtime>({1,1,2}, 
    { tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones),
      tz::dtime_from_string("2015-03-09 06:38:02 America/New_York", tzones)
    },  
    {{}, {}, {"a","b"}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayDT>(false, a));  
}
// -- rbind scalar
TEST(binds_rbind_named_double_double) {
  auto eout = parse("rbind(a=1.0, b=2.0) \n");
  auto a = arr::Array<double>({2,1}, arr::Vector<double>{1,2}, {{"a","b"}, {}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_rbind_named_bool_bool) {
  auto eout = parse("rbind(a=TRUE, b=FALSE) \n");
  auto a = arr::Array<bool>({2,1}, arr::Vector<bool>{true,false}, {{"a","b"}, {}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayB>(false, a));  
}
TEST(binds_rbind_named_string_string) {
  auto eout = parse("rbind(a=\"s1\", b=\"s2\") \n");
  auto a = arr::Array<arr::zstring>({2,1}, arr::Vector<arr::zstring>{"s1","s2"}, {{"a","b"}, {}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayS>(false, a));  
}
TEST(binds_rbind_named_dtime_dtime) {
  auto eout = parse("a1 <- |.2015-03-09 06:38:01 America/New_York.|;"
        "a2 <- |.2015-03-09 06:38:02 America/New_York.|;"
        "rbind(a=a1, b=a2) \n");
  auto a = arr::Array<Global::dtime>({2,1}, 
    { tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones),
      tz::dtime_from_string("2015-03-09 06:38:02 America/New_York", tzones)
    },  
    {{"a","b"}, {}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayDT>(false, a));  
}
// -- cbind scalar
TEST(binds_cbind_named_double_double) {
  auto eout = parse("cbind(a=1.0, b=2.0) \n");
  auto a = arr::Array<double>({1,2}, arr::Vector<double>{1,2},
    {{}, {"a","b"}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayD>(false, a));  
}
TEST(binds_cbind_named_bool_bool) {
  auto eout = parse("cbind(a=TRUE, b=FALSE) \n");
  auto a = arr::Array<bool>({1,2}, arr::Vector<bool>{true,false}, {{}, {"a","b"}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayB>(false, a));  
}
TEST(binds_cbind_named_string_string) {
  auto eout = parse("cbind(a=\"s1\", b=\"s2\") \n");
  auto a = arr::Array<arr::zstring>({1,2}, {"s1","s2"}, {{}, {"a","b"}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayS>(false, a));  
}
TEST(binds_cbind_named_dtime_dtime) {
  auto eout = parse("a1 <- |.2015-03-09 06:38:01 America/New_York.|;"
        "a2 <- |.2015-03-09 06:38:02 America/New_York.|;"
        "cbind(a=a1, b=a2) \n");
  auto a = arr::Array<Global::dtime>({1,2}, 
    { tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones),
      tz::dtime_from_string("2015-03-09 06:38:02 America/New_York", tzones)
    },  
    {{}, {"a","b"}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayDT>(false, a));  
}
// -- abind scalar string conversion
TEST(binds_abind_named_double_string) {
  auto eout = parse("abind(a=1.0, b=\"2\", along=3) \n");
  auto a = arr::Array<arr::zstring>({1,1,2}, {"1","2"},
    {{}, {}, {"a","b"}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayS>(false, a));  
}
TEST(binds_abind_named_int_string) {
  auto eout = parse("abind(a=1, b=\"2\", along=3) \n");
  auto a = arr::Array<arr::zstring>({1,1,2}, {"1","2"},
    {{}, {}, {"a","b"}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayS>(false, a));  
}
TEST(binds_abind_named_bool_string) {
  auto eout = parse("abind(a=TRUE, b=\"FALSE\", along=3) \n");
  auto a = arr::Array<arr::zstring>({1,1,2}, {"TRUE","FALSE"},
    {{}, {}, {"a","b"}});
  ASSERT_TRUE(eval(eout) ==make_cow<val::VArrayS>(false, a));  
}
TEST(binds_abind_named_dtime_string) {
  auto eout = parse("a1 <- |.2015-03-09 06:38:01 America/New_York.|;"
        "a2 <- \"|.2015-03-09 06:38:02 America/New_York.|\";"
        "abind(a=a1, b=a2, along=3) \n");
  auto a = arr::Array<arr::zstring>({1,1,2}, 
    { "2015-03-09 06:38:01 EDT",
      "|.2015-03-09 06:38:02 America/New_York.|"
    },  
    {{}, {}, {"a","b"}});
  std::cout << val::display(make_cow<val::VArrayS>(false, a)) << std::endl;
  ASSERT_TRUE(eval(eout) == make_cow<val::VArrayS>(false, a));  
}
TEST(binds_ref) {
  // we test this here because the correct working of pass by
  // reference is also dependent on the correct implementation of the
  // bind functions:
  {
    auto eout = parse("a <- matrix(1:9, 3, 3);"
          "cbind(a, 10:12);"
          "a \n");
    auto a = arr::Array<double>({3,3}, {1,2,3,4,5,6,7,8,9});
    ASSERT_TRUE(eval(eout) == make_cow<val::VArrayD>(false, a));  
  }
  {
    // here a should  be modified
    auto eout = parse("a <- matrix(1:9, 3, 3);"
          "cbind(--a, 10:12);"
          "a \n");
    auto a = arr::Array<double>({3,4}, {1,2,3,4,5,6,7,8,9,10,11,12});
    ASSERT_TRUE(eval(eout) == make_cow<val::VArrayD>(false, a));  
  }
}
TEST(binds_mmap_array) {

}
// do time conversions LLL

int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
