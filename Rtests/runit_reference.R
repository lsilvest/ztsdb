## Copyright (C) 2015 Leonardo Silvestri
##
## This file is part of ztsdb.
##
## ztsdb is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## ztsdb is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with ztsdb.  If not, see <http://www.gnu.org/licenses/>.


RUnit_ref_simple <- function() {
    a <- matrix(1:9, 3, 3)
    `*`(--a, 2)
    `*`(  a, 2)                         # check the reference didn't "stick"
    all.equal(a, 2 * matrix(1:9, 3, 3)) 
}
RUnit_ref_simple_access_in_actuals <- function() {
    a <- matrix(1:9, 3, 3)
    `+`(--a, a)
    `+`(  a, a)
    all.equal(a, 2 * matrix(1:9, 3, 3)) 
}
RUnit_ref_simple_nested_access_in_formals <- function() {
    f <- function(x, y=x) `+`(--x, y)
    a <- matrix(1:9, 3, 3)
    f(--a)
    f(  a)
    all.equal(a, 2 * matrix(1:9, 3, 3)) 
}
RUnit_ref_simple_nested_access_in_formals_copy_first <- function() {
    f <- function(x, y=x) `+`(--x, y)
    a <- matrix(1:9, 3, 3)
    f(a)                                # 'a' should be copied
    all.equal(a, matrix(1:9, 3, 3)) 
}
RUnit_ref_simple_nested_ref_access_in_formals <- function() {
    f <- function(x, y=`-`(--x)) { }
    a <- matrix(1:9, 3, 3)
    f(--a)
    f(  a)
    all.equal(a, -matrix(1:9, 3, 3)) 
}
RUnit_ref_simple_nested_access_non_ref_op <- function() {
    f <- function(x, y=x) { `+`(--x, y); x + 2 }
    a <- matrix(1:9, 3, 3)
    f(--a)
    all.equal(a, 2 * matrix(1:9, 3, 3)) 
}
RUnit_ref_simple_nested_access_non_ref_sin <- function() {
    f <- function(x, y=x) { `+`(--x, y); sin(x) }
    a <- matrix(1:9, 3, 3)
    f(--a)
    all.equal(a, 2 * matrix(1:9, 3, 3)) 
}
RUnit_ref_multiple <- function() {
    f <- function(x, y) { `*`(--x, 2); `*`(--y, 3) }
    a <- matrix(1:9, 3, 3)
    b <- matrix(1:9, 3, 3)
    f(--a, --b)
    all.equal(a, 2 * matrix(1:9, 3, 3)) &
    all.equal(b, 3 * matrix(1:9, 3, 3))
}
RUnit_non_ref_user_function_last <- function() {
    ## check that 'a' is not modified when it should't and that last doesn't dangle:
    a <- 1
    f <- function(x) x[] <- 0
    f(a)
    all.equal(a, 1) 
}
RUnit_non_ref_user_function <- function() {
    ## check that 'a' is not modified when it should't
    a <- 1
    f <- function(x) x[] <- 0
    f(a)
    all.equal(a, 1) 
}
