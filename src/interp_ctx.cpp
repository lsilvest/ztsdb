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


#include <unistd.h>
#include "interp_ctx.hpp"
#include "interp_error.hpp"
#include "logging.hpp"
#include "msg_handler.hpp"


extern zlog::Logger lg;


// #define DEBUG

// note that at this point buf must not contain the magic number, nor
// the buffer length, nor the msg type, nor the reqnum; these have
// already been processed and obviously the correct buf length is
// given by len.
void zcore::ReqState::readData(const char* buf, size_t len) {
#ifdef DEBUG
  cout << "ReqState::readData():" << endl;
  cout << "| state:  " << state << endl;
  cout << "| buf:" << endl;
  //  cout << printBuf(buf, len) << endl;
#endif

  switch (state) {
  case CTX_REQ_IDLE: {
    // To simplify things, we want to read code in one go, so as not
    // to have to deal with very complex states of partially
    // constructed ASTs. So we construct a code buffer until the
    // buffer is complete and then we call 'zcore::readCode'
    // to construct the AST:
    codeLen = ntoh64(*(reinterpret_cast<const uint64_t*>(buf)));
    buf += 8;
    len -= 8;
    state = CTX_REQ_CODE;
    // intentional fall-through
  }
  case CTX_REQ_CODE: {
    // append the buf to the code buffer:
    size_t lenToCopy = codeBuf.size() + len >= codeLen ? codeLen - codeBuf.size() : len;
    std::copy(buf, buf+lenToCopy, std::back_inserter<std::vector<char>>(codeBuf));
    if (codeBuf.size() == codeLen) {
      e = unique_ptr<const E>(zcore::readCode(&codeBuf[0], codeBuf.size()));
      codeBuf.clear();
      codeLen = 0;
      buf += lenToCopy;
      len -= lenToCopy;
      state = CTX_REQ_VALUE;
    } else {
      break;
    }
  }
  case CTX_REQ_VALUE: {
    // when reading values, on the contrary to code reading, we
    // neither get a byte length nor do we reassemble
    // packets... computing a length is too difficult and we want to
    // reassemble the values as we go, for performance reasons. A
    // value in a request should be a list of VNamed and the size of
    // the list will determine our end condition:
    size_t offset = 0;
    readValue(buf, len, offset, valstack, valstack_idx); // will throw. 
    // assert we're reading a list of values, if not we have to abort in
    // some way LLL
    if (valstack[0].n == valstack[0].exp) { // current len == expected len
      state = CTX_REQ_DONE;
    }
    break; 
  }  

  default:
    throw std::logic_error(__FUNCTION__ + "unknown CTX state"s);
  }
}


// note that at this point buf must not contain the magic number nor
// the buffer length; these have already been processed and obviously
// the correct buf length is given by len.
void zcore::RspState::readData(const char* buf, size_t len) {
#ifdef DEBUG
  cout << "RspState::readData():" << endl;
  cout << "| buf len: " << len << endl;
  cout << "| buf:" << endl;
  //  cout << printBuf(buf, len) << endl;
#endif

  if (state == CTX_RSP_IDLE) {
    state = CTX_RSP;
  }

  size_t offset = 0;
  readValue(buf, len, offset, valstack, valstack_idx); // will throw

  // note that for 'std_string' encoding, 'n' is larger than 'exp'
  // because of the padding, so we really do need to use 'n >= exp'
  // and not 'n == exp':
  if (valstack[0].n >= valstack[0].exp) { // current len >= expected len
#ifdef DEBUG
    cout << "| state set tp CTX_RSP_DONE" << endl;
#endif

    state = CTX_RSP_DONE;
  }
#ifdef DEBUG
  cout << "| valstack[0].n: " << valstack[0].n << endl;
  cout << "| valstack[0].exp: " << valstack[0].exp << endl;
#endif 
}

