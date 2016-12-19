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


## test meticulously the open/close intervals in all situations, including functions such as align

## constructor
RUnit_constructor_scalar <- function() {
    interval(|.2012-12-12 12:12:12 UTC.|, |.2012-12-12 12:12:13 UTC.|) ==
        |+2012-12-12 12:12:12 UTC -> 2012-12-12 12:12:13 UTC-|
}
RUnit_constructor_scalar_sopen <- function() {
    interval(|.2012-12-12 12:12:12 UTC.|, |.2012-12-12 12:12:13 UTC.|, sopen=T) ==
        |-2012-12-12 12:12:12 UTC -> 2012-12-12 12:12:13 UTC-|
}
RUnit_constructor_scalar_eopen <- function() {
    interval(|.2012-12-12 12:12:12 UTC.|, |.2012-12-12 12:12:13 UTC.|, eopen=T) ==
        |+2012-12-12 12:12:12 UTC -> 2012-12-12 12:12:13 UTC-|
}
RUnit_constructor_scalar_eopen_sopen <- function() {
    interval(|.2012-12-12 12:12:12 UTC.|, |.2012-12-12 12:12:13 UTC.|, eopen=T, sopen=T) ==
        |-2012-12-12 12:12:12 UTC -> 2012-12-12 12:12:13 UTC-|
}
RUnit_constructor <- function() {
    t1 <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=10)
    t2 <- seq(|.2013-12-12 12:12:12 UTC.|, length.out=10)
    i <- seq(|+2012-12-12 12:12:12 UTC -> 2013-12-12 12:12:12 UTC-|, length.out=10)
    all(interval(t1, t2) == i)
}
RUnit_constructor_sopen_scalar <- function() {
    t1 <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=10)
    t2 <- seq(|.2013-12-12 12:12:12 UTC.|, length.out=10)
    i <- seq(|-2012-12-12 12:12:12 UTC -> 2013-12-12 12:12:12 UTC-|, length.out=10)
    all(interval(t1, t2, sopen=T) == i)
}
RUnit_constructor_eopen_scalar <- function() {
    t1 <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=10)
    t2 <- seq(|.2013-12-12 12:12:12 UTC.|, length.out=10)
    i <- seq(|+2012-12-12 12:12:12 UTC -> 2013-12-12 12:12:12 UTC-|, length.out=10)
    all(interval(t1, t2, eopen=T) == i)
}
RUnit_constructor_sopen_eopen <- function() {
    t1 <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=10)
    t2 <- seq(|.2013-12-12 12:12:12 UTC.|, length.out=10)
    i1 <- seq(|+2012-12-12 12:12:12 UTC -> 2013-12-12 12:12:12 UTC-|, length.out=5)
    i2 <- seq(|-2012-12-12 12:12:17 UTC -> 2013-12-12 12:12:17 UTC+|, length.out=5)
    i <- c(i1, i2)
    sopen <- c(F,F,F,F,F, T,T,T,T,T)
    eopen <- !sopen
    all(interval(t1, t2, sopen=sopen, eopen=eopen) == i)
}
## accessors
RUnit_start_end_sopen_eopen <- function() {
    t1 <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=10)
    t2 <- seq(|.2013-12-12 12:12:12 UTC.|, length.out=10)
    all(interval.start(interval(t1, t2)) == t1) &
    all(interval.end(interval(t1, t2)) == t2) &
    all(interval.sopen(interval(t1, t2)) == c(F,F,F,F,F,F,F,F,F,F)) &
    all(interval.eopen(interval(t1, t2)) == c(T,T,T,T,T,T,T,T,T,T)) &
    all(interval.sopen(interval(t1, t2, sopen=T)) == c(T,T,T,T,T,T,T,T,T,T)) &
    all(interval.eopen(interval(t1, t2, eopen=T)) == c(T,T,T,T,T,T,T,T,T,T))
}
RUnit_start_end_sopen_eopen_0 <- function() {
    i <- vector(mode="interval", 0)
    all.equal(interval.start(i), vector(mode="time", 0)) &
    all.equal(interval.end(i), vector(mode="time", 0)) &
    all.equal(interval.sopen(i), vector(mode="logical", 0)) &
    all.equal(interval.eopen(i), vector(mode="logical", 0))
}
RUnit_constructor_accessor_matrix <- function() {
    t1 <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=10)
    t2 <- seq(|.2013-12-12 12:12:12 UTC.|, length.out=10)
    i1 <- seq(|+2012-12-12 12:12:12 UTC -> 2013-12-12 12:12:12 UTC-|, length.out=5)
    i2 <- seq(|-2012-12-12 12:12:17 UTC -> 2013-12-12 12:12:17 UTC+|, length.out=5)
    i <- c(i1, i2)
    i <- cbind(i1, i2)
    all(interval(interval.start(i), interval.end(i),
                 sopen=interval.sopen(i), eopen=interval.eopen(i)) == i)
}
