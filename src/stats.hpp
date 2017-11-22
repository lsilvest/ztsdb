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


#ifndef STATS_HPP
#define STATS_HPP


#include <cstdint>
#include <atomic>


namespace zcore {

  struct NetStats {
    NetStats() { reset(); }

    std::atomic_uint_fast64_t nbInConn;
    std::atomic_uint_fast64_t nbOutConn;
    std::atomic_uint_fast64_t nbCloseInConn;
    std::atomic_uint_fast64_t nbCloseOutConn;
    std::atomic_uint_fast64_t nbOutBuffers;     
    std::atomic_uint_fast64_t nbSendFail;
    std::atomic_uint_fast64_t nbInBuffers;     
    std::atomic_uint_fast64_t nbInBuffersDrop;     
    std::atomic_uint_fast64_t bytesTimedOut;
    std::atomic_uint_fast64_t nbFailInCtx;
    std::atomic_uint_fast64_t nbReadFail;
    std::atomic_uint_fast64_t nbInMalformed;     
    std::atomic_uint_fast64_t readbuflistmax;     

    inline void reset() {
      nbInConn.store(0);              
      nbOutConn.store(0);       
      nbCloseInConn.store(0);   
      nbCloseOutConn.store(0);  
      nbOutBuffers.store(0);     
      nbSendFail.store(0);      
      nbInBuffers.store(0);     
      bytesTimedOut.store(0);       
      nbInBuffersDrop.store(0);     
      nbFailInCtx.store(0);     
      nbReadFail.store(0);      
      nbInMalformed.store(0);   
      readbuflistmax.store(0);  
    }
  };

  
  struct MsgStats {
    MsgStats() { reset(); }

    std::atomic_uint_fast64_t bytesOutREQ;
    std::atomic_uint_fast64_t bytesOutRSP;
    std::atomic_uint_fast64_t bytesInREQ;
    std::atomic_uint_fast64_t bytesInRSP;
    std::atomic_uint_fast64_t bytesAppend;
    std::atomic_uint_fast64_t bytesAppendVector;
    std::atomic_uint_fast64_t nbInREQ;
    std::atomic_uint_fast64_t nbInRSP;
    std::atomic_uint_fast64_t nbAppend;
    std::atomic_uint_fast64_t nbAppendVector;
    std::atomic_uint_fast64_t nbAppendFail;
    std::atomic_uint_fast64_t nbAppendVectorFail;

    inline void reset() {
      bytesOutREQ.store(0);      
      bytesOutRSP.store(0);      
      bytesInREQ.store(0);       
      bytesInRSP.store(0);       
      bytesAppend.store(0);      
      bytesAppendVector.store(0);
      nbInREQ.store(0);       
      nbInRSP.store(0);       
      nbAppend.store(0);      
      nbAppendVector.store(0);
      nbAppendFail.store(0);      
      nbAppendVectorFail.store(0);
    }
  };

  struct CtxStats {
    CtxStats() { reset(); }

    std::atomic_uint_fast64_t nbOutREQ;
    std::atomic_uint_fast64_t nbOutRSP;
    std::atomic_uint_fast64_t nbInREQ;
    std::atomic_uint_fast64_t nbInRSP;
    std::atomic_uint_fast64_t nbAppend;
    std::atomic_uint_fast64_t nbAppendVector;
    std::atomic_uint_fast64_t bytesOutREQ;
    std::atomic_uint_fast64_t bytesOutRSP;
    std::atomic_uint_fast64_t bytesInREQ;
    std::atomic_uint_fast64_t bytesInRSP;
    std::atomic_uint_fast64_t bytesAppend;
    std::atomic_uint_fast64_t bytesAppendVector;

    inline void reset() {
      nbOutREQ.store(0);         
      nbOutRSP.store(0);         
      nbInREQ.store(0);          
      nbInRSP.store(0);          
      nbAppend.store(0);         
      nbAppendVector.store(0);   
      bytesOutREQ.store(0);      
      bytesOutRSP.store(0);      
      bytesInREQ.store(0);       
      bytesInRSP.store(0);       
      bytesAppend.store(0);      
      bytesAppendVector.store(0);
    }
  };
}


#endif
