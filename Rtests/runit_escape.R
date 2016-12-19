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


source("comm_setup_teardown.R")

port1 <- 18001
port2 <- 18002


RUnit_direct_single <- function() {
    a <- matrix(1:9, 3, 3)
    all(a == (con1 ? ++a))
}
RUnit_direct_multiple <- function() {
    a <- matrix(1:9, 3, 3)
    b <- matrix(1, 3, 3)
    all(a + b == (con1 ? ++a + ++b))
}
RUnit_escape <- function() {
    a <- matrix(1:9, 3, 3)
    con1 ? (b <<- matrix(1, 3, 3))
    all(matrix(c(3,5,7,9,11,13,15,17,19), 3, 3) == (con1 ? ++(a * 2) + b))
}
RUnit_escape_nested <- function() {
    con1 ? (a <<- matrix(1:9, 3, 3))
    con1 ? (con2 <<- connection("127.0.0.1", 18002))
    con1 ? con2 ? (b <<- matrix(1, 3, 3))
    all(matrix(c(3,5,7,9,11,13,15,17,19), 3, 3) == (con1 ? con2 ? ++(a * 2) + b))
}
RUnit_escape_function_parameter <- function() {
    a <- 1
    all(matrix(a+1, 3, 3) == (con1 ? matrix(++(a+1), 3, 3)))
}
RUnit_escape_function_parameter_ellipsis <- function() {
    a <- 1
    b <- 2
    all(c(a, b) == (con1 ? c(++a, ++b)))
}
RUnit_escape_inside_function <- function() {
    f <- function() (con1 ? matrix(++(a+1), 3, 3))
    a <- 3
    all(matrix(a+1, 3, 3) == f())
}
RUnit_escape_inside_assignment <- function() {
    a <- 3
    b <- (con1 ? ++a)
    a == b
}
