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


#ifndef CONFIG_CTX_HPP
#define CONFIG_CTX_HPP


#include <map>
#include <boost/filesystem.hpp>
#include "juice/variant.hpp"
#include "config.hpp"
#include "config_parser/cfgparser.hpp"


// this tells the lexer to replace its own definition of yylex by ours (so we can pass in 
// the CfgCtx parameter which is what makes it reentrant):
#undef YY_DECL
#define YY_DECL cfgyy::parser::symbol_type cfgyylex(cfg::CfgCtx& ctx, yyscan_t yyscanner)
YY_DECL;


namespace fsys = boost::filesystem;


namespace cfg {

  /// Read a config file and set a few defaults for undefined
  /// variables when they can only be known at runtime.
  const void read(CfgMap& cfgmap, const fsys::path& p);
  

  struct CfgCtx {

    struct NoConfigFile : std::exception { };

    CfgCtx(CfgMap& cfgmap_p) : cfgmap(cfgmap_p) { }
    
    void add(const kv_t& kv);
    
    /// Parse a file of key-values with format key=value. Value is
    /// specified as an integer, a double or a string (possibly
    /// quoted) which all follow C syntax.
    int parsefile(const std::string& filename);

    /// Parse a string of key-values. The string must be terminated by
    /// a new-line.
    int parse(const std::string& s);

    void error(const cfgyy::location& l, const std::string& m);
    
    // The name of the file being parsed.
    // Used later to pass the file name to the location tracker.
    std::string file;

    cfgyy::parser::location_type yylloc;

    std::string string_literal;
    cfgyy::parser::location_type string_literal_yylloc;
  private:
    CfgMap& cfgmap;
  };
}

#endif
