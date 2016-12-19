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
source("zts.R")
source("utils.R")

port1 <- 17001
port2 <- 17002

## we need to test two things for each type:
## 1. that the ast type can be encoded and sent (e.g. Double, Interval, List, Binop, etc.)
## 2. that the 'Value' can be encoded and sent (e.g. Value with SpVAD, SpVAIVL, SpList, etc.)

RUnit_comm_double_scalar <- function() {
    1 == (con1 ? 1) &
    1.11 == (con1 ? 1.11)
}
RUnit_comm_double_unop <- function() {
    -2 == (con1 ? -2)
}
RUnit_comm_double_binop <- function() {
    2 == (con1 ? 1+1)
}
RUnit_comm_null <- function() {
    is.null(con1 ? NULL)
}
RUnit_comm_bool_scalar <- function() {
    TRUE  == (con1 ? TRUE)
    FALSE == (con1 ? FALSE)
}
RUnit_comm_double <- function() {
    all(matrix(1:10, 2, 5) == (con1 ? matrix(1:10, 2, 5))) &
    all(matrix(1:4, 2, 2, dimnames=list(c("1","2"), c("a","b"))) ==
        (con1 ? matrix(1:4, 2, 2, dimnames=list(c("1","2"), c("a","b")))))
}
RUnit_comm_duration <- function() {
    all(matrix(as.duration(1:10), 2, 5) == (con1 ? matrix(as.duration(1:10), 2, 5))) &
    all(matrix(as.duration(1:4), 2, 2, dimnames=list(c("1","2"), c("a","b"))) ==
        (con1 ? matrix(as.duration(1:4), 2, 2, dimnames=list(c("1","2"), c("a","b")))))
}
RUnit_comm_period <- function() {
    all(matrix(as.period("1m"), 2, 5) == (con1 ? matrix(as.period("1m"), 2, 5))) &
    all(matrix(as.period("2m2d"), 2, 2, dimnames=list(c("1","2"), c("a","b"))) ==
        (con1 ? matrix(as.period("2m2d"), 2, 2, dimnames=list(c("1","2"), c("a","b")))))
}
RUnit_comm_dtime <- function() {
    |.2016-08-28 13:05:00 America/New_York.| == (con1 ? |.2016-08-28 13:05:00 America/New_York.|)
}
RUnit_comm_string <- function() {
    "abc" == (con1 ? "abc")
}
RUnit_comm_interval <- function() {
    |+2016-08-28 12:41:51 America/New_York -> 2016-08-28 12:43:51 America/New_York+| ==
        (con1 ? |+2016-08-28 12:41:51 America/New_York -> 2016-08-28 12:43:51 America/New_York+|)
}
RUnit_comm_list <- function() {
    all.equal.list(list(1,2,"a"), con1 ? list(1,2,"a"))
}
RUnit_comm_symbol <- function() {
    con1 ? (a <<- 1)
    res <- (1 == (con1 ? a))
    con1 ? rm(a)
    res
}
RUnit_comm_etexprsublist <- function() {
    (con1 ? sin(2)) == sin(2)
}
RUnit_comm_zts <- function() {
    all.equal(z1, con1 ? ++z1)
}
