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


#include <exception>
#include <system_error>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/eventfd.h>
#include <sys/fcntl.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include "net_handler.hpp"
#include "misc.hpp"
#include "globals.hpp"
#include "logging.hpp"
#include "api.hpp"


// #define DEBUG


extern zlog::Logger lg;


static auto gennb = GenNbFun<Global::conn_id_t>(); // sequence number generator


Global::conn_id_t net::ConnectionMappings::createConnection(int fd, 
                                                             const sockaddr_in& addr,
                                                             Connection::Direction dir) 
{
  auto id = gennb();
  mx.lock();
  try {
    fdToId.emplace(fd, id);
    connections.emplace(id, Connection(id, fd, addr, dir));
  }
  catch (...) {
    id = 0;
    mx.unlock();
  }
  mx.unlock();

  return id;
}


const net::Connection net::ConnectionMappings::getConnection(Global::conn_id_t id) {
  mx.lock();
  try {
    auto conn = connections.at(id);    // this will throw
    mx.unlock();
    return conn;
  }
  catch (...) {
    mx.unlock();
    throw;
  }
}


Global::conn_id_t net::ConnectionMappings::getId(int fd) {
  mx.lock();
  auto e = fdToId.find(fd);
  auto ret = e == fdToId.end() ? 0 : e->second; 
  mx.unlock();
  return ret;
}


int net::ConnectionMappings::getFd(Global::conn_id_t id) {
  mx.lock();
  auto e = connections.find(id);
  auto ret = e == connections.end() ? 0 : e->second.fd;    
  mx.unlock();
  return ret;
}


void net::ConnectionMappings::deleteConnection(Global::conn_id_t id) {
  mx.lock();
  try {
    // don't think any of this can throw, but err on the side of caution:
    auto it = connections.find(id);
    fdToId.erase(it->second.fd);
    connections.erase(it);
  }
  catch (...) {
    mx.unlock();
    return;
  }
  mx.unlock();
}

std::map<Global::conn_id_t, net::Connection> net::ConnectionMappings::getAllConnections() const {
  mx.lock();
  try {
    auto ret = connections;
    mx.unlock();
    return ret;
  }
  catch (...) {
    mx.unlock();
    throw;
  }
}


void net::BufferMgt::addBufferToReadylist(Global::conn_id_t id, net::Buf&& b) {
#ifdef DEBUG
  std::cout << "comm::BufferMgt::addBufferToReadylist[id=" << id << "]" << std::endl;
#endif
  mx.lock();
  try {
    auto sz = readybuflist.size();
    if (sz > stats.readbuflistmax) {
      stats.readbuflistmax.store(sz);
    }
    if (sz == max_size) {
      // keep the ready to its max_size: 
      readybuflist.pop_front();
      ++stats.nbInBuffersDrop;
    }
    readybuflist.emplace_back(std::make_pair(id, std::move(b)));
  }
  catch (...) {
    // can't see anything else than memory exhaustion; just register we dropped:
    ++stats.nbInBuffersDrop;    
  }
  mx.unlock();
}


bool net::BufferMgt::get_data(Global::conn_id_t& id, Buf& buf) { 
#ifdef DEBUG
  std::cout << "comm::BufferMgt::get_data" << std::endl;
#endif
  mx.lock();
  try {
    if (readybuflist.size() > 0) {
      auto& p = readybuflist.front();
      std::swap(p.second, buf);
      id = p.first;
      readybuflist.pop_front();
      mx.unlock();
      return true;
    }
    else {
      mx.unlock();
      return false;
    }
  }
  catch (...) {
    mx.unlock();
    return false;
  }
}


size_t net::BufferMgt::gc() {
  size_t bytes = 0;
  std::chrono::system_clock::time_point::duration combufttl =
    std::chrono::seconds(get<int64_t>(cfg::cfgmap.get("commbuf.ttl.secs")));
  auto now = std::chrono::system_clock::now();
  mx.lock();
  for (auto i = bufmap.begin(); i != bufmap.end(); ++i) { 
    if (now - i->second.timestamp >= combufttl) { 
      bytes += i->second.offset;
      bufmap.erase(i);
    } 
  }
  for (auto i = readybuflist.begin(); i != readybuflist.end(); ) {
    if (now - i->second.timestamp >= combufttl) {
      bytes += i->second.offset;
      i = readybuflist.erase(i);
    }
    else {
      ++i;
    }
  }
  mx.unlock();
  return bytes;
}


