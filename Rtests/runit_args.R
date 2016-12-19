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


RUnit_noargs <- function() {
    f <- function() 1
    f() == 1
}

RUnit_args1 <- function() {
    f <- function(x) x
    f(3) == 3
}

RUnit_argsn <- function() {
    f <- function(a, b, c) a + b + c
    f(1,2,3) == 6
}

RUnit_default_not_used <- function() {
    f <- function(a=1, b=2, c=3) a + b + c
    f(1, 2, 3) == 6
}

RUnit_default_partial_used <- function() {
    f <- function(a=1, b=2, c=3) a + b + c
    f(1, 2) == 6
}

RUnit_default_all_used <- function() {
    f <- function(a=1, b=2, c=3) a + b + c
    f() == 6
}

RUnit_default_partial_spec <- function() {
    f <- function(a=1, b=2, c=3) a + b + c
    f(c=4) == 7
}

RUnit_default_all_spec <- function() {
    f <- function(a=1, b=2, c=3) a + 2*b + 3*c
    f(c=4, a=2, b=3) == 20
}

RUnit_scope_default<- function() {
    f <- function(a=1, b=a, c=a) a + 2*b + 3*c
    f() == 6
}

RUnit_scope <- function() {
    f <- function(a=1, b=a, c=a) a + 2*b + 3*c
    f(2) == 12
}

RUnit_scope_complex_default_arg <- function() {
    f <- function(a={x<-2; 1}, b, c) a + 2*b + 3*c + x
    f(b=2, c=3) == 16
}

RUnit_scope_complex_given_arg <- function() {
    f <- function(a, b, c) a + 2*b + 3*c
    f({x<-2; 1}, b=2, c=3) == 14 && x == 2
}
