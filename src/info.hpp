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


#ifndef INFO_HPP
#define INFO_HPP


#include <vector>
#include <set> 


namespace zcore {

  struct ConnectionInfo {
    enum Direction { INCOMING, OUTGOING };

    Global::conn_id_t id;
    std::string ipaddress;
    int port;
    Direction dir;
  };


  struct NetInfo {
    std::vector<ConnectionInfo> conninfo;
    size_t buflistSz;
    size_t siglistSz;
  };

  
  struct MsgInfo {
    std::set<Global::conn_id_t> reqContexts;
    std::set<Global::conn_id_t> rspContexts;
  };


  struct StateInfo {
    Global::reqid_t reqid;   
    Global::reqid_t sourceid;
    Global::conn_id_t peerid;
  };

  struct CtxInfo {
    StateInfo state;
  };
}


#endif
