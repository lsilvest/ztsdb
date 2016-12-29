// -*- compile-command: "make -j -k test" -*-

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


#include "../interp_setup.hpp"

// -- while
TEST(control_while_test) {
  auto eout = parse("a <- 1.0; while (a < 4) a <- a+1; a  \n");
  ASSERT_TRUE(eval(eout) == val::make_array(4.0));
}
TEST(control_while_test_while_evals_to_null) {
  auto eout = parse("a <- 1.0; b <- while (a < 4) a <- a+1  \n");
  ASSERT_TRUE(eval(eout) == val::Value(val::VNull()));  
}
TEST(control_while_non_atomic) {
  auto eout = parse("a <- 1.0; b <- while ({x <- a; x < 4}) a <- a+1  \n");
  ASSERT_TRUE(eval(eout) == val::Value(val::VNull()));  
}
TEST(control_while_assign) {
  auto eout = parse("a <- 4.0; while (b <- a) a <- a-1; b  \n");
  ASSERT_TRUE(eval(eout) == val::make_array(0.0));
}
TEST(control_while_non_atomic_cond_and_body) {
  auto eout = parse("a <- 1.0; while ({x <- a; x < 4}) { a <- a-1; a <- a+2 }; a  \n");
  ASSERT_TRUE(eval(eout) == val::make_array(4.0));  
}
TEST(control_while_as_function_result) {
  auto eout = parse("b <- 1.0; f <- function(a) while (a < 4) {a <- a + 1; b <- b + 1 }; f(1.0); b \n");
  ASSERT_TRUE(eval(eout) == val::make_array(1.0));  
}
TEST(control_while_nested) {
  auto eout = parse("SZ  <- 10\n"
                    "REP <- 10\n"
                    "fa <- c(1.0:SZ)\n"
                    "i <- j <- 1\n"
                    "while (i<=SZ) {\n"
                    "    while (j <= REP) {\n"
                    "        fa[j] <- fa[i] + 1\n"
                    "        j <- j + 1\n"
                    "    }\n"
                    "    i <- i + 1\n"
                    "}\n");
  ASSERT_TRUE(eval(eout) == val::Value(val::VNull()));  
}
TEST(control_if_cond_bool_true) {
  auto eout = parse("if (TRUE) 1.0 \n");
  ASSERT_TRUE(eval(eout) == val::make_array(1.0));  
}
TEST(control_if_cond_bool_false) {
  auto eout = parse("if (FALSE) 1.0 \n");
  ASSERT_TRUE(eval(eout) == val::VNull());  
}
TEST(control_ifelse_cond_bool_true) {
  auto eout = parse("if (TRUE) 1.0 else 2.0 \n");
  ASSERT_TRUE(eval(eout) == val::make_array(1.0));  
}
TEST(control_ifelse_cond_bool_false) {
  auto eout = parse("if (FALSE) 1.0 else 2.0 \n");
  ASSERT_TRUE(eval(eout) == val::make_array(2.0));  
}
TEST(control_ifelse_cond_int_true) {
  auto eout = parse("if (1) 1.0 else 2.0 \n");
  ASSERT_TRUE(eval(eout) == val::make_array(1.0));  
}
TEST(control_ifelse_cond_int_false) {
  auto eout = parse("if (0) 1.0 else 2.0 \n");
  ASSERT_TRUE(eval(eout) == val::make_array(2.0));  
}
TEST(control_ifelse_cond_double_true) {
  auto eout = parse("if (-1.0) 1.0 else 2.0 \n");
  ASSERT_TRUE(eval(eout) == val::make_array(1.0));  
}
TEST(control_ifelse_cond_double_false) {
  auto eout = parse("if (0.0) 1.0 else 2.0 \n");
  ASSERT_TRUE(eval(eout) == val::make_array(2.0));  
}
TEST(control_ifelse_cond_string) {
  auto eout = parse("if (\"\") 1.0 else 2.0 \n");
  ASSERT_THROW(eval(eout), std::range_error, 
               "conversion not defined for array of character to array of logical");  
}
TEST(control_ifelse_cond_matrix) {
  auto eout = parse("if (matrix(TRUE,2,2)) 1.0 else 2.0 \n");
  ASSERT_THROW(eval(eout), std::out_of_range, "expecting scalar value");  
}
TEST(control_ifelse_cond_complex_true) {
  auto eout = parse("if ({f <- function(x) x; (f(1) + f(1)) == 2 }) 1.0 else 2.0 \n");
  ASSERT_TRUE(eval(eout) == val::make_array(1.0));  
}
TEST(control_ifelse_cond_complex_false) {
  auto eout = parse("if ({f <- function(x) x; (f(1) + f(1)) != 2 }) 1.0 else 2.0 \n");
  ASSERT_TRUE(eval(eout) == val::make_array(2.0));  
}
TEST(control_ifelse_body_complex_true) {
  auto eout = parse("if (TRUE) { f <- function(x) x; y <- f(1) + f(1); y } else 2.0 \n");
  ASSERT_TRUE(eval(eout) == val::make_array(2.0));  
}
TEST(control_ifelse_body_complex_false) {
  auto eout = parse("if (FALSE) 1 else { f <- function(x) x; y <- f(1) + f(1); y+1 } \n");
  ASSERT_TRUE(eval(eout) == val::make_array(3.0));  
}
TEST(control_ifelse_body_empty_true) {
  auto eout = parse("if (TRUE) {} else {} \n");
  ASSERT_TRUE(eval(eout) == val::VNull());  
}
TEST(control_ifelse_body_empty_false) {
  auto eout = parse("if (FALSE) {} else {} \n");
  ASSERT_TRUE(eval(eout) == val::VNull());
}

int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
