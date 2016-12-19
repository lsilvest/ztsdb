// Copyright (C) 2015 Leonardo Silvestri
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

#ifndef TEST_UTILS_HPP
#define TEST_UTILS_HPP


#include <regex>
#include <ftw.h>
#include <fstream>
#undef INFO
#include "logging.hpp"


// helpers to wipe out a directory:
inline int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
  return remove(fpath);
}

inline int cleandir(const char* d) {
  return nftw(d, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}


zlog::Logger lg;


// to be used as a fixture (see http://crpcut.sourceforge.net/1.9.3/test_fixtures.html).
struct log_to_file {
  log_to_file() {
    auto f = fopen("test.log", "w+"); // truncate file
    if (f == nullptr) {
      std::cout << "log_to_file() open: " << strerror(errno) << std::endl;
    }
    fclose(f);
    lg.init("test.log", "America/New_York");
    lg.setLevel(zlog::SV_DEBUG);
  }
  ~log_to_file() {
    lg.uninit();
    std::remove("test.log");
  }
};


// looks through the log file and returns 'true' is a match is found
// for 's', 'false' otherwise.
inline bool matchLog(const std::string& s) {
  lg.uninit();                  // this will close test.log
  std::string line;
  std::ifstream ifs("test.log");
  if (!ifs.is_open()) {
    throw std::out_of_range("can't open test.log");
  }
  while (getline(ifs, line)) {
    std::cout << line << std::endl;
    if (line.find(s) != std::string::npos) {
      std::cout << "found " << s << " in line " << line << std::endl;
      return true;
    }
  }
  return false;
}

// runs Rscript on a piece or R code and strips the result of the
// usual [1] that R prepends
inline std::string runR(const std::string& command) {
  std::cout << command << std::endl;
  FILE* f = popen(("Rscript -e " "'" + command + "'").c_str(), "r");
  char* lineptr = nullptr;
  size_t dummy = 10;
  ssize_t n = getline(&lineptr, &dummy, f);
  if (n < 0) {
    throw std::system_error(std::error_code(errno, std::system_category()), "runR: getline");
  }
  std::regex r("\\[\\d+\\]\\s([\\-a-zA-Z0-9]+)\n");
  std::cmatch sm;    // same as std::match_results<string::const_iterator> sm;
  std::regex_match(lineptr, sm, r);
  if (sm.size() != 2) {
    throw std::domain_error("runR, regex failed to find output");
  }
  auto res = std::string(sm[1]);
  free(lineptr);
  return res;
}

#endif
