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


#ifndef INTERP_CTX_HPP
#define INTERP_CTX_HPP

#include <unordered_map>
#include <cstdint>
#include <vector>
#include <stack>
#include <chrono>
#include <signal.h>
#include "env.hpp"
#include "interp.hpp"
#include "ast.hpp"
#include "valuevar.hpp"
#include "stats.hpp"
#include "info.hpp"
#include "encode.hpp"


namespace zcore {

  struct MsgHandlerBase;

  enum IncomingReqState {
    CTX_REQ_IDLE,
    CTX_REQ_CODE,
    CTX_REQ_VALUE,
    CTX_REQ_DONE,
  };

  enum IncomingRspState {
    CTX_RSP_IDLE,
    CTX_RSP,
    CTX_RSP_DONE
  };

  /// This structure represents the state of an incoming request. The
  /// incoming request comes as a buffer in which an AST is encoded
  /// (already in normalized form). It is decoded and stored in
  /// 'e'. 'valstack' is used to decode and store the list of decoded
  /// values that form the bound variables (this list might be empty).
  struct ReqState {
    ReqState(Global::reqid_t reqid_p, Global::reqid_t sourceid_p) : 
      state(CTX_REQ_IDLE), reqid(reqid_p), sourceid(sourceid_p), 
      codeLen(0), valstack_idx(0), timestamp(std::chrono::system_clock::now()) { }

    IncomingReqState state;
    Global::reqid_t reqid, sourceid;
    std::unique_ptr<const E> e;
    size_t codeLen;
    vector<char> codeBuf;
    size_t valstack_idx;
    vector<ValState> valstack;
    std::chrono::system_clock::time_point timestamp; ///< so we can free request states that
                                                     ///  take too long to complete
    void readData(const char* buf, size_t len);
  };

  /// This structure represents the state of an incoming response (to
  /// one of our outgoing requests). An incoming response is always
  /// one 'Value', which is decoded and stored in 'valstack'.
  struct RspState {
    RspState(const string& ip_p, 
             int port_p, 
             Global::conn_id_t peerid_p, 
             const string& s,
             val::SpFuture& future_p) : 
      ip(ip_p), port(port_p), peerid(peerid_p), name(s), state(CTX_RSP_IDLE), 
      valstack_idx(0), timestamp(std::chrono::system_clock::now()), future(future_p) { }

    // the following 3 are only to be able to deliver a meaninful
    // error message:
    const string ip;
    const int port;
    const Global::conn_id_t peerid;

    const string name;
    IncomingRspState state;
    size_t valstack_idx;
    vector<ValState> valstack;
    std::chrono::system_clock::time_point timestamp; ///< so we can free response states that
                                                     ///  take too long to complete 
    val::SpFuture future;
    void readData(const char* buf, size_t len);
  };

  struct InterpState {
    Global::reqid_t reqid;      ///< who we are
    Global::reqid_t sourceid;   ///< on behalf of who we are interpreting
    Global::conn_id_t peerid;   ///< where to send response back

    /// The following 3 represent the state of the interpreter:
    std::unique_ptr<const E> e; ///< the expression under evaluation;
                                ///  this is the point of ownership so
                                ///  we have proper deletion of the
                                ///  parser expressions
    shared_ptr<interp::Kont> k;
    vector<interp::shpfrm> fstack;

    // RSP state, we can query multiple time per expression (before
    // blocking on the evaluation of a future), so we need a map:
    std::map<Global::reqid_t, RspState> responses;

    void popAndClearUntil(const shared_ptr<interp::BaseFrame> r);
  };

  // there is one interp context per peer; 'r' is the evaluation
  // environment for any incoming request from the peer, and so it is
  // peer specific and is purged when a peer goes down; its ancestor
  // is global, so a <<- global assignment will write to the top level
  // (and global) server environment.
  struct InterpCtx {
    
    InterpCtx(MsgHandlerBase& ir_p, interp::shpfrm& global) : 
      r(make_shared<interp::Frame>("working", global, global)), ir(ir_p), s(nullptr) { }
    
    std::shared_ptr<interp::BaseFrame> r;
    MsgHandlerBase& ir;
    
    // May be multiple interleaved requests coming in:
    std::map<Global::reqid_t, ReqState> requests;
    