void net::BufferMgt::removeAllBuffersForId(Global::conn_id_t id) {
  mx.lock();
  bufmap.erase(id);
  mx.unlock();
  // 'readyBufList' does not need to be cleaned here; once a context
  // has disappeared, the upper layer (interp_run) will just throw
  // away the orphaned buffers as it picks them up.
}


size_t net::BufferMgt::getReadyBufSz() const {
  mx.lock();
  auto sz = readybuflist.size();
  mx.unlock();
  return sz;
}


void net::SignallingMgt::add_sig(Global::conn_id_t id, Status st) {
  mx.lock();
  try {
    if (l.size() == max_size) {
      l.pop_front();
    }
    l.emplace_back(id, st);
  }
  catch (...) {

  }
  mx.unlock();
}


bool net::SignallingMgt::get_sig(Global::conn_id_t& id, Status& st) {
  mx.lock();
  try {
    if (l.size() > 0) {
      auto& p = l.front();
      id = p.first;
      st = p.second;
      l.pop_front();
      mx.unlock();
      return true;
    }
    else {
      mx.unlock();
      return false;
    }
  }
  catch (...) {
    mx.unlock();
    return false;
  }
}


size_t net::SignallingMgt::getListSz() const {
  mx.lock();
  auto sz = l.size();
  mx.unlock();
  return sz;
}


// NetHandler ----------------------------------------------


net::NetHandler::NetHandler(const std::string ip_addr, 
                 int port_p, 
                 int data_out_fd_p, 
                 int sig_out_fd_p,
                 size_t datalist_max_size,
                 size_t siglist_max_size)
  : ready(0), port(port_p), data_out_fd(data_out_fd_p), sig_out_fd(sig_out_fd_p), 
    bufmgt(datalist_max_size, stats), sigmgt(siglist_max_size) {
#ifdef DEBUG
  std::cout << "NetHandler::comm()" << std::endl;
  std::cout << "| port:" << port << std::endl;
#endif

  if (port > 0) {
    // open and bind socket
    bzero(&addr,sizeof(addr));
    addr.sin_family = AF_INET;
    if (!ip_addr.size()) {
      // INADDR_ANY: the socket accepts connections to all the IPs of
      // the machine
      addr.sin_addr.s_addr=htonl(INADDR_ANY);
    }
    else {
      // when an address is specified on the command line or in conf file:
      int res = inet_pton(AF_INET, ip_addr.c_str(), &addr.sin_addr);
      if (res == 0) {
        throw std::logic_error("inet_pton: "s + ip_addr + " is not a valid IPv4 address");
      }
      else if (res == -1) {
        throw std::system_error(std::error_code(errno, std::system_category()), "inet_pton failed");
      }
    }
    addr.sin_port=htons(port);
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
      throw std::system_error(std::error_code(errno, std::system_category()), 
                              "NetHandler: cannot create TCP socket");
    }
    int one = 1; 
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if (fcntl(fd, F_SETFL, O_NONBLOCK) ==  -1) {
      throw std::system_error(std::error_code(errno, std::system_category()), 
                              "NetHandler: fcntl(fd, F_SETFL, O_NONBLOCK)");
    }

    /// \todo figure out what TCP params we need and add them to the options
    ///       setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one)); 
    ///       set up keepalive? http://tldp.org/HOWTO/TCP-Keepalive-HOWTO/programming.html
    
    if (bind(fd, (sockaddr*)&addr, sizeof(addr)) == -1) {
      throw std::system_error(std::error_code(errno, std::system_category()), 
                              "NetHandler: cannot bind TCP socket");
    }
  }
  // not an error, it just means comm only has client capabilities.
}


