## Copyright (C) 2016-2017 Leonardo Silvestri
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


## Here we test that objects are appropriately copied. In particular,
## memory-mapped objects have different copying semantics.



## a copy is appropriately made of y
RUnit_vars <- function() {
    x <- 1:10
    y <- x
    x[1] <- 11
    all(x == c(11, 2:10)) & all(y == 1:10)
}

## test that zts.idx and zts.data are const ref but that we can get a
## copy with the subscript operator
source("zts.R")
RUnit_zts_parts_constref <- function() {
    i <- zts.idx(z1)[]
    d <- zts.data(z1)[,]
    tryCatch(zts.idx(z1) <- 1, TRUE) &
    tryCatch(zts.data(z1) <- 1, TRUE) &
    all(dim(i) == dim(zts.idx(z1))) &
    all(dim(d) == dim(zts.data(z1)))
}


## test locked mmap structure cannot be referenced by another var
RUnit_array_locked <- function() {
    dir <- system("mktemp -d", intern=T)
    system(paste("rmdir", dir))         # remove it as it will be recreated by 'matrix'
    a <- matrix(1:9, 3, 3, file=dir)
    b <- a
    tryCatch(sin(--b), TRUE)
}
RUnit_zts_bind_locked <- function() {
    dir <- system("mktemp -d", intern=T)
    system(paste("rmdir", dir))         # remove it as it will be recreated by 'matrix'
    z <- zts(idx1, 1:27, file=dir)
    tryCatch(cbind(--z, z), TRUE) ## if not copying we get: Error:
                                  ## cannot bind to self. need to look
                                  ## at the error LLL
}
RUnit_array_bind_locked <- function() {
    dir <- system("mktemp -d", intern=T)
    system(paste("rmdir", dir))         # remove it as it will be recreated by 'matrix'
    a <- matrix(1:9, 3, 3, file=dir)
    tryCatch(cbind(--a, a), TRUE) ## if not copying we get: Error: cannot bind to self.
}

## test locking mechanism
RUnit_lock <- function() {
  a <- 1
  is.null(lock(--a)) & is.locked(a)
}
RUnit_unlock <- function() {
  a <- 1
  lock(--a)
  is.null(unlock(--a)) & !is.locked(a)
}
RUnit_unlock_mmapped <- function() {
  ## can't unlock a mmaped array
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated by 'matrix'
  a <- matrix(1:9, 3, 3, file=dir)
  is.locked(a) &
  tryCatch(unlock(--a), .Last.error == "cannot unlock file mapped object")
}
source("zts_large.R")
RUnit_lock_zts <- function() {
  z <- get_large_zts(1000)
  is.null(lock(--z)) & is.locked(z)
}
RUnit_unlock_zts <- function() {
  z <- get_large_zts(1000)
  lock(--z)
  is.null(unlock(--z)) & !is.locked(z)
}
RUnit_unlock_mmapped_zts <- function() {
  z <- get_large_zts(1000, TRUE)
  is.locked(z) &
  tryCatch(unlock(--z), .Last.error == "cannot unlock file mapped object")
}
## should fail on other types
RUnit_lock_unlock_builtin <- function() {
  !is.locked(sin) &
  tryCatch(unlock(--sin), .Last.error == "incorrect argument type") &
  tryCatch(lock(--sin), .Last.error == "incorrect argument type")
}
RUnit_lock_unlock_list <- function() {
  !is.locked(list(1)) &
  tryCatch(unlock(list(1)), .Last.error == "incorrect argument type") &
  tryCatch(lock(list()), .Last.error == "incorrect argument type")
}
