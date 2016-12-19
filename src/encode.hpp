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


#ifndef ENCODE_HPP
#define ENCODE_HPP


#include <cstdint>
#include "valuevar.hpp"
#include "ast.hpp"
#include "misc.hpp"
#include "globals.hpp"
#include "net_handler.hpp"
#include "zts.hpp"


namespace zcore {

  /// State of a value being decoded. Allows to decode values split
  /// over multiple buffers.
  struct ValState {
    val::Value val;             ///< value under construction
    uint64_t   exp;             ///< expected length
    uint64_t   n;               ///< current  length
    /// For Values composed of other Values, the stage indicates the
    /// reading state so we know where to continue when a new packet
    /// comes in; for example, in the case of reading an array, stage 0
    /// indicates we are still reading the dimension/names list, stage
    /// 1 indicates we are reading the array values.
    unsigned stage;        
  };

  template<typename T>
  val::Value getOrderedVector(const arr::Array<T>& t) {
    val::SpVI v = make_shared<val::VI>();
    for (size_t j=0; j<t.ncols(); ++j) {
      v->push_back(t.getcol(j).isOrdered());
    }
    return v;
  }
  
  val::Value convertToList(const vector<unique_ptr<arr::Dname>>& names);


  /// This class handles the encoding/decoding of a message. A
  /// 'message' is a layer of abstraction above the chunk, itself a
  /// layer above the TCP segment (the reassembly of which is handled
  /// by NetHandler). An encode object is created each time a message of a
  /// given type (e.g. REQ, RSP) is sent. It handles the encoding of
  /// the contents and their (outgoing) segmentation in chunks. It
  /// also handles the reassembly of code buffers and the ongoing
  /// construction of a transmitted value. The main implementation
  /// goal is to avoid any copy of incoming and outgoing
  /// objects. Outgoing objects are sent in chunks straight from the
  /// object and incoming objects are reconstructed in place from
  /// chunks.
  struct Encode {
    Encode(net::NetHandler& c, 
           Global::conn_id_t peerid_p, 
           Global::reqid_t reqid_p, 
           Global::reqid_t sourceid_p, 
           Global::MsgType msgt_p) : 
      com(c), peerid(peerid_p), reqid(reqid_p), sourceid(sourceid_p), msgt(msgt_p),
      offset(net::INIT_OFFSET), // so net has space to insert magic nb and size
      bytes(0)
    { 
      *this << uint64_t(msgt);
      *this << reqid;
      *this << sourceid;
    }

    net::NetHandler& com;
    const Global::conn_id_t peerid;
    const Global::reqid_t reqid;
    const Global::reqid_t sourceid;
    const Global::MsgType msgt;

    char buf[Global::CHUNKSZ];
    size_t offset;
    size_t bytes;

    Encode& operator <<(const val::Value& v);
    Encode& operator <<(const E* e);

    template<typename T> 
    Encode& operator <<(const T& t) {
      if (offset + sizeof(T) > Global::CHUNKSZ) {
        flush();
      }
      auto h = hton<T>(t);
      memcpy(buf + offset, &h, sizeof(T));
      offset += sizeof(T);
      return *this;
    }

    template<typename T>
    Encode& operator <<(const arr::Array<T>& t) {
      *this << t.size();

      // an array is a list of 3 (stage 0)
      auto rlist = val::VList(vector<pair<string, val::Value>>());
      rlist.push_back(make_pair("", val::Value(make_shared<val::VI>(t.dim))));
      rlist.push_back(make_pair("", convertToList(t.names)));
      rlist.push_back(make_pair("", getOrderedVector(t)));
      *this << val::Value(make_cow<val::VList>(false, rlist)); // Value, else the wrong
                                                               // template func is used
      
      // followed by (after stage is set to 1) a series of integers or doubles, etc.
      for (idx_type col=0; col < t.v.size(); ++col) {
        for (idx_type j=0; j < t.dim[0]; ++j) {
          *this << static_cast<T>(t.getcol(col)[j]);
        }
      }
      // for some reason (with GCC 4.9) the above is faster than:
      // for (auto& col : t.v)
      //   for (auto j : col)
      //     *this << j;
      return *this;
    }


    Encode& operator <<(const val::VI& t);
    Encode& operator <<(const string& t);
    Encode& operator <<(const arr::zstring& t);
    Encode& operator <<(const tz::interval& t);
    Encode& operator <<(const tz::period& t);
    Encode& operator <<(const arr::zts& t);    
    Encode& operator <<(const bool t);    


    ssize_t flush() { 
      auto res = com.send(peerid, buf, offset);
      if (res > 0) {
        bytes += res;
      }
      offset = net::INIT_OFFSET; 
      *this << msgt;
      *this << reqid;
      *this << sourceid;
      return res;               // what's the point? LLL is it not
                                // better to check res == offset? and
                                // should we throw?      
    }

    ssize_t flush_end() {       // LLL
      auto res = com.send(peerid, buf, offset); 
      if (res > 0) {
        bytes += res;
      }
      offset = net::INIT_OFFSET; 
      return res;               // what's the point? LLL is it not
                                // better to check res == offset? and
                                // should we throw?      
    }

  private:

    Encode& operator <<(const val::VClos& t);
    Encode& operator <<(const val::VConn& t);
    // note, we don't allow transmission of VBuiltinG, VFuture (a
    // domain_error exception is raised)
 
  };
 
  /// Get the encoded length of an expression.
  size_t codeLength(const E* e);

  /// Read an encoded expression of type 'E' from a buffer of length len.
  E* readCode(const char* buf, size_t len);

  /// Read an encoded value of type 'val::Value' from a buffer of length len.
  /// Offset 'off' and value state 'ss' will be updated; the latter are
  /// needed because the value might span more than buf.
  void readValue(const char* buf, size_t len, size_t& off, vector<ValState>& ss, size_t& idx);

  /// Returns 'true' if 'v' can be encoded and sent, 'false' otherwise.
  bool isTransmissible(const val::Value& v);
}

#endif
