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


#ifndef CSV_HPP
#define CSV_HPP


#include "array.hpp"
#include "zts.hpp"
#include "cow_ptr.hpp"


namespace arr {

  // array:
  template<typename T> 
  cow_ptr<Array<T>> readcsv_array(const string& file,
                                  bool header,
                                  const char sep,
                                  const string& mmapfile);
  template<typename T>                           
  void writecsv_array(const Array<T>& a, const string& file, bool header, const char sep);

  // zts:
  cow_ptr<zts> readcsv_zts(const string& file, bool header, const char sep, const string& mmapfile);
  void writecsv_zts(const zts& z, const string& file, bool header, const char sep);

}


#endif 
