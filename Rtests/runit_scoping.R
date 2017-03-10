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


RUnit_outer_var_read <- function() {
    f <- function() x
    x <- 1
    f() == 1
}
RUnit_outer_outer_read <- function() {
    f <- function() { g <- function() x; g() }
    x <- 2    
    f() == 2                            ## dynamic scoping, this is not the same as R!
}
RUnit_outer_var_write <- function() {
    f <- function() x <<- x + 2
    x <- 1
    f()
    x == 3
}
RUnit_outer_outer_write <- function() {
    f <- function() { g <- function() x <<- x + 3; g() }
    x <- 2
    f()
    x == 5
}
RUnit_fun_fun <- function() {
    f <- function() function(x) x
    g <- f()
    g(4) == 4
}
RUnit_list_of_funcs <- function() {
    f <- function() function(x) x
    l <- list(f(), f())
    l[[1]](1) + l[[2]](2) == 3
}
