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
#include <sys/eventfd.h>
#include "parser_ctx.hpp"
#include "logging.hpp"
#include "net_client.hpp"
#include "anf.hpp"
#include "display.hpp"


tz::Zones tzones("/usr/share/zoneinfo"); // should be configurable LLL
zlog::Logger lg;
cfg::CfgMap cfg::cfgmap;


using namespace val;

static void* executeNetHandler(void* args_p) {
  auto args = static_cast<std::pair<net::NetHandler*, volatile bool&>*>(args_p);
  auto c = args->first;
  try {
    c->run(args->second);
  }
  catch (std::exception& e) {
    std::cout << "exception is: " << e.what() << std::endl;
    lg.log(zlog::SV_ERROR, e.what());
    throw;
  }

  return nullptr;
}


int main(int argc, char* argv[]) {
  if (argc != 4) {
    std::cerr << "usage: " << argv[0] << " <ip_address> <port> <expression>" << std::endl;
    return EXIT_FAILURE;
  }
  string ip_address = argv[1];
  int port = atoi(argv[2]);
  auto expr = argv[3];

  int data_com_ir = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
  if (data_com_ir == -1) {
    throw std::system_error(std::error_code(errno, std::system_category()), 
                            "eventfd failed for data_com_ir");
  }
  int sig_com_ir = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
  if (sig_com_ir == -1) {
    throw std::system_error(std::error_code(errno, std::system_category()), 
                            "eventfd failed for sig_com_ir");
  }
  
  net::NetHandler com("", 0, data_com_ir, sig_com_ir);
 
  volatile bool stop;
  static auto args = std::pair<net::NetHandler*, volatile bool&>{&com, stop};
  pthread_t comThread;
  pthread_create(&comThread, NULL, executeNetHandler, &args);

  client::Client c(com, data_com_ir, sig_com_ir);
  auto peer_conn_id = c.connect(ip_address, port);

  ParserCtx pctx;
#ifdef _DEBUG
  pctx.trace_scanning = true;
  pctx.trace_parsing  = true;
#endif
  
  auto res = pctx.parse(std::make_shared<std::string>(expr + "\n"s));
  if (res != 0) {
    std::cerr << "error parsing: " << expr << std::endl;
    return EXIT_FAILURE;
  }
  
  anf::convertToANF(pctx.prog.get());
    
  auto l = arr::make_cow<val::VList>(false);

  res = c.sendReq(peer_conn_id, 0, pctx.prog.get(), l);
  c.run();

  std::cout << val::display(c.getVal()) << std::endl;

  return 0;
}
