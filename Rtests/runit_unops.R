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


## scalar --------------------
## double
RUnit_double_minus <- function() {
    -1 == 0 - 1 & is.nan(-NaN)
}
RUnit_double_plus <- function() {
    +1 == 1 & is.nan(+NaN)
}
RUnit_double_not <- function() {
    !6 == FALSE & !0 == TRUE
}

## bool
RUnit_bool_not <- function() {
    !FALSE == TRUE && !TRUE == FALSE
}
RUnit_bool_undefined_ops <- function() {
    tryCatch(-TRUE, "error") == "error"  &
    tryCatch(+TRUE, "error") == "error"  &
    tryCatch(-FALSE, "error") == "error" &
    tryCatch(+FALSE, "error") == "error"
}

## string
RUnit_string_undefined_ops <- function() {
    tryCatch(-"abc", "error") == "error"  &
    tryCatch(+"abc", "error") == "error"  &
    tryCatch(!"abc", "error") == "error"
}

## time
RUnit_time_undefined_ops <- function() {
    a <- |.2015-03-09 06:38:01 America/New_York.|
    tryCatch(-a, "error") == "error"  &
    tryCatch(+a, "error") == "error"  &
    tryCatch(!a, "error") == "error"
}

## interval
RUnit_interval_undefined_ops <- function() {
    a <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York-|;
    tryCatch(-a, "error") == "error"  &
    tryCatch(+a, "error") == "error"  &
    tryCatch(!a, "error") == "error"
}

## duration
RUnit_duration_minus <- function() {
    -as.duration(1) == as.duration(0) - as.duration(1)
}
RUnit_duration_plus <- function() {
    +as.duration(1) == as.duration(1)
}
RUnit_duration_undefined_ops <- function() {
    tryCatch(!as.duration(1), "error") == "error"
}

## array --------------------
## double
RUnit_array_double_minus <- function() {
    all(matrix(-1,2,2) == 0 - matrix(1,2,2))
}
RUnit_array_double_plus <- function() {
    all(matrix(+1,3,3) == matrix(1,3,3))
}
RUnit_array_double_not <- function() {
    all(!matrix(6,2,2) == matrix(FALSE,2,2) &
        !matrix(0,2,2) == matrix(TRUE,2,2))
}

## bool
RUnit_array_bool_not <- function() {
    all(!c(FALSE,TRUE) == c(TRUE,FALSE))
}
RUnit_array_bool_undefined_ops <- function() {
    tryCatch(-as.matrix(TRUE,2,2),  "error") == "error"  &
    tryCatch(+as.matrix(TRUE,2,2),  "error") == "error"  &
    tryCatch(-as.matrix(FALSE,2,2), "error") == "error"  &
    tryCatch(+as.matrix(FALSE,2,2), "error") == "error"
}

## string
RUnit_array_string_undefined_ops <- function() {
    tryCatch(-c("abc","a"), "error") == "error"  &
    tryCatch(+c("abc","a"), "error") == "error"  &
    tryCatch(!c("abc","a"), "error") == "error"
}

## time
RUnit_array_time_undefined_ops <- function() {
    a <- matrix(|.2015-03-09 06:38:01 America/New_York.|,2,2)
    tryCatch(-a, "error") == "error"  &
    tryCatch(+a, "error") == "error"  &
    tryCatch(!a, "error") == "error"
}

## interval
RUnit_array_interval_undefined_ops <- function() {
    a <-matrix(|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York-|,2,2)
    tryCatch(-a, "error") == "error"  &
    tryCatch(+a, "error") == "error"  &
    tryCatch(!a, "error") == "error"
}

## duration
RUnit_array_duration_minus <- function() {
    all(-matrix(as.duration(1),2,2) == matrix(as.duration(-1),2,2))
}
RUnit_array_duration_plus <- function() {
    all(+matrix(as.duration(1),2,2) == matrix(as.duration(1),2,2))
}
RUnit_array_duration_undefined_ops <- function() {
    tryCatch(!matrix(as.duration(1),2,2), TRUE)
}

## period
RUnit_array_period_minus <- function() {
    all(-matrix(as.period("1m1d/01:00:00"),2,2) == matrix(as.period("-1m-1d/-01:00:00"),2,2))
}
RUnit_array_period_plus <- function() {
    all(+matrix(as.period("1m1d/01:00:00"),2,2) == matrix(as.period("1m1d/01:00:00"),2,2))
}
RUnit_array_period_undefined_ops <- function() {
    tryCatch(!matrix(as.period("01:00:00"),2,2), TRUE)
}

## inplace -------------------------------
## array --------------------
## double
RUnit_array_inplace_double_minus <- function() {
    a <- matrix(-1,2,2)
    all.equal(`-`(--a), `-`(matrix(-1,2,2))) &
    all.equal(a, matrix(1,2,2))
}
RUnit_array_inplace_double_plus <- function() {
    a <- matrix(-1,2,2)
    all.equal(`+`(--a), `+`(matrix(-1,2,2))) &
    all.equal(a, matrix(-1,2,2))
}
RUnit_array_inplace_double_not <- function() {
    ## should be error LLL
    T
}

## bool
RUnit_array_inplace_bool_not <- function() {
    a <- matrix(c(T,F,T,F),2,2)
    all.equal(`!`(--a), `!`(matrix(c(T,F,T,F),2,2))) &
    all.equal(a, matrix(c(F,T,F,T), 2, 2))
}

## duration
RUnit_array_inplace_duration_minus <- function() {
    a <- matrix(-as.duration(1),2,2)
    all.equal(`-`(--a), `-`(matrix(as.duration(1),2,2)))
    all.equal(a, matrix(as.duration(1),2,2))
}
RUnit_array_inplace_duration_plus <- function() {
    a <- matrix(-as.duration(1),2,2)
    all.equal(`+`(--a), `+`(matrix(as.duration(-1),2,2)))
    all.equal(a, matrix(as.duration(-1),2,2))
}

## period
RUnit_array_inplace_period_minus <- function() {
    a <- matrix(as.period("1y1m/10:12:12.123_123"),2,2)
    all.equal(`-`(--a), `-`(matrix(as.period("1y1m/10:12:12.123_123"),2,2)))
    all.equal(a, matrix(as.period("-1y-1m/-10:12:12.123_123"),2,2))
}
RUnit_array_inplace_period_plus <- function() {
    a <- matrix(as.period("1y1m/10:12:12.123_123"),2,2)
    all.equal(`+`(--a), `+`(matrix(as.period("1y1m/10:12:12.123_123"),2,2)))
    all.equal(a, matrix(as.period("1y1m/10:12:12.123_123"),2,2))
}
