# Usage

On a ztsdb instance, create an empty time-series; in the example below
we create a list containing a time series:

    data  <-  matrix(0, 0, 3, dimnames=list(NULL, c("a","b","c")))
    idx   <-  as.time(NULL)
    a     <<- list(b=zts(idx, data))

In a terminal, run 'append', making sure to use the port on which the
ztsdb instance is listening (in this example we use 19300):

    ./append 127.0.0.1 19300 10 a,b 3 100

This will run 'append', generating 10 messages per second until 100
messages have been sent, updating element 'b' of list 'a' which has 3
columns.

The append command line usage is:

    usage: append <ip> <port> <rate> <varname[,name1,name2,...]> <ncols> [max-msgs]

Where 'ip' and 'port' are the IP address and port of the remote ztsdb
instance, rate is the number of messages per second to generate,
'varname' is the name of the variable on the remote instance that
contains the time-series. If the time-series is nested in a list,
'name1', 'name2' are used optionally to indicate recursively the list
entry names. 'ncols' is the number of columns of the time-series to
update. Optionally, the number 'max-msgs' can be specified to stop
appending after reaching that number.
