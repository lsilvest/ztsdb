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

RUnit_c <- function() {
  all.equal(c(1,2,3), 1:3)
}
RUnit_c_null <- function() {
  all.equal(c(), NULL) &
  all.equal(c(NULL), NULL) &
  all.equal(c(NULL,NULL), NULL) &
  all.equal(c(1,NULL), 1) &
  all.equal(c(NULL,1,NULL), 1)
}
RUnit_c_names <- function() {
  exp <- 1:3
  rownames(exp) <- c("a","b","c")
  all.equal(c(a=1,b=2,c=3), exp)
}
RUnit_c_names_composed_unnamed_unique <- function() {
  exp <- 1:3
  rownames(exp) <- c("a","b","c")
  all.equal(c(a=1,b=2,c=c(3)), exp)
}
RUnit_c_names_composed_unnamed <- function() {
  exp <- 1:4
  rownames(exp) <- c("a","b","c1","c2")
  all.equal(c(a=1,b=2,c=c(3,4)), exp)
}
RUnit_c_names_composed_named <- function() {
  exp <- 1:4
  rownames(exp) <- c("a","b","c.cc","c.dd")
  all.equal(c(a=1,b=2,c=c(cc=3,dd=4)), exp)
}
## test proper vectorization
RUnit_c_matrix <- function() {
  all.equal(c(matrix(1:9,3,3), 10), 1:10)
}
RUnit_c_matrix <- function() {
  all.equal(c(matrix(1:9,3,3), 10), 1:10)
}
RUnit_c_array <- function() {
  all.equal(c(1:2, array(3:10,c(2,2,2))), 1:10)
}
## test null vectors and empty matrix/array
RUnit_c_vector_null <- function() {
  all.equal(c(vector("double", 0), 1), 1) &
  all.equal(c(1, vector("double", 0), 2), 1:2)
}
RUnit_c_matrix_empty <- function() {
  all.equal(c(matrix(0, 3, 0), 1), 1) &
  all.equal(c(1, matrix(0, 3, 0), 2, array(0, c(3,3,0))), 1:2)
}
## test with list
RUnit_c_list <- function() {
  all.equal(c(list(1),list(2),list(3)), list(1,2,3))
}
RUnit_c_list_names <- function() {
  all.equal(c(a=list(1),b=list(2),c=list(3)), list(a=1,b=2,c=3))
}
## test type conversion
## logical < double < character < list
RUnit_c_bool_double <- function() {
  all.equal(c(T,1), c(1,1)) &
  all.equal(c(1,F), c(1,0))
}
RUnit_c_time_double <- function() {
  t <- as.time("1970-01-01 00:00:00 America/New_York")
  tryCatch(c(1, t), .Last.error == "conversion not defined for time to double")
}
RUnit_c_interval_double <- function() {
  t <- as.interval("+1970-01-01 00:00:00 America/New_York->1970-01-01 01:00:00 America/New_York-")
  tryCatch(c(1, t), .Last.error == "conversion not defined for interval to double")
}
RUnit_c_double_character <- function() {
  all.equal(c("a",1), c("a","1"))
}
RUnit_c_bool_character <- function() {
  all.equal(c("a", T), c("a", "TRUE"))
}
RUnit_c_time_character <- function() {
  t <- as.time("1970-01-01 00:00:00 America/New_York")
  all.equal(c("a", t), c("a","1970-01-01 05:00:00 UTC"))
}
RUnit_c_builtin_double <- function() {
  all.equal(c(sin, 1), list(sin, 1))
  
}
RUnit_c_bool_character_double_list <- function() {
  all.equal(c(T,"a",1,list(1)), list(T,"a", 1, 1))
}

## test inplace
RUnit_c_double_inplace <- function() {
  a <- 1:2
  c(--a, 3:4)
  all.equal(a, 1:4)
}
RUnit_c_character_inplace <- function() {
  a <- as.character(1:2)
  c(--a, as.character(3:4))
  all.equal(a, as.character(1:4))
}
RUnit_c_bool_inplace <- function() {
  a <- c(T,T)
  c(--a, F, F)
  all.equal(a, c(T,T,F,F))
}
RUnit_c_bool_double_inplace <- function() {
  a <- T
  tryCatch(c(--a, 2:3), .Last.error == "cannot convert the type of a reference")
}
## test composite names
RUnit_c_compose_names <- function() {
  a <- c(c=3,d=4)
  all.equal(c(a=1:2, b=a), c(a1=1,a2=2,b.c=3,b.d=4))
}
