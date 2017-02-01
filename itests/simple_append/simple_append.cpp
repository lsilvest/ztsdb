// -*- compile-command: "make -k -j8 simple_append" -*-

// Copyright (C) 2016 Leonardo Silvestri
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


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>
#include <limits>
#include <vector>
#include <system_error>
#include "zcpp_stdlib.hpp"
#include "timezone/zone.hpp"


// need to accomodate non-standard location LLL
tz::Zones tzones("/usr/share/zoneinfo");


static void simple_append(const std::string ip, 
                          int port,
                          const std::string varname, 
                          size_t ncols)
{
  // open TCP connection:
  sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr=inet_addr(ip.c_str());
  addr.sin_port=htons(port);
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    throw std::system_error(std::error_code(errno, std::system_category()), "socket");
  }
  int cres = connect(fd, (sockaddr *) &addr, sizeof(struct sockaddr_in));
  if (cres == -1) {
    close(fd);
    throw std::system_error(std::error_code(errno, std::system_category()), "connect");
  }

  const auto now = std::chrono::system_clock::now();
        
  // create and populate a data vector:
  std::vector<double> data(ncols);
  for (unsigned j=0; j<ncols; ++j) {
    data[j] = j;   // set at position j the value j
  }
        
  // create the append message:
  auto msg = arr::make_append_msg(varname, std::vector<Global::dtime>{now}, data);

  // send it:
  ssize_t wres = write(fd, msg.first.get(), msg.second);
  if (wres < 0) {
    throw std::system_error(std::error_code(errno, std::system_category()), "write");
  }

  if (close(fd) < 0) {
    throw std::system_error(std::error_code(errno, std::system_category()), "close");
  }
}


// params are
// 1. IP
// 2. port
// 3. name of variable to append to (assumed to be a zts)
// 4. number of columns to append
int main(int argc, char* argv[]) {
  enum { IP=1, PORT, VARNAME, NCOLS };

  // grab a message rate (# per second)
  if (argc != 5) {
    std::cerr << "usage: " << argv[0] << " <ip> <port> <varname> <ncols>" << std::endl;
    return -1;
  }
  
  int port = std::stoi(argv[PORT]);
  size_t ncols = std::stoull(argv[NCOLS]);

  simple_append(argv[IP], port, argv[VARNAME], ncols);

  return 0;
}
