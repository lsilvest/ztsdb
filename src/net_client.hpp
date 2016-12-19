// (C) 2016 Leonardo Silvestri
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


#ifndef NET_CLIENT_HPP
#define NET_CLIENT_HPP


#include <poll.h>
#include "encode.hpp"
#include "net_handler.hpp"


namespace client {

  enum IncomingRspState {
    RSP_IDLE,
    RSP,
    RSP_DONE
  };


  struct RspState {
    RspState() : state(RSP_IDLE), valstack_idx(0), reqid(0) { }
    RspState(Global::reqid_t reqid_p) : state(RSP_IDLE), valstack_idx(0), reqid(reqid_p) { }

    IncomingRspState state;

    size_t valstack_idx;
    vector<zcore::ValState> valstack;

    Global::reqid_t reqid;
  };


  struct Client {

    Client(net::NetHandler& com_p, 
           int fd_read_data_p,
           int fd_read_sig_p);
    
    int run();

    // this one will create a new connection, and hence a new RspState
    Global::conn_id_t connect(const string& ip, int port);

    size_t sendReq(Global::conn_id_t peerid,
                   Global::conn_id_t sourceid, 
                   const E* e, 
                   const val::SpVList& boundvars);

    int readRspData(Global::reqid_t reqid, 
                    Global::reqid_t sourceid, 
                    const char* buf, 
                    size_t len);

    const val::Value& getVal() const;

  private:
    net::NetHandler& com;

    RspState rsp; // incoming response
    nfds_t nfds;

    int fd_read_data;
    int fd_read_sig;
    static const nfds_t nfds_max = 2;
    struct pollfd fds[nfds_max];
  };

}


#endif

