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


#include <vector>
#include <iostream>
#include <sstream>
#include "interp_error.hpp"


using namespace std::string_literals;


static std::string buildLineWithCarets(unsigned cbegin, unsigned cend) {
  if (cbegin > cend) {
    throw std::out_of_range("cend must be >= cbegin"); 
  }
  if (cbegin == cend) ++cend;
  return std::string(cbegin-1, ' ') + std::string(cend - cbegin, '^');
}


/// Figure out how many digits are need to display a number in base
/// 10.
static unsigned computeWidth(unsigned u) {
  unsigned digits = 1;
  while (u /= 10) ++digits;
  return digits;
}

  
std::string interp::errorLines(const yy::location& loc, unsigned lnBefore) 
{
  if (loc.begin.line == 0 || loc.begin.column == 0 || 
      loc.end.line   == 0 || loc.end.column   == 0) {
    throw std::out_of_range("missing line/column position"); 
  }
  if (!loc.begin.s) {
    throw std::out_of_range("no text in location buffer"); 
  }
  if ((*loc.begin.s)[loc.begin.s->size()-1] != '\n') {
    throw std::out_of_range("buffer doesn't end with a newline"); 
  }
  if (lnBefore > loc.begin.line - 1) {
    lnBefore = loc.begin.line - 1;
  }

  // Define a circular buffer to hold the pointers to the beginning of
  // lines:
  const unsigned circSz = lnBefore + 2;
  std::vector<const char*> circ(circSz);
  unsigned circIdx = 0u;

  const char* c = loc.begin.s->c_str();
  
  circ[circIdx] = c;
  unsigned nlNb = 0u;
  while (nlNb < loc.begin.line && *c != 0) {
    if (*c == '\n') {
      circIdx = (circIdx + 1) % circSz;
      circ[circIdx] = c + 1;
      //      std::cout << "marked at: " << circ[circIdx] << std::endl;
      ++nlNb;
    }
    c++;
  }
  //  std::cout << "circIdx:" << circIdx << ", nlNb: " << nlNb << std::endl;

  std::stringstream ss;
  if (nlNb == loc.begin.line) {
    // copy out the lines to build the result string:
    unsigned width = computeWidth(loc.begin.line);
    for (unsigned i=0; i<lnBefore+1; ++i) {
      unsigned idx = ((circIdx >= lnBefore + 1) ? 
                      circIdx - 1 - lnBefore + i : (circIdx + circSz) - 1 - lnBefore + i) % circSz;
      ss.width(width);
      ss << loc.begin.line - lnBefore + i << ": ";
      ss << std::string(circ[idx], circ[(idx + 1) % circSz] - circ[idx]);
    }
    ss << std::string(width + 2, ' ');
    ss << buildLineWithCarets(loc.begin.column, 
                              loc.begin.line == loc.end.line ? loc.end.column : loc.begin.column);
  }
  else if (nlNb == loc.begin.line-1) {
    // this is OK when it's unexpected end of file error
  }
  else {
    throw std::out_of_range("unable to find line "s + std::to_string(loc.begin.line));
  }

  return ss.str();
}
