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


/// Tests the fast 'append' of rows to an array or a time series.


#include <fcntl.h>
#include <unistd.h>
#include <crpcut.hpp>
#include <thread>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/eventfd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
//#include <time.h>
#undef INFO
#include "net_handler.hpp"
#include "encode.hpp"
#include "parser_ctx.hpp"
#include "msg_handler.hpp"
#include "anf.hpp"
#include "load_builtin.hpp"
#include "timezone/zone.hpp"
#include "timezone/ztime.hpp"
#include "logging.hpp"
#include "zcpp.hpp"
#include "zcpp_stdlib.hpp"
#include "zts.hpp"
#include "../utils.hpp"


tz::Zones tzones("/usr/share/zoneinfo");
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

static volatile bool stop = false;
static int PORT = 23434;

/// create two instances of the database, send a query from one to the
/// other and return the evaluated result.
static std::tuple<zcore::MsgHandler*,pthread_t,pthread_t,net::NetHandler*> 
queryAndRun(const string& query) 
{
  int port1 = PORT;
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
  auto com1 = new net::NetHandler("", port1, data_com1_ir1, sig_com1_ir1);
  int pipe_here_ir1[2];
  int res = pipe2(pipe_here_ir1, O_CLOEXEC);
  if (res == -1) {
    throw std::system_error(std::error_code(errno, std::system_category()), 
			    "pipe failed for here_ir1");
  }
  auto ir1 = new zcore::MsgHandler(*com1, global, data_com1_ir1, sig_com1_ir1, pipe_here_ir1[0]);

  // run the tcp comm thread:
  static auto args1 = tpl_t{*com1, stop};
  pthread_t t1;
  pthread_create(&t1, NULL, executeNet, &args1); 

  // load the builtin functions (needed for "array, matrix, etc."):
  core::loadBuiltinFunctions(global.get());

  // get the query to the local context as if it had been written on
  // the console:
  res = write(pipe_here_ir1[1], query.c_str(), query.size());
  pthread_t i1;
  pthread_create(&i1, NULL, executeMsgHandler, ir1); 

  return std::make_tuple(ir1, t1, i1, com1);
}


static val::Value 
cancelAndReturnResult(std::tuple<zcore::MsgHandler*,pthread_t,pthread_t,net::NetHandler*>& t,
                      const std::string& varname, int fd) 
{
  // grab local context in order to get to the result:
  zcore::MsgHandler* ir = get<0>(t);
  const auto& localCtx = ir->getLocalCtx();
  auto lv = localCtx.r->find(varname); // Clang 4.0 doesn't like const auto here...

  stop = true;

  pthread_cancel(get<1>(t));
  pthread_cancel(get<2>(t));
  delete ir;
  delete get<3>(t);

  // Note that we have to close after the pthread_cancel; that's
  // because otherwise the close will cause the thread to get out of
  // the epoll_wait and the pthread_cancel might then be called
  // outside of a cancellation point, in which case we get "FATAL:
  // exception not rethrown".
  if (close(fd) < 0) {
    throw std::system_error(std::error_code(errno, std::system_category()), "close");
  }
  
  return lv;       // copy
}


static int open_send_close(const Global::buflen_pair& msg) {
  // pause before sending anything to make sure any previously issued
  // command has been executed:
  const timespec pausetime{0, 100000000L};
  nanosleep(&pausetime, NULL);

  // open TCP connection:
  sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr=inet_addr("127.0.0.1");
  addr.sin_port=htons(PORT);
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    throw std::system_error(std::error_code(errno, std::system_category()), "socket");
  }
  const unsigned attempts = 10;
  for (unsigned i=0; i<attempts; ++i) {
    int cres = connect(fd, (sockaddr *) &addr, sizeof(struct sockaddr_in));
    if (cres == -1) {
      if (i == attempts - 1) {
        close(fd);
        throw std::system_error(std::error_code(errno, std::system_category()), "connect");
      }
    }
    else {
      break;
    }
  }
  ssize_t wres = write(fd, msg.first.get(), msg.second);
  if (wres < 0) {
    throw std::system_error(std::error_code(errno, std::system_category()), "write");
  }

  // pause again to make sure data has been appended:
  nanosleep(&pausetime, NULL);

  return fd;
}


