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



/// Provides all the configuration names and default values when they
/// make sense, 'Empty' otherwise.

#include "config.hpp"


using namespace std::literals;


cfg::CfgMap::CfgMap() :
  m{ 
     { "address"s,             ""s                      }, 
     { "port"s,                0L                       },  
     { "timezone.path"s,       "/usr/share/zoneinfo/"s  },
     { "logfile.path"s,        "/tmp"s                  },
     { "logfile.name"s,        "ztsdb.log"s             },
     { "config.path"s,         ""s                      },
     { "log.level"s,           "INFO"s                  },
     { "init.code"s,           ""s                      },
     { "prompt"s,              "> "s                    },
                                                    
     { "timezone"s,            "UTC"s                   },
     { "digits"s,              7L                       }, 
     { "scipen"s,              0L                       }, 
     { "width"s,               100L                     }, 
     { "max.print"s,           99999L                   },    
                              
     { "data.q.size"s,         100000L                  },  
     { "sig.q.size"s,          50L                      },  
     { "commbuf.ttl.secs"s,    60L                      },  
     { "in.req.ttl.secs"s,     180L                     },  
     { "in.rsp.ttl.secs"s,     180L                     }  
   }
 {
   
 }


int cfg::CfgMap::insert(const std::string& s, cfg::CfgVariant value) {
  auto i = m.insert(std::make_pair(s, value));
  if (!i.second) {
    return -1;
  }
  return 1;
}


void cfg::CfgMap::set(const std::string& s, cfg::CfgVariant value)
{
  auto i = m.find(s);
  if (i == m.end()) {
    throw std::out_of_range("can't find key");
  }
  else {
    // cast a double to an int if necessary, but all other type
    // mismatches are an error (because the initial type in the map,
    // as defined in the 'Cfg' constructor above, determines the type
    // of the variable).
    if (i->second.which() == cfg::INT && value.which() == cfg::DOUBLE) {
      i->second = static_cast<int64_t>(Juice::get<double>(value));
    }
    else if (i->second.which() != value.which()) {
      throw std::invalid_argument("type mismatch");
    }
    else {
      i->second = value;
    }
  }
}


cfg::CfgVariant cfg::CfgMap::get(const std::string& s) const
{
  return m.at(s);
}


cfg::CfgVariant cfg::CfgMap::operator[](const std::string& s) { 
  return m[s]; 
}
