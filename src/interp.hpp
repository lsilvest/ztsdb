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


/// \file 
/// Defines 'Kont', the fundamental structure on which the
/// interpreter is built, and 'step', the function that invokes the
/// interpreter.


#ifndef INTERP_HPP
#define INTERP_HPP

#include <string>
#include <stdexcept>
#include "ast.hpp"
#include "valuevar.hpp"
#include "env.hpp"


/// \cond
namespace zcore {
  struct InterpCtx;
}
/// \endcond


/// Struct and functions implementing the interpreter.
namespace interp {

  /// A continuation is simply what still needs to be evaluated. Most
  /// notably, continuations contain a variable 'var' for which to
  /// evaluate the continuation, the code to be evaluated ('control'),
  /// and are chained via 'next'.
  struct Kont { 

    /// Modifies the type of assignment that is performed. When we
    /// finish the evaluation of the 'control' in a continuation, we
    /// assign the result to the 'var' of the next continuation; the
    /// assignment is modified according to these values. For example
    /// type 'GLOBAL' will make the assignment in the global
    /// environment rather than in the local one.
    enum AssignType : uint64_t { 
      NORMAL   = 0x00000000,    ///< assign in the local frame
      GLOBAL   = 0x00000001,    ///< assign in the global frame
      ELLIPSIS = 0x00000010,    ///< result is an argument that is part 
                                ///  of an ellipsis
      WHILE    = 0x00000100,    ///< result is the eval of a while loop
      COND     = 0x00001000,    ///< result is the eval of a while condition
      ARG      = 0x00010000,    ///< result is the eval of a function arg
      SILENT   = 0x00100000,    ///< result of assignment will not be printed
      END      = 0x01000000,    ///< indicates the last assignment
      REF      = 0x10000000     ///< indicates the pass-by-reference
    };

    Kont() = delete;

    const E* var;               ///< Variable for which we are
                                ///  evaluating (but remember we
                                ///  evaluate for next->var).
    const E* control;           ///< The expression to evaluate.
    shared_ptr<BaseFrame> r;    ///< Pointer to frame in which to
                                ///  evaluate this continuation.
    shared_ptr<Kont> next;      ///< Next continuation.
    uint64_t atype;             ///< Assignment type.

    /// Return a string containing a printable representation of the
    /// continuation.
    inline operator string() const { 
      stringstream ss;

      // Print out all the assignment flags.
      auto atype_to_string = [](uint64_t a) {
        string s;
        if (a == AssignType::NORMAL) {
          s = "N";
        }
        else {
          if (a & GLOBAL)   s += "G,";
          if (a & ELLIPSIS) s += "E,";
          if (a & WHILE)    s += "W,";
          if (a & COND)     s += "C,";
          if (a & ARG)      s += "A,";
          if (a & SILENT)   s += "S,";
          if (a & END)      s += "END,";
          if (a & REF)      s += "REF,";
          if (s.length()) {
            s = s.substr(0, s.length() - 1);
          }
        }
        return s; };

      ss << (next && next->var ? to_string(*next->var) : "") << "="
         << (control ? to_string(*control) : "nullptr")
         << " ["
         << " a:" << atype_to_string(atype)
         << " r:" << r << dec 
         << "]"
         << " -> " << (next ? string(*next) : "nullptr"); 
       return ss.str();
    }
  };

  /// Evaluate the current continuation and return (i.e. step through
  /// one continuation and then return). From the point of view of the
  /// interpreter, the current state and what needs to be interpreted
  /// is entirely defined in the continuation 'k' and the frame
  /// stack. The calling context 'ic' is used when the interpreter is
  /// required to send out a query (the context is needed to generate
  /// the message with the appropriate identifiers, etc.).
  shared_ptr<Kont> step(shared_ptr<Kont>& k, 
                        vector<shpfrm>& frameStack, 
                        zcore::InterpCtx& ic);


  shared_ptr<Kont> buildElChain(const ElNode* eln, 
                                unsigned n, 
                                shared_ptr<BaseFrame> r, 
                                shared_ptr<Kont>& k);

} // end namespace interp


#endif