// this ends up returning 0 if not waiting and 1 if waiting (not finished)...
// need to make this explicit LLL
size_t zcore::InterpCtx::readRspData(Global::reqid_t reqid,
                                     Global::reqid_t sourceid,
                                     const char* buf,
                                     size_t len) {
#ifdef DEBUG
  cout << "InterpCtx::readRspData():" << endl;
  cout << "| reqid:   " << reqid << endl;
  cout << "| sourceid:" << sourceid << endl;
#endif
  stats.bytesInRSP += len;

  // find the ReqState
  auto state_pair = states.find(sourceid);
  if (state_pair == states.end()) {
    /// that's OK, we may have been interrupted, the query could have
    /// been garbage collected (if it evaluated to an error), we might
    /// have restarted, etc.
#ifdef DEBUG
    cout << __FUNCTION__ << ": interp state not found for sourceid=" << sourceid << endl;
#endif
    // should be a stat? LLL:
    lg.log(zlog::SV_DEBUG, "could not find interp state for sourceid=%d", sourceid);
    return 0;
  }
  auto& state = state_pair->second;
  // find the 'RspState':
  auto rsp = state.responses.find(reqid);
  if (rsp == state.responses.end()) {
    // bad enough to warrant deleting the InterpState:
#ifdef DEBUG
    cout << __FUNCTION__ << ": state not found for requid " << reqid << endl;
#endif
    states.erase(state_pair);
    lg.log(zlog::SV_DEBUG, "could not find response state for requid=%d", reqid);
    return 0;
  }
  rsp->second.readData(buf, len);
  if (rsp->second.state == CTX_RSP_DONE) {
    // now get this value to the interpreter and have it continue its execution: 
    // error handling LLL
#ifdef DEBUG
    cout << "| rsp->second.name: " << rsp->second.name << endl;
#endif
    if (rsp->second.future.use_count() > 1 && rsp->second.future->getvalptr()) {
      *rsp->second.future->getvalptr() = std::move(rsp->second.valstack[0].val);
    }
    ++stats.nbInRSP;

    return interpret(state);
  }

  return 1;                     // indicates we are not finished
}



static ssize_t readHeader(const char* buf,
                          size_t len,
                          size_t& off,
                          val::Value& val,
                          std::shared_ptr<interp::BaseFrame>& r) {
  // we need to check throughout here that we are not going futher than slen!!! LLL
  // find the string name:
  auto slen = ntoh64(*(reinterpret_cast<const uint64_t*>(buf)));
  auto ns = slen >> 32;
  slen &= 0xffffffff;
  off += sizeof(uint64_t);

  // the first name must be a variable name retrievable from the global environment:
  auto sz = (buf + off++)[0];
  const string s(buf + off, buf + off + sz);
  val = r->global->find(s);
  off += sz + 1;    

  // subsequent names must be list elements:
  for (size_t i=1; i<ns; ++i) {
    if (val.which() != val::vt_list) {
      lg.log(zlog::SV_DEBUG, "invalid append: incorrect type");
      return -1;
    }
    auto& l = get<val::SpVList>(val);
    auto sz = (buf + off++)[0];
    const string s(buf + off, buf + off + sz);
    val = (*l)[s];
    off += sz + 1; 
  }
  off = slen;
  return 0;
}