    // replace this by the reqid LLL, it will be less dangerous! LLL
    Global::reqid_t currentState; // then we can call 's' the retrieved InterpState LLL
    InterpState* s;

    size_t readRspData(Global::reqid_t reqid, 
                       Global::reqid_t sourceid, 
                       const char* buf, 
                       size_t len);
    ssize_t readReqData(Global::conn_id_t peerid, 
                        Global::reqid_t reqid, 
                        Global::reqid_t sourceid, 
                        const char* buf, 
                        size_t len);
    /// Read a buffer containing an array to append.
    ssize_t readAppendData(const char* buf, size_t len);
    /// Read a buffer containing a vector to append.
    ssize_t readAppendVectorData(const char* buf, size_t len);

    /// Establish connection to given ip/port.
    Global::conn_id_t connect(const string& ip_p, int port_p); 

    /// Add a timer.
    void addTimer(val::SpTimer& tmr);
    virtual void removeTimer(val::SpTimer& tmr);

    void sendReq(const string& varName, 
                 val::VConn con, 
                 E* e, 
                 const val::SpVList& vl, 
                 val::SpFuture& future);
    void setStop();

    virtual int interpret(InterpState& state) = 0;

    /// When deleting a state, sends or displays an error message.
    virtual void sendGcStateMessage(const string& ip, 
                                    int port, 
                                    Global::conn_id_t peerid, 
                                    const InterpState& state) = 0;

    /// 
    void gc_requests(std::chrono::system_clock::time_point::duration ttl);
    void gc_states(std::chrono::system_clock::time_point::duration ttl);

    /// Terminate all pending requests and reset the interpreter state.
    void reset();

    /// Stats:
    const MsgStats& getMsgStats() const;
    void resetMsgStats();
    const NetStats& getNetStats() const;
    void resetNetStats();
    const CtxStats& getCtxStats() const;
    void resetCtxStats();

    /// Info:
    zcore::NetInfo getNetInfo() const;
    zcore::MsgInfo getMsgInfo() const;
    zcore::CtxInfo getCtxInfo() const;

    static volatile sig_atomic_t sigint;

    virtual ~InterpCtx();

  protected:
    CtxStats stats;
    std::map<Global::reqid_t, InterpState> states;
  };

  
  struct InterpCtxLocal : InterpCtx {
    InterpCtxLocal(MsgHandlerBase& ir_p, interp::shpfrm& global) : 
      InterpCtx(ir_p, global) { }

    virtual int interpret(InterpState& state);
    virtual void sendGcStateMessage(const string& ip, 
                                    int port, 
                                    Global::conn_id_t peerid, 
                                    const InterpState& state);

    int processReqData(Global::conn_id_t peerid, std::unique_ptr<const E> e);

  };


  struct InterpCtxRemote : InterpCtx {
    InterpCtxRemote(MsgHandlerBase& ir_p, interp::shpfrm& global) : 
      InterpCtx(ir_p, global) { }

    virtual int interpret(InterpState& state);
    virtual void sendGcStateMessage(const string& ip, 
                                    int port, 
                                    Global::conn_id_t peerid, 
                                    const InterpState& state);
  };

  struct InterpCtxTimer : InterpCtx {
    InterpCtxTimer(val::SpTimer& timer_p, MsgHandlerBase& ir_p, interp::shpfrm& global) : 
      InterpCtx(ir_p, global), timer_wptr(timer_p) { timer_p->start(); }

    virtual void removeTimer(val::SpTimer& tmr);
    virtual int interpret(InterpState& state);
    virtual void sendGcStateMessage(const string& ip, 
                                    int port, 
                                    Global::conn_id_t peerid, 
                                    const InterpState& state);
    int interpretTimer(int fd);
    std::weak_ptr<val::VTimer> timer_wptr;
  };

}

#endif 

// abbreviations: ir = InterpRun
//                ic = InterpCtx
//
//     UP (id)            ir                         ic
// ---------------------> | create new ic w/ id      |   
//     msg part 1/3 (id)  |                          |
// ---------------------> | -----------------------> |
//     msg part 2/3 (id)  |                          |
// ---------------------> | -----------------------> |
//     msg part 3/3 (id)  |                          |
// ---------------------> | -----------------------> | 

