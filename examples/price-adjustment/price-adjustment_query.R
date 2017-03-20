## please change the ip/port to the ztsdb instance you have running
## make sure to source the ztsdb server part of this example in
## order to populate the data

## create a connection 'c1' towards a ztsdb instance
c1 <- connection("127.0.0.1", 123123)


ids <- list(AAPL="AAPL", MSFT="MSFT", XOM="XOM")
adjusted_list <- lapply(ids, function(x) c1 ? get_adjusted_price(++x))


## The result will be a list of three 'xts' containing adjusted
## data:
##
## str(adjusted_list)
## List of 3
##  $ AAPL:An ‘xts’ object on 1980-12-12/2017-02-27 containing:
##   Data: num [1:9130, 1] 0.43 0.408 0.378 0.387 0.399 ...
##  - attr(*, "dimnames")=List of 2
##   ..$ : NULL
##   ..$ : chr "Close"
##   Indexed by objects of class: [POSIXct,POSIXt] TZ: 
##   xts Attributes:  
## List of 1
##   ..$ descr: chr ""
##  $ MSFT:An ‘xts’ object on 1986-03-13/2017-02-27 containing:
##   Data: num [1:7805, 1] 0.0656 0.068 0.0691 0.0674 0.0662 ...
##  - attr(*, "dimnames")=List of 2
##   ..$ : NULL
##   ..$ : chr "Close"
##   Indexed by objects of class: [POSIXct,POSIXt] TZ: 
##   xts Attributes:  
## List of 1
##   ..$ descr: chr ""
##  $ XOM :An ‘xts’ object on 1970-01-02/2017-02-27 containing:
##   Data: num [1:11881, 1] 1.04 1.06 1.06 1.05 1.05 ...
##  - attr(*, "dimnames")=List of 2
##   ..$ : NULL
##   ..$ : chr "Close"
##   Indexed by objects of class: [POSIXct,POSIXt] TZ: 
##   xts Attributes:  
## List of 1
##   ..$ descr: chr ""
