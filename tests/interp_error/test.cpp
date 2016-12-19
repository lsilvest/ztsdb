// -*- compile-command: "make -k test" -*-

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


#include <crpcut.hpp>
#include "interp_error.hpp"


// constructors ------------------
TEST(test_error_middle) {
  const std::string buf("aa 11 a\n"
                        "bb 22 b\n"
                        "cc 33 c\n"
                        "dd 44 d\n");
  
  yy::location loc(yy::position{"filename.R", 2, 4, std::make_shared<const std::string>(buf)},
                   yy::position{"filename.R", 2, 4, std::make_shared<const std::string>(buf)});
  
  auto res = interp::errorLines(loc, 1);
  auto expectedRes = std::string("1: aa 11 a\n"
                                 "2: bb 22 b\n"
                                 "      ^");
  ASSERT_TRUE(res == expectedRes);
}
TEST(test_error_beginning) {
  const std::string buf("aa 11 a\n"
                        "bb 22 b\n"
                        "cc 33 c\n"
                        "dd 44 d\n");
  
  yy::location loc(yy::position{"filename.R", 1, 4, std::make_shared<const std::string>(buf)},
                   yy::position{"filename.R", 1, 4, std::make_shared<const std::string>(buf)});
  
  auto res = interp::errorLines(loc, 1);
  auto expectedRes = std::string("1: aa 11 a\n"
                                 "      ^");
  ASSERT_TRUE(res == expectedRes);
}
TEST(test_error_end) {
  const std::string buf("# test comment\n"
                        "f <- function(x) x\n"
                        "f(1, 2)\n");

  yy::location loc(yy::position{"filename.R", 3, 6, std::make_shared<const std::string>(buf)},
                   yy::position{"filename.R", 3, 6, std::make_shared<const std::string>(buf)});
  
  auto res = interp::errorLines(loc, 1);
  auto expectedRes = std::string("2: f <- function(x) x\n"
                                 "3: f(1, 2)\n"
                                 "        ^");
  ASSERT_TRUE(res == expectedRes);
}
TEST(test_error_0_lines_before) {
  const std::string buf("# test comment\n"
                        "f <- function(x) x\n"
                        "f(1, 2)\n");

  yy::location loc(yy::position{"filename.R", 3, 6, std::make_shared<const std::string>(buf)},
                   yy::position{"filename.R", 3, 6, std::make_shared<const std::string>(buf)});
  
  auto res = interp::errorLines(loc, 0);
  auto expectedRes = std::string("3: f(1, 2)\n"
                                 "        ^");
  ASSERT_TRUE(res == expectedRes);
}
TEST(test_error_circ) {
  // test the circular buffer by getting a line far enough to force a
  // wrap.
  const std::string buf("aa 11 a\n"
                        "bb 22 b\n"
                        "cc 33 c\n"
                        "dd 44 d\n"
                        "ee 55 e\n"
                        "ff 66 f\n"
                        "gg 77 g\n");
  
  yy::location loc(yy::position{"filename.R", 7, 4, std::make_shared<const std::string>(buf)},
                   yy::position{"filename.R", 7, 4, std::make_shared<const std::string>(buf)});
  
  auto res = interp::errorLines(loc, 1);
  auto expectedRes = std::string("6: ff 66 f\n"
                                 "7: gg 77 g\n"
                                 "      ^");
  ASSERT_TRUE(res == expectedRes);
}
TEST(test_error_double_digits) {
  // test the circular buffer by getting a line far enough to force a
  // wrap.
  const std::string buf("aa 11 a\n"
                        "bb 22 b\n"
                        "cc 33 c\n"
                        "dd 44 d\n"
                        "ee 55 e\n"
                        "ff 66 f\n"
                        "gg 77 g\n"
                        "hh 88 h\n"
                        "ii 99 i\n"
                        "jj 00 j\n"
                        "kk 11 k\n"
);
  
  yy::location loc(yy::position{"filename.R", 10, 1, std::make_shared<const std::string>(buf)},
                   yy::position{"filename.R", 10, 1, std::make_shared<const std::string>(buf)});
  
  auto res = interp::errorLines(loc, 1);
  auto expectedRes = std::string(" 9: ii 99 i\n"
                                 "10: jj 00 j\n"
                                 "    ^");
  ASSERT_TRUE(res == expectedRes);
}
TEST(test_error_circ_span) {
  const std::string buf("aa 11 a\n"
                        "bb 22 b\n"
                        "cc 33 c\n"
                        "dd 44 d\n"
                        "ee 55 e\n"
                        "ff 66 f\n"
                        "gg 77 g\n");
  
  yy::location loc(yy::position{"filename.R", 7, 4, std::make_shared<const std::string>(buf)},
                   yy::position{"filename.R", 7, 6, std::make_shared<const std::string>(buf)});

  auto res = interp::errorLines(loc, 1);
  auto expectedRes = std::string("6: ff 66 f\n"
                                 "7: gg 77 g\n"
                                 "      ^^");
  ASSERT_TRUE(res == expectedRes);
}
TEST(test_error_multiline) {
  const std::string buf("aa 11 a\n"
                        "bb 22 b\n"
                        "cc 33 c\n"
                        "dd 44 d\n"
                        "ee 55 e\n"
                        "ff 66 f\n"
                        "gg 77 g\n");
  
  yy::location loc(yy::position{"filename.R", 7, 4, std::make_shared<const std::string>(buf)},
                   yy::position{"filename.R", 7, 6, std::make_shared<const std::string>(buf)});

  auto res = interp::errorLines(loc, 3);
  auto expectedRes = std::string("4: dd 44 d\n"
                                 "5: ee 55 e\n"
                                 "6: ff 66 f\n"
                                 "7: gg 77 g\n"
                                 "      ^^");
  ASSERT_TRUE(res == expectedRes);
}
TEST(test_error_multiline_more_available) {
  const std::string buf("aa 11 a\n"
                        "bb 22 b\n"
                        "cc 33 c\n"
                        "dd 44 d\n"
                        "ee 55 e\n"
                        "ff 66 f\n"
                        "gg 77 g\n");
  
  yy::location loc(yy::position{"filename.R", 7, 1, std::make_shared<const std::string>(buf)},
                   yy::position{"filename.R", 7, 3, std::make_shared<const std::string>(buf)});

  auto res = interp::errorLines(loc, 10);
  auto expectedRes = std::string("1: aa 11 a\n"
                                 "2: bb 22 b\n"
                                 "3: cc 33 c\n"
                                 "4: dd 44 d\n"
                                 "5: ee 55 e\n"
                                 "6: ff 66 f\n"
                                 "7: gg 77 g\n"
                                 "   ^^");
  ASSERT_TRUE(res == expectedRes);
}
TEST(test_error_throw_missing_line_column) {
  const std::string buf("aa 11 a\n"
                        "bb 22 b\n"
                        "cc 33 c\n"
                        "dd 44 d\n");

  yy::location loc(yy::position{"filename.R", 0, 4, std::make_shared<const std::string>(buf)},
                   yy::position{"filename.R", 1, 4, std::make_shared<const std::string>(buf)});
  
  ASSERT_THROW(interp::errorLines(loc, 1), std::out_of_range, "missing line/column position");
}
TEST(test_error_throw_no_text) {
  yy::location loc(yy::position{"filename.R", 1, 4, nullptr},
                   yy::position{"filename.R", 1, 4, nullptr});
  
  ASSERT_THROW(interp::errorLines(loc, 1), std::out_of_range, "no text in location buffer");
}
TEST(test_error_throw_missing_nl) {
  const std::string buf("aa 11 a\n"
                        "bb 22 b\n"
                        "cc 33 c\n"
                        "dd 44 d");
  
  yy::location loc(yy::position{"filename.R", 1, 4, std::make_shared<const std::string>(buf)},
                   yy::position{"filename.R", 1, 4, std::make_shared<const std::string>(buf)});
  
  ASSERT_THROW(interp::errorLines(loc, 1), std::out_of_range, "buffer doesn't end with a newline");
}


int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
