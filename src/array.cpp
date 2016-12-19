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


#include "array.hpp"


// build the names in each dimension:
void arr::buildNames(vector<unique_ptr<Dname>>& names,
                     const Vector<idx_type> dim, 
                     const AllocFactory& allocf,
                     const vector<Vector<zstring>> names_p) 
{
  if (names_p.size() && names_p.size() != dim.size()) {
    throw range_error("length of 'dimnames' [" + std::to_string(names_p.size()) + 
                      "] must match that of 'dims' [" + std::to_string(dim.size()) + "]");
  }
  for (idx_type j=0; j<dim.size(); ++j) {
    if (names_p.size() && names_p[j].size() > 0) {
      if (names_p[j].size() != dim[j]) {
        throw range_error("length of 'dimnames' [" + to_string(j+1) + 
                          "] not equal to array extent");
      }
      names.emplace_back(
        make_unique<Dname>(dim[j], 
                           Vector<zstring>(names_p[j],
                                           allocf.get("names" + std::to_string(j)))));
    } else {
      // build even if no names were provided; this makes adding
      // names later much easier; the cost is a scalar assignment:
      names.emplace_back(
        make_unique<Dname>(dim[j],
                           Vector<zstring>(0, 
                                           getInitValue<zstring>(),
                                           allocf.get("names" + std::to_string(j)))));
    }
  }
}
