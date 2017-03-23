// (C) 2017 Leonardo Silvestri
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


#ifndef ZCPP_ZTS_H
#define ZCPP_ZTS_H


#include "array.hpp"
#include "base_types.hpp"
#include "misc.hpp"
#include "zts.hpp"
#include "zcpp.hpp"
#include "globals.hpp"


/// Provides a C++ interface to append data to arrays and time
/// series. This interface will send messsages to a ztsdb
/// instance. These messages bypass the interpreter, so they have
/// decent performance.


namespace arr {
  Global::buflen_pair make_append_msg(const std::vector<std::string>& name, const arr::zts& z);
}



#endif
