## Copyright (C) 2017 Leonardo Silvestri
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


## double
RUnit_vector_0_double <- function() {
  ## works because we consider a vector of 0 to implicitly have 1 column
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- vector(mode="double", length=0, file=dir)
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_vector_10_double <- function() {
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- vector(mode="double", length=10, file=dir)
  a[] <- 1:10
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_matrix_0x0_double <- function() {
  ## doesn't work because we do not have any columns
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- matrix(0, 0, 0, file=dir)
  b <- a[]
  rm(a)
  tryCatch(load(dir), .Last.error == paste("no data in directory", dir))
}
RUnit_matrix_0x2_double <- function() {
  ## works because we have two columns
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- matrix(0, 0, 2, dimnames=list(NULL, c("a", "b")), file=dir)
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_matrix_3x2_double <- function() {
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- matrix(1:6, 3, 2, dimnames=list(NULL, c("a", "b")), file=dir)
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_array_0x0x0_double <- function() {
  ## doesn't work because we do not have any columns
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- array(0, c(0, 0, 0), file=dir)
  b <- a[]
  rm(a)
  tryCatch(load(dir), .Last.error == paste("no data in directory", dir))
}
RUnit_array_0x2x3_double <- function() {
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- array(0, c(0,2,3), dimnames=list(NULL, c("a","b"), c("1","2","3")), file=dir)
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_array_1x3x2_double <- function() {
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- array(1:6, c(1,3,2), dimnames=list(NULL, c("1","2","3"), c("a","b")), file=dir)
  b <- a[,drop=F]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}

## logical
RUnit_vector_0_logical <- function() {
  ## works because we consider a vector of 0 to implicitly have 1 column
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- vector(mode="logical", length=0, file=dir)
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_vector_10_logical <- function() {
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- vector(mode="logical", length=10, file=dir)
  a[] <- c(T,T,T,F,F,F,T,T,T,F)
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_matrix_0x0_logical <- function() {
  ## doesn't work because we do not have any columns
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- matrix(F, 0, 0, file=dir)
  b <- a[]
  rm(a)
  tryCatch(load(dir), .Last.error == paste("no data in directory", dir))
}
RUnit_matrix_0x2_logical <- function() {
  ## works because we have two columns
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- matrix(F, 0, 2, dimnames=list(NULL, c("a", "b")), file=dir)
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_matrix_3x2_logical <- function() {
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- matrix(c(T,T,F,F,T,T), 3, 2, dimnames=list(NULL, c("a", "b")), file=dir)
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_array_0x0x0_logical <- function() {
  ## doesn't work because we do not have any columns
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- array(T, c(0, 0, 0), file=dir)
  b <- a[]
  rm(a)
  tryCatch(load(dir), .Last.error == paste("no data in directory", dir))
}
RUnit_array_0x2x3_logical <- function() {
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- array(T, c(0,2,3), dimnames=list(NULL, c("a","b"), c("1","2","3")), file=dir)
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_array_1x3x2_logical <- function() {
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- array(c(T,T,F,F,T,T), c(1,3,2), dimnames=list(NULL, c("1","2","3"), c("a","b")), file=dir)
  b <- a[,drop=F]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}

## character
RUnit_vector_0_character <- function() {
  ## works because we consider a vector of 0 to implicitly have 1 column
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- vector(mode="character", length=0, file=dir)
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_vector_10_character <- function() {
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- vector(mode="character", length=10, file=dir)
  a[] <- as.character(1:10)
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_matrix_0x0_character <- function() {
  ## doesn't work because we do not have any columns
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- matrix("a", 0, 0, file=dir)
  b <- a[]
  rm(a)
  tryCatch(load(dir), .Last.error == paste("no data in directory", dir))
}
RUnit_matrix_0x2_character <- function() {
  ## works because we have two columns
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- matrix("a", 0, 2, dimnames=list(NULL, c("a", "b")), file=dir)
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_matrix_3x2_character <- function() {
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- matrix(as.character(1:6), 3, 2, dimnames=list(NULL, c("a", "b")), file=dir)
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_array_0x0x0_character <- function() {
  ## doesn't work because we do not have any columns
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- array("a", c(0, 0, 0), file=dir)
  b <- a[]
  rm(a)
  tryCatch(load(dir), .Last.error == paste("no data in directory", dir))
}
RUnit_array_0x2x3_character <- function() {
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- array("a", c(0,2,3), dimnames=list(NULL, c("a","b"), c("1","2","3")), file=dir)
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_array_1x3x2_character <- function() {
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- array(as.character(1:6), c(1,3,2), dimnames=list(NULL, c("1","2","3"), c("a","b")), file=dir)
  b <- a[,drop=F]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}

## time
tm <- |.2017-04-04 00:01:01 UTC.| + as.duration(1:10)
RUnit_vector_0_time <- function() {
  ## works because we consider a vector of 0 to implicitly have 1 column
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- vector(mode="time", length=0, file=dir)
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_vector_10_time <- function() {
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- vector(mode="time", length=10, file=dir)
  a[] <- tm
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_matrix_0x0_time <- function() {
  ## doesn't work because we do not have any columns
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- matrix(tm[1], 0, 0, file=dir)
  b <- a[]
  rm(a)
  tryCatch(load(dir), .Last.error == paste("no data in directory", dir))
}
RUnit_matrix_0x2_time <- function() {
  ## works because we have two columns
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- matrix(tm[1], 0, 2, dimnames=list(NULL, c("a", "b")), file=dir)
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_matrix_3x2_time <- function() {
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- matrix(tm[1:6], 3, 2, dimnames=list(NULL, c("a", "b")), file=dir)
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_array_0x0x0_time <- function() {
  ## doesn't work because we do not have any columns
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- array(tm[1], c(0, 0, 0), file=dir)
  b <- a[]
  rm(a)
  tryCatch(load(dir), .Last.error == paste("no data in directory", dir))
}
RUnit_array_0x2x3_time <- function() {
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- array(tm[1], c(0,2,3), dimnames=list(NULL, c("a","b"), c("1","2","3")), file=dir)
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_array_1x3x2_time <- function() {
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- array(tm[1:6], c(1,3,2), dimnames=list(NULL, c("1","2","3"), c("a","b")), file=dir)
  b <- a[,drop=F]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}

## interval
ival <- |+2017-04-04 00:01:01 UTC -> 2017-04-05 12:12:12 UTC-| + as.duration(1:10)
RUnit_vector_0_interval <- function() {
  ## works because we consider a vector of 0 to implicitly have 1 column
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- vector(mode="interval", length=0, file=dir)
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_vector_10_interval <- function() {
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- vector(mode="interval", length=10, file=dir)
  a[] <- ival
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_matrix_0x0_interval <- function() {
  ## doesn't work because we do not have any columns
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- matrix(ival[1], 0, 0, file=dir)
  b <- a[]
  rm(a)
  tryCatch(load(dir), .Last.error == paste("no data in directory", dir))
}
RUnit_matrix_0x2_interval <- function() {
  ## works because we have two columns
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- matrix(ival[1], 0, 2, dimnames=list(NULL, c("a", "b")), file=dir)
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_matrix_3x2_interval <- function() {
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- matrix(ival[1:6], 3, 2, dimnames=list(NULL, c("a", "b")), file=dir)
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_array_0x0x0_interval <- function() {
  ## doesn't work because we do not have any columns
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- array(ival[1], c(0, 0, 0), file=dir)
  b <- a[]
  rm(a)
  tryCatch(load(dir), .Last.error == paste("no data in directory", dir))
}
RUnit_array_0x2x3_interval <- function() {
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- array(ival[1], c(0,2,3), dimnames=list(NULL, c("a","b"), c("1","2","3")), file=dir)
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_array_1x3x2_interval <- function() {
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- array(ival[1:6], c(1,3,2), dimnames=list(NULL, c("1","2","3"), c("a","b")), file=dir)
  b <- a[,drop=F]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}

## period
p <- c(as.period("1d"), as.period("2d"), as.period("3d"), as.period("4d"), as.period("5d"),
       as.period("6d"), as.period("7d"), as.period("8d"), as.period("9d"), as.period("10d"))
RUnit_vector_0_period <- function() {
  ## works because we consider a vector of 0 to implicitly have 1 column
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- vector(mode="period", length=0, file=dir)
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_vector_10_period <- function() {
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- vector(mode="period", length=10, file=dir)
  a[] <- p
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_matrix_0x0_period <- function() {
  ## doesn't work because we do not have any columns
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- matrix(p[1], 0, 0, file=dir)
  b <- a[]
  rm(a)
  tryCatch(load(dir), .Last.error == paste("no data in directory", dir))
}
RUnit_matrix_0x2_period <- function() {
  ## works because we have two columns
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- matrix(p[1], 0, 2, dimnames=list(NULL, c("a", "b")), file=dir)
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_matrix_3x2_period <- function() {
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- matrix(p[1:6], 3, 2, dimnames=list(NULL, c("a", "b")), file=dir)
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_array_0x0x0_period <- function() {
  ## doesn't work because we do not have any columns
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- array(p[1], c(0, 0, 0), file=dir)
  b <- a[]
  rm(a)
  tryCatch(load(dir), .Last.error == paste("no data in directory", dir))
}
RUnit_array_0x2x3_period <- function() {
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- array(p[1], c(0,2,3), dimnames=list(NULL, c("a","b"), c("1","2","3")), file=dir)
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_array_1x3x2_period <- function() {
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- array(p[1:6], c(1,3,2), dimnames=list(NULL, c("1","2","3"), c("a","b")), file=dir)
  b <- a[,drop=F]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}

## zts
RUnit_zts_0x2 <- function() {
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- zts(as.time(NULL), matrix(0, 0, 2, dimnames=list(NULL, c("a","b"))), file=dir)
  b <- a[,]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_zts_1x2 <- function() {
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  a <- zts(tm[1], matrix(1:2, 1, 2, dimnames=list(NULL, c("a","b"))), file=dir)
  b <- a[,]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_zts_2x3x4 <- function() {
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  dimnames <- list(NULL, c("a","b","c"), as.character(1:4))
  a <- zts(tm[1:2], array(1:24, c(2,3,4), dimnames=dimnames), file=dir)
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}

## large double, character and zts
RUnit_double_large <- function() {
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  n <- 1e6
  a <- matrix(1:(n*3), n, 3, dimnames=list(NULL, c("a", "b", "c")), file=dir)
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_character_large <- function() {
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  n <- 1e3
  a <- matrix(as.character(1:(n*3)), n, 3, dimnames=list(NULL, c("a", "b", "c")), file=dir)
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
RUnit_zts_large <- function() {
  dir <- system("mktemp -d", intern=T)
  system(paste("rmdir", dir))         # remove it as it will be recreated
  n <- 1e6
  a <- zts(tm[1]+as.duration(1:n), matrix(1:(n*2), n, 2, dimnames=list(NULL, c("a","b"))), file=dir)
  b <- a[]
  rm(a)
  a <- load(dir)
  all.equal(a, b)
}
