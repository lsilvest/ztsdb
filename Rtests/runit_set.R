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

one_second <- as.duration(1e9)
one_minute <- 60*one_second
one_hour   <- 60*one_minute

## not naming convention for interval tests:
## cc : start closed, end closed
## co : start closed, end open
## oc : etc.
## oo


## intersection
## --------------------------------------------------------------------------
## interval - interval:
## 1: c-----------c
## 2: c-----------c
## r: c-----------c
RUnit_intersect_time_interval_cc_cc__2_eq_1 <- function() {
    i1 <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    i2 <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    r  <- i2
    intersect(i1, i2) == r
}
## 1: c-----------c
## 2: o-----------c
## r: o-----------c
RUnit_intersect_time_interval_cc_oc__2_eq_1 <- function() {
    i1 <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    i2 <- |-2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    r  <- i2
    intersect(i1, i2) == r &
    intersect(i2, i1) == r
}
## 1: c-----------c
## 2: c-----------o
## r: c-----------o
RUnit_intersect_time_interval_cc_co__2_eq_1 <- function() {
    i1 <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    i2 <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York-|
    r  <- i2
    intersect(i1, i2) == r &
    intersect(i2, i1) == r
}
## 1: c-----------o
## 2: o-----------c
## r: o-----------o
RUnit_intersect_time_interval_co_oc__2_eq_1 <- function() {
    i1 <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York-|
    i2 <- |-2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    r  <- |-2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York-|
    intersect(i1, i2) == r &
    intersect(i2, i1) == r
}
## 1: o-----------o
## 2: o-----------o
## r: o-----------o
RUnit_intersect_time_interval_oo_oo__2_eq_1 <- function() {
    i1 <- |-2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York-|
    i2 <- |-2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York-|
    r  <- i2
    intersect(i1, i2) == r
}
## 1: c-----------c
## 2: o-----------o
## r: o-----------o
RUnit_intersect_time_interval_cc_oo__2_eq_1 <- function() {
    i1 <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    i2 <- |-2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York-|
    r  <- i2
    intersect(i1, i2) == r
}

## union
## --------------------------------------------------------------------------
## interval - interval:
## 1: c-----------c
## 2: c-----------c
## r: c-----------c
RUnit_union_time_interval_cc_cc__2_eq_1 <- function() {
    i1 <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    i2 <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    r  <- i2
    union(i1, i2) == r &
    union(i2, i1) == r
}
## 1: c-----------c
## 2: o-----------c
## r: c-----------c
RUnit_union_time_interval_cc_oc__2_eq_1 <- function() {
    i1 <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    i2 <- |-2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    r  <- i1
    union(i1, i2) == r &
    union(i2, i1) == r
}
## 1: c-----------c
## 2: c-----------o
## r: c-----------c
RUnit_union_time_interval_cc_co__2_eq_1 <- function() {
    i1 <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    i2 <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York-|
    r  <- i1
    union(i1, i2) == r &
    union(i2, i1) == r
}
## 1: c-----------o
## 2: o-----------c
## r: c-----------c
RUnit_union_time_interval_co_oc__2_eq_1 <- function() {
    i1 <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York-|
    i2 <- |-2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    r  <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    union(i1, i2) == r &
    union(i2, i1) == r
}
## 1: o-----------o
## 2: o-----------o
## r: o-----------o
RUnit_union_time_interval_oo_oo__2_eq_1 <- function() {
    i1 <- |-2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York-|
    i2 <- |-2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York-|
    r  <- i2
    union(i1, i2) == r &
    union(i2, i1) == r
}
## 1: c-----------c
## 2: o-----------o
## r: c-----------c
RUnit_union_time_interval_cc_oo__2_eq_1 <- function() {
    i1 <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    i2 <- |-2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York-|
    r  <- i1
    union(i1, i2) == r &
    union(i2, i1) == r
}

