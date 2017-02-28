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


tz <- "America/New_York"

## for this example, create a continuous 1-second increment time
## series with random data:
start <- |.2015-01-01 00:00:00 America/New_York.|
end   <- |.2016-01-01 00:00:00 America/New_York.|
idx   <- seq(start, end, by=as.duration("00:00:01"))
## data  <- runif(nrow(idx)*3)
data <- array(1, c(nrow(idx)*3))
z     <- zts(idx, data)

## create a daily interval between 9am and 4pm for all the above days
_9am   <- as.duration("09:00:00")
_4pm   <- as.duration("16:00:00")
ivl   <- interval(seq(`+`(start,_9am,tz), `+`(end,_9am,tz), by=as.period("1d"), tz=tz),
                  seq(`+`(start,_4pm,tz), `+`(end,_4pm,tz), by=as.period("1d"), tz=tz))

## create an interval of weekends:
days    <- seq(start, end, by=as.period("1d"), tz=tz) # sequence of days
we_days <- days[dayweek(days, tz) == 6 | dayweek(days, tz) == 0]
we_ivl  <- interval(we_days, `+`(we_days,as.period("1d"),tz))

## take the set difference of the daily interval with the weekends:
ivl_week <- setdiff(ivl, we_ivl)

## finally, subset the time series to get only the observations
## between open and close on weekdays:
z_open_close <- z[ivl_week, ]