void net::NetHandler::run(volatile bool& stop) {
#ifdef DEBUG
  std::cout << "NetHandler[" << port << "]::run()" << std::endl;
#endif

  epollfd = epoll_create1(0);
  if (epollfd == -1) {
    throw std::system_error(std::error_code(errno, std::system_category()), "epoll_create1");
  }
  // now we have epollfd we are ready to create outgoing connection,
  // so set the atomic variable to signal this:
  ready.store(1);

  ev.events = EPOLLIN;

  if (port > 0) {
    const int backlog = 5;
    listen(fd, backlog);
    ev.data.fd = fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
      throw std::system_error(std::error_code(errno, std::system_category()), "epoll_ctl");
    }
  }

  epoll_event events[Global::EPOLL_MAX_EVENTS];

  for (;;) {
    int nfds = epoll_wait(epollfd, events, Global::EPOLL_MAX_EVENTS, EPOLL_TIMEOUT);

    if (stop) return;

    if (nfds < 0) {
      if (errno != EINTR) {     // only OK if interrupted by signal
        std::cout << "errno is:  " << errno << std::endl;
        throw std::system_error(std::error_code(errno, std::system_category()), "epoll_wait");
      }
      continue;
    } 
    else if (nfds == 0) {       // timeout
      bufmgt.gc();
      continue;
    }
    
    for (int i = 0; i < nfds; ++i) {
      // connection request: -------------------------------------
      if (events[i].data.fd == fd) {
        acceptConnection();
      }
      // data on TCP connection: ---------------------------------
      else {
        bool first_read = true;
        ssize_t nread = 0;
        while (nread >= 0) {
          int peerfd = events[i].data.fd;
#ifdef DEBUG
          std::cout << "data for fd: " << peerfd << std::endl;
#endif
          auto id = connMappings.getId(peerfd);

          auto elt = bufmgt.bufmap.find(id); // find an active buffer for this peer

          // no active buffer, so this is a new msg:
          if (elt == bufmgt.bufmap.end()) {
            // if less than 16 bytes, try again, because we need a
            // length and the magic number to allocate a new buffer:
            int bytesAvail;
            auto res = ioctl(peerfd, FIONREAD, &bytesAvail);
            if (res < 0) { 
              lg.log(zlog::SV_ERROR, "failed ioctl FIONREAD");
              disconnect(id);   // this will also send a CLOSE to the upper layer
              break;
            }
            if (bytesAvail < 16) { // size + magic number
              if (first_read && bytesAvail == 0) {
                // if we've been signalled but there is not data on
                // the socket, we infer the connection is down:
                disconnect(id);   // this will also send a CLOSE to the upper layer
              }
              break;
            }
            first_read = false;

            // check we have the magic number:
            size_t magicnb;
            if (read(id, peerfd, (char*)&magicnb, sizeof(magicnb)) <= 0) break;
            magicnb = ntoh<size_t>(magicnb);
            if (magicnb != Global::MAGICNB) {
              ++stats.nbInMalformed;
              break;
            } 
	  
            // get the msg length:
            size_t msglen;
            if (read(id, peerfd, (char*)&msglen, sizeof(msglen)) <= 0) continue;
            msglen = ntoh<size_t>(msglen);

            // create a new buffer in bufmap:
            bufmgt.bufmap.emplace(std::make_pair(id, Buf(id, msglen-16)));
          } 

          // continue buffer reassembly:
          auto& msg = bufmgt.bufmap.at(id);
          auto nread = read(id, peerfd, &msg.data[msg.offset], msg.len - msg.offset);
          if (nread <= 0) continue;
          msg.offset += nread;
	
          if (msg.offset == msg.len) {
            bufmgt.addBufferToReadylist(id, std::move(msg));
            bufmgt.bufmap.erase(id);
            ++stats.nbInBuffers;

            // let upper layer know there's a msg ready:
            size_t count = 1;
            ssize_t res = write(data_out_fd, (void*)&count, sizeof(count));
            if (res == -1 ) {
              throw std::system_error(std::error_code(errno, std::system_category()), 
                                      "write(data_out_fd)");
            }
          }
        }
      }
    }
  }
}


