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


#ifndef ZONE_HPP
#define ZONE_HPP


#include <memory>
#include <map>
#include <unordered_map>
#include <set>
#include <string>
#include <boost/filesystem.hpp>
#include "tz.hpp"
#include "../globals.hpp"


namespace tz {

  namespace fsys = boost::filesystem;

  struct Zone {
    friend std::string to_string(const Zone& z);
    friend std::string summary(const Zone& z);
    friend struct Zones;

    Zone(fsys::path p);         // will throw

    std::pair<Global::dtime::duration, const char*> getoffset(Global::dtime dt) const;

    Global::dtime::duration getoffset(Global::dtime dt, int& pos) const;
    Global::dtime::duration getoffset(Global::dtime dt, int& pos, int dir) const;

    std::set<Global::dtime::duration> getReverseOffset(Global::dtime dt) const;

  private:
    // Reverse lookup of offsets of times that already have a time
    // zone offset applied. The set can contain 0 elements in the case
    // of an impossible date representation, 1 element if there is no
    // ambiguity, and 2 elements when there is two potential offsets
    // that can be applied:
    std::map<time_t, std::set<int>> rl;
    // the state as read by the iana-provided 'tzload' function:
    std::unique_ptr<tz::state> s;

    void buildRl();             // build the reverse lookup map
  }; 
  

  struct Zones {
    friend std::string to_string(const Zones& zs);

    Zones();
    Zones(fsys::path p);        // will throw

    void init(fsys::path p);    // will throw

    const Zone& find(const std::string& s) const; 
    // Global::dtime::duration getoffset(const std::string& abbrev) const; 

  private:
    std::unordered_map<std::string, Zone> m; 
  };


  std::string to_string(const Zone& z);
  std::string summary(const Zone& z);
  std::string to_string(const Zones& zs);

}



#endif
