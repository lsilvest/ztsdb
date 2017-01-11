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


## Here we test that objects are appropriately copied. In particular,
## memory-mapped objects have different copying semantics.


source("zts.R") 
source("utils.R")


RUnit_zts_constructor <- function() {
    a <- zts(idx1, 1:27, dim=c(9, 3), dimnames=list(NULL, c("one", "two", "three")))
    b <- zts(idx1, 1:27, dimnames=list(NULL, c("one", "two", "three")))
    c_data <- matrix(1:27, 9, 3, dimnames=list(NULL, c("one", "two", "three")))
    c <- zts(idx1, c_data)
    all.equal(a, b) & all.equal(b, c)
}



