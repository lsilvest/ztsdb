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


#include <iostream>
#include <fstream>
#include <crpcut.hpp>
#undef INFO
#include "config.hpp"
#include "config_ctx.hpp"
#include "logging.hpp"
#include "timezone/zone.hpp"


tz::Zones tzones("/usr/share/zoneinfo");
zlog::Logger lg("hello.log", "GMT");
cfg::CfgMap cfg::cfgmap;


using namespace Juice;


TEST(config_simple) {
  std::ofstream ofs;
  ofs.open("file.cfg", std::ofstream::out | std::ofstream::app);
  ofs << "a = 1.234" << std::endl
      << "b = \"America/New_York\"" << std::endl
      << "c = 21321331" << std::endl
      << "d = abcd" << std::endl;
  ofs.close();

  cfg::CfgCtx cfgc(cfg::cfgmap);
  cfgc.parsefile("file.cfg");
  std::remove("file.cfg");

  ASSERT_TRUE(get<double>(cfg::cfgmap["a"]) == 1.234);
  ASSERT_TRUE(get<std::string>(cfg::cfgmap["b"]) == "America/New_York");
  ASSERT_TRUE(get<int64_t>(cfg::cfgmap["c"]) == 21321331);
  ASSERT_TRUE(get<std::string>(cfg::cfgmap["d"]) == "abcd");
}
TEST(config_missing_end_nl) {
  std::ofstream ofs;
  ofs.open("file.cfg", std::ofstream::out | std::ofstream::app);
  ofs << "a = 1.234";
  ofs.close();

  cfg::CfgCtx cfgc(cfg::cfgmap);
  cfgc.parsefile("file.cfg");
  std::remove("file.cfg");

  ASSERT_TRUE(get<double>(cfg::cfgmap["a"]) == 1.234);
}
TEST(config_double) {
  std::ofstream ofs;
  ofs.open("file.cfg", std::ofstream::out | std::ofstream::app);
  ofs << "a_1 = 1." << std::endl
      << "b_2 = .2" << std::endl
      << "c_3 = 1.023234234e5" << std::endl
      << "d_4 = 1.023234234E-05" << std::endl
      << "e_5 = -0.0000001" << std::endl;
  ofs.close();

  cfg::CfgCtx cfgc(cfg::cfgmap);
  cfgc.parsefile("file.cfg");
  std::remove("file.cfg");

  ASSERT_TRUE(get<double>(cfg::cfgmap["a_1"]) == 1.0);
  ASSERT_TRUE(get<double>(cfg::cfgmap["b_2"]) == 0.2);
  ASSERT_TRUE(get<double>(cfg::cfgmap["c_3"]) == 1.023234234e5);
  ASSERT_TRUE(get<double>(cfg::cfgmap["d_4"]) == 1.023234234E-5);
  ASSERT_TRUE(get<double>(cfg::cfgmap["e_5"]) == -0.0000001);
}
TEST(config_integer) {
  std::ofstream ofs;
  ofs.open("file.cfg", std::ofstream::out | std::ofstream::app);
  ofs << "a_1 = 0" << std::endl
      << "b_2 = 123123123123123" << std::endl
      << "c_3 = -12321" << std::endl
      << "d_4 = 0xabbaabba" << std::endl
      << "e_5 = 000002" << std::endl;
  ofs.close();

  cfg::CfgCtx cfgc(cfg::cfgmap);
  cfgc.parsefile("file.cfg");
  std::remove("file.cfg");

  ASSERT_TRUE(get<int64_t>(cfg::cfgmap["a_1"]) == 0);
  ASSERT_TRUE(get<int64_t>(cfg::cfgmap["b_2"]) == 123123123123123);
  ASSERT_TRUE(get<int64_t>(cfg::cfgmap["c_3"]) == -12321);
  ASSERT_TRUE(get<int64_t>(cfg::cfgmap["d_4"]) == 0xabbaabba);
  ASSERT_TRUE(get<int64_t>(cfg::cfgmap["e_5"]) == 000002);
}
TEST(config_unquoted_symbol) {
  std::ofstream ofs;
  ofs.open("file.cfg", std::ofstream::out | std::ofstream::app);
  ofs << "a_1 = a" << std::endl
      << "b_2 = _123123123123123" << std::endl
      << "c_3 = _12asdd" << std::endl
      << "d_4 = 1_2" << std::endl;
  ofs.close();

  cfg::CfgCtx cfgc(cfg::cfgmap);
  cfgc.parsefile("file.cfg");
  std::remove("file.cfg");

  ASSERT_TRUE(get<std::string>(cfg::cfgmap["a_1"]) == "a");
  ASSERT_TRUE(get<std::string>(cfg::cfgmap["b_2"]) == "_123123123123123");
  ASSERT_TRUE(get<std::string>(cfg::cfgmap["c_3"]) == "_12asdd");
  ASSERT_TRUE(get<std::string>(cfg::cfgmap["d_4"]) == "1_2");
}
TEST(config_string_literal) {
  std::ofstream ofs;
  ofs.open("file.cfg", std::ofstream::out | std::ofstream::app);
  ofs << "a_1 = \"123abcd\"" << std::endl
      << "b_2 = \"\\a\\b\\f\\r\\t\\v\"" << std::endl
      << "c_3 = \"\\7\\60\\161\"" << std::endl
      << "d_4 = \"\\xa\"" << std::endl;
  ofs.close();

  cfg::CfgCtx cfgc(cfg::cfgmap);
  cfgc.parsefile("file.cfg");
  std::remove("file.cfg");

  std::cout << get<std::string>(cfg::cfgmap["a_1"])<< std::endl;
  std::cout << get<std::string>(cfg::cfgmap["b_2"])<< std::endl;
  std::cout << get<std::string>(cfg::cfgmap["c_3"])<< std::endl;
  std::cout << get<std::string>(cfg::cfgmap["d_4"])<< std::endl;
  ASSERT_TRUE(get<std::string>(cfg::cfgmap["a_1"]) == "123abcd");
  ASSERT_TRUE(get<std::string>(cfg::cfgmap["b_2"]) == "\a\b\f\r\t\v");
  ASSERT_TRUE(get<std::string>(cfg::cfgmap["c_3"]) == "\a0q");
  ASSERT_TRUE(get<std::string>(cfg::cfgmap["d_4"]) == "\xa");
}
TEST(config_multiple_eq) {
  const char* logfile = "newlog.log";
  lg.changeFile(logfile);

  std::ofstream ofs;
  ofs.open("file.cfg", std::ofstream::out | std::ofstream::app);
  ofs << "a_1 ==" << std::endl;
  ofs.close();

  cfg::CfgCtx cfgc(cfg::cfgmap);
  cfgc.parsefile("file.cfg");

  std::ifstream ifs(logfile);
  std::string line;
  ASSERT_TRUE(getline(ifs, line));
  ASSERT_PRED(crpcut::match<crpcut::regex>(".*unexpected EQ.*"), line);

  std::remove("file.cfg");
  std::remove(logfile);
}
TEST(config_multiple_symbol) {
  const char* logfile = "newlog.log";
  lg.changeFile(logfile);

  std::ofstream ofs;
  ofs.open("file.cfg", std::ofstream::out | std::ofstream::app);
  ofs << "a b = 2" << std::endl;
  ofs.close();

  cfg::CfgCtx cfgc(cfg::cfgmap);
  cfgc.parsefile("file.cfg");

  std::ifstream ifs(logfile);
  std::string line;
  ASSERT_TRUE(getline(ifs, line));
  ASSERT_PRED(crpcut::match<crpcut::regex>(".*unexpected SYMBOL.*"), line);

  std::remove("file.cfg");
  std::remove(logfile);
}
TEST(config_multiple_values) {
  const char* logfile = "newlog.log";
  lg.changeFile(logfile);

  std::ofstream ofs;
  ofs.open("file.cfg", std::ofstream::out | std::ofstream::app);
  ofs << "a = 2 3" << std::endl;
  ofs.close();

  cfg::CfgCtx cfgc(cfg::cfgmap);
  cfgc.parsefile("file.cfg");

  std::ifstream ifs(logfile);
  std::string line;
  ASSERT_TRUE(getline(ifs, line));
  ASSERT_PRED(crpcut::match<crpcut::regex>(".*unexpected INTEGER.*"), line);

  std::remove("file.cfg");
  std::remove(logfile);
}
TEST(config_empty_file) {
  const char* logfile = "newlog.log";
  lg.changeFile(logfile);

  std::ofstream ofs;
  ofs.open("file.cfg", std::ofstream::out | std::ofstream::app);
  ofs.close();

  cfg::CfgCtx cfgc(cfg::cfgmap);
  cfgc.parsefile("file.cfg");

  std::ifstream ifs(logfile);
  std::string line;
  ASSERT_FALSE(getline(ifs, line)); // check it's not an error
  std::remove("file.cfg");
  std::remove(logfile);
}
TEST(config_only_comments) {
  const char* logfile = "newlog.log";
  lg.changeFile(logfile);

  std::ofstream ofs;
  ofs.open("file.cfg", std::ofstream::out | std::ofstream::app);
  ofs << "         \t\t\t# sdfdsf sdfsdf a_1 ==" << std::endl
      << std::endl
      << "#### a_1 ==" << std::endl
      << "# everything beyond the first # is a comment" << std::endl;
  ofs.close();

  cfg::CfgCtx cfgc(cfg::cfgmap);
  cfgc.parsefile("file.cfg");

  std::ifstream ifs(logfile);
  std::string line;
  ASSERT_FALSE(getline(ifs, line)); // check it's not an error
  std::remove("file.cfg");
  std::remove(logfile);
}


int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}

