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


#ifndef CONFIG_HPP
#define CONFIG_HPP


#include <map>
#include "juice/variant.hpp"


namespace cfg {

  typedef Juice::Variant<int64_t, 
                         double, 
                         std::string> CfgVariant;

  enum ct_type { INT, DOUBLE, STRING };

  struct CfgMap {
    CfgMap();

    int insert(const std::string& s, CfgVariant value);
    void set(const std::string& s, CfgVariant value);
    CfgVariant get(const std::string& s) const;
    
    typedef std::map<std::string, CfgVariant> configmap_t;
    typedef std::pair<std::string, CfgVariant> kv_t;
    
    configmap_t::iterator begin() { return m.begin(); }
    configmap_t::iterator end() { return m.end(); }

    CfgVariant operator[](const std::string& s);

  private:
    configmap_t m;
  };

  typedef std::map<std::string, CfgVariant> configmap_t;
  typedef std::pair<std::string, CfgVariant> kv_t;

  extern CfgMap cfgmap;

}

#endif
