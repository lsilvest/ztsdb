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


#include <Rcpp.h>
#include <string>
#include <memory>
#include <thread>
#include <unistd.h>
#include <fcntl.h>

#include <ztsdb/net_handler.hpp>
#include <ztsdb/net_client.hpp>


class Zconnection {
public:
  Zconnection(std::string _addr, int _port);

  SEXP query(std::string s, SEXP e, SEXP env);


  ~Zconnection();

private:
  std::string addr;    // only needed for printout, maybe we can get it from comm LLL
  int port;            // only needed for printout, maybe we can get it from comm LLL

  Global::conn_id_t peer_conn_id; ///> the identifier for the connection 

  static std::unique_ptr<net::NetHandler> com;
  static std::unique_ptr<client::Client> net;
  static volatile bool stop;
};


