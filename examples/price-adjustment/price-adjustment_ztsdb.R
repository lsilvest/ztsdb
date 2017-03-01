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


## Description
##
## This example shows how to do the adjustment of a price series using
## a dividend series and a split/reverse-split series. Because all the
## key functions used (e.g. 'align', 'op.zts', etc.) are implemented
## in C++ it is often feasible to have the database do these kind of
## calculations on the fly as part of a data query. The recipient of
## this data, maybe in an R session, thus receives immediately usable
## data.
##

## CAVEAT: modify 'path' below to the correct location of the data:
## path <- "/path/to/ztsdb/examples/price-adjustment-data/"
path <- "/home/lsilvest/repos/ztsdb/examples/price-adjustment/price-adjustment-data/"
if (system(paste("test -d", path))) {
  stop("can't find data directory for demo: please modify 'path' in the script")
}


## Note that the data used in this example is from 'Quandl'
## (https://www.quandl.com/data/WIKI/AAPL-Apple-Inc-AAPL-Prices-Dividends-Splits-and-Trading-Volume)
## and comes with the following notice:
##
## This data is in the public domain. You may copy, distribute,
## disseminate or include the data in other products for commercial
## and/or noncommercial purposes.


## The two functions below calculate time-series with the multipliers
## needed for the adjustment:
get_adjust_div_zts <<- function(close, div) {
  pricedaym1 <- align(move(close, 1), zts.idx(div)) # adj must start on previous day
  cumprod(1 - (div / pricedaym1), rev=TRUE)
}

get_adjust_split_zts <<- function(split) {
  cumprod(1 / split, rev=TRUE)          # split should not have any zeroes!
}


## In the real world this is the kind of data that would be present in
## the DBMS, but for this demo, we load it from CSV files.
price <<- list()
div   <<- list()
split <<- list()
read_stock_data <- function(id_list) {
  for (id in id_list) {
    data <- read.csv(paste0(path, id, ".csv"), format="%Y-%m-%d", tz="America/New_York")
    price[[id]] <- data[, "Close"]
    div  [[id]] <- data[data[, "Ex-Dividend"] != 0, "Ex-Dividend"]
    split[[id]] <- data[data[, "Split Ratio"] != 1, "Split Ratio"]
  }
}
id_list <- list("AAPL", "MSFT", "XOM")
read_stock_data(id_list)


# This function returns the adjusted price for a given 'id':
get_adjusted_price <<- function(id) {
  close <- price[[id]][, "Close"]
  last_close <- tail(zts.idx(close), 1)
  div <- div[[id]]
  split <- split[[id]]
  adjust_div <- get_adjust_div_zts(close, div[zts.idx(div) <= last_close, ])
  adjust_split <- get_adjust_split_zts(split[zts.idx(split) <= last_close, ])
  ## The 'op.zts' operator is used to adjust first for dividends and
  ## then for splits; in this demo we adjust the close price series, but
  ## another more realistic scenario would be the adjustment of intraday
  ## data; the 'op.zts' multiplication will also handle this scenario
  ## correctly:
  price_adj <- op.zts(adjust_div, close, "*")
  price_adj <- op.zts(adjust_split, price_adj, "*")
  price_adj
}
