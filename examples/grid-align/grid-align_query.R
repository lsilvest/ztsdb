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

## create a connection 'c1' towards a ztsdb instance
c1 <- connection("127.0.0.1", 12300)

## create on the remote ztsdb instance some data so that we can
## test the query:
c1 ? { d <- as.time("2015-01-01 00:00:00 America/New_York")
       start <- as.time("2015-01-01 00:00:00 America/New_York")
       idx <- c(start,
                start + cumsum(runif(1e6-1))*as.duration("00:00:01"),
                start + 10*as.duration("24:00:00") + cumsum(runif(1e6,0,0.5))*as.duration("00:00:01"))
       z   <<- zts(idx, matrix(1:2e6, 2e6, 1)) }


## get a few densities:
minutes <- (c1 ? density(z, as.duration("00:01:00")))
hours   <- (c1 ? density(z, as.duration("01:00:00")))
days    <- (c1 ? density(z, as.period("1d"), tz=++tz))
## the following shows how to force the start of the day on a boundary
## (here z is already aligned, so it has no effect):
days2   <- (c1 ? { start  <- floor(head(zts.idx(z),1), "day", tz=++tz)
                   density(z, as.period("1d"), start, tz=++tz) })

## to get a sample of densities:
hourly_minute_sample <- (c1 ? density(z, as.duration("01:00:00"), ival=as.duration("00:01:00")))


## if running in an R session, this will generate a plot of the above:
##
## png("density.png", width=10, height=5, units="in", res=100)
## par(mfrow=c(1,3))
## plot.xts(minutes, main="minute density");
## plot.xts(hours, main="hourly density");
## plot.xts(days, main="dayly density")
## dev.off()
##
## png("density_sample.png")
## plot.xts(hourly_minute_sample, main="minute density");
## dev.off()



## get dayly mean, min, max aggregations:
days_mean <- (c1 ? grid_align(z, as.duration("24:00:00"), start, method="mean"))
days_min < - (c1 ? grid_align(z, as.duration("24:00:00"), start, method="min"))
days_max  <- (c1 ? grid_align(z, as.duration("24:00:00"), start, method="max"))


## if running in an R session, this will generate a plot of the above:
##
## png("grid_align.png", width=10, height=5, units="in", res=100)
## par(mfrow=c(1,3))
## plot.xts(days_mean,ylim=c(0,2e6),main="daily mean");
## plot.xts(days_min,ylim=c(0,2e6), main="daily min");
## plot.xts(days_max,ylim=c(0,2e6), main="daily max")
## dev.off()
