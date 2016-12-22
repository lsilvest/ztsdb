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


#ifndef COMM_HPP
#define COMM_HPP

#include <netinet/in.h>
#include <deque>
#include <map>
#include <unordered_map>
#include <utility>
#include <mutex>
#include <chrono>
#include <memory>
#include <boost/circular_buffer.hpp>
#include <sys/epoll.h>
#include "misc.hpp"
#include "stats.hpp"
#include "info.hpp"
#include "config.hpp"



/// Contains the low level network communication: TCP connection
/// management and buffering.
namespace net {

  // Message format (number are bytes):
  // 

  // | MAGIC_NB |   SIZE   |   TYPE   |
  // |----------|----------|----------|
  // |     8    |     8    |     8    |
  // 
  // When type REQ
  //
  // When type RSP
  //
  // When type APPEND
  // | var name length            | 
  // | var names string + padding |
  // | dim length                 |
  // | dim 0                      |
  // | ...                        |
  // | v0 data type               |
  // | v0 data                    |
  // | ...                        |
  // | v1 data type               |
  // | v1 data                    |
  // | ...                        |
  // 
  // all above can be memcopied in!
  

  // When type ZTS
  // | var name length            | 
  // | var names string + padding |
  // | length time
  // | time                       |
  // | length                     |
  // | ...                        |
  // | data                       |
  // | ...                        |
  //
  // length must be a multiple of (length time)* (Mul 2..n of dims)


  const unsigned INIT_OFFSET = 16; // the magic number, followed by size

  using namespace std::chrono;
  using namespace std::literals;
  using time_point = std::chrono::system_clock::time_point;

  struct Buf {
   
    Buf() : id(0), len(0), offset(0), timestamp(0s), data(nullptr) { }
    Buf(Global::conn_id_t id_p, size_t len_p) : id(id_p), len(len_p), offset(0),
                                      timestamp(std::chrono::system_clock::now()),
                                              data(std::make_unique<char[]>(len_p)) { }
    
    
    Global::conn_id_t id;       // owner of the buffer
    size_t  len;
    size_t  offset;		// usable buffer has offset=len
    time_point timestamp;       // can't allow buffers to live forever
    std::unique_ptr<char[]> data;
  };
  
  
  struct Connection {
    enum Direction { INCOMING, OUTGOING };

    Connection(Global::conn_id_t id_p, int fd_p, const sockaddr_in& addr_p, Direction dir_p) :
      id(id_p), fd(fd_p), addr(addr_p), dir(dir_p)  { }

    Global::conn_id_t id;
    int fd;
    sockaddr_in addr;           // contains both IP address and port
    Direction dir;
  };

  struct ConnectionMappings {

    Global::conn_id_t createConnection(int fd, const sockaddr_in& addr, Connection::Direction dir);
    const Connection getConnection(Global::conn_id_t id);
    Global::conn_id_t getId(int fd);
    int getFd(Global::conn_id_t id);
    void deleteConnection(Global::conn_id_t id);

    std::map<Global::conn_id_t, Connection> getAllConnections() const;

  private:
    /// Tables to keep track and retrieve connections:
    std::map<int, Global::conn_id_t> fdToId;
    std::map<Global::conn_id_t, Connection> connections;
    mutable std::mutex mx;
  };
  
  
  struct BufferMgt {
    BufferMgt(size_t max_size_p, zcore::NetStats& stats_p) 
      : max_size(max_size_p), stats(stats_p) { }

    /// Add a buffer to the ready list.
    void addBufferToReadylist(Global::conn_id_t id, Buf&& b);
    bool get_data(Global::conn_id_t& id, Buf& buf);

    /// Garbage collect the buffer queue. Returns the number of bytes
    /// that were deleted. 'd' is the amount of time a buffer can
    /// live.
    size_t gc();

    /// Remove all the buffers (both partial and completed) that
    /// belong to 'id'.
    void removeAllBuffersForId(Global::conn_id_t id);

    /// A queue for partially completed buffers, i.e. buffers that are
    /// still being assembled.
    std::unordered_map<Global::conn_id_t, Buf> bufmap;

    size_t getReadyBufSz() const;
    
  private:
    /// Completed buffers, i.e. ready to be used by the upper level.
    std::deque<std::pair<Global::conn_id_t, Buf>> readybuflist;
    mutable std::mutex mx;      // only need to protect 'readybuflist'

    const size_t max_size;
    zcore::NetStats& stats;
  };
  
  
  struct SignallingMgt {
    SignallingMgt(size_t max_size_p=10) : max_size(max_size_p) { }

    enum Status { UP, DOWN };

    void add_sig(Global::conn_id_t id, Status st);
    bool get_sig(Global::conn_id_t& id, Status& st);

    size_t getListSz() const;

  private:
    const size_t max_size;
    std::deque<std::pair<Global::conn_id_t, Status>> l;
    mutable std::mutex mx;
  };



  using namespace std::literals;

  struct NetHandler {
    
    NetHandler(const std::string ip_addr, 
         int port_p, 
         int data_out_fd, 
         int signalling_out_fd,
         size_t datalist_max_size=1e5,
         size_t siglist_max_size=10);

    void run(volatile bool& stop);

    ssize_t send(Global::conn_id_t id, char*  buf, size_t  len);

    bool get_data(Global::conn_id_t& id, Buf& buf) { return bufmgt.get_data(id, buf); }
    bool get_sig(Global::conn_id_t& id, SignallingMgt::Status& st) { return sigmgt.get_sig(id, st); }

    /// Establish TCP connection to a given addres/port. Either
    /// returns the created 'conn_id_t' or throws.
    Global::conn_id_t connect(const std::string& ip, int port);
    /// Disconnect the TCP connection (if still up), take out 'id'
    /// from the mappings and inform the upper layer.
    void disconnect(Global::conn_id_t id);

    static constexpr time_point::duration BUF_TIME_TO_LIVE = 60s;

    inline const zcore::NetStats& getNetStats() { return stats; }
    inline void resetNetStats() { stats.reset(); }

    zcore::NetInfo getNetInfo() const;

    std::atomic_uint_fast64_t ready;
  private:
    void acceptConnection();
    ssize_t read(Global::conn_id_t id, int fd, char* buf, size_t n);  

    int fd;                     // TCP listen sock
    int port;			// local listen port

    sockaddr_in addr;		// local address

    /// Used for synchronization w/ upper level thread. Both these
    /// file descriptors must refer to eventfd file descriptors. The
    /// lower level increments the count for each retrievable data or
    /// signalling buffer. This allows the upper level poll to trigger
    /// and know exactly how many messages it should retrieve.
    const int data_out_fd; 
    const int sig_out_fd; 

    ConnectionMappings connMappings;

    BufferMgt bufmgt;
    SignallingMgt sigmgt;

    zcore::NetStats stats;
    
    std::mutex mx;              // global mutex

    int epollfd;
    epoll_event ev;

    static const int EPOLL_TIMEOUT = 1000;
  };

}



#endif
