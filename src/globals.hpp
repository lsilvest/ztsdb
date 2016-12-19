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


#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <cstdint>
#include <chrono>
#include <exception>
#include <limits>


namespace Global {

  // static const uint64_t zna = 0x7ff00000000007a2; // 0x7a2 == 1954, the R convention
  // const double ZNA  = *reinterpret_cast<const double*>(&zna);
  const double ZNAN = std::numeric_limits<double>::quiet_NaN();
  const double ZINF = std::numeric_limits<double>::infinity();

  struct NANF {
    static double f() { return Global::ZNAN; }
  };

  // inline bool isna(double d) { return *reinterpret_cast<const uint64_t*>(&d) == zna; }

  const size_t CHUNKSZ = 131072; // 2^17
  // static const size_t CHUNKSZ = 1024;
  const size_t MAGICNB = 0x9316481736403219L;

  const size_t EPOLL_MAX_EVENTS = 64; // max number of peers...

  using dtime = std::chrono::system_clock::time_point;
  using duration = dtime::duration;

  struct QuitException : std::exception {
    QuitException(int status_p) : status(status_p) { }
    int status;
  };

  // alignment boundary for strings:
  const size_t STRALIGN = 8;

  /// Message types used in the headers of the TCP messages between
  /// database instances.
  enum class MsgType : uint64_t { 
    REQ, 
    RSP, 
    APPEND,
    APPEND_VECTOR
  };

  /// Connection ID. We can't rely on file descriptors as identifiers
  /// because we are in a multithreaded environment. The issue is
  /// reuse of a file descriptor number while we still have contexts
  /// identified by the same number.
  using conn_id_t = uint64_t;

  /// Used to encode the request numbers in the REQ/RSP packets.
  using reqid_t = uint64_t;

  /// The name of the environment variable that holds the path to the data directory. 
  const char* const ZTSDBDIR_ENV = "ZTSDBDIR";
  const char* const CONFIG_FILENAME = "ztsdb.conf";
  const char* const LOGFILE_EXTENSION = ".log";
}


#endif
