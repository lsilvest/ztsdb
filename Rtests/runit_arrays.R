## Copyright (C) 2016 Leonardo Silvestri
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



RUnit_vector_logical <- function() {
    all.equal(vector(length=5), c(F,F,F,F,F))
}
RUnit_vector_double <- function() {
    all.equal(vector(mode="double", length=5), c(0,0,0,0,0))
}
RUnit_vector_character <- function() {
    all.equal(vector(mode="character", length=5), c("","","","",""))
}
RUnit_vector_time <- function() {
    t0 <- |.1970-01-01 00:00:00 UTC.|
    all.equal(vector(mode="time", length=5), c(t0,t0,t0,t0,t0))
}
RUnit_vector_duration <- function() {
    all.equal(vector(mode="duration", length=5), as.duration(c(0,0,0,0,0)))
}
RUnit_vector_interval <- function() {
    i0 <- |+1970-01-01 00:00:00 UTC -> 1970-01-01 00:00:00 UTC+|
    all.equal(vector(mode="interval", length=5), c(i0,i0,i0,i0,i0))
}
RUnit_vector_double_0length <- function() {
    dim(vector(length=0)) == 0 & typeof(vector(length=0)) == "logical"
}
RUnit_matrix_0_0_dim_incorrect_names <- function() {
    tryCatch(matrix(0, 0, 0, dimnames=list(c("1"), NULL)), TRUE)
}
RUnit_dimnames_get <- function() {
    a <- matrix(1:9, 3, 3, dimnames=list(c("a","b","c"), as.character(1:3)))
    all(dimnames(a)[1] == c("a","b","c") & dimnames(a)[2] == as.character(1:3))
}
RUnit_dimnames_set <- function() {
    a <- matrix(1:9, 3, 3, dimnames=list(c("a","b","c"), as.character(1:3)))
    b <- matrix(1:9, 3, 3, dimnames=list(NULL, as.character(4:6)))
    dimnames(a) <-list(NULL, as.character(4:6))
    all.equal(a, b)
}

### constructors from NULL
RUnit_cons_null_double <- function() {
    all.equal(as.double(NULL), vector(mode="double", length=0))
}
RUnit_cons_null_character <- function() {
    all.equal(as.character(NULL), vector(mode="character", length=0))
}
RUnit_cons_null_logical <- function() {
    all.equal(as.logical(NULL), vector(mode="logical", length=0))
}
RUnit_cons_null_time <- function() {
    all.equal(as.time(NULL), vector(mode="time", length=0))
}
RUnit_cons_null_interval <- function() {
    all.equal(as.interval(NULL), vector(mode="interval", length=0))
}
RUnit_cons_null_period <- function() {
    all.equal(as.period(NULL), vector(mode="period", length=0))
}