ssize_t net::NetHandler::read(Global::conn_id_t id, int fd, char* buf, size_t n) {
  auto nread = ::recv(fd, buf, n, MSG_DONTWAIT);
#ifdef DEBUG
  std::cout << "NetHandler::read: read " << nread << " bytes" << std::endl;
#endif
  if (nread < 0) {
    // it's OK as long as it's a EWOULDBLOCK or EAGAIN,
    if (!(errno == EWOULDBLOCK || errno == EAGAIN)) {
      lg.log(zlog::SV_ERROR, ("NetHandler::read failed read: "s + std::strerror(errno)).c_str());
      disconnect(id);
    }
  }
  else if (nread == 0) {
    disconnect(id);
  }
  return nread;
}



/// Send 'buf' to peer 'id'. 
ssize_t net::NetHandler::send(Global::conn_id_t id, char* buf, size_t len) {
#ifdef DEBUG
  std::cout << "NetHandler[" << port << "]::send" << std::endl;
  std::cout << "| id:  " << id << std::endl;
  std::cout << "| len: " << len << std::endl;
#endif
  mx.lock();
  auto fd = connMappings.getFd(id);
  mx.unlock();
  ssize_t res = 0;
  if (fd > 0) {
#ifdef DEBUG
    std::cout << "| fd:  " << fd << std::endl;
#endif
    *(reinterpret_cast<uint64_t*>(buf))   = hton64(Global::MAGICNB);
    *(reinterpret_cast<uint64_t*>(buf+8)) = hton64(len);
    
    res = write(fd, buf, len);
    if (res < 0) {
      ++stats.nbSendFail;
    }
    else {
      ++stats.nbOutBuffers;
    }
  }
  else {
    throw std::range_error("no connection to peer"); // decide if we want to throw or not LLL
  }
  return res;
}


void net::NetHandler::acceptConnection() {
  // see https://banu.com/blog/2/how-to-use-epoll-a-complete-example-in-c/
  while (1) {
    sockaddr_in peeraddr;
    socklen_t sz = sizeof(peeraddr);
    
    int newfd = accept(fd, (sockaddr *)&peeraddr, &sz);
    if (newfd == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        break;                  // all incoming connections processed
      }
      else {
        throw std::system_error(std::error_code(errno, std::system_category()), "accept");
      }
    }
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = newfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, newfd, &ev) == -1) {
      throw std::system_error(std::error_code(errno, std::system_category()), "epoll_ctl: ADD");
    }

    auto id = connMappings.createConnection(newfd, peeraddr, net::Connection::Direction::INCOMING);

#ifdef DEBUG
    std::cout << "new incoming conn: " << id << std::endl;
#endif
    sigmgt.add_sig(id, net::SignallingMgt::UP);

    ++stats.nbInConn;
    uint64_t count = 1;
    ssize_t res = write(sig_out_fd, (void*)&count, sizeof(count));
    if (res == -1 ) {
      lg.log(zlog::SV_ERROR, "NetHandler::acceptConnection, write(UP) failed: %s", 
             std::error_code(errno, std::system_category()).message().c_str());      
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // if we can't tell our upper layer to establish a context for
        // it, then we have no other choice but to close it:
        disconnect(id);
      } 
      else if (errno == EPIPE) {
        throw std::system_error(std::error_code(errno, std::system_category()), 
                                "NetHandler::acceptConnection, write(UP) unknown error");
      }
    } else if (res != 8) {
      // hopefully this can't happen...
      throw std::out_of_range("unknown error: write(UP) != 8");
    }
    lg.log(zlog::SV_INFO, "UP @ %s: %d", inet_ntoa(peeraddr.sin_addr), peeraddr.sin_port);
  } // end while(1)
}


