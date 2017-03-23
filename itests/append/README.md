# Usage

On a ztsdb instance, create an empty time-series; in the example below
we create a list containing a time series:

    data  <-  matrix(0, 0, 3, dimnames=list(NULL, c("a","b","c")))
    idx   <-  as.time(NULL)
    a     <<- list(b=zts(idx, data))


In a terminal, run 'append', making sure to use the port on which the
ztsdb instance is listening (in this example we use 10000):

    ./append 127.0.0.1 10000 10 a,b 3 100

This will run append generating 10 messages per second until 100
messages have been sent, updating element 'b' of list 'a' which has 3
columns.

