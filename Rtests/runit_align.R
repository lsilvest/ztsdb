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

one_second <- as.duration(1e9)
one_minute <- 60*one_second
one_hour   <- 60*one_minute

RUnit_align_idx <- function() {
    a <- seq(|.2015-01-01 12:00:00 America/New_York.|,
             |.2015-01-01 15:00:00 America/New_York.|,
             by=one_second)
    b <- seq(|.2015-01-01 13:00:00 America/New_York.|,
             |.2015-01-01 14:00:00 America/New_York.|,
             by=one_second)
    all(a[align.idx(a, b)] == b)
}
RUnit_align_closest <- function() {
    idx <- seq(|.2015-01-01 12:00:00 America/New_York.|,
               |.2015-01-01 15:00:00 America/New_York.|,
               by=one_second)
    data <- 0:10800
    z <- zts(idx, matrix(data, length(data), 1))
    b <- seq(|.2015-01-01 13:00:00 America/New_York.|,
             |.2015-01-01 14:00:00 America/New_York.|,
             by=one_minute)
    a <- align(z, b, -one_minute, method="closest")
    all(a == z[seq(3601, 7201, 60), ])
}
RUnit_align_count <- function() {
    idx <- seq(|.2015-01-01 12:00:00 America/New_York.|,
               |.2015-02-01 12:00:00 America/New_York.|,
               by=one_second)
    data <- 1:length(idx)
    z <- zts(idx, matrix(data, length(data), 1))
    ## create a gap in z, so we know what happens to the alignment
    ## when there are no observations around the time point:
    i <- c(|+2015-01-01 15:00:00 America/New_York -> 2015-01-02 15:00:00 America/New_York-|)
    z <- z[setdiff(idx, i),]
    to <- c(seq(|.2015-01-01 12:00:00 America/New_York.|,
                |.2015-02-01 00:00:00 America/New_York.|,
                by=one_hour),
            ## make a gap in the middle:
            seq(|.2015-02-01 03:00:00 America/New_York.|,
                |.2015-02-01 12:00:00 America/New_York.|,
                by=one_hour))
    a <- align(z, to, -one_hour, method="count")
    exp_data <- matrix(c(0, array(3600, length(to)-1)), length(to))
    exp <- zts(idx[to], exp_data)
    exp[1,] <- 0
    exp[|-2015-01-01 15:00:00 America/New_York -> 2015-01-02 15:00:00 America/New_York+|,] <- 0
    all(a == exp)
}
