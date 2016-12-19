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


RUnit_function_noargs <- function() {
    f <- function() 2
    f() == 2
}
RUnit_function_1arg <- function() {
    f <- function(x) x
    f(2) == 2 &
    f(x=3) == 3
}
RUnit_function_2args <- function() {
    f <- function(x, y) x/y
    f(2,4) == 0.5 &
    f(4,2) == 2 &
    f(x=4,y=2) == 2 &
    f(y=2,x=4) == 2
}
RUnit_function_3args <- function() {
    f <- function(x, y, z) x/y + z
    f(2,4,1) == 1.5 &
    f(4,2,10) == 12
}
RUnit_function_1arg_default <- function() {
    f <- function(x=2) x
    f() == 2 &
    f(3) == 3    
}
RUnit_function_3args_default <- function() {
    f <- function(x=2,y=1,z=3) x/y + z
    f() == 5 &
    f(3) == 6 & 
    f(10,5) == 5
}
RUnit_function_3args_default_ref <- function() {
    f <- function(x=2,y=x^2,z=3) x/y + z
    f() == 3.5 &
    f(4) == 3.25 & 
    f(8) == 3.125
}
RUnit_function_too_few_args <- function() {
    f <- function(x,y,z)  x/y + z
    tryCatch(f(), "error") == "error" &
    tryCatch(f(1), "error") == "error" &
    tryCatch(f(1,2), "error") == "error"
}
RUnit_function_too_many <- function() {
    f <- function(x,y,z)  x/y + z
    tryCatch(f(1,2,3,4), "error") == "error"
}
RUnit_function_recursive <- function() {
    f <- function(x) if (x==1) x else x*f(x-1)
    f(6) == 6*5*4*3*2
}
RUnit_function_mut_recursive <- function() {
    even <- function(x) floor(x/2) == x/2
    f <- function(x) if (x<=1) x else if (even(x)) x*g(x-1) else x*f(x-1)
    g <- function(x) if (x<=3) x else if (!even(x)) x*f(x-3) else x*g(x-3)
    f(12) == 12*11*8*7*4*3
}
