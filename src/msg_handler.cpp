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


#include <cmath>
#include <thread>
#include <poll.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include "parser_ctx.hpp"
#include "globals.hpp"
#include "anf.hpp"
#include "interp.hpp"
#include "encode.hpp"
#include "ast.hpp"
#include "interp_error.hpp"
#include "logging.hpp"
#include "msg_handler.hpp"


// #define _DEBUG


using namespace zcore;

/// For use in the fds array.
enum { SIG, DATA, KEYB };


extern zlog::Logger lg;


static void sigint_handler(int signum)
{
  // std::cout << "signum: " << signum << std::endl;
  zcore::InterpCtx::sigint = 1;
  zcore::MsgHandler::waitingOnResp = 0;
}


zcore::MsgHandler::MsgHandler(net::NetHandler& com_p, 
                              interp::shpfrm& global_p, 
                              int fd_read_data_p,
                              int fd_read_sig_p,
                              int fd_input_p, 
                              bool once_p,
                              const std::string& initialCode) : 
  com(com_p), global(global_p), fd_read_data(fd_read_data_p), 
  fd_read_sig(fd_read_sig_p), fd_input(fd_input_p), once(once_p), 
  localContext(make_unique<InterpCtxLocal>(*this, global))
{ 
  resetMsgStats();

  // set up the signal handler:
  struct sigaction action;
  action.sa_handler = sigint_handler;
  sigfillset(&action.sa_mask);
  //sigemptyset(&action.sa_mask);
  //sigaddset(&action.sa_mask, SIGINT);
  action.sa_flags = 0;
  sigaction(SIGINT, &action, NULL);

  pthread_sigmask(SIG_UNBLOCK, &action.sa_mask, NULL);

  epollfd = epoll_create1(0);
  if (epollfd == -1) {
    throw std::system_error(std::error_code(errno, std::system_category()), "epoll_create1");
  }
    
  // now setup on both keyboard and port:
  epoll_event ev;
  memset(&ev, 0, sizeof(epoll_event));

  ev.events = EPOLLIN;
  ev.data.fd = fd_read_sig;  // tells us signalling available for reading
  if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd_read_sig, &ev) == -1) {
    lg.log(zlog::SV_ERROR, "about1");
    throw std::system_error(std::error_code(errno, std::system_category()), "epoll_ctl");
  }

  ev.events = EPOLLIN;
  ev.data.fd = fd_read_data; // tells us data buffers available for reading
  if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd_read_data, &ev) == -1) {
    lg.log(zlog::SV_ERROR, "about1");
    throw std::system_error(std::error_code(errno, std::system_category()), "epoll_ctl");
  }
  if (fd_input >= 0) {
    enableKeyboardPoll();
  }

  if (!initialCode.empty()) {
    ParserCtx pctx;
#ifdef _DEBUG
    pctx.trace_scanning = true;
    pctx.trace_parsing  = true;
#endif
    
    auto res = pctx.parse(std::make_shared<std::string>(initialCode));
    if (res == 0) {
      anf::convertToANF(pctx.prog.get());
      suspendKeyboardPoll();
      waitingOnResp = localContext->processReqData(0, std::move(pctx.prog));
      if (!waitingOnResp) {
        enableKeyboardPoll();
      }
    }
    else {
      std::cerr << pctx.errorString << std::endl;
      try {
        std::cerr << interp::errorLines(pctx.errorLoc, 1) << std::endl;
      }
      catch (...) {
        /// \todo it's OK; at some point we can try to see if we can
        //        limit the cases where we can't get location info...
      }
      // 'pctx.prog' will be deleted when 'pctx' goes out of scope 
    }
  }
  else {
    // think about how to not have the prompt printout both in this
    // class and in InterpCtx... LLL
    if (isatty(fd_input)) {
      auto prompt = get<string>(cfg::cfgmap.get("prompt"));
      cout << prompt << flush;
    }
  }
}