// test that the append routines errors out on a 0-sized message:
static Global::buflen_pair make_zero_length_append_msg_zts(const std::vector<std::string>& names,
                                                           size_t& offset)
{
  const std::vector<Global::dtime> idx{};
  const std::vector<double> v{};
  const auto headersz  = getHeaderLength(names);
  const auto datasz    = v.size()*sizeof(double);
  const auto rawvecsz  = sizeof(RawVector<double>);
  const auto idxdatasz = idx.size()*sizeof(Global::dtime);
  const auto totalsz   = headersz + rawvecsz + idxdatasz + rawvecsz + datasz;
  auto buf = std::make_pair(std::make_unique<char[]>(totalsz), totalsz);
  writeHeader(buf, Global::MsgType::APPEND_VECTOR, names);

  const RawVector<Global::dtime> idx_rv{TypeNumber<Global::dtime>::n, idx.size(), 1};
  memcpy(buf.first.get() + headersz, &idx_rv, rawvecsz);
  memcpy(buf.first.get() + headersz + rawvecsz, &idx.front(), idxdatasz);    

  const RawVector<double> v_rv{TypeNumber<double>::n, v.size(), 1};
  memcpy(buf.first.get() + headersz + rawvecsz + idxdatasz, &v_rv, rawvecsz);
  memcpy(buf.first.get() + headersz + rawvecsz + idxdatasz + rawvecsz, &v.front(), datasz);

  offset = headersz;
  return buf;
}

