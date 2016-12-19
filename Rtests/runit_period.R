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


RUnit_period_contructor <- function() {
    as.period("1w") == as.period("7d") &
    as.period("1y1w1d") == as.period("12m8d") &
    as.period("1w-1d") == as.period("6d")
    as.period("0w0d") == as.period("0d")
}
RUnit_period_contructor_from_numbers <- function() {
    period(1,2)  == as.period("1m2d") &
    period(0,0)  == as.period("0m0d")
    period(-1,7) == as.period("-1m1w")
}
RUnit_period_add <- function() {
    as.period("1d")   + as.period("2d")   == as.period("3d")    &
    as.period("1d")   + as.period("-2d")  == as.period("-1d")   &
    as.period("1m1d") + as.period("2d")   == as.period("1m3d")  &
    as.period("1m1d") + as.period("1m2d") == as.period("2m3d")  &
    as.period("1d")   + as.period("-1m")  == as.period("-1m1d") &
    as.period("1m")   + as.period("-1d")  == as.period("1m-1d")
}
RUnit_period_sub <- function() {
    as.period("1d")     - as.period("2d")   == as.period("-1d")   &
    as.period("1d")     - as.period("-2d")  == as.period("3d")    &
    as.period("1m2d")   - as.period("2d")   == as.period("1m0d")  &
    as.period("1m1d")   - as.period("1m2d") == as.period("-1d")   &
    as.period("-1d")    - as.period("-1m")  == as.period("1m-1d") &
    as.period("-1m-3d") - as.period("-4d")  == as.period("-1m1d")
}
RUnit_period_mul <- function() {
    as.period("1d") * 3 == as.period("3d") &
    3 * as.period("-2d") == as.period("-6d") &
    as.period("1m2d") * 2 == as.period("2m4d") &
    as.period("1m1d") * -2 == as.period("-2m-2d")
}
RUnit_period_div <- function() {
    as.period("1d") / 2 == as.period("0d") &
    as.period("10d") / -2 == as.period("-5d") &
    as.period("10m2d") / 2  == as.period("5m1d") &
    as.period("-100m") / 100 == as.period("-1m")
}
RUnit_dt_period_add <- function() {
    dt <- |.2016-01-01 12:00:00 America/New_York.|
    tz <- "America/New_York"
    `+`(dt, as.period("120d"), tz) == |.2016-04-30 12:00:00 America/New_York.| &
    `+`(as.period("-365d"), dt, tz) == |.2015-01-01 12:00:00 America/New_York.| &
    `+`(as.period("3m2d"), dt, tz) == |.2016-04-03 12:00:00 America/New_York.| &
    `+`(dt, as.period("1y"), tz) == |.2017-01-01 12:00:00 America/New_York.| &
    `+`(as.period("1w"), dt, tz) == |.2016-01-08 12:00:00 America/New_York.|
}
RUnit_dt_period_sub <- function() {
    dt <- |.2016-01-01 12:00:00 America/New_York.|
    tz <- "America/New_York"
    `-`(dt, as.period("365d"), tz) == |.2015-01-01 12:00:00 America/New_York.| &
    `-`(dt, as.period("-365d"), tz) == |.2016-12-31 12:00:00 America/New_York.| & # leap year
    `-`(dt, as.period("3m2d"), tz) == |.2015-09-29 12:00:00 America/New_York.| &
    `-`(dt, as.period("1y"), tz) == |.2015-01-01 12:00:00 America/New_York.| &
    `-`(dt, as.period("1w"), tz) == |.2015-12-25 12:00:00 America/New_York.|
}
