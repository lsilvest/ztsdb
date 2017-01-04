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


## This example shows how to continuously calculate a minute-mean
## time-series from a time-series of smaller time granularity.
##
## 'z' is created and we assume it is continuously updated (use for
## example the 'append' itest that is part of the ztsdb project). The
## timer 't1' is run every 10 milliseconds and on completion of a
## minute in time-series 'z' it calculates the mean of that minute and
## appends it to time-series 'mmean'.


## create a couple of zts of size 0 x 3

data  <-  matrix(0, 0, 3)
idx   <-  as.time(NULL)
## we create 'z' here and suppose sub-minute granularity updates
z     <<- zts(idx, data, dimnames=list(NULL, c("a","b","c")))

## the calculated minute-means 'mmean':
mmean <<- zts(idx, data, dimnames=list(NULL, c("a","b","c")))


ten_ms <- as.duration(1e7)
t1 <- timer(ten_ms,
            loop = {
                ## if a minute is complete, calculate its mean and add it to 'mmean':
                current_minute <- floor(tail(zts.idx(z), 1), "minute")
                if (current_minute >= last_minute + one_minute) {
                    last_minute <- current_minute
                    m <- align(z, last_minute, -one_minute, method="mean")
                    rbind(--mmean, m)
                }
                ## keep the size of z reasonable:
                if (nrow(z) > 2e6) {
                    zts.resize(--z, start=nrow(z)-1e6)
                }
            },
            once = {
                one_minute  <- as.duration("00:01:00")
                last_minute <- floor(Sys.time(), "minute")
            })

                  
