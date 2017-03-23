// -*- compile-command: "g++ -g -std=c++1y append.cpp -o append -lstdc++ -lztsdb -lboost_system" -*-

// Copyright (C) 2017 Leonardo Silvestri
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


#include <sys/timerfd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <string>
#include <iostream>
#include <sstream>
#include <limits>
#include <ztsdb/zcpp.hpp>
#include <ztsdb/cow_ptr.hpp>


static void loop_append(const std::string ip, 
                        int port,
                        size_t rate, 
                        const std::vector<std::string>& names, 
                        size_t ncols, 
                        size_t max_msg)
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

  
  // create and set msg_timerfd:
  int msg_timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
  if (fd == -1) {
    throw std::system_error(std::error_code(errno, std::system_category()), "timerfd_create");
  }
  itimerspec tmr;
  bzero(&tmr, sizeof(tmr));  
  if (rate == 1) {
    tmr.it_value.tv_sec = 1;
    tmr.it_value.tv_nsec = 0;
    tmr.it_interval.tv_sec = 1;
    tmr.it_interval.tv_nsec = 0;  
  }
  else {
    tmr.it_value.tv_sec = 0;
    tmr.it_value.tv_nsec = 1e9/rate;
    tmr.it_interval.tv_sec = 0;
    tmr.it_interval.tv_nsec = 1e9/rate;
  }
  if (timerfd_settime(msg_timerfd, 0, &tmr, NULL) == -1) {
    throw std::system_error(std::error_code(errno, std::system_category()), "timerfd_settime");
  }

  // create and set second_timerfd, a timer that goes off every second:
  int second_timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
  if (fd == -1) {
    throw std::system_error(std::error_code(errno, std::system_category()), "timerfd_create");
  }
  bzero(&tmr, sizeof(tmr));
  tmr.it_value.tv_sec = 1;
  tmr.it_value.tv_nsec = 0;
  tmr.it_interval.tv_sec = 1;
  tmr.it_interval.tv_nsec = 0;
  if (timerfd_settime(second_timerfd, 0, &tmr, NULL) == -1) {
    throw std::system_error(std::error_code(errno, std::system_category()), "timerfd_settime");
  }


  // create epollfd and add timer to epoll:
  int epollfd = epoll_create1(0);
  if (epollfd == -1) {
    throw std::system_error(std::error_code(errno, std::system_category()), "epoll_create1");
  }
  epoll_event ev;
  ev.events = EPOLLIN;
  ev.data.fd = msg_timerfd;
  if (epoll_ctl(epollfd, EPOLL_CTL_ADD, msg_timerfd, &ev) == -1) {
    throw std::system_error(std::error_code(errno, std::system_category()), "epoll_ctl");
  }
  ev.data.fd = second_timerfd;
  if (epoll_ctl(epollfd, EPOLL_CTL_ADD, second_timerfd, &ev) == -1) {
    throw std::system_error(std::error_code(errno, std::system_category()), "epoll_ctl");
  }


  const auto firstnow = std::chrono::system_clock::now();
  arr::Vector<double> data(ncols);
  
  // loop append:
  const size_t EPOLL_MAX_EVENTS = 10;
  epoll_event events[EPOLL_MAX_EVENTS];
  size_t nmsgs = 0;
  size_t seconds = 0;
  bool done = false;
  while (!done) {
    int nfds = epoll_wait(epollfd, events, EPOLL_MAX_EVENTS, -1);
    for (int i = 0; i < nfds; ++i) { 
      if (events[i].data.fd == msg_timerfd) {
        const auto now = std::chrono::system_clock::now();
        
        for (unsigned j=0; j<ncols; ++j) {
          setv_nocheck(data, j, (now - firstnow).count()/1e9 + i/10.0);
        }
        
        uint64_t count;
        ssize_t rres = read(msg_timerfd, &count, sizeof(count));
        if (rres < 0) {
          throw std::system_error(std::error_code(errno, std::system_category()), "read");
        }

        // create and send the append message:
        auto msg = arr::make_append_msg(names, 
                                        arr::Vector<Global::dtime>{now}, 
                                        data);
        nmsgs++;

        ssize_t wres = write(fd, msg.first.get(), msg.second);
        if (wres < 0) {
          throw std::system_error(std::error_code(errno, std::system_category()), "write");
        }

        if (nmsgs == max_msg) {
          done = true;
        }
      }
      else if (events[i].data.fd == second_timerfd) {
        uint64_t count;
        ssize_t rres = read(second_timerfd, &count, sizeof(count));
        if (rres < 0) {
          throw std::system_error(std::error_code(errno, std::system_category()), "read");
        }
        ++seconds;
        std::cout << "seconds: " << seconds << ", nb of msgs sent: " << nmsgs << std::endl;
      }
    }
  }
  std::cout << "total nb of msgs sent: " << nmsgs << std::endl;

  if (close(fd) < 0) {
    throw std::system_error(std::error_code(errno, std::system_category()), "close");
  }
}


// params are
// 1. IP
// 2. port
// 3. a rate (# per seconds)
// 4. name of variable to append to (assumed to be a zts)
// 5. number of columns to append
// 6. maximum number of messages to send
int main(int argc, char* argv[]) {
  enum { IP=1, PORT, RATE, VARNAMES, NCOLS, MAX_MSG };

  // grab a message rate (# per second)
  if (argc < 6 || argc > 7) {
    std::cerr << "usage: " << argv[0] 
              << " <ip> <port> <rate> <varname[,name1,name2,...]> <ncols> [max-msgs]" << std::endl;
    return -1;
  }
  
  int port = std::stoi(argv[PORT]);
  size_t rate = std::stoull(argv[RATE]);
  size_t ncols = std::stoull(argv[NCOLS]);
  size_t max_msg = argc == 6 ? std::numeric_limits<size_t>::max() :
    std::stoll(argv[MAX_MSG]); // -1 means run forever

  std::istringstream varnames_ss(argv[VARNAMES]);
  std::string token;
  std::vector<std::string> names;
  while (std::getline(varnames_ss, token, ',')) {
    names.push_back(token);
  }
  loop_append(argv[IP], port, rate, names, ncols, max_msg);

  return 0;
}