ssize_t zcore::InterpCtx::readAppendData(const char* buf, size_t len) {
#ifdef DEBUG
  cout << "InterpCtx::readAppendData():" << endl;
  //  cout << printBuf(buf, len) << endl;
#endif
  try {
    size_t off = 0;
    val::Value val;
    auto res = readHeader(buf, len, off, val, r);
    if (res < 0) return res;
    
    switch(val.which()) {
    case val::vt_zts:
      get<val::SpZts>(val).get()->append(buf + off, len - off, off);   // get() to avoid the copy
      break;
    case val::vt_double:
      get<val::SpVAD>(val).get()->append(buf + off, len - off, off);   // get() to avoid the copy
      break;
    case val::vt_time:
      get<val::SpVADT>(val).get()->append(buf + off, len - off, off);  // get() to avoid the copy
      break;
    case val::vt_duration:
      get<val::SpVADUR>(val).get()->append(buf + off, len - off, off); // get() to avoid the copy
      break;
    case val::vt_interval:
      get<val::SpVAIVL>(val).get()->append(buf + off, len - off, off); // get() to avoid the copy
      break;
    case val::vt_bool:
      get<val::SpVAB>(val).get()->append(buf + off, len - off, off);   // get() to avoid the copy
      break;
      // can't do strings efficiently... 
      // we should specialize encoding functions to prevent strings being encoded
    default:
      // don't want to log, but instead increase a stat!!! LLL
      lg.log(zlog::SV_DEBUG, "invalid append: incorrect type");
      return -1;
    }
  }
  catch (std::exception& e) {
    lg.log(zlog::SV_DEBUG, "invalid append: %s", e.what());
    return -1;
  }

  ++stats.nbAppend;
  stats.bytesAppend += len;  
  return 0;
}


ssize_t zcore::InterpCtx::readAppendVectorData(const char* buf, size_t len) {
#ifdef DEBUG
  cout << "InterpCtx::readAppendVectorData():" << endl;
  //  cout << printBuf(buf, len) << endl;
#endif

  try {
    size_t off = 0;
    val::Value val;
    auto res = readHeader(buf, len, off, val, r);
    if (res < 0) return res;

    switch(val.which()) {
    case val::vt_zts:
      get<val::SpZts>(val).get()->appendVector(buf + off, len - off); // get() to avoid the copy
      break;
    case val::vt_double:
      get<val::SpVAD>(val).get()->appendVector(buf + off, len - off); // get() to avoid the copy
      break;
    case val::vt_time:
      get<val::SpVADT>(val).get()->appendVector(buf + off, len - off); // get() to avoid the copy
      break;
    case val::vt_duration:
      get<val::SpVADUR>(val).get()->appendVector(buf + off, len - off); // get() to avoid the copy
      break;
    case val::vt_interval:
      get<val::SpVAIVL>(val).get()->appendVector(buf + off, len - off); // get() to avoid the copy
      break;
    case val::vt_bool:
      get<val::SpVAB>(val).get()->appendVector(buf + off, len - off); // get() to avoid the copy
      break;
      // can't do strings efficiently... 
      // we should specialize encoding functions to prevent strings being encoded
    default:
      lg.log(zlog::SV_DEBUG, "invalid append: incorrect type");
      return -1;
    }
  }
  catch (std::exception& e) {
    lg.log(zlog::SV_DEBUG, "invalid append: %s", e.what());
    return -1;
  }
  ++stats.nbAppendVector;
  stats.bytesAppendVector += len;
  return 0;
}



void zcore::InterpState::popAndClearUntil(const interp::shpfrm r) {
  for (auto i = fstack.size(); i > 0; --i) {
#ifdef DEBUG
    cout << __FUNCTION__ << ": fstack[" << i-1 << "]:" << fstack[i-1] << " != " << r << " ? " << endl;
#endif
    if (fstack[i-1] != r) {
      fstack[i-1]->clear();
      fstack.pop_back();
    }
    else {
      return;
    }
  }
}


