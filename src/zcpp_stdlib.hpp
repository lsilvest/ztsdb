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


#ifndef ZCPP_STDLIB_H
#define ZCPP_STDLIB_H


#include <vector>
#include "globals.hpp"

namespace arr {
  
  Global::buflen_pair make_append_msg(const std::string& name, 
                                      const std::vector<Global::dtime>& idx, 
                                      const std::vector<double>& v);
  
}

#endif