## setdiff
## --------------------------------------------------------------------------
## time - time:
## 1: |-----------|
## 2:    |---|
## r: |--|   |----|
RUnit_setdiff_time_time__2_subset_of_1 <- function() {
    s1 <- seq(|.2015-01-01 12:00:00 America/New_York.|, length.out=10, by=one_second)
    s2 <- seq(|.2015-01-01 12:00:03 America/New_York.|, length.out=3, by=one_second)
    r  <- c(seq(|.2015-01-01 12:00:00 America/New_York.|, length.out=3, by=one_second),
            seq(|.2015-01-01 12:00:06 America/New_York.|, length.out=4, by=one_second))
    all(setdiff(s1, s2) == r)
}

## time - interval:
## 1: |-----------|
## 2:    |---|
## r: |--|   |----|
RUnit_setdiff_time_interval_cc__2_subset_of_1 <- function() {
    s1 <- seq(|.2015-01-01 12:00:00 America/New_York.|, length.out=10, by=one_second)
    i2 <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    r  <- c(seq(|.2015-01-01 12:00:00 America/New_York.|, length.out=3, by=one_second),
            seq(|.2015-01-01 12:00:06 America/New_York.|, length.out=4, by=one_second))
    all(setdiff(s1, i2) == r)
}
RUnit_setdiff_time_interval_oc__2_subset_of_1 <- function() {
    s1 <- seq(|.2015-01-01 12:00:00 America/New_York.|, length.out=10, by=one_second)
    i2 <- |-2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    r  <- c(seq(|.2015-01-01 12:00:00 America/New_York.|, length.out=4, by=one_second),
            seq(|.2015-01-01 12:00:06 America/New_York.|, length.out=4, by=one_second))
    all(setdiff(s1, i2) == r)
}
RUnit_setdiff_time_interval_co__2_subset_of_1 <- function() {
    s1 <- seq(|.2015-01-01 12:00:00 America/New_York.|, length.out=10, by=one_second)
    i2 <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York-|
    r  <- c(seq(|.2015-01-01 12:00:00 America/New_York.|, length.out=3, by=one_second),
            seq(|.2015-01-01 12:00:05 America/New_York.|, length.out=5, by=one_second))
    all(setdiff(s1, i2) == r)
}
RUnit_setdiff_time_interval_oo__2_subset_of_1 <- function() {
    s1 <- seq(|.2015-01-01 12:00:00 America/New_York.|, length.out=10, by=one_second)
    i2 <- |-2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York-|
    r  <- c(seq(|.2015-01-01 12:00:00 America/New_York.|, length.out=4, by=one_second),
            seq(|.2015-01-01 12:00:05 America/New_York.|, length.out=5, by=one_second))
    all(setdiff(s1, i2) == r)
}