// this function doesn't do the decoding, but handles the request
// creation and interpreter invocation
ssize_t zcore::InterpCtx::readReqData(Global::conn_id_t peerid,
                                      Global::reqid_t reqid,
                                      Global::reqid_t sourceid,                                     
                                      const char* buf, 
                                      size_t len) 
{
  auto res = requests.emplace(make_pair(reqid, ReqState(reqid, sourceid)));
  // note that res can be perfectly be pre-existing
  auto& req = res.first->second;
 
  try {
    req.readData(buf, len);
  }
  catch (std::exception& e) {
    // return that to the peer if possible LLL
    requests.erase(res.first);
    lg.log(zlog::SV_DEBUG, "invalid request; peerid=%d, reqid=%d, sourceid=%d", 
           peerid, reqid, sourceid);
    return -1;
  }
  if (req.state == CTX_REQ_DONE) {
    // we create a shadow frame in order to locally insert the bound
    // variables; this makes sure that the expression has access to
    // these bound variables and that they will get destroyed after
    // expression evaluation (including if an exception occurs):
    interp::shpfrm sr = make_shared<interp::ShadowFrame>(r, nullptr, r->ec, nullptr);

    // get the bound variables and insert them in the shadow frame:
    auto bndvars = std::move(req.valstack[0].val);
    auto bndvarsList = get<val::SpVList>(bndvars);
    for (arr::idx_type i=0; i<bndvarsList->size(); ++i) {
      sr->add(bndvarsList->a.getnames(0)[i], val::Value(bndvarsList->a[i]));
    }

    auto halt = make_shared<interp::Kont>(interp::Kont{nullptr, nullptr, sr, 
          nullptr, interp::Kont::NORMAL});
    auto k = make_shared<interp::Kont>(interp::Kont{nullptr, req.e.get(), sr, 
          halt, interp::Kont::NORMAL});
    auto state = states.emplace(make_pair(reqid, 
                                          InterpState{reqid,
                                                      sourceid,
                                                      peerid,
                                                      std::move(req.e),
                                                      k,
                                                      std::vector<interp::shpfrm>{sr},
                                                      std::map<Global::reqid_t, RspState>()}));

    ++stats.nbInREQ;
    stats.bytesInREQ += len;

    requests.erase(res.first);
    interpret(state.first->second);
  }
  return 0;
}



static auto getNextId = GenNbFun<Global::reqid_t>(); // reqid sequence generator

Global::conn_id_t zcore::InterpCtx::connect(const string& ip, int port) {
  return ir.connect(ip, port, *this);
}


void zcore::InterpCtx::addTimer(val::SpTimer& tmr) {
  ir.addTimer(tmr->fd, make_unique<zcore::InterpCtxTimer>(tmr, ir, r->global));
}


void zcore::InterpCtx::removeTimer(val::SpTimer& tmr) {
  /// carefull, can we remove timer to self? LLL
  ir.removeTimer(tmr->fd);
}


void zcore::InterpCtx::sendReq(const string& varName, 
                               val::VConn con, 
                               E* e, 
                               const val::SpVList& vl, 
                               val::SpFuture& future) {
#ifdef DEBUG
  cout << "InterpCtx::sendReq():" << endl;
  cout << "| varName:" << varName << endl;
  cout << "| e:" << to_string(*e) << endl;
#endif  
  auto reqid = getNextId();
  s->responses.emplace(make_pair(reqid, RspState(con.ip, con.port, con.id, varName, future)));
  Global::reqid_t sourceid = s->reqid;
  auto n = ir.sendReq(this, reqid, sourceid, con, e, vl);
  ++stats.nbOutREQ;
  stats.bytesOutREQ += n;
}


void zcore::InterpCtx::reset() {
  requests.clear();
  states.clear();
  s = nullptr;
}


zcore::InterpCtx::~InterpCtx() {
  reset();
  r->clear();
}

const zcore::MsgStats& zcore::InterpCtx::getMsgStats() const { return ir.getMsgStats(); }
void zcore::InterpCtx::resetMsgStats() { ir.resetMsgStats(); }

const zcore::NetStats& zcore::InterpCtx::getNetStats() const { return ir.getNetStats(); }
void zcore::InterpCtx::resetNetStats() { ir.resetNetStats(); }

const zcore::CtxStats& zcore::InterpCtx::getCtxStats() const { return stats; }
void zcore::InterpCtx::resetCtxStats() { stats.reset(); }

zcore::NetInfo zcore::InterpCtx::getNetInfo() const { return ir.getNetInfo(); } 
zcore::MsgInfo zcore::InterpCtx::getMsgInfo() const { return ir.getMsgInfo(); } 
zcore::CtxInfo zcore::InterpCtx::getCtxInfo() const 
{
  if (s) {
    return CtxInfo{zcore::StateInfo{s->reqid, s->sourceid, s->peerid}};
  }
  else {
    return CtxInfo{zcore::StateInfo{0,0,0}};
  }
} 


