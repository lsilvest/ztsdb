## This is free and unencumbered software released into the public domain.
##
## Anyone is free to copy, modify, publish, use, compile, sell, or
## distribute this software, either in source code form or as a compiled
## binary, for any purpose, commercial or non-commercial, and by any
## means.
##
## In jurisdictions that recognize copyright laws, the author or authors
## of this software dedicate any and all copyright interest in the
## software to the public domain. We make this dedication for the benefit
## of the public at large and to the detriment of our heirs and
## successors. We intend this dedication to be an overt act of
## relinquishment in perpetuity of all present and future rights to this
## software under copyright law.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
## EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
## MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
## IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
## OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
## ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
## OTHER DEALINGS IN THE SOFTWARE.
##
## For more information, please refer to <http://unlicense.org/>


tz <- "America/New_York"

## for this example, create a continuous 1-second increment time
## series with random data:
start <- |.2015-01-01 00:00:00 America/New_York.|
end   <- |.2016-01-01 00:00:00 America/New_York.|
idx   <- seq(start, end, by=as.duration("00:00:01"))

## unfortunately, 'rnorm' is terribly slow and that is due to the C++
## STL implementation...
data  <- 1000 + cumsum(array(rnorm(nrow(idx)*3, 0, 0.05), c(nrow(idx)*3)))
z     <- zts(idx, matrix(data, length(idx), 3))

## create a daily interval between 9am and 4pm for all the above days
_930am <- as.duration("09:30:00")
_4pm   <- as.duration("16:00:00")
ivl    <- interval(seq(`+`(start,_930am,tz), `+`(end,_930am,tz), by=as.period("1d"), tz=tz),
                   seq(`+`(start,_4pm,tz),   `+`(end,_4pm,tz),   by=as.period("1d"), tz=tz),
                   eopen=FALSE)

## create an interval of weekends:
days    <- seq(start, end, by=as.period("1d"), tz=tz) # sequence of days
we_days <- days[dayweek(days, tz) == 6 | dayweek(days, tz) == 0]
we_ivl  <- interval(we_days, `+`(we_days,as.period("1d"),tz))

## take the set difference of the daily interval with the weekends:
ivl_week <- setdiff(ivl, we_ivl)

## finally, subset the time series to get only the observations
## between open and close on weekdays:
z_open_close <- z[ivl_week, ]
