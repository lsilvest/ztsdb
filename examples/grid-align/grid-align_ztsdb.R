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


grid_align <<- function(z,                         # time-series
                        by,                        # the grid size
                        method,                    # "count", "min", "max", "median", "mean", "closest"
                        ival=by,                   # the interval size
                        start=head(zts.idx(z),1),  # start of the grid
                        end=tail(zts.idx(z),1),    # end of the grid
                        tz=NULL)                   # time zone when using 'period'
{
  if (typeof(by) == "duration") {
    grid <- seq(start+by, end, by=by)
    if (tail(grid,1) < end) {
      c(--grid, tail(grid,1) + by)
    }
  }
  else if (typeof(by) == "period") {
    if (is.null(tz)) stop("tz must be specified when 'by' is a period")
    grid <- seq(`+`(start,by,tz), end, by=by, tz=tz)
    if (tail(grid,1) < end) {
      c(--grid, `+`(tail(grid,1),by,tz))
    }
  }
  else stop("invalid type for 'by', must be 'duration' or 'period'")
   
  align(z, grid, -ival, as.duration(0), method=method, tz=tz)
}


density <<- function(z, by, ival=by, start=head(zts.idx(z),1), end=tail(zts.idx(z),1), tz=NULL)
  grid_align(z, by, "count", ival, start, end, tz)