// test vector, 2D, 3D and the allowed types
TEST(comm_append_array_double) {
  auto tpl = queryAndRun("a <<- matrix(1:9, 3, 3)\n");

  const unsigned len = 9;
  auto data = arr::Vector<double>(len);
  std::iota(data.begin(), data.end(), 1);
  auto a = make_cow<val::VArrayD>(false, Vector<arr::idx_type>{3,3}, data);

  // create and send the append message (we just append 'a' to 'a'):
  auto msg = arr::make_append_msg({"a"s}, *a);
  int fd = open_send_close(msg);
  
  // get the result in and cleanup:
  auto res = cancelAndReturnResult(tpl, "a", fd);

  // append to 'a' a copy of the the original 'a' (can't append to oneself):
  auto b = *a;
  a->rbind(b);

  cout << val::display(res) << std::endl;
  cout << val::display(a) << std::endl;

  ASSERT_TRUE(res == a);
}
TEST(comm_append_array_double_nested_1) {
  auto tpl = queryAndRun("a <<- list(a1=0, a2=matrix(1:9, 3, 3))\n");

  const unsigned len = 9;
  auto a1 = make_cow<val::VArrayD>(false, Vector<arr::idx_type>{1}, arr::Vector<double>{0.0});
  auto data = arr::Vector<double>(len);
  std::iota(data.begin(), data.end(), 1);
  auto a2 = make_cow<val::VArrayD>(false, Vector<arr::idx_type>{3,3}, data);
  // append to 'a2' a copy of the the original 'a2' (can't append to oneself):
  auto b = *a2;
  a2->rbind(b);
  auto l = make_cow<val::VList>(false);
  l->a.concat(val::Value(a1), "a1"s);
  l->a.concat(val::Value(a2), "a2"s);
  
  // create and send the append message (we just append 'a' to 'a'):
  auto msg = arr::make_append_msg({"a"s, "a2"s}, b);
  int fd = open_send_close(msg);
  
  // get the result in and cleanup:
  auto res = cancelAndReturnResult(tpl, "a", fd);

  cout << val::display(res) << std::endl;
  cout << val::display(l) << std::endl;

  ASSERT_TRUE(res == l);
}
TEST(comm_append_array_double_nested_2) {
  auto tpl = queryAndRun("a <<- list(a1=0, l2=list(x=1, y=matrix(1:9, 3, 3)))\n");

  const unsigned len = 9;
  auto a1 = make_cow<val::VArrayD>(false, Vector<arr::idx_type>{1}, arr::Vector<double>{0.0});
  auto data = arr::Vector<double>(len);
  std::iota(data.begin(), data.end(), 1);
  auto l2 = make_cow<val::VList>(false);
  auto x = make_cow<val::VArrayD>(false, Vector<arr::idx_type>{1}, arr::Vector<double>{1.0});
  auto y = make_cow<val::VArrayD>(false, Vector<arr::idx_type>{3,3}, data);
  auto b = *y;
  y->rbind(b);
  l2->a.concat(val::Value(x), "x"s);
  l2->a.concat(val::Value(y), "y"s);
  // append to 'a2' a copy of the the original 'a2' (can't append to oneself):
  auto l = make_cow<val::VList>(false);
  l->a.concat(val::Value(a1), "a1"s);
  l->a.concat(val::Value(l2), "l2"s);
  
  // create and send the append message (we just append 'a' to 'a'):
  auto msg = arr::make_append_msg({"a"s, "l2"s, "y"s}, b);
  int fd = open_send_close(msg);
  
  // get the result in and cleanup:
  auto res = cancelAndReturnResult(tpl, "a", fd);

  cout << val::display(res) << std::endl;
  cout << val::display(l) << std::endl;

  ASSERT_TRUE(res == l);
}
TEST(comm_append_array_1D_double) {
  auto tpl = queryAndRun("a <<- 1:9\n");

  const unsigned len = 9;
  auto data = arr::Vector<double>(len);
  std::iota(data.begin(), data.end(), 1);
  auto a = make_cow<val::VArrayD>(false, Vector<arr::idx_type>{9}, data);

  auto msg = arr::make_append_msg({"a"s}, *a);
  auto fd = open_send_close(msg);
  
  auto res = cancelAndReturnResult(tpl, "a", fd);

  auto b = *a;
  a->concat(b);

  cout << val::display(res) << std::endl;
  cout << val::display(a) << std::endl;

  ASSERT_TRUE(res == a);
}
TEST(comm_append_array_bool) {
  auto tpl = queryAndRun("a <<- matrix(c(T,T,T,F,F,F), 3, 2)\n");
  auto a = make_cow<val::VArrayB>(false, 
                                  Vector<arr::idx_type>{3,2}, 
                                  Vector<bool>{true,true,true,false,false,false});

  // create and send the append message (we just append 'a' to 'a'):
  auto msg = arr::make_append_msg({"a"s}, *a);
  auto fd = open_send_close(msg);
  
  // get the result in and cleanup:
  auto res = cancelAndReturnResult(tpl, "a", fd);

  // append to 'a' a copy of the the original 'a' (can't append to oneself):
  auto b = *a;
  a->rbind(b);

  cout << val::display(res) << std::endl;
  cout << val::display(a) << std::endl;

  ASSERT_TRUE(res == a);
}
TEST(comm_append_array_duration) {
  auto tpl = queryAndRun("a <<- array(as.duration(1:8), c(2,2,2))\n");

  auto a = make_cow<val::VArrayDUR>(false, 
                                    Vector<arr::idx_type>{2,2,2}, 
                                    Vector<Global::duration>{
                                      Global::duration(1), Global::duration(2),
                                      Global::duration(3), Global::duration(4),
                                      Global::duration(5), Global::duration(6),
                                      Global::duration(7), Global::duration(8),
                                    });

  // create and send the append message (we just append 'a' to 'a'):
  auto msg = arr::make_append_msg({"a"s}, *a);
  auto fd = open_send_close(msg);
  
  // get the result in and cleanup:
  auto res = cancelAndReturnResult(tpl, "a", fd);

  // append to 'a' a copy of the the original 'a' (can't append to oneself):
  auto b = *a;
  a->rbind(b);

  cout << val::display(res) << std::endl;
  cout << val::display(a) << std::endl;

  ASSERT_TRUE(res == a);
}
TEST(comm_append_array_time) {
  auto tpl = queryAndRun("a <<- matrix(c(|.2015-03-09 06:38:01 America/New_York.|, "
                         "               |.2015-03-09 06:38:02 America/New_York.|, "
                         "               |.2015-03-09 06:38:03 America/New_York.|), 3, 1)\n");

  auto a = make_cow<val::VArrayDT>
    (false, 
     Vector<arr::idx_type>{3,1}, 
     Vector<Global::dtime>{
       tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones),
         tz::dtime_from_string("2015-03-09 06:38:02 America/New_York", tzones),
         tz::dtime_from_string("2015-03-09 06:38:03 America/New_York", tzones)});

  // create and send the append message (we just append 'a' to 'a'):
  auto msg = arr::make_append_msg({"a"s}, *a);
  auto fd = open_send_close(msg);
  
  // get the result in and cleanup:
  auto res = cancelAndReturnResult(tpl, "a", fd);

  // append to 'a' a copy of the the original 'a' (can't append to oneself):
  auto b = *a;
  a->rbind(b);

  cout << val::display(res) << std::endl;
  cout << val::display(a) << std::endl;

  ASSERT_TRUE(res == a);
}
TEST(comm_append_array_interval) {
  auto tpl = queryAndRun("i1 <- |-2015-03-09 06:38:01 America/New_York "
                         "-> 2015-03-10 06:38:01 America/New_York+|;"
                         "i2 <- |-2015-03-10 06:38:01 America/New_York "
                         "-> 2015-03-11 06:38:01 America/New_York+|;"
                         "a <<- matrix(c(i1, i2), 1, 2)\n");

  auto i1 = tz::interval_from_string("|-2015-03-09 06:38:01 America/New_York "
                                     "-> 2015-03-10 06:38:01 America/New_York+|", tzones);
  auto i2 = tz::interval_from_string("|-2015-03-10 06:38:01 America/New_York "
                                     "-> 2015-03-11 06:38:01 America/New_York+|", tzones);
  auto a = make_cow<val::VArrayIVL>(false, 
                                    Vector<arr::idx_type>{1,2}, 
                                    Vector<tz::interval>{i1, i2});
  
  // create and send the append message (we just append 'a' to 'a'):
  auto msg = arr::make_append_msg({"a"s}, *a);
  auto fd = open_send_close(msg);
  
  // get the result in and cleanup:
  auto res = cancelAndReturnResult(tpl, "a", fd);

  // append to 'a' a copy of the the original 'a' (can't append to oneself):
  auto b = *a;
  a->rbind(b);

  cout << val::display(res) << std::endl;
  cout << val::display(a) << std::endl;

  ASSERT_TRUE(res == a);
}
TEST(comm_append_zts) {
  auto tpl = queryAndRun("idx <- c(|.2015-03-09 06:38:01 America/New_York.|, "
                         "         |.2015-03-09 06:38:02 America/New_York.|, "
                         "         |.2015-03-09 06:38:03 America/New_York.|); "
                         "z <<- zts(idx, matrix(1.0:9, 3, 3, "
                                    "dimnames=list(NULL, c(\"one\", \"two\", \"three\"))))\n");

  // original z:
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones);
  auto dt2 = tz::dtime_from_string("2015-03-09 06:38:02 America/New_York", tzones);
  auto dt3 = tz::dtime_from_string("2015-03-09 06:38:03 America/New_York", tzones);
  auto a = arr::Array<double>({3,3}, {1,2,3,4,5,6,7,8,9}, {{}, {"one", "two", "three"}});
  const auto z = make_cow<arr::zts>(false, 
                                    Vector<Global::dtime>{dt1,dt2,dt3}, 
                                    std::move(a));

  // zts to be appended to z:
  auto dt4 = tz::dtime_from_string("2015-03-09 06:38:04 America/New_York", tzones);
  auto dt5 = tz::dtime_from_string("2015-03-09 06:38:05 America/New_York", tzones);
  auto dt6 = tz::dtime_from_string("2015-03-09 06:38:06 America/New_York", tzones);
  auto aa = arr::Array<double>({3,3}, {1,2,3,4,5,6,7,8,9});
  const arr::zts az(arr::Array<Global::dtime>({dt4,dt5,dt6}), std::move(aa));

  // expected
  auto ee = arr::Array<double>({6,3}, 
                               {1,2,3,1,2,3,4,5,6,4,5,6,7,8,9,7,8,9}, 
                               {{}, {"one", "two", "three"}});
  const auto expected = make_cow<arr::zts>(false, 
                                           Vector<Global::dtime>{dt1,dt2,dt3,dt4,dt5,dt6}, 
                                           std::move(ee));

  auto msg = arr::make_append_msg({"z"s}, az);
  auto fd = open_send_close(msg);
  
  auto res = cancelAndReturnResult(tpl, "z", fd);
  const auto& p = get<val::SpZts>(res);
  for (unsigned i=0;i<p->size();++i) std::cout << p->getArray()[i] << std::endl;
  ASSERT_TRUE(res == expected);
}
TEST(comm_append_zts_not_ascending, log_to_file) {
  auto tpl = queryAndRun("idx <- c(|.2015-03-09 06:38:01 America/New_York.|, "
                         "         |.2015-03-09 06:38:02 America/New_York.|, "
                         "         |.2015-03-09 06:38:03 America/New_York.|); "
                         "z <<- zts(idx, matrix(1.0:9, 3, 3, "
                                    "dimnames=list(NULL, c(\"one\", \"two\", \"three\"))))\n");

  // original z:
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones);
  auto dt2 = tz::dtime_from_string("2015-03-09 06:38:02 America/New_York", tzones);
  auto dt3 = tz::dtime_from_string("2015-03-09 06:38:03 America/New_York", tzones);
  auto aa = arr::Array<double>({3,3}, {1,2,3,4,5,6,7,8,9});
  const arr::zts az(arr::Array<Global::dtime>({dt1,dt2,dt3}), std::move(aa));

  auto msg = arr::make_append_msg({"z"s}, az);
  auto fd = open_send_close(msg);
  
  auto res = cancelAndReturnResult(tpl, "z", fd);

  ASSERT_TRUE(matchLog("index not ascending"));
}
TEST(comm_append_array_incorrect_dimension, log_to_file) {
  auto tpl = queryAndRun("a <<- matrix(1:9, 3, 3)\n");

  const unsigned len = 9;
  auto data = arr::Vector<double>(len);
  std::iota(data.begin(), data.end(), 1);
  auto a = make_cow<val::VArrayD>(false, Vector<arr::idx_type>{3,3}, data);
  auto aa = val::VArrayD({9}, data);

  auto msg = arr::make_append_msg({"a"s}, aa); // append incorrect size
  auto fd = open_send_close(msg);
  
  // get the result in and cleanup:
  auto res = cancelAndReturnResult(tpl, "a", fd);
  
  ASSERT_TRUE(matchLog("incorrect dimensions for append"));
}
TEST(comm_append_array_missing_data, log_to_file) {
  auto tpl = queryAndRun("a <<- matrix(1:9, 3, 3)\n");

  const unsigned len = 9;
  auto data = arr::Vector<double>(len);
  std::iota(data.begin(), data.end(), 1);
  auto a = make_cow<val::VArrayD>(false, Vector<arr::idx_type>{3,3}, data);

  auto msg = arr::make_append_msg({"a"s}, *a);

  // shorten the message in order to create the error  
  msg.second -= 8;
  auto newMsgSz = hton64(msg.second);
  memcpy(msg.first.get() + 8, &newMsgSz, sizeof(msg.second));
         
  auto fd = open_send_close(msg);
  
  // get the result in and cleanup:
  auto res = cancelAndReturnResult(tpl, "a", fd);
  
  ASSERT_TRUE(matchLog("missing data"));
}
TEST(comm_append_array_incorrect_type, log_to_file) {
  auto tpl = queryAndRun("a <<- matrix(1:9, 3, 3)\n");

  arr::Vector<bool> data{true,true,true,false,false,false,true,true,true};
  auto a = make_cow<val::VArrayB>(false, Vector<arr::idx_type>{3,3}, data);

  auto msg = arr::make_append_msg({"a"s}, *a);

  auto fd = open_send_close(msg);
  
  // get the result in and cleanup:
  auto res = cancelAndReturnResult(tpl, "a", fd);
  
  ASSERT_TRUE(matchLog("incorrect type"));
}
TEST(comm_append_vector_double) {
  auto tpl = queryAndRun("a <<- matrix(1:9, 3, 3)\n");

  const unsigned len = 9;
  auto data = arr::Vector<double>(len);
  std::iota(data.begin(), data.end(), 1);
  auto a = make_cow<val::VArrayD>(false, Vector<arr::idx_type>{3,3}, data);

  // create and send the append message (we just append 'a' to 'a'):
  auto msg = arr::make_append_msg({"a"s}, Vector<double>{1,2,3,4,5,6,7,8,9});
  auto fd = open_send_close(msg);
  
  // get the result in and cleanup:
  auto res = cancelAndReturnResult(tpl, "a", fd);

  // append to 'a' a copy of the the original 'a' (can't append to oneself):
  auto b = *a;
  a->rbind(b);

  ASSERT_TRUE(res == a);
}
TEST(comm_append_vector_zts) {
  auto tpl = queryAndRun("idx <- c(|.2015-03-09 06:38:01 America/New_York.|, "
                         "         |.2015-03-09 06:38:02 America/New_York.|, "
                         "         |.2015-03-09 06:38:03 America/New_York.|); "
                         "z <<- zts(idx, matrix(1.0:9, 3, 3, "
                                    "dimnames=list(NULL, c(\"one\", \"two\", \"three\"))))\n");

  // original z:
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones);
  auto dt2 = tz::dtime_from_string("2015-03-09 06:38:02 America/New_York", tzones);
  auto dt3 = tz::dtime_from_string("2015-03-09 06:38:03 America/New_York", tzones);
  auto a = arr::Array<double>({3,3}, {1,2,3,4,5,6,7,8,9}, {{}, {"one", "two", "three"}});
  const auto z = make_cow<arr::zts>(false, 
                                    Vector<Global::dtime>{dt1,dt2,dt3}, 
                                    std::move(a));

  // zts to be appended to z:
  auto dt4 = tz::dtime_from_string("2015-03-09 06:38:04 America/New_York", tzones);
  auto dt5 = tz::dtime_from_string("2015-03-09 06:38:05 America/New_York", tzones);
  auto dt6 = tz::dtime_from_string("2015-03-09 06:38:06 America/New_York", tzones);
  auto aa = arr::Array<double>({3,3}, {1,2,3,4,5,6,7,8,9});
  const arr::zts az(arr::Array<Global::dtime>({dt4,dt5,dt6}), std::move(aa));

  // expected
  auto ee = arr::Array<double>({6,3}, 
                               {1,2,3,1,2,3,4,5,6,4,5,6,7,8,9,7,8,9}, 
                               {{}, {"one", "two", "three"}});
  const auto expected = make_cow<arr::zts>(false, 
                                           Vector<Global::dtime>{dt1,dt2,dt3,dt4,dt5,dt6}, 
                                           std::move(ee));

  auto msg = arr::make_append_msg({"z"s},
                                  Vector<Global::dtime>{dt4,dt5,dt6},
                                  Vector<double>{1,2,3,4,5,6,7,8,9});


  auto fd = open_send_close(msg);
  auto res = cancelAndReturnResult(tpl, "z", fd);

  cout << val::display(res) << std::endl;
  cout << val::display(expected) << std::endl;

  ASSERT_TRUE(res == expected);
}
TEST(comm_append_vector_zts_idx_data_mismatch_make_msg) {
  auto dt4 = tz::dtime_from_string("2015-03-09 06:38:04 America/New_York", tzones);
  auto dt5 = tz::dtime_from_string("2015-03-09 06:38:05 America/New_York", tzones);
  ASSERT_THROW(arr::make_append_msg({"z"s},
                                    Vector<Global::dtime>{dt4,dt5},
                                    Vector<double>{1,2,3,4,5,6,7,8,9}),
               std::out_of_range,
               "make_append_msg: idx and v have incompatible lengths");
}
TEST(comm_append_vector_zts_idx_data_mismatch_receive_msg, log_to_file) {

}
TEST(comm_append_vector_zts_idx_unsorted_make_msg) {
  auto dt4 = tz::dtime_from_string("2015-03-09 06:38:04 America/New_York", tzones);
  auto dt5 = tz::dtime_from_string("2015-03-09 06:38:05 America/New_York", tzones);
  ASSERT_THROW(arr::make_append_msg({"z"s},
                                    Vector<Global::dtime>{dt5,dt4},
                                    Vector<double>{1,2,3,4,5,6}),
               std::out_of_range,
               "make_append_msg: idx must be sorted");
}
TEST(comm_append_vector_zts_idx_unsorted_receive_msg, log_to_file) {
  auto tpl = queryAndRun("idx <- c(|.2015-03-09 06:38:01 America/New_York.|, "
                         "         |.2015-03-09 06:38:02 America/New_York.|, "
                         "         |.2015-03-09 06:38:03 America/New_York.|); "
                         "z <<- zts(idx, matrix(1.0:9, 3, 3, "
                                    "dimnames=list(NULL, c(\"one\", \"two\", \"three\"))))\n");

  // zts to be appended to z:
  auto dt4 = tz::dtime_from_string("2015-03-09 06:38:04 America/New_York", tzones);
  auto dt5 = tz::dtime_from_string("2015-03-09 06:38:05 America/New_York", tzones);
  auto dt6 = tz::dtime_from_string("2015-03-09 06:38:06 America/New_York", tzones);

  auto msg = arr::make_append_msg({"z"s},
                                  Vector<Global::dtime>{dt4,dt5,dt6},
                                  Vector<double>{1,2,3,4,5,6,7,8,9});
  // tweek this message by copying dt5 on dt4: 
  memcpy(msg.first.get()+8*8, msg.first.get()+9*8, sizeof(Global::dtime));
  
  auto fd = open_send_close(msg);
  cancelAndReturnResult(tpl, "z", fd);

  ASSERT_TRUE(matchLog("index not ascending"));
}
TEST(comm_append_vector_zts_idx_not_increasing_receive_msg, log_to_file) {
  auto tpl = queryAndRun("idx <- c(|.2015-03-09 06:38:01 America/New_York.|, "
                         "         |.2015-03-09 06:38:02 America/New_York.|, "
                         "         |.2015-03-09 06:38:03 America/New_York.|); "
                         "z <<- zts(idx, matrix(1.0:9, 3, 3, "
                                    "dimnames=list(NULL, c(\"one\", \"two\", \"three\"))))\n");

  // zts to be appended to z:
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones);
  auto dt2 = tz::dtime_from_string("2015-03-09 06:38:02 America/New_York", tzones);
  auto dt3 = tz::dtime_from_string("2015-03-09 06:38:03 America/New_York", tzones);

  auto msg = arr::make_append_msg({"z"s},
                                  Vector<Global::dtime>{dt1,dt2,dt3},
                                  Vector<double>{1,2,3,4,5,6,7,8,9});

  auto fd = open_send_close(msg);
  cancelAndReturnResult(tpl, "z", fd);

  ASSERT_TRUE(matchLog("append index not ascending"));
}
TEST(comm_make_append_msg_zts_empty_Vector) {
  ASSERT_THROW(arr::make_append_msg({"z"s},
                                    Vector<Global::dtime>{},
                                    Vector<double>{}),
               std::out_of_range,
               "make_append_msg: idx has size 0");
}
TEST(comm_make_append_msg_zts_empty_vector) {
  ASSERT_THROW(arr::make_append_msg({"z"s},
                                    std::vector<Global::dtime>{},
                                    std::vector<double>{}),
               std::out_of_range,
               "make_append_msg: idx has size 0");
}
TEST(comm_make_append_msg_array_empty_Vector) {
  ASSERT_THROW(arr::make_append_msg({"a"s}, arr::Vector<double>{}),
               std::out_of_range,
               "make_append_msg: no data");
}
TEST(comm_make_append_msg_array_empty_Array) {
  auto a = val::VArrayD(Vector<arr::idx_type>{}, Vector<double>{});
  ASSERT_THROW(arr::make_append_msg({"a"s}, arr::Vector<double>{}),
               std::out_of_range,
               "make_append_msg: no data");
}
TEST(comm_zts_appendVector_empty_message) {
  size_t offset;
  auto msg = make_zero_length_append_msg_zts({"z"s}, offset);
  // original z:
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones);
  auto dt2 = tz::dtime_from_string("2015-03-09 06:38:02 America/New_York", tzones);
  auto dt3 = tz::dtime_from_string("2015-03-09 06:38:03 America/New_York", tzones);
  auto a = arr::Array<double>({3,3}, {1,2,3,4,5,6,7,8,9}, {{}, {"one", "two", "three"}});
  auto z = make_cow<arr::zts>(false, 
                              Vector<Global::dtime>{dt1,dt2,dt3}, 
                              std::move(a));
  ASSERT_THROW(z->appendVector(msg.first.get()+offset, msg.second),
               std::out_of_range,
               "arr::zts::appendVector: time vector has size 0");
}

int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
