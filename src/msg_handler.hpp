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


#ifndef MSG_HANDLER_HPP
#define MSG_HANDLER_HPP

#include <string>
#include <unordered_map>
#include <memory>
#include <cstdint>
#include <poll.h>
#include "env.hpp"
#include "interp_ctx.hpp"
#include "stats.hpp"
#include "config.hpp"
#include "net_handler.hpp"


namespace zcore {

  /// Provide a minimal InterpRun that has few dependencies. This
  /// allows us to do unit testing of the interpreter without having
  /// to include the communication layer, etc.
  struct MsgHandlerBase {
    virtual Global::conn_id_t connect(const string& ip, int port, InterpCtx& ic) {
      throw std::logic_error("sendReq not implemented");
    }
    virtual size_t sendReq(InterpCtx* reqCtx, Global::reqid_t reqid, Global::reqid_t sourceid,
                           val::VConn con, const E* e, const val::SpVList& boundvars) {
      throw std::logic_error("sendReq not implemented");
    }
    virtual size_t sendRsp(Global::conn_id_t id, Global::reqid_t reqid, Global::reqid_t sourceid, 
                           const val::Value& rsp) {
      throw std::logic_error("sendReq not implemented");
    }
    inline virtual void suspendKeyboardPoll() { 
      throw std::logic_error("suspendKeyboardPoll not implemented"); 
    }
    inline virtual void enableKeyboardPoll() { 
      throw std::logic_error("enableKeyboardPoll not implemented"); 
    }
    inline virtual void addTimer(int fd, std::unique_ptr<InterpCtxTimer> ctx) { 
      throw std::logic_error("enableKeyboardPoll not implemented"); 
    }
    inline virtual void removeTimer(int fd) { 
      throw std::logic_error("enableKeyboardPoll not implemented"); 
    }
    virtual const NetStats& getNetStats() const {  
      throw std::logic_error("getNetStats not implemented"); 
    }
    virtual void resetNetStats() { 
      throw std::logic_error("resetNetStats not implemented"); 
    }
    virtual zcore::NetInfo getNetInfo() const { 
      throw std::logic_error("getNetInfo not implemented"); 
    }
    virtual zcore::MsgInfo getMsgInfo() const { 
      throw std::logic_error("getMsgInfo not implemented"); 
    }

    const MsgStats& getMsgStats() { return stats; }
    void resetMsgStats() { stats.reset(); }

    virtual ~MsgHandlerBase() { }

  protected:
    MsgStats stats;
  };

  // just the higher level protocol:
  struct MsgHandler : public MsgHandlerBase {
    MsgHandler(net::NetHandler& com_p,
              interp::shpfrm& global_p, 
              int fd_read_data_p,
              int fd_read_sig_p,
              int fd_input_p=STDIN_FILENO, 
              bool once_p=false,
              const std::string& initialCode="");

    int run();

    Global::conn_id_t connect(const string& ip, int port, InterpCtx& ic);
    size_t sendReq(InterpCtx* reqCtx, 
                   Global::reqid_t reqid, 
                   Global::reqid_t sourceid, 
                   val::VConn con, 
                   const E* e, 
                   const val::SpVList& boundvars);
    size_t sendRsp(Global::conn_id_t id, 
                   Global::reqid_t reqid, 
                   Global::reqid_t sourceid, 
                   const val::Value& rsp);

    static volatile sig_atomic_t waitingOnResp; 

    void suspendKeyboardPoll();
    void enableKeyboardPoll();

    void addTimer(int fd, std::unique_ptr<InterpCtxTimer> ctx);
    void removeTimer(int fd);

    inline const InterpCtxLocal& getLocalCtx() const { return *localContext; }

    const NetStats& getNetStats() const;
    void resetNetStats();
    zcore::NetInfo getNetInfo() const;
    zcore::MsgInfo getMsgInfo() const;

  private:
    net::NetHandler& com;
    
    interp::shpfrm global;

    int fd_read_data;
    int fd_read_sig;
    int fd_input;
    bool once;
    unique_ptr<InterpCtxLocal> localContext;

    // each one of these needs to be garbage collected:
    unordered_map<Global::conn_id_t, unique_ptr<InterpCtx>> reqContexts; // incoming requests
    unordered_map<Global::conn_id_t, InterpCtx&> rspContexts;            // incoming responses
    unordered_map<int, unique_ptr<InterpCtxTimer>> timerContexts;        // timers
    
    int epollfd;
    epoll_event events[Global::EPOLL_MAX_EVENTS];
    static const int EPOLL_TIMEOUT = 1000;
  };

}

#endif
