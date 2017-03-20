# About 

This directory contains a set of examples that show some aspects of
ztsdb. 

# Running the examples

To run an example, start a ztsdb instance on port 123123 in the
directory of the test

    ztsdb -p 123123

One can use an already running ztsdb instance, but one then might have
to change the `path` and the port number in the script file.

When there is only one file for a particular example then this file
should be sourced in the ztsdb instance. If there are multiple files,
the file that needs to be sourced by the ztsdb instance is postfixed
by _ztsdb_. Similarly a query file (runnable either in an R session or
by another ztsdb instance) is postfixed by _query_. Sourcing is done
with the following command:

    source("file_to_source.R")