void zcore::InterpCtx::gc_requests(std::chrono::system_clock::time_point::duration ttl) {
  auto now = std::chrono::system_clock::now();
  for (auto iter = requests.begin(); iter != requests.end(); ) {
    if (now - iter->second.timestamp > ttl) {
      iter = requests.erase(iter);
    }
    else {
      ++iter;
    }
  }
}


void zcore::InterpCtx::gc_states(std::chrono::system_clock::time_point::duration ttl) {
  // the condition for timing out a state is if there is a rspState
  // that is blocked (even if the other are not blocked (or probably
  // have already completed)).
  auto now = std::chrono::system_clock::now();
  for (auto siter = states.begin(); siter != states.end(); ) {
    bool expired = false;
    std::tuple<string, int, Global::conn_id_t> rspInfo;
    for (auto riter = siter->second.responses.begin(); 
         riter != siter->second.responses.end(); 
         ++riter) {
      if (now - riter->second.timestamp > ttl) {
        rspInfo = std::make_tuple(riter->second.ip, riter->second.port, riter->second.peerid);
        expired = true;
        break;
      }
    }
    if (expired) {
      if (&(siter->second) == s) {
        sendGcStateMessage(std::get<0>(rspInfo), std::get<1>(rspInfo), std::get<2>(rspInfo), *s);
#ifdef DEBUG
        std::cout << "gc: set s to nullptr" << std::endl;
#endif
        s = nullptr;
      }
      siter = states.erase(siter);
    }
    else {
      ++siter;
    }
  }
}

// derived InterpCtx -------------------------------


int zcore::InterpCtxLocal::processReqData(Global::conn_id_t peerid, 
                                          std::unique_ptr<const E> e) 
{
#ifdef DEBUG
  cout << "InterpCtxLocal::processReqData():" << endl;
  cout << "| peerid: " << peerid << endl;
  cout << "| e:" << to_string(*e) << endl;
#endif  
  auto reqid = getNextId();
  static const auto sourceid = static_cast<Global::reqid_t>(0); // by convention, always 0 for local
#ifdef DEBUG
  cout << "| reqid: " << reqid << endl;
  cout << "| sourceid: " << sourceid << endl;
#endif  

  interp::shpfrm sr = make_shared<interp::ShadowFrame>(r, nullptr, r->ec, nullptr);
  auto halt = make_shared<interp::Kont>(interp::Kont{nullptr, nullptr, sr, 
        nullptr, interp::Kont::NORMAL});
  auto k = make_shared<interp::Kont>(interp::Kont{nullptr, e.get(), sr, halt, interp::Kont::NORMAL});
  auto state = states.emplace(make_pair(reqid, 
                                        InterpState{reqid,
                                                    sourceid,
                                                    peerid,
                                                    std::move(e),
                                                    k,
                                                    vector<interp::shpfrm>{sr},
                                                    std::map<Global::reqid_t, RspState>()}));

  return interpret(state.first->second);
}