int MsgHandler::run() {
#ifdef _DEBUG
  cout << "InterpRun::run()" << endl;
#endif

  for (;;) {
    int nfds = epoll_wait(epollfd, events, Global::EPOLL_MAX_EVENTS, EPOLL_TIMEOUT);
    if (nfds == 0) {
      // timeout, do interpretation context garbage collection:
      std::chrono::system_clock::time_point::duration inreqttl =
        std::chrono::seconds(get<int64_t>(cfg::cfgmap.get("in.req.ttl.secs")));
      std::chrono::system_clock::time_point::duration inrspttl =
        std::chrono::seconds(get<int64_t>(cfg::cfgmap.get("in.rsp.ttl.secs")));
      for (auto& e: reqContexts) {
        e.second->gc_requests(inreqttl);
        e.second->gc_states(inrspttl);
      }
      if (localContext) {
        localContext->gc_requests(inreqttl);
        localContext->gc_states(inrspttl);
      }
    } 
    else if (nfds < 0) {
      if (errno != EINTR) {
        throw std::system_error(std::error_code(errno, std::system_category()), "interp_run epoll_wait");
      }      
      if (zcore::InterpCtx::sigint == 1) { // guard against spurious EINTR
	if (isatty(fd_input)) {
	  cout << "> " << flush;
	}
	// std::cout << "calling reset()" << std::endl;
	localContext->reset();
	waitingOnResp = 0;
	enableKeyboardPoll();
      }
    } 

    // we do a first loop to prioritize signalling; if we don't we end
    // up with potential race conditions in the case of a connection
    // to self immediately followed by a request (e.g. con1 ? { con1
    // <- connection("127.0.0.1", 15001); con1 ? 2 }.
    for (int i = 0; i < nfds; ++i) {
      // signalling: ----------------------------------------------
      if (events[i].data.fd == fd_read_sig) {
        size_t val;
        ssize_t n = read(fd_read_sig, &val, sizeof(val));
        if (n < 0) {
          // ignore, log? LLL
        }

        Global::conn_id_t id;
        net::SignallingMgt::Status st;
        while (val-- > 0 && com.get_sig(id, st)) {
          switch (st) {
          case net::SignallingMgt::UP: {
#ifdef _DEBUG
            cout << "comm::UP @ " << id << endl;
#endif
            reqContexts.emplace(id, make_unique<InterpCtxRemote>(*this, global));
            break;
          }
          case net::SignallingMgt::DOWN: {
#ifdef _DEBUG
            cout << "comm::DOWN @ " << id << endl;
#endif
            rspContexts.erase(id); // because we prioritize signalling so RSP can come after DOWN
            reqContexts.erase(id);
            break;
          }
          default:
            throw std::range_error("unknown comm event " + std::to_string(val));
          }
        }
      }
    } // end for

    for (int i = 0; i < nfds; ++i) {
      if (events[i].data.fd == fd_read_sig) continue;

      // data: ----------------------------------------------
      if (events[i].data.fd == fd_read_data) {
        //#define BROKEN_VERSION_FOR_TESTING
#ifdef BROKEN_VERSION_FOR_TESTING
        // this just sinks all incoming messages; useful to build a
        // broken version to test error paths.
        size_t dummy;
        while (read(fd_read_data, &dummy, sizeof(dummy)) > 0) ;
        continue;
#endif
        size_t val;
        ssize_t n = read(fd_read_data, &val, sizeof(val));
        if (n < 0) {
          throw std::system_error(std::error_code(errno, std::system_category()), "read(fd_read_data)");
        }

        // get message signalled by the event:
        Global::conn_id_t id;
        net::Buf combuf;
        while (val-- > 0 && com.get_data(id, combuf)) { // second probably not necessary LLL
          const char* buf = combuf.data.get();
        
          auto len = combuf.len;
  
#ifdef _DEBUG
          cout << "val: " << hex << val << dec << endl;
          cout << "comm::DATA @ " << id << endl;
          cout << "received " << len << " bytes" << endl;
          cout << "id " << id << endl;
          cout << "buf " << endl;
          //          cout << printBuf(buf, len) << endl;
#endif

          // get msg type and reqnum to dispatch accordingly:
          Global::MsgType mt;
          memcpy(&mt, buf, sizeof(mt));
          mt = static_cast<Global::MsgType>(ntoh64(static_cast<uint64_t>(mt)));
          
          // APPEND or APPEND_VECTOR ----------------------------
          if (mt == Global::MsgType::APPEND || mt == Global::MsgType::APPEND_VECTOR) {
            auto ic = reqContexts.find(id);
            if (ic == reqContexts.end()) {
              lg.log(zlog::SV_DEBUG, "can't find interp context in reqContexts for id=%d", id);
              break;
            }
            auto& evalCtx = *ic->second;
            if (mt == Global::MsgType::APPEND) {
              stats.bytesAppend += len;
              ++stats.nbAppend;
              try {
                if (evalCtx.readAppendData(buf+8, len-8) != 0) { // shouldn't throw
                  ++stats.nbAppendFail;
                }
              }
              catch (std::exception& e) {
                lg.log(zlog::SV_DEBUG, "unexpected exception: %s", e.what());
              }    
            }
            else {
              stats.bytesAppendVector += len;
              ++stats.nbAppendVector;
              try {
                if (evalCtx.readAppendVectorData(buf+8, len-8) != 0) {
                  ++stats.nbAppendVectorFail;                  
                }
              }
              catch (std::exception& e) {
                lg.log(zlog::SV_DEBUG, "unexpected exception: %s", e.what());
              }    
            }
          }
          // REQ or RSP ----------------------------
          else {
            Global::reqid_t reqid;
            memcpy(&reqid, buf+8, sizeof(reqid));
            reqid = ntoh64(reqid);

            Global::reqid_t sourceid;
            memcpy(&sourceid, buf+16, sizeof(sourceid));
            sourceid = ntoh64(sourceid);
#ifdef _DEBUG
            cout << "reqid:" << reqid << ", sourceid:" << sourceid << endl;
#endif	 
            if (mt == Global::MsgType::REQ) {
              auto ic = reqContexts.find(id);
              if (ic == reqContexts.end()) {
                lg.log(zlog::SV_DEBUG, "can't find interp context in reqContexts for id=%d", id);
                break;
              }
              auto& evalCtx = *ic->second;
              stats.bytesInREQ += len;
              ++stats.nbInREQ;
              try {
                evalCtx.readReqData(id, reqid, sourceid, buf+24, len-24);
              }
              catch (std::exception& e) {
                lg.log(zlog::SV_DEBUG, "unexpected exception: %s", e.what());
              }          
            } 
            else if (mt == Global::MsgType::RSP) {
              auto ic = rspContexts.find(id);
              if (ic == rspContexts.end()) {
                lg.log(zlog::SV_DEBUG, "can't find interp context in rspContexts for id=%d", id);
                break;
              }
              stats.bytesInRSP += len;
              ++stats.nbInRSP;
              auto& evalCtx = ic->second;
              try {
                auto waiting = evalCtx.readRspData(reqid, sourceid, buf+24, len-24);
                if (!waiting && once) {
                  return 0; // return if 'once' flag and local ctx is not waiting
                }
              }
              catch (std::exception& e) {
                lg.log(zlog::SV_DEBUG, "unexpected exception: %s", e.what());
                if (once) return -1;
              }
            }  
          }
        }
      }
      // keyboard: ----------------------------------------------
      else if (events[i].data.fd == fd_input) {
#ifdef _DEBUG
        cout << "interpret on stdio context" << endl;
#endif
        waitingOnResp = 0;
        const size_t BUF_SZ = 100000;
        char* buf = static_cast<char*>(malloc(BUF_SZ));
        if (buf == nullptr) {
          // out of memory
          continue;
        }

        size_t offset = 0;
        bool done = false;
        while (!done) {
          auto n = read(fd_input, buf + offset, BUF_SZ-1);
          if (n <= 0) {
            // This is really interesting: we receive some keyboard
            // input events when the process is backgrounded (and they
            // are not necessarily on newline, they seem to come a short
            // period after some input has been done). But the read
            // fails with errno==5 (I/O error).
            free(buf); buf = nullptr;
            break;
          }
          offset += n;
          if (static_cast<size_t>(n) < BUF_SZ-1) { // we know here n >= 0
            done = true;
          }
          else {
            buf = static_cast<char*>(realloc(buf, offset + BUF_SZ-1));
            if (buf == nullptr) break;
          }
        }
        if (buf == nullptr) {
          // out of memory
          continue;
        }

        if (!waitingOnResp && offset == 1 && buf[0] == '\n') {
          free(buf);
          if (isatty(fd_input)) {
            cout << "> " << flush;
          }
          continue;
        }

        buf[offset] = 0;
        ParserCtx pctx;
#ifdef _DEBUG
        // pctx.trace_scanning = true;
        // pctx.trace_parsing  = true;
#endif
        auto bufptr = std::shared_ptr<void>(buf, free); // give custody to bufptr
        auto res = pctx.parse(std::make_shared<const std::string>(buf)); // NB: copy of buf
        if (res == 0) {
          anf::convertToANF(pctx.prog.get());
#ifdef _DEBUG
          std::cout << "anf: " << std::endl;
          std::cout << to_string(*pctx.prog.get()) << std::endl;
#endif

          suspendKeyboardPoll();
          waitingOnResp = localContext->processReqData(0, std::move(pctx.prog));
          if (!waitingOnResp) {
            if (once) { 
              return 0;
            }
            enableKeyboardPoll();
          }
        }
        else {
          std::cerr << pctx.errorString << std::endl;
          try {
            std::cerr << interp::errorLines(pctx.errorLoc, 1) << std::endl;
          }
          catch (...) {
            /// \todo it's OK; at some point we can try to see if we can
            ///        limit the cases where we can't get location info...
          }
          if (isatty(fd_input)) {
            cout << "> " << flush;
          }
          // 'pctx.prog' will be deleted when 'pctx' goes out of scope 
        }
      }
      // it's a timer: ----------------------------------------------
      else {
        uint64_t count;
        ssize_t res = read(events[i].data.fd, &count, sizeof(count));
        if (res < 0) {
          throw std::system_error(std::error_code(errno, std::system_category()), "read");
        }
        auto ic = timerContexts.find(events[i].data.fd);
        if (ic == timerContexts.end()) {
          lg.log(zlog::SV_DEBUG, "can't find timer context in timerContexts for id=%d", 
                 events[i].data.fd);
          break;
        }
        auto& evalCtx = *ic->second;
        try {
          evalCtx.interpretTimer(events[i].data.fd);
        }
        catch (std::exception& e) {
          lg.log(zlog::SV_DEBUG, "unexpected exception: %s", e.what());
        }          
      }
    } // end for loop
  } // end for loop
  return 0;
}
  

