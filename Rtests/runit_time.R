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

## constructors
RUnit_from_numbers <- function() {
    tz <- "America/New_York"
    time(2012, 1, 12, tz=tz) == |.2012-01-12 00:00:00 America/New_York.| &
    time(2012, 1, 12, 12, tz=tz) == |.2012-01-12 12:00:00 America/New_York.| &
    time(2012, 1, 12, 12, 12, tz=tz) == |.2012-01-12 12:12:00 America/New_York.| &
    time(2012, 1, 12, 12, 12, 12, tz=tz) == |.2012-01-12 12:12:12 America/New_York.| &
    time(2012, 1, 12, 12, 12, 12, 9e8, tz=tz) == |.2012-01-12 12:12:12.9 America/New_York.|
}
RUnit_from_numbers_daylight <- function() {
    tz <- "America/New_York"
    time(2012, 7, 12, tz=tz) == |.2012-07-12 00:00:00 America/New_York.| &
    time(2012, 7, 12, 12, tz=tz) == |.2012-07-12 12:00:00 America/New_York.| &
    time(2012, 7, 12, 12, 12, tz=tz) == |.2012-07-12 12:12:00 America/New_York.| &
    time(2012, 7, 12, 12, 12, 12, tz=tz) == |.2012-07-12 12:12:12 America/New_York.| &
    time(2012, 7, 12, 12, 12, 12, 9e8, tz=tz) == |.2012-07-12 12:12:12.9 America/New_York.|
}
RUnit_from_numbers_array <- function() {
    y <- matrix(2011:2014, 2, 2)
    o <- matrix(6:9, 2, 2)
    tz <- "America/New_York"
    d <- c(|.2011-06-06 06:00:00 America/New_York.|, |.2012-07-07 07:00:00 America/New_York.|,
           |.2013-08-08 08:00:00 America/New_York.|, |.2014-09-09 09:00:00 America/New_York.|)
    all(time(y, o, o, o, tz=tz) == matrix(d, 2, 2))
}
## accessors
RUnit_dayweek <- function() {
    dayweek(|.2016-09-26 12:12:12 UTC.|, "America/New_York") == 1
}
RUnit_daymonth <- function() {
    daymonth(|.2016-09-26 12:12:12 UTC.|, "America/New_York") == 26
}
RUnit_month <- function() {
    month(|.2016-09-26 12:12:12 UTC.|, "America/New_York") == 9
}
RUnit_year <- function() {
    year(|.2016-09-26 12:12:12 UTC.|, "America/New_York") == 2016
}