int zcore::InterpCtxLocal::interpret(InterpState& state)
{
  s = &state;
  sigint = 0;
  while (s->k->next && !sigint) {
    try {
      s->k = interp::step(s->k, s->fstack, *this);
    }
    catch (const interp::FutureException& e) {
#ifdef DEBUG
      // this is not an error
      std::cout << "attempt to evaluate future: " << e.what() << std::endl;    
#endif
      // leave the state as it is, we'll be coming back to it!
      return 1;
    }
    catch (const Global::QuitException& e) {
      throw;
    }
    catch (const std::exception& e) {
      r->add(".Last.error", val::VError{e.what()});
      // do we have an escape continuation to execute?
      if (s->k->r->getec()) {
        auto k = s->k->r->getec();
        s->k->r->resetec();
        s->popAndClearUntil(k->r);
        s->k = k;           // yes, so execute it!
      }
      else {
        // no, so print the error and then cleanup the stack
	std::cerr << "Error: " << std::flush;
        try {
          // if it's an eval exception, also print the location info:
          auto& ee = dynamic_cast<const interp::EvalException&>(e);
          if (ee.loc.begin.line) {
            std::cerr << ee.loc << ": ";
          } 
          std::cerr << e.what() << std::endl;
          std::cerr << interp::errorLines(ee.loc, 1) << std::endl;
        } catch (...) {
          // this is anything else that did not happen directly in the
          // interpreter (for example it might have happened on the
          // lower layers during a req/rsp, etc.); in this case we
          // don't have a precise location, so we use the location of
          // the control of the step which is better than nothing:
          if (s->k->control->loc.begin.line) {
            std::stringstream ss;
            std::cerr << s->k->control->loc << ": ";
          }
          std::cerr << e.what() << std::endl;
          try {
            std::cerr << interp::errorLines(s->k->control->loc, 1) << std::endl;
          }
          catch (...) {
            // in general we should have info here...
          }
        }
        if (isatty(STDIN_FILENO)) {
          auto prompt = get<string>(cfg::cfgmap.get("prompt"));
          cout << prompt << flush;
        }
        // cleanup the stack
        for (auto &elt : s->fstack) { elt->clear(); }
        states.erase(state.reqid);
        s = nullptr;
        return 0;
      }
    }
  }

  if (!sigint && !(s->k->atype & interp::Kont::SILENT)) {
    const auto& lv = r->find(".Last.value");
    if (lv.which() == val::vt_future) {
      return 1;
    }
    else {
      std::cout << val::display(lv) << std::endl;
    }
  }
  ir.enableKeyboardPoll();
  if (isatty(STDIN_FILENO)) {
    auto prompt = get<string>(cfg::cfgmap.get("prompt"));
    cout << prompt << flush;
  }

  states.erase(state.reqid);
  s = nullptr;
  return 0;
}


void zcore::InterpCtxLocal::sendGcStateMessage(const string& ip,
                                               int port, 
                                               Global::conn_id_t peerid,
                                               const InterpState& state) {
  std::cerr << "Error: response timeout from " << ip << ':' << std::to_string(port) 
            << " [" + std::to_string(peerid) + ']' << std::endl;  
  ir.enableKeyboardPoll();
  if (isatty(STDIN_FILENO)) {
    auto prompt = get<string>(cfg::cfgmap.get("prompt"));
    cout << prompt << flush;
  }
}


