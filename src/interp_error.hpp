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


#ifndef INTERP_ERROR_HPP
#define INTERP_ERROR_HPP


#include <string>
#include "main_parser/location.hpp"


namespace interp {

  struct FutureException : public std::out_of_range {
    FutureException(const std::string& what_p) : std::out_of_range(what_p) { }
  };

  struct EvalException : public std::out_of_range {
    EvalException(const std::string& what_p, const yy::location& loc_p) : 
      std::out_of_range(what_p), loc(loc_p) { }

    const yy::location loc;
  };

  /// Builds a string extracting from 'loc' the 'nlinesBefore' and
  /// 'nlinesAfter' together with the erorr line and a line underneath
  /// pointing to the error.
  std::string errorLines(const yy::location& loc, unsigned nlinesBefore);


} // end namespace interp

#endif
