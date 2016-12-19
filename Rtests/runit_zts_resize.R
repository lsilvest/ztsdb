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

source("zts_large.R")
## need large enough to make sure we get page unmapping 


RUnit_zts_resize_integers <- function() {
    z <- get_large_zts()
    start <- floor(nrow(z) / 3)
    end   <- floor(nrow(z) / 4 * 3)
    all.equal(zts.resize(z, start, end),
              z[start:(end-1), ])
}
RUnit_zts_resize_dtime <- function() {
    z <- get_large_zts()
    start.idx <- floor(nrow(z) / 3)
    end.idx   <- floor(nrow(z) / 4 * 3)
    start <- zts.idx(z)[start.idx]
    end   <- zts.idx(z)[end.idx]
    all.equal(zts.resize(z, start, end),
              z[start.idx:(end.idx-1), ])
}
RUnit_zts_resize_integers_inplace <- function() {
    z <- get_large_zts()
    start <- floor(nrow(z) / 3)
    end   <- floor(nrow(z) / 4 * 3)
    exp   <- z[start:(end-1), ] 
    zts.resize(--z, start, end)
    all.equal(z, exp)
}
RUnit_zts_resize_dtime_inplace <- function() {
    z <- get_large_zts()
    start.idx <- floor(nrow(z) / 3)
    end.idx   <- floor(nrow(z) / 4 * 3)
    start <- zts.idx(z)[start.idx]
    end   <- zts.idx(z)[end.idx]
    exp   <- z[start.idx:(end.idx-1), ] 
    zts.resize(--z, start, end)
    all.equal(z, exp)
}
## boundary conditions and errors
source("zts.R")
RUnit_zts_resize_end_larger <- function() {
    all.equal(zts.resize(z1, 1, 100), z1)
}
RUnit_zts_resize_end_zero_length <- function() {
    all.equal(zts.resize(z1, 1, 1), head(z1,0))
}
RUnit_zts_resize_large_start <- function() {
    all.equal(zts.resize(z1, 100, 100), head(z1,0))
}
RUnit_zts_resize_dim0_zero <- function() {
    all.equal(zts.resize(head(z1,0), 1, 2), head(z1,0))
}
RUnit_zts_end_larger <- function() {
    tryCatch(zts.resize(z1, 3, 2), .Last.error=="start must be <= to end")
}
RUnit_zts_start_neg <- function() {
    tryCatch(zts.resize(z1, -1, 2), .Last.error=="start out of bounds")
}
RUnit_zts_end_neg <- function() {
    tryCatch(zts.resize(z1, 1, -1), .Last.error=="end out of bounds")
}
