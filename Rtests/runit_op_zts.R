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

source("zts_large.R")


RUnit_op_zts_plus <- function() {
    z2 <- get_large_zts(100);
    z2[,] <- 1;
    z1 <- z2[c(10, 40, 75), 1]
    z1[,] <- c(1.1, 2.2, 3.3)
    expected <- rbind(z2[1:9,]   + 1.1,
                      z2[10:39,] + 2.2,
                      z2[40:74,] + 3.3,
                      z2[75:100,])
    all.equal(expected, op.zts(z1, z2, "+"))
}
RUnit_op_zts_mul <- function() {
    z2 <- get_large_zts(100);
    z2[,] <- 1;
    z1 <- z2[c(10, 40, 75), 1]
    z1[,] <- c(1.1, 2.2, 3.3)
    expected <- rbind(z2[1:9,]   * 1.1,
                      z2[10:39,] * 2.2,
                      z2[40:74,] * 3.3,
                      z2[75:100,])
    all.equal(expected, op.zts(z1, z2, "*"))
}
RUnit_op_zts_div <- function() {
    z2 <- get_large_zts(100);
    z2[,] <- 1;
    z1 <- z2[c(10, 40, 75), 1]
    z1[,] <- c(1.1, 2.2, 3.3)
    expected <- rbind(z2[1:9,]   / 1.1,
                      z2[10:39,] / 2.2,
                      z2[40:74,] / 3.3,
                      z2[75:100,])
    all.equal(expected, op.zts(z1, z2, "/"))
}
RUnit_op_zts_minus <- function() {
    z2 <- get_large_zts(100);
    z2[,] <- 1;
    z1 <- z2[c(10, 40, 75), 1]
    z1[,] <- c(1.1, 2.2, 3.3)
    expected <- rbind(z2[1:9,]   - 1.1,
                      z2[10:39,] - 2.2,
                      z2[40:74,] - 3.3,
                      z2[75:100,])
    all.equal(expected, op.zts(z1, z2, "-"))
}
