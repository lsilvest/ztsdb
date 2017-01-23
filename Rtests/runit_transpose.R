## Copyright (C) 2017 Leonardo Silvestri
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


## double
RUnit_transpose_scalar <- function() {
    t(1) == 1
}
RUnit_transpose_vector <- function() {
    all.equal(t(1:10), matrix(1:10, 1, 10))
}
RUnit_transpose_named_vector <- function() {
    all.equal(t(c(a=1,b=2,c=3)), matrix(1:3, 1, 3, dimnames=list(NULL, c("a","b","c"))))
}
RUnit_transpose_square_matrix <- function() {
    all.equal(t(matrix(1:9, 3, 3)), matrix(c(1,4,7,2,5,8,3,6,9), 3, 3))
}
RUnit_transpose_square_matrix <- function() {
    all.equal(t(matrix(1:9, 3, 3)), matrix(c(1,4,7,2,5,8,3,6,9), 3, 3))
}
RUnit_transpose_named_square_matrix <- function() {
    dim1 <- c("a","b","c")
    dim2 <- c("d","e","f")
    all.equal(t(matrix(1:9, 3, 3, dimnames=list(dim1, dim2))),
              matrix(c(1,4,7,2,5,8,3,6,9), 3, 3, dimnames=list(dim2, dim1)))
}
RUnit_transpose_rectangular_matrix <- function() {
    all.equal(t(matrix(1:10, 5, 2)),
              matrix(c(1,6,2,7,3,8,4,9,5,10), 2, 5))
}
RUnit_transpose_named_rectangular_matrix <- function() {
    dim1 <- c("a","b","c","d","e")
    dim2 <- c("f","g")
    all.equal(t(matrix(1:10, 5, 2, dimnames=list(dim1, dim2))),
              matrix(c(1,6,2,7,3,8,4,9,5,10), 2, 5, dimnames=list(dim2, dim1)))
}
RUnit_transpose_null <- function() {
    tryCatch(t(NULL), .Last.error=="argument is not a matrix")
}
RUnit_transpose_incorrect_type <- function() {
    tryCatch(t(sin), .Last.error=="argument is not a matrix")
}
RUnit_transpose_array <- function() {
    tryCatch(t(array(1:27, c(3,3,3))), .Last.error=="argument is not a matrix")
}

## character
RUnit_transpose_character_scalar <- function() {
    t("1") == "1"
}
RUnit_transpose_character_vector <- function() {
    all.equal(t(as.character(1:10)), as.character(matrix(1:10, 1, 10)))
}
RUnit_transpose_character_named_vector <- function() {
    all.equal(t(c(a="1",b="2",c="3")),
              matrix(as.character(1:3), 1, 3, dimnames=list(NULL, c("a","b","c"))))
}
RUnit_transpose_character_square_matrix <- function() {
    all.equal(t(matrix(as.character(1:9), 3, 3)), matrix(as.character(c(1,4,7,2,5,8,3,6,9)), 3, 3))
}
RUnit_transpose_character_square_matrix <- function() {
    all.equal(t(matrix(as.character(1:9), 3, 3)), as.character(matrix(c(1,4,7,2,5,8,3,6,9), 3, 3)))
}
RUnit_transpose_character_named_square_matrix <- function() {
    dim1 <- c("a","b","c")
    dim2 <- c("d","e","f")
    all.equal(t(as.character(matrix(1:9, 3, 3, dimnames=list(dim1, dim2)))),
              as.character(matrix(c(1,4,7,2,5,8,3,6,9), 3, 3, dimnames=list(dim2, dim1))))
}
RUnit_transpose_character_rectangular_matrix <- function() {
    all.equal(t(matrix(as.character(1:10), 5, 2)),
              matrix(as.character(c(1,6,2,7,3,8,4,9,5,10)), 2, 5))
}
RUnit_transpose_character_named_rectangular_matrix <- function() {
    dim1 <- c("a","b","c","d","e")
    dim2 <- c("f","g")
    all.equal(t(matrix(as.character(1:10), 5, 2, dimnames=list(dim1, dim2))),
              matrix(as.character(c(1,6,2,7,3,8,4,9,5,10)), 2, 5, dimnames=list(dim2, dim1)))
}

## test try a few other types
RUnit_transpose_bool_scalar <- function() {
    t(TRUE) == TRUE
}
RUnit_transpose_time_scalar <- function() {
    tm <- |.2016-08-06 06:38:01 America/New_York.|
    t(tm) == tm
}
RUnit_transpose_interval_scalar <- function() {
    ival <- |+2016-08-06 06:38:01 America/New_York -> 2016-09-06 06:38:01 America/New_York-|
    t(ival) == ival
}
RUnit_transpose_duration_scalar <- function() {
    dur <- as.duration(1)
    t(dur) == dur
}
