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


#include <fcntl.h>
#include <unistd.h>
#include <crpcut.hpp>
#include <thread>
#include <pthread.h>
#include <sys/eventfd.h>
#undef INFO
#include "../../src/net_handler.hpp"
#include "encode.hpp"
#include "parser_ctx.hpp"
#include "../../src/msg_handler.hpp"
#include "anf.hpp"
#include "load_builtin.hpp"
#include "timezone/zone.hpp"
#include "timezone/ztime.hpp"
#include "logging.hpp"


tz::Zones tzones("/usr/share/zoneinfo");
zlog::Logger lg;
cfg::CfgMap cfg::cfgmap;

using tpl_t = std::pair<std::reference_wrapper<net::NetHandler>, 
                        std::reference_wrapper<volatile bool>>;

interp::shpfrm base    = make_shared<interp::Frame>("base"s);
interp::shpfrm global  = make_shared<interp::Frame>("global"s,  global, base);

static void* executeNet(void* args_p) {
  auto args = static_cast<tpl_t*>(args_p);
  auto& c = static_cast<net::NetHandler&>(args->first);
  c.run(args->second);
  return nullptr;
}

static void* executeMsgHandler(void* ctx) {
  auto ir = static_cast<zcore::MsgHandler*>(ctx);
  ir->run();
  return nullptr;
}


/// create two instances of the database, send a query from one to the
/// other and return the evaluated result.
static val::Value getValue(const string& query) {
  // setup peer1 ----------------------------------------
  int port1 = 21331;
  int data_com1_ir1 = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
  if (data_com1_ir1 == -1) {
    throw std::system_error(std::error_code(errno, std::system_category()), 
                            "eventfd failed for data_com1_ir1");
  }
  int sig_com1_ir1 = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
  if (sig_com1_ir1 == -1) {
    throw std::system_error(std::error_code(errno, std::system_category()), 
                            "eventfd failed for sig_com1_ir1");
  }

  net::NetHandler com1("", port1, data_com1_ir1, sig_com1_ir1);
  int pipe_here_ir1[2];
  int res = pipe2(pipe_here_ir1, O_CLOEXEC);
  if (res == -1) {
    throw std::system_error(std::error_code(errno, std::system_category()), 
			    "pipe failed for here_ir1");
  }
  zcore::MsgHandler ir1(com1, global, data_com1_ir1, sig_com1_ir1, pipe_here_ir1[0], true);

  // setup peer2 ----------------------------------------
  int port2 = 21332;
  int data_com2_ir2 = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
  if (data_com2_ir2 == -1) {
    throw std::system_error(std::error_code(errno, std::system_category()), 
                            "eventfd failed for data_com2_ir2");
  }
  int sig_com2_ir2 = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
  if (sig_com2_ir2 == -1) {
    throw std::system_error(std::error_code(errno, std::system_category()), 
                            "eventfd failed for sig_com2_ir2");
  }

  net::NetHandler com2("", port2, data_com2_ir2, sig_com2_ir2);
  zcore::MsgHandler ir2(com2, global, data_com2_ir2, sig_com2_ir2);


  // run the tcp comm threads:
  volatile bool stop = 0;
  auto args1 = tpl_t{com1, stop};
  pthread_t t1;
  pthread_create(&t1, NULL, executeNet, &args1); 
  auto args2 = tpl_t{com2, stop};
  pthread_t t2;
  pthread_create(&t2, NULL, executeNet, &args2); 


  // run the interp cores:
  pthread_t i2;
  pthread_create(&i2, NULL, executeMsgHandler, &ir2); 

  // load the builtin functions (needed for "connection"):
  core::loadBuiltinFunctions(global.get());

  // create and invoke the parser and anf to get a syntax tree:
  ParserCtx pctx;
  string remoteQuery = 
    "options(max.print=1);"
    "con <- connection(\"127.0.0.1\", " +  std::to_string(port2) + "); con ? (" + query + ")\n";
  std::cout << remoteQuery << std::endl;

  if (write(pipe_here_ir1[1], remoteQuery.c_str(), remoteQuery.size()) == -1) {
    throw std::system_error(std::error_code(errno, std::system_category()), 
                            "write pipe_here_ir1");    
  }
  ir1.run();

  // grab local context in order to get to the result:
  const auto& localCtx = ir1.getLocalCtx();
  const auto& lv = localCtx.r->find(".Last.value");

  pthread_cancel(t1);
  pthread_cancel(t2);
  pthread_cancel(i2);
  
  return lv;       // copy
}



TEST(comm_double) {
  ASSERT_TRUE(getValue("1.0") == val::make_array(1.0));
}
TEST(comm_bool_true) {
  ASSERT_TRUE(getValue("TRUE") == val::make_array(true));
}
TEST(comm_bool_false) {
  ASSERT_TRUE(getValue("FALSE") == val::make_array(false));
}
TEST(comm_string) {
  ASSERT_TRUE(getValue("\"hello\"") == val::make_array(zstring("hello")));
}
TEST(comm_long_string) {
  // LLL
}
TEST(comm_dtime) {
  ASSERT_TRUE(getValue("|.2015-03-09 06:38:01 America/New_York.|") ==
              val::make_array(tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones)));
}
TEST(comm_NULL) {
  ASSERT_TRUE(getValue("NULL") == val::VNull());
}
TEST(comm_long_expression) {
  // R can do 4993 by default
  // at 4994 it prints: 
  // Error: evaluation nested too deeply: infinite recursion / options(expressions=)?
  // so, it can be controlled by options(expressions=)
  //
  // but the idea here is to test the breaking up of an expression in multiple packets:
  // const unsigned len = 5000; need a more recent version of make for this to run
  const double len = 1000;
  string s;
  for (unsigned i=0; i<len-1; ++i) {
    s += "1+";
  }
  s += "1";
  ASSERT_TRUE(getValue(s) == val::make_array(len));
}
TEST(comm_vector_double) {
  const unsigned len = 10;
  auto data = arr::Vector<double>(len);
  std::iota(data.begin(), data.end(), 1);
  auto v = make_cow<val::VArrayD>(false, Vector<arr::idx_type>{len}, data);
  ASSERT_TRUE(getValue("1.0:" + std::to_string(len)) == v);
}
TEST(comm_long_vector_double) {
  // test the breaking up of a value (vector) in multiple packets:
  const unsigned len = 100*1000;
  auto data = arr::Vector<double>(len);
  std::iota(data.begin(), data.end(), 1);
  auto v = make_cow<val::VArrayD>(false, Vector<arr::idx_type>{len}, data);
  ASSERT_TRUE(getValue("1:" + std::to_string(len)) == v);
}


int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