/// pass the iterator to InterpState for easier deletion! LLL
int zcore::InterpCtxRemote::interpret(InterpState& state)
{
  s = &state;
  sigint = 0;
  while (s->k->next && !sigint) {
    try {
      s->k = interp::step(s->k, s->fstack, *this);
    }
    catch (const interp::FutureException &e) {
#ifdef DEBUG
      // this is not an error:
      std::cout << "attempt to evaluate future: " << e.what() << std::endl;
#endif
      return 1;
    }
    catch (const Global::QuitException& e) {
      // for the moment don't do anything, although we might want to let
      // it through and terminate the context... LLL
      // although the context terminates on TCP connection down?
    }
    catch (const exception& e) {
      r->add(".Last.error", val::VError{e.what()});
      // do we have an escape continuation to execute?
      if (s->k->r->getec()) {
        auto k = s->k->r->getec();
        s->k->r->resetec();
        s->popAndClearUntil(k->r);
        s->k = k;           // yes, so execute it!
      }
      else {
        // no, so just raise the error:
        for (auto &elt : s->fstack) { elt->clear(); }
        std::string locInfo;
        try {
          // if it's an eval exception, also print the location info;
          // it won't have a file name if it's not from a sourced
          // file, but it will have line/column:
          auto ee = dynamic_cast<const interp::EvalException&>(e);
          if (ee.loc.begin.line) {
            std::stringstream ss;
            ss << ee.loc << ": ";
            locInfo = ss.str();
          } 
        } catch (...) {
          // this is anything else that did not happen directly in the
          // interpreter (for example it might have happened on the
          // lower layers during a req/rsp, etc.); in this case we
          // don't have a precise location, so we use the location of
          // the control of the step which is better than nothing:
          if (s->k->control->loc.begin.line) {
            std::stringstream ss;
            ss << s->k->control->loc << ": ";
            locInfo = ss.str();
          }
        }
        auto n = ir.sendRsp(state.peerid, 
                            state.reqid, 
                            state.sourceid, 
                            val::VError{"Remote error: "s + locInfo + e.what()});
        ++stats.nbOutRSP;
        stats.bytesOutRSP += n;
        states.erase(state.reqid);
        s = nullptr;
        return 0;
      }
    }
  }
  if (sigint) {
    auto n = ir.sendRsp(state.peerid, state.reqid, state.sourceid, val::VError{"remote SIGINT"s});
    ++stats.nbOutRSP;
    stats.bytesOutRSP += n;
  }
  else if (state.k->atype & interp::Kont::SILENT) {
    // means the last evaluation was an assignment, so return TRUE
    // to let the peer know it worked:
    auto n = ir.sendRsp(state.peerid, state.reqid, state.sourceid, val::make_array(true));
    ++stats.nbOutRSP;
    stats.bytesOutRSP += n;
  } 
  else {
    // it was not an assignment and the evaluation that we need to
    // return is in .Last.value:
    const auto& rsp = r->findR(".Last.value");
    if (rsp.which() == val::vt_future) {
      return 1;
    }
    if (val::any_of(rsp, [](const val::Value& v) { return !zcore::isTransmissible(v); })) {
      auto n = ir.sendRsp(state.peerid, state.reqid, state.sourceid, 
                          val::VError{"Remote error: untransmissible type ("s 
                              + std::to_string(rsp.which()) + ")"});
      ++stats.nbOutRSP;
      stats.bytesOutRSP += n;
      return 0;
    }

    auto n = ir.sendRsp(state.peerid, state.reqid, state.sourceid, rsp);
    ++stats.nbOutRSP;
    stats.bytesOutRSP += n;
  }
  states.erase(state.reqid);
  s = nullptr;
 
  return 0;
}


void zcore::InterpCtxRemote::sendGcStateMessage(const string& ip, 
                                                int port, 
                                                Global::conn_id_t peerid, 
                                                const InterpState& state) {
  // note that the remote error can only be got if the timeout on the
  // requesting process is larger than the timeout on the process that
  // is executing:
  auto n = ir.sendRsp(state.peerid, 
                      state.reqid, 
                      state.sourceid, 
                      val::VError{"Remote error: response time out from "s + ip + ":" 
                          + std::to_string(port) + " [" + std::to_string(peerid) + ']'});
  ++stats.nbOutRSP;
  stats.bytesOutRSP += n;
}


void zcore::InterpCtxTimer::removeTimer(val::SpTimer& tmr) {
  auto tmr_ptr = timer_wptr.lock();
  if (!tmr_ptr) {
    // this should never happen LLL
    return;
  }
  
  if (tmr->fd == tmr_ptr->fd) {
    throw std::range_error("cannot remove own timer");
  }
}


