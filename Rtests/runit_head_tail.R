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


## head ---------------------------
## double
RUnit_head_default <- function() {
    all.equal(head(matrix(1:20, 10, 2)),
              matrix(c(1:6, 11:16), 6, 2))
}
RUnit_head_1 <- function() {
    all.equal(head(matrix(1:20, 10, 2), 1),
              matrix(c(1, 11), 1, 2))
}
RUnit_head_m6 <- function() {
    all.equal(head(matrix(1:20, 10, 2), -6),
              matrix(c(1:4, 11:14), 4, 2))
}
RUnit_head_m1 <- function() {
    all.equal(head(matrix(1:20, 10, 2), -1),
              matrix(c(1:9, 11:19), 9, 2))
}
RUnit_head_m10 <- function() {
    all.equal(head(matrix(1:20, 10, 2), -10),
              matrix(0, 0, 2))
}
RUnit_head_m11 <- function() {
    all.equal(head(matrix(1:20, 10, 2), -11),
              matrix(0, 0, 2))
}
RUnit_head_0 <- function() {
    all.equal(head(matrix(1:20, 10, 2), 0),
              matrix(0, 0, 2))
}
## other array types and shapes
RUnit_head_string <- function () {
    all.equal(head(as.character(1:10), 4),
              as.character(1:4))
}
## zts
source("zts.R")
RUnit_head_zts <- function() {
    all.equal(head(z1), z1[1:6,])
}
RUnit_head_zts_1 <- function() {
    all.equal(head(z1, 1), z1[1,])
}
RUnit_head_zts_m6 <- function() {
    all.equal(head(z1, -6), z1[1:(nrow(z1)-6),])
}
RUnit_head_zts_m1 <- function() {
    all.equal(head(z1, -1), z1[1:(nrow(z1)-1),])
}
## errors
## RUnit_head_ref <- function() {
##     tryCatch(head(--z1), .Last.error == "this function does not allow pass by reference") 
## }


## Tail ----------------------------
## double
RUnit_tail_default <- function() {
    dimnames <- list(paste0("[", 5:10, ",]"), NULL)
    all.equal(tail(matrix(1:20, 10, 2)),
              matrix(c(5:10, 15:20), 6, 2, dimnames=dimnames))
}
RUnit_tail_default_addrownums_FALSE <- function() {
    all.equal(tail(matrix(1:20, 10, 2), addrownums=FALSE),
              matrix(c(5:10, 15:20), 6, 2))
}
RUnit_tail_1 <- function() {
    dimnames <- list("[10,]", NULL)
    all.equal(tail(matrix(1:20, 10, 2), 1),
              matrix(c(10,20), 1, 2, dimnames=dimnames))
}
RUnit_tail_1_addrownums_FALSE <- function() {
    all.equal(tail(matrix(1:20, 10, 2), 1, addrownums=FALSE),
              matrix(c(10,20), 1, 2))
}
RUnit_tail_10 <- function() {
    dimnames <- list(paste0("[", 1:10, ",]"), NULL)
    all.equal(tail(matrix(1:20, 10, 2), 10, addrownums=TRUE),
              matrix(c(1:20), 10, 2, dimnames=dimnames))
}
RUnit_tail_m6 <- function() {
    dimnames <- list(paste0("[", 7:10, ",]"), NULL)
    all.equal(tail(matrix(1:20, 10, 2), -6, addrownums=TRUE),
              matrix(c(7:10,17:20), 4, 2, dimnames=dimnames))
}
RUnit_tail_m6_addrownums_FALSE <- function() {
    all.equal(tail(matrix(1:20, 10, 2), -6, addrownums=FALSE),
              matrix(c(7:10,17:20), 4, 2))
}
RUnit_tail_m10 <- function() {
    all.equal(tail(matrix(1:20, 10, 2), -10, addrownums=TRUE),
              matrix(0, 0, 2))
}
RUnit_tail_m11 <- function() {
    all.equal(tail(matrix(1:20, 10, 2), -11, addrownums=FALSE),
              matrix(0, 0, 2))
}
RUnit_tail_0 <- function() {
    all.equal(tail(matrix(1:20, 10, 2), 0),
              matrix(0, 0, 2))
}
## other array types and shapes
RUnit_tail_string <- function () {
    all.equal(tail(as.character(1:10), 4, addrownums=FALSE),
              as.character(7:10))
}
## zts
RUnit_tail_zts_default <- function() {
    all.equal(tail(z1), z1[4:9,])
}
RUnit_tail_zts_addrownums_FALSE <- function() {
    all.equal(tail(z1, addrownums=FALSE), z1[4:9,])
}
RUnit_tail_zts_1 <- function() {
    all.equal(tail(z1, 1), z1[9,])
}
RUnit_tail_zts_m6 <- function() {
    all.equal(tail(z1, -6), z1[7:9,])
}
RUnit_tail_zts_m1 <- function() {
    all.equal(tail(z1, -1), z1[2:9,])
}
## errors
RUnit_head_ref <- function() {
    tryCatch(head(--z1), .Last.error == "this function does not allow pass by reference") 
}
RUnit_tail_ref <- function() {
    tryCatch(tail(--z1), .Last.error == "this function does not allow pass by reference") 
}
