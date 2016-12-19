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


## double ------------
## plus
RUnit_double_add <- function() {
    a <- matrix(1, 2, 3)
    b <- matrix(2, 2, 3)
    all(`+`(a, b) == matrix(3, 2, 3) &
        a == a &                        # a was not modified
        b == b)                         # neither was b
}
RUnit_double_add_inplace <- function() {
    a <- matrix(1, 2, 3)
    b <- matrix(2, 2, 3)
    res <- matrix(3, 2, 3)
    all(`+`(--a, b) == res &
        a == res &                        # a was modified
        b == b)                           # b was not
}
## minus
RUnit_double_minus <- function() {
    a <- matrix(1, 2, 3)
    b <- matrix(2, 2, 3)
    all(`-`(a, b) == matrix(-1, 2, 3) &
        a == a &                        # a was not modified
        b == b)                         # neither was b
}
RUnit_double_minus_inplace <- function() {
    a <- matrix(1, 2, 3)
    b <- matrix(2, 2, 3)
    res <- matrix(-1, 2, 3)
    all(`-`(--a, b) == res &
        a == res &                        # a was modified
        b == b)                           # b was not
}
## mul
RUnit_double_mul <- function() {
    a <- matrix(2, 2, 3)
    b <- matrix(3, 2, 3)
    all(`*`(a, b) == matrix(6, 2, 3) &
        a == a &                        # a was not modified
        b == b)                         # neither was b
}
RUnit_double_mul_inplace <- function() {
    a <- matrix(2, 2, 3)
    b <- matrix(3, 2, 3)
    res <- matrix(6, 2, 3)
    all(`*`(--a, b) == res &
        a == res &                        # a was modified
        b == b)                           # b was not
}
## div
RUnit_double_div <- function() {
    a <- matrix(3, 2, 3)
    b <- matrix(2, 2, 3)
    all(`/`(a, b) == matrix(1.5, 2, 3) &
        a == a &                        # a was not modified
        b == b)                         # neither was b
}
RUnit_double_div_inplace <- function() {
    a <- matrix(3, 2, 3)
    b <- matrix(2, 2, 3)
    res <- matrix(1.5, 2, 3)
    all(`/`(--a, b) == res &
        a == res &                        # a was modified
        b == b)                           # b was not
}
## power
RUnit_double_power <- function() {
    a <- matrix(2, 2, 3)
    b <- matrix(3, 2, 3)
    all(`^`(a, b) == matrix(8, 2, 3) &
        a == a &                        # a was not modified
        b == b)                         # neither was b
}
RUnit_double_power_inplace <- function() {
    a <- matrix(2, 2, 3)
    b <- matrix(3, 2, 3)
    res <- matrix(8, 2, 3)
    all(`^`(--a, b) == res &
        a == res &                        # a was modified
        b == b)                           # b was not
}
## zts_zts ------------
getZts <- function(x) {
    idx  <- seq(|.2016-09-13 17:09:00 America/New_York.|, by=as.period("1d"), length.out=3, tz="UTC")
    data <- matrix(x, 3, 2)
    zts(idx, data)
}
## plus
RUnit_zts_zts_plus <- function() {
    z1 <- getZts(1)
    z2 <- getZts(2)
    z3 <- getZts(3)
    all(`+`(z1, z2) == z3 &
        z1 == z1 &                        # z1 was not modified
        z2 == z2)                         # neither was z2
}
RUnit_zts_zts_plus_inplace <- function() {
    z1 <- getZts(1)
    z2 <- getZts(2)
    z3 <- getZts(3)
    all(`+`(--z1, z2) == z3 &
        z1 == z3 &                        # z1 was modified
        z2 == z2)                         # z2 was not
}
## minus
RUnit_zts_zts_minus <- function() {
    z1 <- getZts(1)
    z2 <- getZts(2)
    z3 <- getZts(-1)
    all(`-`(z1, z2) == z3 &
        z1 == z1 &                        # z1 was not modified
        z2 == z2)                         # neither was z2
}
RUnit_zts_zts_minus_inplace <- function() {
    z1 <- getZts(1)
    z2 <- getZts(2)
    z3 <- getZts(-1)
    all(`-`(--z1, z2) == z3 &
        z1 == z3 &                        # z1 was modified
        z2 == z2)                         # z2 was not
}
## mul
RUnit_zts_zts_mul <- function() {
    z1 <- getZts(2)
    z2 <- getZts(3)
    z3 <- getZts(6)
    all(`*`(z1, z2) == z3 &
        z1 == z1 &                        # z1 was not modified
        z2 == z2)                         # neither was z2
}
RUnit_zts_zts_mul_inplace <- function() {
    z1 <- getZts(2)
    z2 <- getZts(3)
    z3 <- getZts(6)
    all(`*`(--z1, z2) == z3 &
        z1 == z3 &                        # z1 was modified
        z2 == z2)                         # z2 was not
}
## div
RUnit_zts_zts_div <- function() {
    z1 <- getZts(1)
    z2 <- getZts(2)
    z3 <- getZts(0.5)
    all(`/`(z1, z2) == z3 &
        z1 == z1 &                        # z1 was not modified
        z2 == z2)                         # neither was z2
}
RUnit_zts_zts_div_inplace <- function() {
    z1 <- getZts(1)
    z2 <- getZts(2)
    z3 <- getZts(0.5)
    all(`/`(--z1, z2) == z3 &
        z1 == z3 &                        # z1 was modified
        z2 == z2)                         # z2 was not
}
## power
RUnit_zts_zts_power <- function() {
    z1 <- getZts(2)
    z2 <- getZts(3)
    z3 <- getZts(8)
    all(`^`(z1, z2) == z3 &
        z1 == z1 &                        # z1 was not modified
        z2 == z2)                         # neither was z2
}
RUnit_zts_zts_power_inplace <- function() {
    z1 <- getZts(2)
    z2 <- getZts(3)
    z3 <- getZts(8)
    all(`^`(--z1, z2) == z3 &
        z1 == z3 &                        # z1 was modified
        z2 == z2)                         # z2 was not
}
## zts_double ------------
## plus
RUnit_zts_double_plus <- function() {
    z1 <- getZts(1)
    d <- matrix(2, 3, 2)
    z3 <- getZts(3)
    all(`+`(z1, d) == z3 &
        z1 == z1 &                        # z1 was not modified
        d == d)                           # neither was z2
}
RUnit_zts_double_plus_inplace <- function() {
    z1 <- getZts(1)
    d <- matrix(2, 3, 2)
    z3 <- getZts(3)
    all(`+`(--z1, d) == z3 &
        z1 == z3 &                        # z1 was modified
        d == d)                           # z2 was not
}
## minus
RUnit_zts_double_minus <- function() {
    z1 <- getZts(1)
    d <- matrix(2, 3, 2)
    z3 <- getZts(-1)
    all(`-`(z1, d) == z3 &
        z1 == z1 &                        # z1 was not modified
        d == d)                           # neither was z2
}
RUnit_zts_double_minus_inplace <- function() {
    z1 <- getZts(1)
    d <- matrix(2, 3, 2)
    z3 <- getZts(-1)
    all(`-`(--z1, d) == z3 &
        z1 == z3 &                        # z1 was modified
        d == d)                           # z2 was not
}
## mul
RUnit_zts_double_mul <- function() {
    z1 <- getZts(2)
    d <- matrix(3, 3, 2)
    z3 <- getZts(6)
    all(`*`(z1, d) == z3 &
        z1 == z1 &                        # z1 was not modified
        d == d)                           # neither was z2
}
RUnit_zts_double_mul_inplace <- function() {
    z1 <- getZts(2)
    d <- matrix(3, 3, 2)
    z3 <- getZts(6)
    all(`*`(--z1, d) == z3 &
        z1 == z3 &                        # z1 was modified
        d == d)                           # z2 was not
}
## div
RUnit_zts_double_div <- function() {
    z1 <- getZts(1)
    d <- matrix(2, 3, 2)
    z3 <- getZts(0.5)
    all(`/`(z1, d) == z3 &
        z1 == z1 &                        # z1 was not modified
        d == d)                           # neither was z2
}
RUnit_zts_double_div_inplace <- function() {
    z1 <- getZts(1)
    d <- matrix(2, 3, 2)
    z3 <- getZts(0.5)
    all(`/`(--z1, d) == z3 &
        z1 == z3 &                        # z1 was modified
        d == d)                           # z2 was not
}
## power
RUnit_zts_double_power <- function() {
    z1 <- getZts(2)
    d <- matrix(3, 3, 2)
    z3 <- getZts(8)
    all(`^`(z1, d) == z3 &
        z1 == z1 &                        # z1 was not modified
        d == d)                           # neither was z2
}
RUnit_zts_double_power_inplace <- function() {
    z1 <- getZts(2)
    d <- matrix(3, 3, 2)
    z3 <- getZts(8)
    all(`^`(--z1, d) == z3 &
        z1 == z3 &                        # z1 was modified
        d == d)                           # z2 was not
}
## time_duration ------------
## plus
RUnit_time_duration_add <- function() {
    a <- matrix(|.2016-09-13 17:09:00 America/New_York.|, 2, 3)
    b <- matrix(as.duration(2e9), 2, 3)
    res <- matrix(|.2016-09-13 17:09:02 America/New_York.|, 2, 3)
    all(`+`(a, b) == res &
        a == a &                        # a was not modified
        b == b)                         # neither was b
}
RUnit_time_duration_add_inplace <- function() {
    a <- matrix(|.2016-09-13 17:09:00 America/New_York.|, 2, 3)
    b <- matrix(as.duration(2e9), 2, 3)
    res <- matrix(|.2016-09-13 17:09:02 America/New_York.|, 2, 3)
    all(`+`(--a, b) == res &
        a == res &                        # a was modified
        b == b)                           # b was not
}
## minus
RUnit_time_duration_minus <- function() {
    a <- matrix(|.2016-09-13 17:09:00 America/New_York.|, 2, 3)
    b <- matrix(as.duration(2e9), 2, 3)
    res <- matrix(|.2016-09-13 17:08:58 America/New_York.|, 2, 3)
    all(`-`(a, b) == res &
        a == a &                        # a was not modified
        b == b)                         # neither was b
}
RUnit_time_duration_minus_inplace <- function() {
    a <- matrix(|.2016-09-13 17:09:00 America/New_York.|, 2, 3)
    b <- matrix(as.duration(2e9), 2, 3)
    res <- matrix(|.2016-09-13 17:08:58 America/New_York.|, 2, 3)
    all(`-`(--a, b) == res &
        a == res &                        # a was modified
        b == b)                           # b was not
}
## time_period ------------
## plus
RUnit_time_period_add <- function() {
    a <- matrix(|.2016-09-13 17:09:00 America/New_York.|, 2, 3)
    b <- matrix(as.period("2m2d"), 2, 3)
    res <- matrix(|.2016-11-15 17:09:00 America/New_York.|, 2, 3)
    all(`+`(a, b, "America/New_York") == res &
        a == a &                        # a was not modified
        b == b)                         # neither was b
}
RUnit_time_period_add_inplace <- function() {
    a <- matrix(|.2016-11-15 17:09:00 America/New_York.|, 2, 3)
    b <- matrix(as.period("-2m-2d"), 2, 3)
    res <- matrix(|.2016-09-13 17:09:00 America/New_York.|, 2, 3)
    all(`+`(--a, b, "America/New_York") == res &
        a == res &                        # a was modified
        b == b)                           # b was not
}
## minus
RUnit_time_period_minus <- function() {
    a <- matrix(|.2016-11-15 17:09:00 America/New_York.|, 2, 3)
    b <- matrix(as.period("2m2d"), 2, 3)
    res <- matrix(|.2016-09-13 17:09:00 America/New_York.|, 2, 3)
    all(`-`(a, b, "America/New_York") == res &
        a == a &                        # a was not modified
        b == b)                         # neither was b
}
RUnit_time_period_minus_inplace <- function() {
    a <- matrix(|.2016-11-13 17:09:00 America/New_York.|, 2, 3)
    b <- matrix(as.period("2m2d"), 2, 3)
    res <- matrix(|.2016-09-11 17:09:00 America/New_York.|, 2, 3)
    all(`-`(--a, b, "America/New_York") == res &
        a == res &                        # a was modified
        b == b)                           # b was not
}
## duration_duration ------------
## plus
RUnit_duration_add <- function() {
    a <- matrix(as.duration(1), 2, 3)
    b <- matrix(as.duration(2), 2, 3)
    all(`+`(a, b) == matrix(as.duration(3), 2, 3) &
        a == a &                        # a was not modified
        b == b)                         # neither was b
}
RUnit_duration_add_inplace <- function() {
    a <- matrix(as.duration(1), 2, 3)
    b <- matrix(as.duration(2), 2, 3)
    res <- matrix(as.duration(3), 2, 3)
    all(`+`(--a, b) == res &
        a == res &                        # a was modified
        b == b)                           # b was not
}
## minus
RUnit_duration_minus <- function() {
    a <- matrix(as.duration(1), 2, 3)
    b <- matrix(as.duration(2), 2, 3)
    all(`-`(a, b) == matrix(as.duration(-1), 2, 3) &
        a == a &                        # a was not modified
        b == b)                         # neither was b
}
RUnit_duration_minus_inplace <- function() {
    a <- matrix(as.duration(1), 2, 3)
    b <- matrix(as.duration(2), 2, 3)
    res <- matrix(as.duration(-1), 2, 3)
    all(`-`(--a, b) == res &
        a == res &                        # a was modified
        b == b)                           # b was not
}
## duration_double ------------
## mul
RUnit_duration_double_mul <- function() {
    a <- matrix(as.duration(2), 2, 3)
    b <- matrix(3, 2, 3)
    all(`*`(a, b) == matrix(as.duration(6), 2, 3) &
        a == a &                        # a was not modified
        b == b)                         # neither was b
}
RUnit_duration_double_mul_inplace <- function() {
    a <- matrix(as.duration(2), 2, 3)
    b <- matrix(3, 2, 3)
    res <- matrix(as.duration(6), 2, 3)
    all(`*`(--a, b) == res &
        a == res &                        # a was modified
        b == b)                           # b was not
}
## div
RUnit_duration_double_div <- function() {
    a <- matrix(as.duration(3), 2, 3)
    b <- matrix(2, 2, 3)
    all(`/`(a, b) == matrix(as.duration(1.5), 2, 3) &
        a == a &                        # a was not modified
        b == b)                         # neither was b
}
RUnit_duration_double_div_inplace <- function() {
    a <- matrix(as.duration(3), 2, 3)
    b <- matrix(2, 2, 3)
    res <- matrix(as.duration(1), 2, 3)
    all(`/`(--a, b) == res &
        a == res &                        # a was modified
        b == b)                           # b was not
}
## interval_duration ------------
## plus
RUnit_interval_duration_add <- function() {
    a <- matrix(|+2016-09-13 17:09:00 America/New_York->2016-09-14 17:09:00 America/New_York-|, 2, 3)
    b <- matrix(as.duration(2e9), 2, 3)
    res <- matrix(|+2016-09-13 17:09:02 America/New_York->2016-09-14 17:09:02 America/New_York-|, 2, 3)
    all(`+`(a, b) == res &
        a == a &                        # a was not modified
        b == b)                         # neither was b
}
RUnit_interval_duration_add_inplace <- function() {
    a <- matrix(|+2016-09-13 17:09:00 America/New_York->2016-09-14 17:09:00 America/New_York-|, 2, 3)
    b <- matrix(as.duration(2e9), 2, 3)
    res <- matrix(|+2016-09-13 17:09:02 America/New_York->2016-09-14 17:09:02 America/New_York-|, 2, 3)
    all(`+`(--a, b) == res &
        a == res &                        # a was modified
        b == b)                           # b was not
}
## minus
RUnit_interval_duration_minus <- function() {
    a <- matrix(|+2016-09-13 17:09:00 America/New_York->2016-09-14 17:09:00 America/New_York-|, 2, 3)
    b <- matrix(as.duration(2e9), 2, 3)
    res <- matrix(|+2016-09-13 17:08:58 America/New_York->2016-09-14 17:08:58 America/New_York-|, 2, 3)
    all(`-`(a, b) == res &
        a == a &                        # a was not modified
        b == b)                         # neither was b
}
RUnit_interval_duration_minus_inplace <- function() {
    a <- matrix(|+2016-09-13 17:09:00 America/New_York->2016-09-14 17:09:00 America/New_York-|, 2, 3)
    b <- matrix(as.duration(2e9), 2, 3)
    res <- matrix(|+2016-09-13 17:08:58 America/New_York->2016-09-14 17:08:58 America/New_York-|, 2, 3)
    all(`-`(--a, b) == res &
        a == res &                        # a was modified
        b == b)                           # b was not
}
## interval_period ------------
## plus
RUnit_interval_period_add <- function() {
    a <- matrix(|+2016-09-13 17:09:00 America/New_York->2016-09-14 17:09:00 America/New_York-|, 2, 3)
    b <- matrix(as.period("2m2d"), 2, 3)
    res <- matrix(|+2016-11-15 17:09:00 America/New_York->2016-11-16 17:09:00 America/New_York-|, 2, 3)
    all(`+`(a, b, "America/New_York") == res &
        a == a &                        # a was not modified
        b == b)                         # neither was b
}
RUnit_interval_period_add_inplace <- function() {
    a <- matrix(|+2016-11-15 17:09:00 America/New_York->2016-11-16 17:09:00 America/New_York-|, 2, 3)
    b <- matrix(as.period("-2m-2d"), 2, 3)
    res <- matrix(|+2016-09-13 17:09:00 America/New_York->2016-09-14 17:09:00 America/New_York-|, 2, 3)
    all(`+`(--a, b, "America/New_York") == res &
        a == res &                        # a was modified
        b == b)                           # b was not
}
## minus
RUnit_interval_period_minus <- function() {
    a <- matrix(|+2016-11-15 17:09:00 America/New_York->2016-11-16 17:09:00 America/New_York-|, 2, 3)
    b <- matrix(as.period("2m2d"), 2, 3)
    res <- matrix(|+2016-09-13 17:09:00 America/New_York->2016-09-14 17:09:00 America/New_York-|, 2, 3)
    all(`-`(a, b, "America/New_York") == res &
        a == a &                        # a was not modified
        b == b)                         # neither was b
}
RUnit_interval_period_minus_inplace <- function() {
    a <- matrix(|+2016-11-13 17:09:00 America/New_York->2016-11-14 17:09:00 America/New_York-|, 2, 3)
    b <- matrix(as.period("2m2d"), 2, 3)
    res <- matrix(|+2016-09-11 17:09:00 America/New_York->2016-09-12 17:09:00 America/New_York-|, 2, 3)
    all(`-`(--a, b, "America/New_York") == res &
        a == res &                        # a was modified
        b == b)                           # b was not
}
## period_period ------------
## plus
RUnit_period_add <- function() {
    a <- matrix(as.period("1m1d"), 2, 3)
    b <- matrix(as.period("2m2d"), 2, 3)
    all(`+`(a, b) == matrix(as.period("3m3d"), 2, 3) &
        a == a &                        # a was not modified
        b == b)                         # neither was b
}
RUnit_period_add_inplace <- function() {
    a <- matrix(as.period("1m1d"), 2, 3)
    b <- matrix(as.period("2m2d"), 2, 3)
    res <- matrix(as.period("3m3d"), 2, 3)
    all(`+`(--a, b) == res &
        a == res &                        # a was modified
        b == b)                           # b was not
}
## minus
RUnit_period_minus <- function() {
    a <- matrix(as.period("1m1d"), 2, 3)
    b <- matrix(as.period("2m2d"), 2, 3)
    all(`-`(a, b) == matrix(as.period("-1m-1d"), 2, 3) &
        a == a &                        # a was not modified
        b == b)                         # neither was b
}
RUnit_period_minus_inplace <- function() {
    a <- matrix(as.period("1m1d"), 2, 3)
    b <- matrix(as.period("2m2d"), 2, 3)
    res <- matrix(as.period("-1m-1d"), 2, 3)
    all(`-`(--a, b) == res &
        a == res &                        # a was modified
        b == b)                           # b was not
}

