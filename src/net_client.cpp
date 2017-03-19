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


#include "net_client.hpp"
#include "logging.hpp"
#include "valuevector.hpp"


// #define _DEBUG


static auto getNextId = GenNbFun<Global::reqid_t>(); // reqid sequence generator


/// For use in the fds array.
enum { SIG, DATA };



client::Client::Client(net::NetHandler& com_p, 
                            int fd_read_data_p,
                            int fd_read_sig_p) : 
  com(com_p), fd_read_data(fd_read_data_p), 
  fd_read_sig(fd_read_sig_p)
{
  nfds = nfds_max;
  fds[SIG].fd = fd_read_sig;    // tells us signalling available for reading
  fds[SIG].events = POLLIN;
  fds[DATA].fd = fd_read_data;  // tells us data buffers available for reading
  fds[DATA].events = POLLIN;
}


int client::Client::run() {
#ifdef _DEBUG
  std::cout << "client::Client::run()" << std::endl;
#endif

  for (;;) {
    const int timeout_ms = 1000; // note: poll is milliseconds
    int res = poll(fds, nfds, timeout_ms);

    if (res == 0) {
      // LLL some kind of gc?
    }
    else if (fds[SIG].revents & POLLIN) {
      size_t val;
      ssize_t n = read(fd_read_sig, &val, sizeof(val));
      if (n < 0) {
        // ignore, log?
      
      }
      // I think we might receive DOWN, but not UP I believe LLL
      //
      // data: ----------------------------------------------
    }
    else if (fds[DATA].revents & POLLIN) {
      size_t val;
      ssize_t n = read(fd_read_data, &val, sizeof(val));
      if (n < 0) {
        throw std::system_error(std::error_code(errno, std::system_category()), "read(fd_read_data)");
      }

      // get message signalled by the event:
      Global::conn_id_t id;
      net::Buf combuf;
      while (n-- > 0 && com.get_data(id, combuf)) {
        const char* buf = combuf.data.get();
        
        auto len = combuf.len;
        Global::MsgType mt;
        memcpy(&mt, buf, sizeof(mt));
        mt = static_cast<Global::MsgType>(ntoh64(static_cast<uint64_t>(mt)));
          
        // APPEND or APPEND_VECTOR ----------------------------
        if (mt == Global::MsgType::APPEND || mt == Global::MsgType::APPEND_VECTOR) {
          // error, but don't really want to log here, that would be an easy DOS; or log once?
          // make it a stat? LLL
        }
        // REQ or RSP ----------------------------
        Global::reqid_t reqid;
        memcpy(&reqid, buf+8, sizeof(reqid));
        reqid = ntoh64(reqid);

        Global::reqid_t sourceid;
        memcpy(&sourceid, buf+16, sizeof(sourceid));
        sourceid = ntoh64(sourceid);
#ifdef _DEBUG
        cout << "reqid:" << reqid << ", sourceid:" << sourceid << endl;
#endif	 
        if (mt != Global::MsgType::RSP) {
          // error, but don't really want to log here, that would be an easy DOS; or log once?
          // make it a stat? LLL
          std::cout << "message was not a response!" << std::endl;
        } 

        try {
          auto waiting = readRspData(reqid, sourceid, buf+24, len-24);
          if (!waiting) {
            return 0;
          }
        }
        catch (std::exception& e) {
          // lg.log(zlog::SV_DEBUG, "unexpected exception: %s", e.what());
        }
      }
    }
  }
    
}



Global::conn_id_t client::Client::connect(const string& ip, int port) {
  auto conn_id = com.connect(ip, port);
#ifdef _DEBUG
  std::cout << "InterpRun::connect: conn_id is: " << conn_id << std::endl; 
#endif
  return conn_id;
}


size_t client::Client::sendReq(Global::conn_id_t peer_conn_id,
                                 Global::reqid_t sourceid, 
                                 const E* e, 
                                 const val::SpVList& boundvars) {
#ifdef _DEBUG
  cout << "sendReq" << endl;
#endif

  // auto id = getNextId();
  // rsp.reqid = id;
  // rsp.state = RSP_IDLE;
  // rsp.valstack_idx = 0;
  // rsp.valstack = vector<ValState>();

  rsp = RspState(getNextId());

  try {
    zcore::Encode ec(com, peer_conn_id, rsp.reqid, sourceid, Global::MsgType::REQ);
    ec << e;
    ec << val::Value(boundvars);
#ifdef _DEBUG
    cout << "| REQ buffer:" << endl;
    cout << printBuf(ec.buf, ec.offset) << endl;
#endif
    ec.flush();
  
    // stats.bytesOutREQ += ec.bytes;
    return ec.offset;
  } catch (std::exception& e) {
    //lg.log(zlog::SV_DEBUG, "unexpected exception: %s", e.what());
    com.disconnect(peer_conn_id);     // LLL
    throw;
  }
}


// note that at this point buf must not contain the magic number nor
// the buffer length; these have already been processed and obviously
// the correct buf length is given by len.
int client::Client::readRspData(Global::reqid_t reqid, 
                                  Global::reqid_t sourceid, 
                                  const char* buf, 
                                  size_t len) {
#ifdef _DEBUG
  cout << "Client::readRspData():" << endl;
  cout << "| buf len: " << len << endl;
  cout << "| buf:" << endl;
  //  cout << printBuf(buf, len) << endl;
#endif

  if (rsp.state == RSP_IDLE) {
    rsp.state = RSP;
  }

  size_t offset = 0;
  readValue(buf, len, offset, rsp.valstack, rsp.valstack_idx); // will throw

  // note that for 'std_string' encoding, 'n' is larger than 'exp'
  // because of the padding, so we really do need to use 'n >= exp'
  // and not 'n == exp':
  if (rsp.valstack[0].n >= rsp.valstack[0].exp) { // current len >= expected len
#ifdef _DEBUG
    cout << "| state set tp RSP_DONE" << endl;
#endif

    rsp.state = RSP_DONE;
    return 0;
  }
#ifdef _DEBUG
  cout << "| rsp.valstack[0].n: " << rsp.valstack[0].n << endl;
  cout << "| rsp.valstack[0].exp: " << rsp.valstack[0].exp << endl;
#endif 
  return 1;
}


const val::Value& client::Client::getVal() const {
  return rsp.valstack[0].val;
}
