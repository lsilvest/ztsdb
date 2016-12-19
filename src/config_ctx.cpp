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


#include <fstream>
#include <sstream>
#include <cctype>
#include <string>
#include "config_ctx.hpp"
#include "config_parser/cfglexer.hpp"
#include "config_parser/cfgparser.hpp"
#include "logging.hpp"


extern zlog::Logger lg;


void cfg::CfgCtx::add(const cfg::kv_t& kv) {
  if (cfgmap.insert(kv.first, kv.second) < 0) {
    std::string k;
    k.resize(kv.first.size());
    std::transform(kv.first.begin(), kv.first.end(), k.begin(), tolower);
    cfgmap.set(kv.first, kv.second);
  }
}


int cfg::CfgCtx::parsefile(const std::string& filename) {
  file = filename;
  std::ifstream t(filename);
  if (!t) {
    throw cfg::CfgCtx::NoConfigFile();
  }
  std::stringstream buffer;
  buffer << t.rdbuf();
  buffer << std::endl;
  return parse(buffer.str());  
}


int cfg::CfgCtx::parse(const std::string& s) {
  yyscan_t scanner;     
  cfgyylex_init(&scanner);
  //scan_begin(scanner);
  YY_BUFFER_STATE state = cfgyy_scan_string(s.c_str(), scanner);

  cfgyy::parser p(*this, scanner);
  // p.set_debug_level(1);
  
  int res = p.parse();

  cfgyy_delete_buffer(state, scanner);

  cfgyylex_destroy(scanner);

  //scan_end(scanner);
  return res;
}


void cfg::CfgCtx::error(cfgyy::location const& l, std::string const& m) {
  std::stringstream ss;
  ss << l << ": " << m;
  lg.log(zlog::SV_ERROR, ss.str().c_str());
}


const void cfg::read(cfg::CfgMap& cfgmap, const fsys::path& p) {
  cfg::CfgCtx cfgc(cfg::cfgmap);
  cfgc.parsefile(p.c_str());
}