Global::conn_id_t MsgHandler::connect(const string& ip, int port, InterpCtx& ic) {
  auto conn_id = com.connect(ip, port);
#ifdef _DEBUG
  std::cout << "InterpRun::connect: conn_id is: " << conn_id << std::endl; 
#endif
  if (conn_id > 0) {
    rspContexts.emplace(conn_id, ic);
  }
  return conn_id;
}


size_t MsgHandler::sendReq(InterpCtx* reqCtx,
                          Global::reqid_t reqid, 
                          Global::reqid_t sourceid, 
                          val::VConn con, 
                          const E* e, 
                          const val::SpVList& boundvars) {
#ifdef _DEBUG
  cout << "sendReq" << endl;
  cout << "| reqid: " << reqid << endl;
  cout << "| sourceid: " << sourceid << endl;
#endif

  // check boundvar elements belong to transmissible types (recursively) LLL
  if (val::any_of(boundvars, [](const val::Value& v) { 
        return !zcore::isTransmissible(v); })) {
    throw std::range_error("untransmissible type");
  }

  auto peerid = con.id;
  if (peerid == 0) {
    throw std::range_error("send request failed: no connection to peer");
  }
  try {
    zcore::Encode ec(com, peerid, reqid, sourceid, Global::MsgType::REQ);
    ec << e;
    ec << val::Value(boundvars);
#ifdef _DEBUG
    cout << "| REQ buffer:" << endl;
    cout << printBuf(ec.buf, ec.offset) << endl;
#endif
    ec.flush();
    
    stats.bytesOutREQ += ec.bytes;
    return ec.offset;
  } catch (const std::exception& e) {
    lg.log(zlog::SV_DEBUG, "unexpected exception: %s", e.what());
    // com.disconnect(peerid);
    throw;
  }
}