## logical, logical ------------
## or
RUnit_logical_or <- function() {
    a <- matrix(c(T,T,T,F,F,F), 2, 3)
    b <- matrix(c(T,F,T,F,T,F), 2, 3)
    all(`|`(a, b) == matrix(c(T,T,T,F,T,F), 2, 3) &
        a == a &                        # a was not modified
        b == b)                         # neither was b
}
RUnit_logical_or_inplace <- function() {
    a <- matrix(c(T,T,T,F,F,F), 2, 3)
    b <- matrix(c(T,F,T,F,T,F), 2, 3)
    res <- matrix(c(T,T,T,F,T,F), 2, 3)
    all(`|`(--a, b) == res &
        a == res &                        # a was modified
        b == b)                           # b was not
}
## and
RUnit_logical_and <- function() {
    a <- matrix(c(T,T,T,F,F,F), 2, 3)
    b <- matrix(c(T,F,T,F,T,F), 2, 3)
    all(`&`(a, b) == matrix(c(T,F,T,F,F,F), 2, 3) &
        a == a &                        # a was not modified
        b == b)                         # neither was b
}
RUnit_logical_and_inplace <- function() {
    a <- matrix(c(T,T,T,F,F,F), 2, 3)
    b <- matrix(c(T,F,T,F,T,F), 2, 3)
    res <- matrix(c(T,F,T,F,F,F), 2, 3)
    all(`&`(--a, b) == res &
        a == res &                        # a was modified
        b == b)                           # b was not
}
