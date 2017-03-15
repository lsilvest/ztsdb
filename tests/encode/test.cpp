// -*- compile-command: "make -k -j test" -*-

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
// #include "cutest/cutest.h"
#include "net_handler.hpp"
#include "encode.hpp"
#include "display.hpp"
#include "parser_ctx.hpp"
#include "timezone/ztime.hpp"
#include "timezone/zone.hpp"
#include "valuevar.hpp"
#undef INFO
#include "logging.hpp"


tz::Zones tzones("/usr/share/zoneinfo");
zlog::Logger lg;
cfg::CfgMap cfg::cfgmap;


// create encoder and use the parser to create the expression
static void parse(const char* buf, E*& ein, E*& eout) {
  ParserCtx pctx;
  net::NetHandler com("",
                 0,             // dummy port
                 0,             // dummy data out fd 
                 0);            // dummy sign out fd
  zcore::Encode enc(com, 6, 0xabba, 0xabba, Global::MsgType::REQ);              // dummy fd

  //pctx.trace_parsing  = true;
  pctx.trace_scanning  = true;
  if (pctx.parse(std::make_shared<const std::string>(buf)) != 0) {
    // fail the section or report an error LLL
    cerr << "parser error" << endl;
  }
  ein = pctx.prog->clone();
  // std::cout << "parser found: " << ::to_string(*ein) << std::endl;
  enc << (const E*)(ein);       // needs the cast to find the right overloaded func
  eout = zcore::readCode(enc.buf+48, enc.offset-48);
}


// same as above, no parse
static void encode(const E* ein, E*&eout) {
  net::NetHandler com("",
                 0,             // dummy port
                 0,             // dummy data out fd 
                 0);            // dummy sign out fd
  zcore::Encode enc(com, 6, 0xabba, 0xabba, Global::MsgType::REQ); // dummy fd
  enc << ein;                   // strangely works... ?
  eout = zcore::readCode(enc.buf+48, enc.offset-48);
}

static E* ein;
static E* eout;


// ----- test encode/decode of E expressions (from ast.hpp)
TEST(Encode_NULL) {
  parse("NULL\n", ein, eout);
  ASSERT_TRUE(isEqual(ein, eout));
}
TEST(Encode_Next) {
  parse("next\n", ein, eout);
  ASSERT_TRUE(isEqual(ein, eout));
}
TEST(Encode_Break) {
  parse("break\n", ein, eout);
  ASSERT_TRUE(isEqual(ein, eout));
}
TEST(Encode_Bool_TRUE) {
  parse("TRUE\n", ein, eout);
  ASSERT_TRUE(isEqual(ein, eout));
}
TEST(Encode_Bool_FALSE) {
  parse("FALSE\n", ein, eout);
  ASSERT_TRUE(isEqual(ein, eout));
}
TEST(Encode_Double) {
  parse("1234567890.1234567890\n", ein, eout);
  ASSERT_TRUE(isEqual(ein, eout));
}
TEST(Encode_Symbol) {
  parse("GiacomoCasanova\n", ein, eout);
  ASSERT_TRUE(isEqual(ein, eout));
}
TEST(Encode_String) {
  parse("\"JorgeLuisBorges\"\n", ein, eout);
  ASSERT_TRUE(isEqual(ein, eout));
}
// Escpares are never directly encoded, they are always transformed to
// boundvar:
TEST(Encode_Escape) {
  ASSERT_THROW(parse("++HenryMiller\n", ein, eout), 
               std::exception, 
               "codeLength: unknow type 12");
}
TEST(Encode_Boundvar) {
  auto ein = new Boundvar("a", yy::missing_loc());
  encode(ein, eout);
  ASSERT_TRUE(isEqual(ein, eout));
}
TEST(Encode_Unop) {
  parse("-2\n", ein, eout);
  ASSERT_TRUE(isEqual(ein, eout));
}
TEST(Encode_Binop) {
  parse("2+4\n", ein, eout);
  ASSERT_TRUE(isEqual(ein, eout));
}
TEST(Encode_Binop_nested) {
  parse("2 + 3.0 * 3 - (2-2)\n", ein, eout);
  ASSERT_TRUE(isEqual(ein, eout));
}
TEST(Encode_El) {
  parse("{1;2;3}\n", ein, eout);
  ASSERT_TRUE(isEqual(ein, eout));
}
TEST(Encode_If) {
  parse("if (TRUE) TRUE\n", ein, eout);
  ASSERT_TRUE(isEqual(ein, eout));
}
TEST(Encode_While) {
  parse("while (TRUE) TRUE\n", ein, eout);
  ASSERT_TRUE(isEqual(ein, eout));
}
TEST(Encode_IfElse) {
  parse("if (TRUE) TRUE else FALSE\n", ein, eout);
  ASSERT_TRUE(isEqual(ein, eout));
}
TEST(Encode_For) {
  parse("for (a in b) TRUE\n", ein, eout);
  ASSERT_TRUE(isEqual(ein, eout));
}
TEST(Encode_LeftAssign) {
  parse("a <- TRUE\n", ein, eout);
  ASSERT_TRUE(isEqual(ein, eout));
}
TEST(Encode_SpecialAssign) {
  parse("a <<- TRUE\n", ein, eout);
  ASSERT_TRUE(isEqual(ein, eout));
}
TEST(Encode_TaggedExpr) {
  auto ein = new TaggedExpr(new Symbol("a", yy::missing_loc()), 
                            new Double(1, yy::missing_loc()),
                            yy::missing_loc());
  encode(ein, eout);
  ASSERT_TRUE(isEqual(ein, eout));
}
TEST(Encode_Function) {
  parse("function(a) TRUE\n", ein, eout);
  ASSERT_TRUE(isEqual(ein, eout));
}
TEST(Encode_ExprSublist) {
  parse("f(a,b,c)\n", ein, eout);
  ASSERT_TRUE(isEqual(ein, eout));
}
TEST(Encode_SubsetList) {
  parse("f[a,b,c]\n", ein, eout);
  ASSERT_TRUE(isEqual(ein, eout));
}
TEST(Encode_colon_operator) {
  parse("1.0:10\n", ein, eout);
  ASSERT_TRUE(isEqual(ein, eout));
}