size_t MsgHandler::sendRsp(Global::conn_id_t peerid, 
                          Global::reqid_t reqid, 
                          Global::reqid_t sourceid, 
                          const val::Value& rsp) {
#ifdef _DEBUG
  cout << "sendRsp" << endl;
  cout << "| peerid:" << peerid << endl;
  cout << "| reqid:" << reqid << endl;
  cout << "| sourceid:" << reqid << endl;
  cout << "| rsp:" << val::to_string(rsp) << endl;
  if (rsp.which() == val::vt_double) {
    auto& dd = get<val::SpVAD>(rsp);
    cout << "| rsp use_count: " << dd.use_count() << endl;
  }
  // We could use the 'use_count' to thread the sendRsp. If the
  // response has a use count of 1, then we know it's a temporary
  // value (only '.Last.value' refers to it) and we could take
  // ownership of the value and thread the sending, like that the main
  // thread would not have to wait on the sending itself.
#endif

  try {
    zcore::Encode ec(com, peerid, reqid, sourceid, Global::MsgType::RSP);
    ec << rsp;
    ec.flush_end();

    stats.bytesOutRSP += ec.bytes;
#ifdef _DEBUG
    cout << "| RSP buffer:" << endl;
    //  cout << printBuf(ec.buf, ec.offset);
#endif

    return ec.bytes;
  } catch (std::exception& e) {
    lg.log(zlog::SV_DEBUG, "unexpected exception: %s", e.what());
    com.disconnect(peerid);
    throw;
  }
}


