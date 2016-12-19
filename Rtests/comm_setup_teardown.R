## Copyright (C) 2015 Leonardo Silvestri
##
## This file is part of ztsdb.
##
## ztsdb is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## ztsdb is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with ztsdb.  If not, see <http://www.gnu.org/licenses/>.


con1 <- NULL
con2 <- NULL


.startZtsdb <- function(port) {
    cmd <- paste0(system("pwd", intern=T), "/../build/src/ztsdb -p ", port,
                  " & echo $! > /tmp/ztsdb_", port, ".pid")
    system(cmd, ignore.stdout=TRUE, ignore.stderr=TRUE, wait=FALSE)    
}
.stopZtsdb <- function(port) {
    cmd <- paste0("kill -9 `cat /tmp/ztsdb_", port, ".pid`")
    system(cmd, wait=T)
}


.setUp <- function() {
    ## start a couple of remote servers whith which we can communicate
    .startZtsdb(port1)
    .startZtsdb(port2)
    i <- 1
    done <- FALSE
    while (!done) {
        done <- TRUE
        tryCatch(con1 <<- connection("127.0.0.1", port1), {T; done <- FALSE})
    }
    done <- FALSE
    while (!done) {
        done <- TRUE
        tryCatch(con2 <<- connection("127.0.0.1", port2), {T; done <- FALSE})
    }
    TRUE
}


.tearDown <- function() {
    .stopZtsdb(port1)
    .stopZtsdb(port2)
}