Global::conn_id_t net::NetHandler::connect(const std::string& ip, int port) {
#ifdef DEBUG
  std::cout << "NetHandler[" << port << "]::connect()" << std::endl;
  std::cout << "| " << ip << ":" << std::to_string(port) << std::endl;
#endif
  sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr=inet_addr(ip.c_str());
  addr.sin_port=htons(port);
  
  int peerfd = socket(AF_INET, SOCK_STREAM, 0);
  if (peerfd == -1) {
    lg.log(zlog::SV_ERROR, "NetHandler::connect 'socket': %s", 
           std::error_code(errno, std::system_category()).message().c_str());      
    throw std::system_error(std::error_code(errno, std::system_category()), "socket");
  }

  int res = ::connect(peerfd, (sockaddr *) &addr, sizeof(struct sockaddr_in));
  if (res == -1) {
    lg.log(zlog::SV_ERROR, "NetHandler::connect 'connect': %s", 
           std::error_code(errno, std::system_category()).message().c_str());      
    close(peerfd);
    throw std::system_error(std::error_code(errno, std::system_category()), "socket");
  }

  auto id = connMappings.createConnection(peerfd, addr, net::Connection::Direction::OUTGOING);
  ev.events = EPOLLIN | EPOLLET;
  ev.data.fd = peerfd;
  if (epoll_ctl(epollfd, EPOLL_CTL_ADD, peerfd, &ev) == -1) {
    connMappings.deleteConnection(id);
    throw std::system_error(std::error_code(errno, std::system_category()), "epoll_ctl: ADD");
  }
  ++stats.nbOutConn;
  return id;
}


void net::NetHandler::disconnect(Global::conn_id_t id) {
#ifdef DEBUG
  std::cout << "NetHandler[" << port << "]::disconnect()" << std::endl;
  std::cout << "| id: " << id << std::endl;
#endif
  // it's possible to have simultaneous disconnects, so the second one
  // will will have an id of 0, so here we can safely just ignore it,
  // as we've already cleaned up:
  if (id == 0) return;
  
  try {
    auto& conn = connMappings.getConnection(id);
    lg.log(zlog::SV_INFO, "DOWN @ %s: %d", inet_ntoa(conn.addr.sin_addr), conn.addr.sin_port);
    int res = close(conn.fd);
    if (res != 0) {
      lg.log(zlog::SV_ERROR, "comm::NetHandler::disconnect 'close': %s", 
             std::error_code(errno, std::system_category()).message().c_str());
    }
    if (conn.dir == net::Connection::Direction::OUTGOING) {
      ++stats.nbCloseOutConn;
    }
    else {
      ++stats.nbCloseInConn;
    }
  }
  catch (...) {
    lg.log(zlog::SV_INFO, "can't disconnect id: %d", id);
    return;
  }
  // note that, per the man page, fd is taken out of the epoll
  // set automatically on close
    
  connMappings.deleteConnection(id);
  bufmgt.removeAllBuffersForId(id);
    
  uint64_t count = 1;

  sigmgt.add_sig(id, net::SignallingMgt::DOWN);
  ssize_t wres = write(sig_out_fd, (void*)&count, sizeof(count));
  if (wres == -1 ) {
    lg.log(zlog::SV_ERROR, "NetHandler::disconnect 'write': %s", 
           std::error_code(errno, std::system_category()).message().c_str());      
  }
}


zcore::NetInfo net::NetHandler::getNetInfo() const {
  const auto cons = connMappings.getAllConnections();
  zcore::NetInfo info;
  for (auto& cpair : cons) {
    auto& c = cpair.second;
    info.conninfo.emplace_back(zcore::ConnectionInfo{
        c.id, 
        inet_ntoa(c.addr.sin_addr),
        c.addr.sin_port, 
        c.dir==net::Connection::Direction::OUTGOING ? 
          zcore::ConnectionInfo::Direction::OUTGOING :
          zcore::ConnectionInfo::Direction::INCOMING});
  }
  info.buflistSz = bufmgt.getReadyBufSz();
  info.siglistSz = sigmgt.getListSz();
  return info;
}


// constexpr std::chrono::system_clock::time_point::duration comm::NetHandler::BUF_TIME_TO_LIVE;
