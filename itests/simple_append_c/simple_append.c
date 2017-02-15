// -*- compile-command: "make -k -j8 simple_append" -*-

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


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "zc.h"


static void simple_append(const char* ip, int port, const char* varname, size_t ncols)
{
  // open TCP connection:
  struct sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr=inet_addr(ip);
  addr.sin_port=htons(port);
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    fprintf(stderr, "'socket' error: %s\n", strerror(errno));
  }
  int cres = connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
  if (cres == -1) {
    close(fd);
    fprintf(stderr, "'connect' error: %s\n", strerror(errno));
  }

  // create and populate a data vector:
  double* data = malloc(sizeof(double) * ncols);
  size_t j;
  for (j=0; j<ncols; ++j) {
    data[j] = j;   // set at position j the value j
  }


  // get a time: ztsdb requires increasing values of time in a time
  // series; using a monotonic clock guarantees that, but
  // unfortunately, it's not required to be a time since Epoch (in
  // Linux, it's a value since boot time); here we build a monotonic
  // timestamp from the realtime. The clock is a bit off, but it
  // servers its purpose and this is a valid method for multiple
  // appends to a time-series from a single thread; if some kind of
  // synchronization must be achived betwee multiple
  // threads/processes, one could get the machine uptime to have a
  // build a monotonic timestamp on a given machine:
  struct timespec monotonic;
  clock_gettime(CLOCK_MONOTONIC_RAW, &monotonic);
  int64_t timestamp_mono = monotonic.tv_sec * 1000000000L + monotonic.tv_nsec;
  struct timespec realtime;
  clock_gettime(CLOCK_REALTIME, &realtime);
  int64_t timestamp_real = realtime.tv_sec * 1000000000L + realtime.tv_nsec;
  int64_t timestamp_diff = timestamp_real - timestamp_mono;

  // then this process could repeatedly build timestamps like this:
  int64_t timestamp = timestamp_mono + timestamp_diff;
  
  // create the append message:
  char* buf;
  size_t buflen;
  int res = make_append_msg(varname, &timestamp, 1, data, ncols, &buf, &buflen);
  free(data);
  
  // send it:
  ssize_t wres = write(fd, buf, buflen);
  free(buf);
  if (wres < 0) {
    fprintf(stderr, "'write' error: %s\n", strerror(errno));
  }

  if (close(fd) < 0) {
    fprintf(stderr, "'close' error: %s\n", strerror(errno));
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
    fprintf(stderr, "usage: %s <ip> <port> <varname> <ncols>\n", argv[0]);
    return -1;
  }
  
  int port = atoi(argv[PORT]);
  char* endptr;
  size_t ncols = strtoull(argv[NCOLS], &endptr, 10);

  simple_append(argv[IP], port, argv[VARNAME], ncols);

  return 0;
}
