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



#include <crpcut.hpp>
#include "parser_ctx.hpp"
#include "anf.hpp"
#include "timezone/zone.hpp"


tz::Zones tzones("/usr/share/zoneinfo");            // LLL this is platform dependent...


// create encoder and use the parser to create the expression
static unique_ptr<El> parse(const char* buf) {
  ParserCtx pctx;
  //pctx.trace_parsing  = true;
  if (pctx.parse(std::make_shared<const std::string>(buf)) != 0) {
    // fail the section or report an error LLL
    cerr << "parser error" << endl;
  }
  return std::move(pctx.prog);
}


// ----- this set should not be modified by normalization
TEST(anf_null) {
  auto eout = parse("NULL\n");
  auto aout = std::unique_ptr<El>(eout->clone());
  anf::convertToANF(aout.get());
  ASSERT_TRUE(isEqual(aout.get(), eout.get())); 
}
TEST(anf_simple_el) {
  auto eout = parse("{ x; y }\n");
  auto aout = std::unique_ptr<El>(eout->clone());
  anf::convertToANF(aout.get());
  cout << to_string(*eout) << endl;
  cout << to_string(*aout) << endl;
  ASSERT_TRUE(isEqual(aout.get(), eout.get())); 
}
TEST(anf_simple_funcall) {
  auto eout = parse("f(x)\n");
  auto aout = std::unique_ptr<El>(eout->clone());
  anf::convertToANF(aout.get());
  cout << to_string(*eout) << endl;
  cout << to_string(*aout) << endl;
  ASSERT_TRUE(isEqual(aout.get(), eout.get())); 
}
TEST(anf_simple_exprsublist) {
  auto eout = parse("f(x=1, y=2) \n");
  auto aout = std::unique_ptr<El>(eout->clone());
  anf::convertToANF(aout.get());
  ASSERT_TRUE(isEqual(aout.get(), eout.get())); 
}
TEST(anf_simple_function) {
  auto eout = parse("function(x) x \n");
  auto aout = std::unique_ptr<El>(eout->clone());
  anf::convertToANF(aout.get());
  ASSERT_TRUE(isEqual(aout.get(), eout.get())); 
}
TEST(anf_multiarg_function) {
  auto eout = parse("function(x, y, z) x + y + z \n");
  auto aout = std::unique_ptr<El>(eout->clone());
  anf::convertToANF(aout.get());
  ASSERT_TRUE(isEqual(aout.get(), eout.get())); 
}
// ---- test correct modifications are made
TEST(anf_non_atomic_el) {
  auto eout = parse("g <- f(x)+f(y); y <- 2; z <- f(k) + 2\n");
  auto aout = std::unique_ptr<El>(eout->clone());
  anf::convertToANF(aout.get());
  ASSERT_TRUE(to_string(*aout) == 
              "{?anf1 <- f(x)\n"
              "?anf2 <- f(y)\n"
              "g <- (?anf1+?anf2)\n"
              "y <- 2\n"
              "?anf3 <- f(k)\n"
              "z <- (?anf3+2)}"); 
}
TEST(anf_nested_el) {
  auto aout = parse("x <- { f(x)+f(y); a } + { b <- g(x)+g(y); k(b) }\n");
  anf::convertToANF(aout.get());
  ASSERT_TRUE(to_string(*aout) == 
              "{?anf1 <- {{?anf2 <- f(x)\n"
              "?anf3 <- f(y)\n"
              "(?anf2+?anf3)}\n"
              "a}\n"
              "?anf4 <- {{?anf5 <- g(x)\n"
              "?anf6 <- g(y)\n"
              "b <- (?anf5+?anf6)}\n"
              "k(b)}\n"
              "x <- (?anf1+?anf4)}"); 
}
TEST(anf_binop_non_atomic_left) {
  auto aout = parse("f(1) + 2\n");
  anf::convertToANF(aout.get());
  cout << to_string(*aout) << endl;
  // not very pretty but variable names like "?anf1" will not parse,
  // so we can't easily build the resulting expression by calling
  // "parse". So we assume a fairly stable expression to string
  // conversion and test that; we do the same in the subsequent tests
  // too:
  ASSERT_TRUE(to_string(*aout) == 
              "{?anf1 <- f(1)\n"
              "(?anf1+2)}"); 
}
TEST(anf_binop_non_atomic_right) {
  auto aout = parse("2 + f(1)\n");
  anf::convertToANF(aout.get());
  ASSERT_TRUE(to_string(*aout) == 
              "{?anf1 <- f(1)\n"
              "(2+?anf1)}"); 
}
TEST(anf_binop_non_atomic_left_right) {
  auto aout = parse("f(1) + f(2)\n");
  anf::convertToANF(aout.get());
  ASSERT_TRUE(to_string(*aout) == 
              "{?anf1 <- f(1)\n"
              "?anf2 <- f(2)\n"
              "(?anf1+?anf2)}");
}
TEST(anf_binop_non_atomic_left_right_nested) {
  auto aout = parse("1 + 2 + f(1) + f(2)\n");
  anf::convertToANF(aout.get());
  ASSERT_TRUE(to_string(*aout) == 
              "{?anf1 <- f(1)\n"
              "?anf2 <- f(2)\n"
              "(((1+2)+?anf1)+?anf2)}");
}
TEST(anf_function_atomic) {
  auto aout = parse("function(x, y, z) x+y+z\n");
  anf::convertToANF(aout.get());
  ASSERT_TRUE(to_string(*aout) == "function(x, y, z) ((x+y)+z)");
}
TEST(anf_function_atomic_no_args) {
  auto aout = parse("function() 1\n");
  cout << to_string(*aout) << endl;
  anf::convertToANF(aout.get());
  ASSERT_TRUE(to_string(*aout) == "function() 1");
}
TEST(anf_function_non_atomic_body) {
  auto aout = parse("function(x) f(1) + f(2)\n");
  anf::convertToANF(aout.get());
  ASSERT_TRUE(to_string(*aout) == 
              "function(x) {?anf1 <- f(1)\n"
              "?anf2 <- f(2)\n"
              "(?anf1+?anf2)}");
}
TEST(anf_function_non_atomic_formlist_1arg) {
  auto aout = parse("function(x=f(1)+f(2)) x\n");
  anf::convertToANF(aout.get());
  ASSERT_TRUE(to_string(*aout) == 
              "function(x={?anf1 <- f(1)\n"
              "?anf2 <- f(2)\n"
              "(?anf1+?anf2)}) x");
}
TEST(anf_function_non_atomic_formlist_2args) {
  auto aout = parse("function(x=f(1)+f(2), y=f(3)+f(4)) x\n");
  anf::convertToANF(aout.get());
  ASSERT_TRUE(to_string(*aout) == 
              "function(x={?anf1 <- f(1)\n"
              "?anf2 <- f(2)\n"
              "(?anf1+?anf2)}, y={?anf3 <- f(3)\n"
              "?anf4 <- f(4)\n"
              "(?anf3+?anf4)}) x");
}
TEST(anf_function_non_atomic_formlist_2args_mixed) {
  auto aout = parse("function(x, y=f(1)+f(2)) NULL\n");
  anf::convertToANF(aout.get());
  ASSERT_TRUE(to_string(*aout) == 
              "function(x, y={?anf1 <- f(1)\n"
              "?anf2 <- f(2)\n"
              "(?anf1+?anf2)}) NULL");
}
TEST(anf_funcall_atomic_noargs) {
  auto aout = parse("f()\n");
  anf::convertToANF(aout.get());
  cout << to_string(*aout) << endl;
  ASSERT_TRUE(to_string(*aout) == 
              "f()");
}
TEST(anf_funcall_atomic_args) {
  auto aout = parse("f(1, 2, 3)\n");
  anf::convertToANF(aout.get());
  cout << to_string(*aout) << endl;
  ASSERT_TRUE(to_string(*aout) == 
              "f(1, 2, 3)");
}
TEST(anf_funcall_non_atomic_1arg) {
  auto aout = parse("f(f(1)+f(2))\n");
  anf::convertToANF(aout.get());
  cout << to_string(*aout) << endl;
  ASSERT_TRUE(to_string(*aout) == 
              "f({?anf1 <- f(1)\n"
              "?anf2 <- f(2)\n"
              "(?anf1+?anf2)})");
}
TEST(anf_funcall_non_atomic_2args) {
  auto aout = parse("f(f(1)+f(2), f(3)+f(4))\n");
  anf::convertToANF(aout.get());
  cout << to_string(*aout) << endl;
  ASSERT_TRUE(to_string(*aout) == 
              "f({?anf1 <- f(1)\n"
              "?anf2 <- f(2)\n"
              "(?anf1+?anf2)}, {?anf3 <- f(3)\n"
              "?anf4 <- f(4)\n"
              "(?anf3+?anf4)})");
}
TEST(anf_funcall_non_atomic_2args_named) {
  auto aout = parse("f(x=f(1)+f(2), y=f(3)+f(4))\n");
  anf::convertToANF(aout.get());
  cout << to_string(*aout) << endl;
  ASSERT_TRUE(to_string(*aout) == 
              "f(x={?anf1 <- f(1)\n"
              "?anf2 <- f(2)\n"
              "(?anf1+?anf2)}, y={?anf3 <- f(3)\n"
              "?anf4 <- f(4)\n"
              "(?anf3+?anf4)})");
}
TEST(anf_subset_atomic_noargs) {
  auto aout = parse("a[]\n");
  anf::convertToANF(aout.get());
  cout << to_string(*aout) << endl;
  ASSERT_TRUE(to_string(*aout) == 
              "subset(a, NULL)");
}
TEST(anf_subset_atomic_args) {
  auto aout = parse("a[1, 2, 3]\n");
  anf::convertToANF(aout.get());
  cout << to_string(*aout) << endl;
  ASSERT_TRUE(to_string(*aout) == 
              "subset(a, 1, 2, 3)");
}
TEST(anf_subset_non_atomic_1arg) {
  auto aout = parse("a[f(1)+f(2)]\n");
  anf::convertToANF(aout.get());
  cout << to_string(*aout) << endl;
  ASSERT_TRUE(to_string(*aout) == 
              "subset(a, {?anf1 <- f(1)\n"
              "?anf2 <- f(2)\n"
              "(?anf1+?anf2)})");
}
TEST(anf_subset_non_atomic_2args) {
  auto aout = parse("a[f(1)+f(2), f(3)+f(4)]\n");
  anf::convertToANF(aout.get());
  cout << to_string(*aout) << endl;
  ASSERT_TRUE(to_string(*aout) == 
              "subset(a, {?anf1 <- f(1)\n"
              "?anf2 <- f(2)\n"
              "(?anf1+?anf2)}, {?anf3 <- f(3)\n"
              "?anf4 <- f(4)\n"
              "(?anf3+?anf4)})");
}
TEST(anf_doublesubset_atomic_noargs) {
  // not that it means anything, but we need to test we handle
  // whatever the user inputs...
  auto aout = parse("a[[]]\n");
  anf::convertToANF(aout.get());
  cout << to_string(*aout) << endl;
  ASSERT_TRUE(to_string(*aout) == 
              "dblsubset(ref: a, NULL)");
}
TEST(anf_doublesubset_atomic_args) {
  auto aout = parse("a[[1, 2, 3]]\n");
  anf::convertToANF(aout.get());
  cout << to_string(*aout) << endl;
  ASSERT_TRUE(to_string(*aout) == 
              "dblsubset(ref: a, 1, 2, 3)");
}
TEST(anf_doublesubset_non_atomic_1arg) {
  auto aout = parse("a[[f(1)+f(2)]]\n");
  anf::convertToANF(aout.get());
  cout << to_string(*aout) << endl;
  ASSERT_TRUE(to_string(*aout) == 
              "dblsubset(ref: a, {?anf1 <- f(1)\n"
              "?anf2 <- f(2)\n"
              "(?anf1+?anf2)})");
}
TEST(anf_doublesubset_non_atomic_2args) {
  auto aout = parse("a[[f(1)+f(2), f(3)+f(4)]]\n");
  anf::convertToANF(aout.get());
  cout << to_string(*aout) << endl;
  ASSERT_TRUE(to_string(*aout) == 
              "dblsubset(ref: a, {?anf1 <- f(1)\n"
              "?anf2 <- f(2)\n"
              "(?anf1+?anf2)}, {?anf3 <- f(3)\n"
              "?anf4 <- f(4)\n"
              "(?anf3+?anf4)})");
}
TEST(anf_left_assign_atomic) {
  auto aout = parse("x <- 2\n");
  anf::convertToANF(aout.get());
  cout << to_string(*aout) << endl;
  ASSERT_TRUE(to_string(*aout) == 
              "x <- 2");
}
TEST(anf_left_assign_non_atomic) {
  auto aout = parse("x <- f(1) + f(2)\n");
  anf::convertToANF(aout.get());
  cout << to_string(*aout) << endl;
  ASSERT_TRUE(to_string(*aout) == 
              "{?anf1 <- f(1)\n"
              "?anf2 <- f(2)\n"
              "x <- (?anf1+?anf2)}");
}
TEST(anf_special_assign_atomic) {
  auto aout = parse("x <<- 2\n");
  anf::convertToANF(aout.get());
  cout << to_string(*aout) << endl;
  ASSERT_TRUE(to_string(*aout) == 
              "x <<- 2");
}
TEST(anf_special_assign_non_atomic) {
  auto aout = parse("x <<- f(1) + f(2)\n");
  anf::convertToANF(aout.get());
  cout << to_string(*aout) << endl;
  ASSERT_TRUE(to_string(*aout) == 
              "{?anf1 <- f(1)\n"
              "?anf2 <- f(2)\n"
              "x <<- (?anf1+?anf2)}");
}
TEST(anf_if_atomic) {
  auto aout = parse("if (x) x\n");
  anf::convertToANF(aout.get());
  cout << to_string(*aout) << endl;
  ASSERT_TRUE(to_string(*aout) == 
              "if (x) x else NULL");
}
TEST(anf_if_non_atomic) {
  auto aout = parse("if (f(1)+f(2)) f(3)+f(4)\n");
  anf::convertToANF(aout.get());
  cout << to_string(*aout) << endl;
  ASSERT_TRUE(to_string(*aout) == 
              "{?anf1 <- f(1)\n"
              "?anf2 <- f(2)\n"
              "if ((?anf1+?anf2)) {?anf3 <- f(3)\n"
              "?anf4 <- f(4)\n"
              "(?anf3+?anf4)} else NULL}");
}
TEST(anf_ifelse_atomic) {
  auto aout = parse("if (x) x else x+1\n");
  anf::convertToANF(aout.get());
  cout << to_string(*aout) << endl;
  ASSERT_TRUE(to_string(*aout) == 
              "if (x) x else (x+1)");
}
TEST(anf_ifelse_non_atomic) {
  auto aout = parse("if (f(1)+f(2)) f(3)+f(4) else f(5)+f(6)\n");
  anf::convertToANF(aout.get());
  cout << to_string(*aout) << endl;
  ASSERT_TRUE(to_string(*aout) == 
              "{?anf1 <- f(1)\n"
              "?anf2 <- f(2)\n"
              "if ((?anf1+?anf2)) {?anf3 <- f(3)\n"
              "?anf4 <- f(4)\n"
              "(?anf3+?anf4)} else {?anf5 <- f(5)\n"
              "?anf6 <- f(6)\n"
              "(?anf5+?anf6)}}");
}
TEST(anf_while_atomic) {
  auto aout = parse("while (x) x\n");
  anf::convertToANF(aout.get());
  cout << to_string(*aout) << endl;
  ASSERT_TRUE(to_string(*aout) == 
              "while (x) x");
}
TEST(anf_while_non_atomic) {
  auto aout = parse("while (f(1)+f(2)) f(3)+f(4)\n");
  anf::convertToANF(aout.get());
  cout << to_string(*aout) << endl;
  ASSERT_TRUE(to_string(*aout) == 
              "while ({?anf1 <- f(1)\n"
              "?anf2 <- f(2)\n"
              "(?anf1+?anf2)}) {?anf3 <- f(3)\n"
              "?anf4 <- f(4)\n"
              "(?anf3+?anf4)}");
}
TEST(anf_for_atomic) {
  auto aout = parse("for (x in a) x\n");
  anf::convertToANF(aout.get());
  cout << to_string(*aout) << endl;
  ASSERT_TRUE(to_string(*aout) == 
              "for: {{?inexpr2 <- a\n"
              "?foridx1 <- 1\n"
              "while ({?anf1 <- length(?inexpr2)\n"
              "(?foridx1<=?anf1)}) {x <- dblsubset(?inexpr2, ?foridx1)\n"
              "x\n"
              "?foridx1 <- (?foridx1+1)}}}");
}
TEST(anf_for_non_atomic) {
  auto aout = parse("for (x in f(1)+f(2)) f(3)+f(4)\n");
  anf::convertToANF(aout.get());
  cout << to_string(*aout) << endl;
  ASSERT_TRUE(to_string(*aout) == 
              "for: {{?anf1 <- f(1)\n"
              "?anf2 <- f(2)\n"
              "?inexpr2 <- (?anf1+?anf2)\n"
              "?foridx1 <- 1\n"
              "while ({?anf3 <- length(?inexpr2)\n"
              "(?foridx1<=?anf3)}) {x <- dblsubset(?inexpr2, ?foridx1)\n"
              "?anf4 <- f(3)\n"
              "?anf5 <- f(4)\n"
              "(?anf4+?anf5)\n"
              "?foridx1 <- (?foridx1+1)}}}");
}
TEST(anf_request_atomic) {
  auto aout = parse("con ? 1\n");
  anf::convertToANF(aout.get());
  cout << to_string(*aout) << endl;
  ASSERT_TRUE(to_string(*aout) == 
              "con ? 1");
}
TEST(anf_request_non_atomic) {
  auto aout = parse("(g(1)+g(2)) ? (f(1)+f(2))\n");
  anf::convertToANF(aout.get());
  ASSERT_TRUE(to_string(*aout) == 
              "{?anf1 <- g(1)\n"
              "?anf2 <- g(2)\n"
              "(?anf1+?anf2) ? {?anf3 <- f(1)\n"
              "?anf4 <- f(2)\n"
              "(?anf3+?anf4)}}");
}
TEST(anf_double_assign) {
  auto eout = parse("i<-j<-1\n");
  auto aout = std::unique_ptr<El>(eout->clone());
  anf::convertToANF(aout.get());
  cout << to_string(*aout) << endl;
  ASSERT_TRUE(to_string(*aout) ==
              "{j <- 1\n"
              "?anf1 <- j\n"
              "i <- ?anf1}"); 
}
TEST(anf_triple_assign) {
  auto eout = parse("i<-j<-k<-1\n");
  auto aout = std::unique_ptr<El>(eout->clone());
  anf::convertToANF(aout.get());
  cout << to_string(*aout) << endl;
  ASSERT_TRUE(to_string(*aout) ==
              "{k <- 1\n"
              "?anf1 <- k\n"
              "j <- ?anf1\n"
              "?anf2 <- j\n"
              "i <- ?anf2}");
}
TEST(anf_escape_direct) {
  auto eout = parse("a <- 1\n"
                    "con ? ++a\n");
  auto aout = std::unique_ptr<El>(eout->clone());
  anf::convertToANF(aout.get());
  ASSERT_TRUE(to_string(*aout) == 
              "{a <- 1\n"
              "?bnd1 <- a\n"
              "con ? ::?bnd1}");
}
TEST(anf_escape_simple_expression) {
  auto eout = parse("con ? ++(a+1) - 2 + 10\n");
  auto aout = std::unique_ptr<El>(eout->clone());
  anf::convertToANF(aout.get());
  ASSERT_TRUE(to_string(*aout) == 
              "{?bnd2 <- (a+1)\n"
              "con ? {?anf1 <- ::?bnd2\n"
              "((?anf1-2)+10)}}");
}
TEST(anf_escape_param) {
  auto eout = parse("a <- 4\n"
                    "con ? {\n"
                    "    a <- 1\n"
                    "    a <- a + ++(3+a) - 2\n"
                    "    matrix(++b, 3, 2)\n"
                    "}\n");
  auto aout = std::unique_ptr<El>(eout->clone());
  anf::convertToANF(aout.get());
  ASSERT_TRUE(to_string(*aout) == 
              "{a <- 4\n"
              "?bnd2 <- (3+a)\n"
              "?bnd3 <- b\n"
              "con ? {a <- 1\n"
              "?anf1 <- ::?bnd2\n"
              "a <- ((a+?anf1)-2)\n"
              "matrix(::?bnd3, 3, 2)}}");
}
TEST(anf_escape_nested) {
  auto eout = parse("a <- 4\n"
                    "con ? {\n"
                    "    a <- 1\n"
                    "    a <- con ? (a + ++(3+a) - 2)\n"
                    "    matrix(++b, 3, 2)\n"
                    "}\n");
  auto aout = std::unique_ptr<El>(eout->clone());
  anf::convertToANF(aout.get());
  ASSERT_TRUE(to_string(*aout) == 
              "{a <- 4\n"
              "?bnd4 <- b\n"
              "con ? {a <- 1\n"
              "?bnd3 <- (3+a)\n"
              "?anf1 <- con ? {?anf2 <- ::?bnd3\n"
              "((a+?anf2)-2)}\n"
              "a <- ?anf1\n"
              "matrix(::?bnd4, 3, 2)}}");
}

int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