// ----- test encode/decode of Value (from valuevar.hpp)

static void encode(const val::Value& vin, val::Value*& vout) {
  net::NetHandler com("",
                      0,             // dummy port
                      0,             // dummy data out fd 
                      0);            // dummy sign out fd
  zcore::Encode enc(com, 6, 0xabba, 0xabba, Global::MsgType::REQ); // dummy fd
  enc << vin;

  size_t offset = 0;
  vector<zcore::ValState> ss;
  size_t idx=0;
  zcore::readValue(enc.buf+40, enc.offset-40, offset, ss, idx);
  vout = new val::Value(ss[0].val);
}


static val::Value* vout;


TEST(Encode_VNull) {
  val::Value vin = val::Value(val::VNull());
  encode(vin, vout);
  ASSERT_TRUE(vin == *vout);
}
TEST(Encode_std_int) {
  val::Value vin = std::make_shared<val::VI>(val::VI{4,1,2,3});  
  encode(vin, vout);
  const auto& a = get<val::SpVI>(vin);
  const auto& b = get<val::SpVI>(*vout);
  cout << val::display(a) << endl;
  cout << val::display(b) << endl;
  ASSERT_TRUE(vin == *vout);
}
TEST(Encode_std_string) {
  val::Value vin = "hello";
  encode(vin, vout);
  ASSERT_TRUE(vin == *vout);
}
TEST(Encode_Vdouble) {
  val::Value vin = val::make_array(1.0);
  encode(vin, vout);
  ASSERT_TRUE(vin == *vout);
}
TEST(Encode_Vdouble_multiple_ordering) {
  auto d = arr::Array<double>({2,2}, {1,2,4,3}, {{"un","deux"}, {"one", "two"}});
  auto vin = val::Value(make_cow<val::VArrayD>(false, d));
  encode(vin, vout);
  const auto& a = get<val::SpVAD>(vin);
  const auto& b = get<val::SpVAD>(*vout);
  cout << val::display(a) << endl;
  cout << val::display(b) << endl;
  ASSERT_TRUE(vin == *vout);
  ASSERT_TRUE(a->getcol(0).isOrdered());
  ASSERT_FALSE(a->getcol(1).isOrdered());
  ASSERT_TRUE(b->getcol(0).isOrdered());
  ASSERT_FALSE(b->getcol(1).isOrdered());
}
TEST(Encode_Vdtime) {
  val::Value vin = val::make_array(tz::dtime_from_string("2015-03-09 06:38:01 America/New_York",
                                                         tzones));
  encode(vin, vout);
  ASSERT_TRUE(vin == *vout);
}
TEST(Encode_Vbool) {
  val::Value vin = val::make_array(true);
  encode(vin, vout);
  const auto z = get<val::SpVAB>(*vout);
  // for (int i=0; i<z->size(); ++i) {
  //   cout << i << ": " << (*z)[i] << endl;
  // }
  ASSERT_TRUE(vin == *vout);
}
TEST(Encode_Vstring) {
  val::Value vin = val::make_array<arr::zstring>("PabloNeruda");
  encode(vin, vout);
  const auto& b = get<val::SpVAS>(*vout);
  cout << val::display(b) << endl;
  ASSERT_TRUE(vin == *vout);
}
TEST(Encode_Vstring_ZeroLen) {
  val::Value vin = val::make_array<arr::zstring>("");
  encode(vin, vout);
  ASSERT_TRUE(vin == *vout);
}
TEST(Encode_VList) {
  auto l = make_cow<val::VList>(false);
  l->a.concat(val::Value(val::make_array(1111111.0)), "one");
  l->a.concat(val::Value(val::make_array(2222222.0)), "two");
  l->a.concat(val::Value(val::make_array(3333333.0)), "three");
  val::Value vin = l;
  cout << "vin:" << endl;
  cout << val::display(get<val::SpVList>(vin)) << endl;
  encode(vin, vout);
  cout << "vout:" << endl;
  cout << val::display(get<val::SpVList>(*vout)) << endl;
  ASSERT_TRUE(get<val::SpVList>(vin) == get<val::SpVList>(*vout));
}
TEST(Encode_VList_empty) {
  val::Value vin = make_cow<val::VList>(false);
  encode(vin, vout);
  ASSERT_TRUE(get<val::SpVList>(vin) == get<val::SpVList>(*vout));
}
// VArrayD
TEST(Encode_VArrayD_2x2_named) {
  auto a = arr::Array<double>({2,2}, {1,2,3,4}, {{"un","deux"}, {"one", "two"}});
  encode(make_cow<val::VArrayD>(false, a), vout);
  const auto& b = get<val::SpVAD>(*vout);
  cout << val::display(b) << endl;
  ASSERT_TRUE(a == *get<val::SpVAD>(*vout));
}
TEST(Encode_VArrayD_2x2_unnamed) {
  auto a = val::VArrayD(Vector<idx_type>{2,2}, {1,2,3,4});
  encode(make_cow<val::VArrayD>(false, a), vout);
  ASSERT_TRUE(a == *get<val::SpVAD>(*vout));
}
TEST(Encode_VArrayD_3x2x4_named) {
  auto v = arr::Vector<double>(24);
  std::iota(v.begin(), v.end(), 0);
  auto a = val::VArrayD({3,2,4}, v, 
    {{"1","2","3"}, {"i","ii"}, {"I","II","III","IV"}});
  encode(make_cow<val::VArrayD>(false, a), vout);
  ASSERT_TRUE(a == *get<val::SpVAD>(*vout));
  delete vout;
}
TEST(Encode_VArrayD_3x2x4_unnamed) {
  auto v = arr::Vector<double>(24);
  std::iota(v.begin(), v.end(), 0);
  auto a = val::VArrayD({3,2,4}, v);
  encode(make_cow<val::VArrayD>(false, a), vout);
  ASSERT_TRUE(a == *get<val::SpVAD>(*vout));
  delete vout;
}
// VArrayS
TEST(Encode_VArrayS_3x2x4_named) {
  auto v = arr::Vector<arr::zstring>{"1","2","3","4","5","6","7","8","9","10","11","12",
                                "13","14","15","16","17","18","19","20","21","22","23","24"};
  auto a = val::VArrayS({3,2,4}, v, 
                         {{"1","2","3"}, {"i","ii"}, {"I","II","III","IV"}});
  encode(make_cow<val::VArrayS>(false, a), vout);
  ASSERT_TRUE(a == *get<val::SpVAS>(*vout));
  delete vout;
}
// VArrayDUR
TEST(Encode_VArrayDUR_3x2x4_named) {
  using namespace std::chrono;
  using namespace std::literals;
  auto v = arr::Vector<Global::duration>{1s,2s,3s,4s,5s,6s,7s,8s,9s,10s,11s,12s,
                                    13s,14s,15s,16s,17s,18s,19s,20s,21s,22s,23s,24s};
  auto a = val::VArrayDUR({3,2,4}, v, 
                         {{"1","2","3"}, {"i","ii"}, {"I","II","III","IV"}});
  encode(make_cow<val::VArrayDUR>(false, a), vout);
  ASSERT_TRUE(a == *get<val::SpVADUR>(*vout));
  delete vout;
}
// VArrayDT
TEST(Encode_VArrayDT_3x2x4_named) {
  using namespace std::chrono;
  using namespace std::literals;
  auto vd = arr::Vector<Global::duration>{1s,2s,3s,4s,5s,6s,7s,8s,9s,10s,11s,12s,
                                    13s,14s,15s,16s,17s,18s,19s,20s,21s,22s,23s,24s};
  auto v = arr::Vector<Global::dtime>();
  for (auto e : vd) {
    v.push_back(Global::dtime(e));
  }
  auto a = val::VArrayDT({3,2,4}, v, 
                         {{"1","2","3"}, {"i","ii"}, {"I","II","III","IV"}});
  encode(make_cow<val::VArrayDT>(false, a), vout);
  ASSERT_TRUE(a == *get<val::SpVADT>(*vout));
  delete vout;
}
// VArrayIVL
TEST(Encode_VArrayIVL_3x2x4_named) {
  using namespace std::chrono;
  using namespace std::literals;
  auto vd = arr::Vector<Global::duration>{1s,2s,3s,4s,5s,6s,7s,8s,9s,10s,11s,12s,
                                    13s,14s,15s,16s,17s,18s,19s,20s,21s,22s,23s,24s};
  auto v = arr::Vector<tz::interval>();
  for (auto e : vd) {
    v.push_back({Global::dtime(e), Global::dtime(e+3600s), 1, 1});
  }
  auto a = val::VArrayIVL({3,2,4}, v, 
                         {{"1","2","3"}, {"i","ii"}, {"I","II","III","IV"}});
  encode(make_cow<val::VArrayIVL>(false, a), vout);
  ASSERT_TRUE(a == *get<val::SpVAIVL>(*vout));
  delete vout;
}
// zts
TEST(Encode_zts) {
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones);
  auto dt2 = tz::dtime_from_string("2015-03-09 06:38:02 America/New_York", tzones);
  auto dt3 = tz::dtime_from_string("2015-03-09 06:38:03 America/New_York", tzones);
  auto a = arr::zts({3,3}, {dt1,dt2,dt3}, {1,2,3,4,5,6,7,8,9}, {{}, {"a","b","c"}});
  encode(make_cow<arr::zts>(false, a), vout);
  ASSERT_TRUE(a == *get<val::SpZts>(*vout));
  delete vout;
}
TEST(Encode_zts_no_names) {
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones);
  auto dt2 = tz::dtime_from_string("2015-03-09 06:38:02 America/New_York", tzones);
  auto dt3 = tz::dtime_from_string("2015-03-09 06:38:03 America/New_York", tzones);
  auto a = arr::zts({3,3}, {dt1,dt2,dt3}, {1,2,3,4,5,6,7,8,9});
  encode(make_cow<arr::zts>(false, a), vout);
  ASSERT_TRUE(a == *get<val::SpZts>(*vout));
  delete vout;
}
TEST(Encode_error) {
  val::Value vin = val::Value(val::VError{"VivantDenon"});
  encode(vin, vout);
  ASSERT_TRUE(get<val::VError>(vin) == get<val::VError>(*vout));  
}
// LLL
// vt_clos,
// vt_builtind,
// vt_builting,
// vt_arrays,
// vt_arrayi,
// vt_arrayb,
// vt_future,
// vt_connection,




int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
