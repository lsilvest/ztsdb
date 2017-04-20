# About 

This directory contains a set of examples that show some aspects of
ztsdb. 

# Running the examples

To run an example, start a ztsdb instance on port 12300 in the
directory of the test

    ztsdb -p 12300

One can use an already running ztsdb instance, but one has to change
the `path` and the port number in the script file.

All files that need to be sourced by the ztsdb instance are postfixed
by _ztsdb_. Similarly a query file (runnable either in an R session or
by another ztsdb instance) is postfixed by _query_. Sourcing is done
with the following command:

    source("file_to_source.R")

When running from an R session, make sure the 'rztsdb' package is
loaded:

    library(rztsdb)

