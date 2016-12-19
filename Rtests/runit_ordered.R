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


## constructors
RUnit_ordered_vector_constructor <- function() {
    is.ordered(1:10) &
   !is.ordered(10:1) &
    is.ordered(1) &
    is.ordered(-1) &
    is.ordered(0)
}
RUnit_ordered_array_constructor <- function() {
    is.ordered(matrix(1:10, 2, 5)) &
   !is.ordered(matrix(10:1, 2, 5))
}
## subassign, ordered
RUnit_ordered_vector_subassign_1_beginning <- function() {
    a <- 1:10
    a[1] <- -1
    is.ordered(a)
}
RUnit_ordered_vector_subassign_1_middle <- function() {
    a <- 1:10
    a[2] <- 2.1
    is.ordered(a)
}
RUnit_ordered_vector_subassign_1_end <- function() {
    a <- 1:10
    a[10] <- 100
    is.ordered(a)
}
RUnit_ordered_vector_subassign_2_beginning <- function() {
    a <- 1:10
    a[1:2] <- c(-1, 0)
    is.ordered(a)
}
RUnit_ordered_vector_subassign_2_middle <- function() {
    a <- 1:10
    a[2:3] <- c(2.1, 3.1)
    is.ordered(a)
}
RUnit_ordered_vector_subassign_2_end <- function() {
    a <- 1:10
    a[9:10] <- 99:100
    is.ordered(a)
}
RUnit_ordered_vector_subassign_all <- function() {
    a <- 1:10
    a[] <- 91:100
    is.ordered(a)
}
## not ordered
RUnit_not_ordered_vector_subassign_1_beginning <- function() {
    a <- 1:10
    a[1] <- 2
    !is.ordered(a)
}
RUnit_not_ordered_vector_subassign_1_middle <- function() {
    a <- 1:10
    a[2] <- 3
    !is.ordered(a)
}
RUnit_not_ordered_vector_subassign_1_end <- function() {
    a <- 1:10
    a[10] <- 9
    !is.ordered(a)
}
RUnit_not_ordered_vector_subassign_2_beginning <- function() {
    a <- 1:10
    a[1:2] <- c(-2,-3)
    !is.ordered(a)
}
RUnit_not_ordered_vector_subassign_2_middle <- function() {
    a <- 1:10
    a[2:3] <- c(3.1, 4.1)
    !is.ordered(a)
}
RUnit_not_ordered_vector_subassign_2_middle_bis <- function() {
    a <- 1:10
    a[2:3] <- c(1.2,1.1)
    !is.ordered(a)
}
RUnit_not_ordered_vector_subassign_2_end <- function() {
    a <- 1:10
    a[9:10] <- c(99,99)
    !is.ordered(a)
}
RUnit_not_ordered_vector_subassign_all <- function() {
    a <- 1:10
    a[] <- 100:91
    !is.ordered(a)
}

## check order on other types of indices and when order indices is inverted LLL

## also check ordering with bind, c, rollmean, power, etc LLL
RUnit_ordered_vector_neg <- function() {
    a <- 10:1
    a <- -a
    is.ordered(a)
}
RUnit_ordered_vector_plus_neg <- function() {
    a <- 10:1
    `-`(--a)
    is.ordered(a)
}
RUnit_ordered_vector_plus <- function() {
    a <- 1:10
    a <- a + 2
    is.ordered(a)
}
RUnit_ordered_vector_plus_inplace <- function() {
    a <- 1:10
    `+`(--a, 2)
    is.ordered(a)
}
RUnit_ordered_vector_power <- function() {
    a <- 1:10
    a <- a^2
    is.ordered(a)
}
RUnit_ordered_vector_power_inplace <- function() {
    a <- 1:10
    `^`(--a, 2)
    is.ordered(a)
}
