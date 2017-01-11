## Copyright (C) 2016,2017 Leonardo Silvestri
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


RUnit_seq_numerical <- function() {
    a <- seq(1, 4, 1)
    b <- seq(1, 4)
    d <- seq(1, length.out=4)
    e <- seq(1, by=1, length.out=4)
    f <- c(1,2,3,4)
    all(a == b) & all(b == d) & all(d == e) & all(e == f)
}
RUnit_seq_numerical_neg <- function() {
    a <- seq(4, 1, -1)
    b <- seq(4, 1)
    d <- seq(4, by=-1, length.out=4)
    e <- c(4,3,2,1)
    all(a == b) & all(b == d) & all(d == e)
}
RUnit_seq_time_duration <- function() {
    a <- seq(|.2015-01-01 12:00:00 America/New_York.|, by=as.duration(3600*1e9), length.out=4)
    b <- seq(|.2015-01-01 12:00:00 America/New_York.|,
             |.2015-01-01 15:00:00 America/New_York.|,
             by=as.duration(3600*1e9))
    d <- c(|.2015-01-01 12:00:00 America/New_York.|,
           |.2015-01-01 13:00:00 America/New_York.|,
           |.2015-01-01 14:00:00 America/New_York.|,
           |.2015-01-01 15:00:00 America/New_York.|)
    all(a == b) & all(b == d)
}
RUnit_seq_time_duration_neg <- function() {
    a <- seq(|.2015-01-01 12:00:00 America/New_York.|, by=as.duration(-3600*1e9), length.out=4)
    b <- seq(|.2015-01-01 12:00:00 America/New_York.|,
             |.2015-01-01 09:00:00 America/New_York.|,
             by=as.duration(-3600*1e9))
    d <- c(|.2015-01-01 12:00:00 America/New_York.|,
           |.2015-01-01 11:00:00 America/New_York.|,
           |.2015-01-01 10:00:00 America/New_York.|,
           |.2015-01-01 09:00:00 America/New_York.|)
    all(a == b) & all(b == d)
}
RUnit_seq_interval_duration <- function() {
    a <- seq(|+2015-01-01 12:00:00 America/New_York -> 2015-01-01 13:00:00 America/New_York-|,
             by=as.duration(3600*1e9), length.out=4)
    b <- seq(|+2015-01-01 12:00:00 America/New_York -> 2015-01-01 13:00:00 America/New_York-|,
             |+2015-01-01 15:00:00 America/New_York -> 2015-01-01 16:00:00 America/New_York-|,
             by=as.duration(3600*1e9))
    d <- c(|+2015-01-01 12:00:00 America/New_York -> 2015-01-01 13:00:00 America/New_York-|,
           |+2015-01-01 13:00:00 America/New_York -> 2015-01-01 14:00:00 America/New_York-|,
           |+2015-01-01 14:00:00 America/New_York -> 2015-01-01 15:00:00 America/New_York-|,
           |+2015-01-01 15:00:00 America/New_York -> 2015-01-01 16:00:00 America/New_York-|)
    all(a == b) && all(b == d)
}
RUnit_seq_interval_duration_neg <- function() {
    a <- seq(|+2015-01-01 12:00:00 America/New_York -> 2015-01-01 13:00:00 America/New_York-|,
             by=as.duration(-3600*1e9), length.out=4)
    b <- seq(|+2015-01-01 12:00:00 America/New_York -> 2015-01-01 13:00:00 America/New_York-|,
             |+2015-01-01 09:00:00 America/New_York -> 2015-01-01 10:00:00 America/New_York-|,
             by=as.duration(-3600*1e9))
    d <- c(|+2015-01-01 12:00:00 America/New_York -> 2015-01-01 13:00:00 America/New_York-|,
           |+2015-01-01 11:00:00 America/New_York -> 2015-01-01 12:00:00 America/New_York-|,
           |+2015-01-01 10:00:00 America/New_York -> 2015-01-01 11:00:00 America/New_York-|,
           |+2015-01-01 09:00:00 America/New_York -> 2015-01-01 10:00:00 America/New_York-|)
    all(a == b) && all(b == d)
}
RUnit_seq_time_period_1d<- function() {
    ## the following sequence goes from EDT to EST
    a <- seq(|.2016-11-04 12:00:00 America/New_York.|, by=as.period("1d"),
             length.out=4, tz="America/New_York")
    exp <- c(|.2016-11-04 12:00:00 America/New_York.|,
             |.2016-11-05 12:00:00 America/New_York.|,
             |.2016-11-06 12:00:00 America/New_York.|,
             |.2016-11-07 12:00:00 America/New_York.|)
    all.equal(a, exp)
}
RUnit_seq_time_period_neg <- function() {
    a <- seq(|.2015-01-01 12:00:00 America/New_York.|,
             by=as.period("/-01:00:00"), length.out=4, tz="America/New_York")
    b <- seq(|.2015-01-01 12:00:00 America/New_York.|,
             |.2015-01-01 09:00:00 America/New_York.|,
             by=as.period("/-01:00:00"), tz="America/New_York")
    d <- c(|.2015-01-01 12:00:00 America/New_York.|,
           |.2015-01-01 11:00:00 America/New_York.|,
           |.2015-01-01 10:00:00 America/New_York.|,
           |.2015-01-01 09:00:00 America/New_York.|)
    all(a == b) & all(b == d)
}
RUnit_seq_time_period_1y <- function() {
    a <- seq(|.2016-11-04 12:00:00 America/New_York.|, by=as.period("1y"),
             length.out=4, tz="America/New_York")
    exp <- c(|.2016-11-04 12:00:00 America/New_York.|,
             |.2017-11-04 12:00:00 America/New_York.|,
             |.2018-11-04 12:00:00 America/New_York.|,
             |.2019-11-04 12:00:00 America/New_York.|)
    all.equal(a, exp)
}
RUnit_seq_time_period_1y1d <- function() {
    ## the following sequence goes from EDT to EST
    a <- seq(|.2016-11-04 12:00:00 America/New_York.|, by=as.period("1y1d"),
             length.out=4, tz="America/New_York")
    exp <- c(|.2016-11-04 12:00:00 America/New_York.|,
             |.2017-11-05 12:00:00 America/New_York.|,
             |.2018-11-06 12:00:00 America/New_York.|,
             |.2019-11-07 12:00:00 America/New_York.|)
    all.equal(a, exp)
}
RUnit_seq_time_period_1y_minus_1d <- function() {
    a <- seq(|.2016-11-07 12:00:00 America/New_York.|, by=as.period("1y-1d"),
             length.out=4, tz="America/New_York")
    exp <- c(|.2016-11-07 12:00:00 America/New_York.|,
             |.2017-11-06 12:00:00 America/New_York.|,
             |.2018-11-05 12:00:00 America/New_York.|,
             |.2019-11-04 12:00:00 America/New_York.|)
    all.equal(a, exp)
}
RUnit_seq_time_period_1y_minus_1d_minus_12h <- function() {
   a <- seq(|.2016-11-07 12:00:00 America/New_York.|, by=as.period("1y-1d/-12:00:00"),
             length.out=4, tz="America/New_York")
    exp <- c(|.2016-11-07 12:00:00 America/New_York.|,
             |.2017-11-06 00:00:00 America/New_York.|,
             |.2018-11-04 12:00:00 America/New_York.|,
             |.2019-11-03 00:00:00 America/New_York.|)
    all.equal(a, exp)
}
RUnit_seq_time_period_duration <- function() {
    a <- seq(|.2015-01-01 12:00:00 America/New_York.|,
             by=as.period("/01:00:00"), length.out=4, tz="America/New_York")
    b <- seq(|.2015-01-01 12:00:00 America/New_York.|,
             |.2015-01-01 15:00:00 America/New_York.|,
             by=as.period("/01:00:00"), tz="America/New_York")
    d <- c(|.2015-01-01 12:00:00 America/New_York.|,
           |.2015-01-01 13:00:00 America/New_York.|,
           |.2015-01-01 14:00:00 America/New_York.|,
           |.2015-01-01 15:00:00 America/New_York.|)
    all(a == b) & all(b == d)
}
RUnit_seq_interval_period_duration <- function() {
    a <- seq(|+2015-01-01 12:00:00 America/New_York -> 2015-01-01 13:00:00 America/New_York-|,
             by=as.period("/01:00:00"), length.out=4, tz="America/New_York")
    b <- seq(|+2015-01-01 12:00:00 America/New_York -> 2015-01-01 13:00:00 America/New_York-|,
             |+2015-01-01 15:00:00 America/New_York -> 2015-01-01 16:00:00 America/New_York-|,
             by=as.period("/01:00:00"), tz="America/New_York")
    d <- c(|+2015-01-01 12:00:00 America/New_York -> 2015-01-01 13:00:00 America/New_York-|,
           |+2015-01-01 13:00:00 America/New_York -> 2015-01-01 14:00:00 America/New_York-|,
           |+2015-01-01 14:00:00 America/New_York -> 2015-01-01 15:00:00 America/New_York-|,
           |+2015-01-01 15:00:00 America/New_York -> 2015-01-01 16:00:00 America/New_York-|)
    all(a == b) && all(b == d)
}
RUnit_seq_interval_period_neg <- function() {
    a <- seq(|+2015-01-01 12:00:00 America/New_York -> 2015-01-01 13:00:00 America/New_York-|,
             by=as.period("-1y-1m/-01:00:00"), length.out=4, tz="America/New_York")
    b <- seq(|+2015-01-01 12:00:00 America/New_York -> 2015-01-01 13:00:00 America/New_York-|,
             |+2011-10-01 09:00:00 America/New_York -> 2011-10-01 10:00:00 America/New_York-|,
             by=as.period("-1y-1m/-01:00:00"), tz="America/New_York")
    d <- c(|+2015-01-01 12:00:00 America/New_York -> 2015-01-01 13:00:00 America/New_York-|,
           |+2013-12-01 11:00:00 America/New_York -> 2013-12-01 12:00:00 America/New_York-|,
           |+2012-11-01 10:00:00 America/New_York -> 2012-11-01 11:00:00 America/New_York-|,
           |+2011-10-01 09:00:00 America/New_York -> 2011-10-01 10:00:00 America/New_York-|)
    all(a == b) && all(b == d)
}
RUnit_seq_nan <- function() {
    tryCatch(seq(from=NaN, to=1, by=1),    .Last.error == "argument cannot be 'NaN' or 'Inf'") &
    tryCatch(seq(from=1,   to=NaN, by=1),  .Last.error == "argument cannot be 'NaN' or 'Inf'") &
    tryCatch(seq(from=1,   to=2,  by=NaN), .Last.error == "argument cannot be 'NaN' or 'Inf'") &
    tryCatch(seq(from=1, length.out=NaN),
             .Last.error == "argument cannot be converted to an integer") &
    tryCatch(1:NaN,   .Last.error == "argument cannot be 'NaN' or 'Inf'") &
    tryCatch(NaN:NaN, .Last.error == "argument cannot be 'NaN' or 'Inf'") &
    tryCatch(NaN:1,   .Last.error == "argument cannot be 'NaN' or 'Inf'")
}
RUnit_seq_inf <- function() {
    tryCatch(seq(from=Inf, to=1, by=1),    .Last.error == "argument cannot be 'NaN' or 'Inf'") &
    tryCatch(seq(from=1,   to=Inf, by=1),  .Last.error == "argument cannot be 'NaN' or 'Inf'") &
    tryCatch(seq(from=1,   to=2,  by=Inf), .Last.error == "argument cannot be 'NaN' or 'Inf'") &
    tryCatch(seq(from=1, length.out=Inf),
             .Last.error == "argument cannot be converted to an integer") &
    tryCatch(1:Inf,   .Last.error == "argument cannot be 'NaN' or 'Inf'") &
    tryCatch(Inf:Inf, .Last.error == "argument cannot be 'NaN' or 'Inf'") &
    tryCatch(Inf:1,   .Last.error == "argument cannot be 'NaN' or 'Inf'")
}
RUnit_seq_minus_inf <- function() {
    tryCatch(seq(from=-Inf, to=1, by=1),    .Last.error == "argument cannot be 'NaN' or 'Inf'") &
    tryCatch(seq(from=1,   to=-Inf, by=1),  .Last.error == "argument cannot be 'NaN' or 'Inf'") &
    tryCatch(seq(from=1,   to=2,  by=-Inf), .Last.error == "argument cannot be 'NaN' or 'Inf'") &
    tryCatch(seq(from=1, length.out=-Inf),
             .Last.error == "argument cannot be negative") &
    tryCatch(1:-Inf,    .Last.error == "argument cannot be 'NaN' or 'Inf'") &
    tryCatch(-Inf:-Inf, .Last.error == "argument cannot be 'NaN' or 'Inf'") &
    tryCatch(-Inf:1,    .Last.error == "argument cannot be 'NaN' or 'Inf'")
}
                           
