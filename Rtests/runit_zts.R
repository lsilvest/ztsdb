## Copyright (C) 2015-2017 Leonardo Silvestri
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
source("zts_large.R")


## constructors
RUnit_zts_constructor <- function() {
    a <- zts(idx1, matrix(1:27, 9, 3, dimnames=list(NULL, c("one", "two", "three"))))
    b <- zts(idx1, matrix(1:27, 9, 3, dimnames=list(NULL, c("one", "two", "three"))))
    c_data <- matrix(1:27, 9, 3, dimnames=list(NULL, c("one", "two", "three")))
    c <- zts(idx1, c_data)
    all.equal(a, b) & all.equal(b, c)
}
RUnit_zts_constructor_0x2 <- function() {
  dimnames <- list(NULL, c("one", "two"))
  z <- zts(as.time(NULL), matrix(0, 0, 2, dimnames=dimnames))
  all.equal(dimnames(z), dimnames) &
  all.equal(zts.idx(z), as.time(NULL)) &
  all.equal(zts.data(z), matrix(0, 0, 2, dimnames=dimnames))
}
RUnit_zts_constructor_0x2_idx_0 <- function() {
  dimnames <- list(NULL, c("one", "two"))
  z <- zts(as.time(NULL), matrix(0, 0, 2, dimnames=dimnames))
  all.equal(dimnames(z), dimnames) &
  all.equal(zts.idx(z), as.time(NULL)) &
  all.equal(zts.data(z), matrix(0, 0, 2, dimnames=dimnames))
}
RUnit_zts_constructor_0x2_idx_0x0 <- function() {
  tryCatch(zts(matrix(as.time(NULL), 0, 0), matrix(0, 0, 2)),
           .Last.error=="zts index must be a vector")
}
RUnit_zts_constructor_0x2_idx_0x1 <- function() {
  dimnames <- list(NULL, c("one", "two"))
  z <- zts(matrix(as.time(NULL), 0, 1), matrix(0, 0, 2, dimnames=dimnames))
  all.equal(dimnames(z), dimnames) &
  all.equal(zts.idx(z), as.time(NULL)) &
  all.equal(zts.data(z), matrix(0, 0, 2, dimnames=dimnames))
}
RUnit_zts_constructor_dim_mismatch_0 <- function() {
  tryCatch(zts(as.time(NULL), matrix(1:27, 9, 3)),
           .Last.error=="mismatched dimensions for index and data")
}
RUnit_zts_constructor_dim_mismatch <- function() {
  tryCatch(zts(idx1[1:8], matrix(1:27, 9, 3)),
           .Last.error=="mismatched dimensions for index and data")
}

## subset
RUnit_zts_subset_double <- function() {
  z <- get_large_zts(3)
  exp <- zts(zts.idx(z)[1:2], matrix(c(1,2,4,5), 2, 2, dimnames=list(NULL,colnames(z)[1:2])))
  all.equal(z[1:2,1:2], exp)
}
RUnit_zts_subset_double_matrix_1col <- function() {
  z <- get_large_zts(3)
  exp <- zts(zts.idx(z)[1:2], matrix(c(1,2,4,5), 2, 2, dimnames=list(NULL,colnames(z)[1:2])))
  all.equal(z[matrix(1:2,2,1), matrix(1:2,2,1)], exp)
}
RUnit_zts_subset_double_empty <- function() {
  z <- get_large_zts(3)
  all.equal(z[as.double(NULL),], get_large_zts(0))
}




