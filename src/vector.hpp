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


#ifndef VECTOR_HPP
#define VECTOR_HPP

#include "vector_base.hpp"

namespace arr {
  // define numbers for types; the vector type needs these in order to
  // know how to read a vector from a file.
  TYPE_NB(double, 0);
  TYPE_NB(bool, 1);
  TYPE_NB(Global::dtime, 2);
  TYPE_NB(arr::zstring, 3);
  TYPE_NB(Global::duration, 4);
  TYPE_NB(tz::interval, 5);
  TYPE_NB(tz::period, 6);

  // define names for types so that we can print out more meaningful
  // error messages:
  TYPE_NAME(double, "double");
  TYPE_NAME(bool, "logical");
  TYPE_NAME(Global::dtime, "time");
  TYPE_NAME(arr::zstring, "string");
  TYPE_NAME(Global::duration, "duration");
  TYPE_NAME(tz::interval, "interval");
  TYPE_NAME(tz::period, "period");




}

// #include "vector_bool.hpp"



#endif