void MsgHandler::suspendKeyboardPoll() {
  if (fd_input >= 0) {
    if (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd_input, nullptr) == -1 && errno != ENOENT) {
    lg.log(zlog::SV_ERROR, "about3");
      throw std::system_error(std::error_code(errno, std::system_category()), "epoll_ctl");
    }  
  }
}


void MsgHandler::enableKeyboardPoll() {
  if (fd_input >= 0) {
    epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));
    ev.events = EPOLLIN;
    ev.data.fd = fd_input;   // keyboard
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd_input, &ev) == -1) {
      if (errno == EPERM) {
        fd_input = -1;
      }
      else if (errno != EEXIST) {
        lg.log(zlog::SV_ERROR, "about4");
        throw std::system_error(std::error_code(errno, std::system_category()), "epoll_ctl");
      }
    }
  }
}


void MsgHandler::addTimer(int fd, std::unique_ptr<InterpCtxTimer> ctx) {
  // immediately evaluate the new timer so the 'once' argument can get evaluated:
  try {
    ctx->interpretTimer(fd);
  }
  catch (std::exception& e) {
    lg.log(zlog::SV_DEBUG, "unexpected exception: %s", e.what());
  }          

  timerContexts.emplace(fd, std::move(ctx));
  epoll_event ev;
  ev.events = EPOLLIN;
  ev.data.fd = fd;
  if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
    throw std::system_error(std::error_code(errno, std::system_category()), "epoll_ctl");
  }
}


void MsgHandler::removeTimer(int fd) {
  timerContexts.erase(fd);
  if (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, nullptr) == -1) {
    throw std::system_error(std::error_code(errno, std::system_category()), "epoll_ctl");
  }
}


const zcore::NetStats& MsgHandler::getNetStats() const { 
  return com.getNetStats(); 
}


void MsgHandler::resetNetStats() { 
  com.resetNetStats(); 
}


zcore::NetInfo MsgHandler::getNetInfo() const { 
  return com.getNetInfo(); 
}


zcore::MsgInfo MsgHandler::getMsgInfo() const {
  zcore::MsgInfo info;
  for (const auto& e : reqContexts) {
    info.reqContexts.insert(e.first);
  }
  for (const auto& e : rspContexts) {
    info.rspContexts.insert(e.first);
  }
  return info;
}


volatile sig_atomic_t zcore::MsgHandler::waitingOnResp = 0; 