## interval - interval:
## 1: c-----------c
## 2: c-----------c
## r: 
RUnit_setdiff_time_interval_cc_cc__2_eq_1 <- function() {
    i1 <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    i2 <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    r  <- vector(mode="interval", length=0)
    all(setdiff(i1, i2) == r)
}
## 1: c-----------c
## 2: o-----------c
## r: c
RUnit_setdiff_time_interval_cc_oc__2_eq_1 <- function() {
    i1 <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    i2 <- |-2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    r  <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:03 America/New_York+|
    setdiff(i1, i2) == r
}
## 1: c-----------c
## 2: c-----------o
## r:             c
RUnit_setdiff_time_interval_cc_co__2_eq_1 <- function() {
    i1 <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    i2 <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York-|
    r  <- |+2015-01-01 12:00:05 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    setdiff(i1, i2) == r
}
## 1: c-----------o
## 2: o-----------c
## r: c
RUnit_setdiff_time_interval_co_oc__2_eq_1 <- function() {
    i1 <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York-|
    i2 <- |-2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    r  <- |+2015-01-01 12:00:03 EST -> 2015-01-01 12:00:03 America/New_York+|
    setdiff(i1, i2) == r
}
## 1: o-----------o
## 2: o-----------o
## r: 
RUnit_setdiff_time_interval_oo_oo__2_eq_1 <- function() {
    i1 <- |-2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York-|
    i2 <- |-2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York-|
    r  <- vector(mode="interval", length=0)
    all.equal(setdiff(i1, i2), r)
}
## 1: c-----------c
## 2: o-----------o
## r: c-----------c
RUnit_setdiff_time_interval_cc_oo__2_eq_1 <- function() {
    i1 <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    i2 <- |-2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York-|
    r  <- c(|+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:03 America/New_York+|,
            |+2015-01-01 12:00:05 America/New_York -> 2015-01-01 12:00:05 America/New_York+|)
    all(setdiff(i1, i2) == r)
}
## 1: c-----------c
## 2:             c-----------o
## r: c-----------o            
RUnit_setdiff_time_interval_cc_co__2_gt_1 <- function() {
    i1 <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    i2 <- |+2015-01-01 12:00:05 America/New_York -> 2015-01-01 12:00:07 America/New_York-|
    r  <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York-|
    all(setdiff(i1, i2) == r)
}
## 1: c-----------c
## 2:             o-----------o
## r: c-----------c            
RUnit_setdiff_time_interval_cc_oo__2_gt_1 <- function() {
    i1 <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    i2 <- |-2015-01-01 12:00:05 America/New_York -> 2015-01-01 12:00:07 America/New_York-|
    r  <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    all(setdiff(i1, i2) == r)
}
## 1: c-----------c
## 2:             o-----------c
## r: c-----------c            
RUnit_setdiff_time_interval_cc_oc__2_gt_1 <- function() {
    i1 <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    i2 <- |-2015-01-01 12:00:05 America/New_York -> 2015-01-01 12:00:07 America/New_York-|
    r  <- |+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    all(setdiff(i1, i2) == r)
}
## 1:             c-----------c
## 2: o-----------c
## r:             o-----------c
RUnit_setdiff_time_interval_cc_oc__2_lt_1 <- function() {
    i1 <- |+2015-01-01 12:00:05 America/New_York -> 2015-01-01 12:00:07 America/New_York+|
    i2 <- |-2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|
    r  <- |-2015-01-01 12:00:05 America/New_York -> 2015-01-01 12:00:07 America/New_York+|
    all(setdiff(i1, i2) == r)
}
## 1:             c-----------c
## 2: o-----------o
## r:             c-----------c
RUnit_setdiff_time_interval_cc_oo__2_lt_1 <- function() {
    i1 <- |+2015-01-01 12:00:05 America/New_York -> 2015-01-01 12:00:07 America/New_York+|
    i2 <- |-2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York-|
    r  <- |+2015-01-01 12:00:05 America/New_York -> 2015-01-01 12:00:07 America/New_York+|
    all(setdiff(i1, i2) == r)
}
## various tests where we add a third interval
## 1:             c-----------c c--------c
## 2: o-----------o
## r:             c-----------c c--------c
RUnit_setdiff_time_interval_cc_oo__2_lt_1_3rd <- function() {
    i1 <- c(|+2015-01-01 12:00:05 America/New_York -> 2015-01-01 12:00:07 America/New_York+|,
            |+2015-01-01 12:00:08 America/New_York -> 2015-01-01 12:00:10 America/New_York+|)
    i2 <- |-2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York-|
    r  <- c(|+2015-01-01 12:00:05 America/New_York -> 2015-01-01 12:00:07 America/New_York+|,
            |+2015-01-01 12:00:08 America/New_York -> 2015-01-01 12:00:10 America/New_York+|)
    all(setdiff(i1, i2) == r)
}
## 1: c-----------c        c--------o
## 2: o--------------------c
## r:                      o--------c
RUnit_setdiff_time_interval_cc_co_oc_3rd <- function() {
    i1 <- c(|+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:05 America/New_York+|,
            |+2015-01-01 12:00:08 America/New_York -> 2015-01-01 12:00:10 America/New_York-|)
    i2 <-   |-2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:08 America/New_York+|
    r  <- c(|+2015-01-01 12:00:03 America/New_York -> 2015-01-01 12:00:03 America/New_York+|,
            |-2015-01-01 12:00:08 America/New_York -> 2015-01-01 12:00:10 America/New_York-|)
    all(setdiff(i1, i2) == r)
}
