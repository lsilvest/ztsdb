## ---------------------------
## so in R we could have something like this:

h <- tsdb_handle("127.0.0.1", 15001)

b <- "column1"
h ( a <<- ts[h$a<2, '::b'] )

l <- h ( { ts[h$a<2, '::b'] } )


# so the h functon would map vector, array, scalars, strings, date times to their counterpart in .
# 
h <- function(x) { 
  substitute(x); # then do  
}

h ( ts[h$a<2, '::b'] )

> h () # prints back connection parameter


############

h <- tsdb ( "|127.0.0.1:15001|" )
h | ts[h$a<2, '::b']
a <- h | ts[h$a<2, '::b']
r <- h | { a <- 1; a * 2 }

############

library(tsdb)

tsdb ( ts[h$a<2, '::b'] )
tsdb ( h <- "|127.0.0.1:15001|" )

h <- tsdb ( "|127.0.0.1:15001|" )
a <- tsdb ( 1 )
b <- tsdb ( "::a" )

i <- h | ts[h$a<2, '::b']
h | a <<- matrix(0, 2, 3)


############

library(tsdb)

lcl <- tsdb("|:15001|")                 # opens a local instance
rmt <- tsdb("|192.168.0.11:15001|")     # opens a local instance, but looks like it's just a client

p <- 2          
s <- lcl | list(2, "::p", 4)
u <- rmt | list(5, 6, 7x)
v <- rmt | { a <<- 3; a }
rmt | a

> str(lcl)
tsdb: addr=127.0.0.1, port=15001, status=open
    
> close(lcl)
> str(lcl)
tsdb: addr=127.0.0.1, port=15001, status=closed

close(rmt)
          
          
## or alternatively we could say we get remote like this:
rmt <- lcl | "|127.0.0.1:15001|"
p <- 2          
s <- lcl | list(2, "::p", 4)
u <- rmt | list(5, 6, 7x)
v <- rmt | { a <<- 3; a }
rmt | a

lcl | { rmt <- "|127.0.0.1:15001|" }
u <- lcl | { rmt | list(5, 6, 7x) }
v <- rmt | { a <<- 3; a }


##### could we?

## or alternatively we could say we get remote like this:
rmt <- lcl ? "|127.0.0.1:15001|"
p <- 2          
s <- lcl ? list(2, "::p", 4)
u <- rmt ? list(5, 6, 7)
v <- rmt ? { a <<- 3; a }
rmt ? a

## this of course will also work:
lcl ? { rmt <- "|127.0.0.1:15001|" }
u <- lcl ? { rmt ? list(5, 6, 7x) }
v <- rmt ? { a <<- 3; a }


## test this:

setClass("ts4",
         representation(idx = "POSIXct"),
         contains = "array")

setGeneric("?", function(x, y) standardGeneric("?"))

setMethod("initialize",
          "ts4",
          function(.Object,
                   .Data,
                   idx) {
            .Data <- as.array(.Data)
            if (length(dim(.Data)) == 1) {
              ## make sure we have a vertical vector
              dim(.Data) <- c(dim(.Data), 1)
            }
            storage.mode(idx)   <- "double"
            storage.mode(.Data) <- "double"
            .Object <- callNextMethod(.Object, .Data, idx=idx)
          })

setMethod("?",
          signature("ts4"),
          function(x, y) {
              g <- substitute(y)
              print(g)
              print(typeof(g))
              g
          })

idx <- as.POSIXct(c("2012-01-24 15:13:01", "2012-01-24 15:14:01", "2012-01-24 15:15:01",
                    "2012-01-25 15:13:01", "2012-01-25 15:14:01", "2012-01-25 15:15:01",
                    "2012-01-26 15:13:01", "2012-01-26 15:14:01", "2012-01-26 15:15:01"),
                  tz="America/New_York")
data <- matrix(1:27, 9, 3, dimnames = list(NULL, c("a1", "a2", "a3")))
t1 <- new("ts4", data, idx=idx)

t1 ? a + b + c


########## subsetting:


data <- matrix(1:27, 9, 3, dimnames = list(NULL, c("a1", "a2", "a3")))

data[a1<a2, a3]
data[data[,"a1"] < data[,"a2"], "a3"]


a <- array(1:27, c(3,3,3), dimnames = list(c("a1", "a2", "a3"), c("b1","b2","b3"), c("c1","c2","c3")))

a[c2 ∩ b2]                           # intersection, defines a vertical vector
a[a2 ∩ c1]                           # intersection, defines a horizontal vector
a[a1 ∩ b1]                           # intersection, defines a front to back vector
a[a1 ∪ a2]                           # union, defines 2x3x3
a[(a1 ∪ a2) ∩ c1]                    # union & inter, defines 2x3x1
a[((a1 ∪ a2) ∩ c1) < 4]              # union & inter and limitation

# could be written as:
a[((a1 | a2) & c1) < 4]              # union & inter and limitation
a[`((a1 < a2) & c1) < 4`]            # comparision & inter and limitation

# with other 
a[((a1 < b[a1]) & c1) < 4]           # union & inter and limitation

## a[b1]      <=>   a[,"b1",]
## a[b1|b2]   <=>   a[,list("b1","b2"),]
## a[((a1 | a2) & c1) < 4]     <=>  
## a[((b1 <= b[b1]) & c1) < 4] <=>