/// pass the iterator to InterpState for easier deletion! LLL
int zcore::InterpCtxTimer::interpret(InterpState& state)
{
  s = &state;
  sigint = 0;
  while (s->k->next && !sigint) {
    try {
      s->k = interp::step(s->k, s->fstack, *this);
    }
    catch (const interp::FutureException &e) {
#ifdef DEBUG
      // this is not an error:
      std::cout << "attempt to evaluate future: " << e.what() << std::endl;
#endif
      return 1;
    }
    catch (const Global::QuitException& e) {
      // for the moment don't do anything, although we might want to let
      // it through and terminate the context... LLL
      // although the context terminates on TCP connection down?
    }
    catch (const exception& e) {
      r->add(".Last.error", val::VError{e.what()});
      // do we have an escape continuation to execute?
      if (s->k->r->getec()) {
        auto k = s->k->r->getec();
        s->k->r->resetec();
        s->popAndClearUntil(k->r);
        s->k = k;           // yes, so execute it!
      }
      else {
        // make sure we log LLL, and at least set VError, do we write to stderr?? LLL
    //     // no, so just raise the error:
    //     std::string locInfo;
    //     try {
    //       // if it's an eval exception, also print the location info;
    //       // it won't have a file name if it's not from a sourced
    //       // file, but it will have line/column:
    //       auto ee = dynamic_cast<const interp::EvalException&>(e);
    //       if (ee.loc.begin.line) {
    //         std::stringstream ss;
    //         ss << ee.loc << ": ";
    //         locInfo = ss.str();
    //       } 
    //     } catch (...) {
    //       // this is anything else that did not happen directly in the
    //       // interpreter (for example it might have happened on the
    //       // lower layers during a req/rsp, etc.); in this case we
    //       // don't have a precise location, so we use the location of
    //       // the control of the step which is better than nothing:
    //       if (s->k->control->loc.begin.line) {
    //         std::stringstream ss;
    //         ss << s->k->control->loc << ": ";
    //         locInfo = ss.str();
    //       }
    //     }
    //     auto n = ir.sendRsp(state.peerid, 
    //                         state.reqid, 
    //                         state.sourceid, 
    //                         val::VError{"Timer error: "s + locInfo + e.what()});
    //     ++stats.nbOutRSP;
    //     stats.bytesOutRSP += n;
        for (auto &elt : s->fstack) { elt->clear(); }
        states.erase(state.reqid);
        s = nullptr;
        return 0;
      }
    }
  } // end while
  states.erase(state.reqid);
  s = nullptr;
 
  return 0;
}


void zcore::InterpCtxTimer::sendGcStateMessage(const string& ip, 
                                                int port, 
                                                Global::conn_id_t peerid, 
                                                const InterpState& state) {
  // note that the remote error can only be got if the timeout on the
  // requesting process is larger than the timeout on the process that
  // is executing:
  auto n = ir.sendRsp(state.peerid, 
                      state.reqid, 
                      state.sourceid, 
                      val::VError{"Timer error: response time out from "s + ip + ":" 
                          + std::to_string(port) + " [" + std::to_string(peerid) + ']'});
  ++stats.nbOutRSP;
  stats.bytesOutRSP += n;
}



int zcore::InterpCtxTimer::interpretTimer(int fd) {
  auto tmr_ptr = timer_wptr.lock();
  if (!tmr_ptr) {
    // this should never happen, so at least log the error! LLL
    return 0;
  }
  
  if (tmr_ptr->loop_max > 0 && tmr_ptr->loop_n == tmr_ptr->loop_max) {
    tmr_ptr->stop();
    return 0;
  }
  
  auto e = tmr_ptr->done_first ? tmr_ptr->loop : tmr_ptr->once;
  if (!tmr_ptr->done_first) {
    tmr_ptr->done_first = true;
  }
  else {
    ++tmr_ptr->loop_n;
  }

  auto reqid = getNextId();
  auto sourceid = static_cast<Global::reqid_t>(fd); // check this!!! LLL
  Global::conn_id_t peerid = 0;

  interp::shpfrm sr = make_shared<interp::ShadowFrame>(r, nullptr, r->ec, nullptr);
  auto halt = make_shared<interp::Kont>(interp::Kont{nullptr, nullptr, sr, 
        nullptr, interp::Kont::NORMAL});
  auto k = make_shared<interp::Kont>(interp::Kont{nullptr, e.get(), sr, halt, interp::Kont::NORMAL});
  auto state = states.emplace(make_pair(reqid, 
                                        InterpState{reqid,
                                            sourceid,
                                            peerid,
                                            std::unique_ptr<E>(e->clone()),
                                            k,
                                            vector<interp::shpfrm>{sr},
                                            std::map<Global::reqid_t, RspState>()}));
    
  return interpret(state.first->second);
}



volatile sig_atomic_t zcore::InterpCtx::sigint = 0;
