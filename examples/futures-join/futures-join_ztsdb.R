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


## CAVEAT: modify 'path' below to the correct location of the data:
##path <- "/path/to/ztsdb/examples/futures-join/data/"
path <- "data/"
if (system(paste("test -d", path))) {
  stop("can't find data dir: start ztsdb in the example dir or modify 'path' in the script")
}

id <- "CD"

## build a vector of roll dates
tz <- "America/New_York"
s <- read.csv(paste0(path, "expiration.csv"), type="character")
expiration <- matrix(as.time(s[, "Expiration"]), nrow(s), 1, dimnames=list(s[,"Contract"], "Date"))
roll <<- list()
roll[[id]] <- `-`(expiration, as.period("3d"), tz)


## build simulated spot price
idx <- seq(`-`(roll[[id]][1], as.period("6m"), tz), tail(roll[[id]], 1), by=as.duration("00:01:00"))
spot <- cumsum(runif(1:length(idx), -0.05, 0.05))
spot <- zts(idx, spot + min(spot) + 100, dimnames=list(NULL, "price"))


## build a simulated time-series that starts 4 months before expiry
data <<- list()
data[[id]] <- list()
for (contract in rownames(expiration)) {
  ival <- interval(`-`(expiration[contract,], as.period("4m"), tz), expiration[contract,], eopen=FALSE)
  data[[id]][[contract]] <- spot[ival, ]
  `+`(--data[[id]][[contract]], runif(1))         # just offset by a constant random value
}



## joining is done by "cutting" out from the appropriate contracts at
## the defined roll dates, taking the differences, piecing them
## together and finally taking the cumulative sum:
join <<- function(roll, data, cols) {
  # build intervals between roll dates
  roll_ivals <- interval(tail(rotate(roll,1), -1), tail(roll,-1), eopen=FALSE)

  joined <- zts(as.time(NULL), matrix(0, 0, 1))
  i <- 1
  for (c in rownames(roll_ivals)) {
    `+`(--i, 1)
    ival <- roll_ivals[c,]
    contract <- data[[c]]
    rbind(--joined, tail(diff(contract[ival, cols], 1), -1))
  }

  cumsum(joined[, cols])
}

