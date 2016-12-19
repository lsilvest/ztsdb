// (C) 2015 Leonardo Silvestri
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


#ifndef ZCPP_H
#define ZCPP_H


#include "array.hpp"
#include "base_types.hpp"
#include "misc.hpp"
#include "zts.hpp"
#include "globals.hpp"
#include "net_handler.hpp"


/// Provides a C++ interface to append data to arrays and time
/// series. This interface will send messsages to a ztsdb
/// instance. These messages bypass the interpreter, so they have
/// decent performance.


namespace arr {

  size_t  getHeaderLength(const std::string& name);
  void    writeHeader(buflen_pair& buf, 
                      Global::MsgType msgtype, 
                      const std::string& name);
    
  template<typename T>
  buflen_pair make_append_msg(const std::string& name, const arr::Array<T>& a) 
  {
    const auto headersz = getHeaderLength(name); 
    const auto totalsz  = headersz + a.getBufferSize();
    auto buf = std::make_pair(std::make_unique<char[]>(totalsz), totalsz);
    a.to_buffer(buf.first.get() + headersz);
    writeHeader(buf, Global::MsgType::APPEND, name);
    return buf;
  }

  template<typename T>
  buflen_pair make_append_msg(const std::string& name, const Vector<T>& v) 
  {
    const auto headersz = getHeaderLength(name);
    const auto totalsz  = headersz + v.getBufferSize();
    auto buf = std::make_pair(std::make_unique<char[]>(totalsz), totalsz);
    writeHeader(buf, Global::MsgType::APPEND_VECTOR, name);
    v.to_buffer(buf.first.get() + headersz);
    return buf;
  }

  buflen_pair make_append_msg(const std::string& name, 
                              const Vector<Global::dtime> idx, 
                              const Vector<double>& v);


  // specialize the above with string and zstring so that it fails... LLL

  buflen_pair make_append_msg(const string& name, const arr::zts& z);
}



#endif
