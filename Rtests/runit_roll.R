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

source("zts.R")

## rev --------------------------------------
RUnit_rev_vector <- function() {
    all.equal(rev(1:3), 3:1)
}
RUnit_rev_vector_copy <- function() {
    a <- 1:4
    b <- rev(a)
    all.equal(b, 4:1) & all.equal(a, 1:4)
}
RUnit_rev_vector_inplace <- function() {
    a <- 1:4
    b <- a
    rev(--a)
    all.equal(a, 4:1) & all.equal(b, 1:4)
}
RUnit_rev_matrix <- function() {
    all.equal(rev(matrix(1:9, 3, 3)), cbind(3:1, 6:4, 9:7))
}
RUnit_rev_array <- function() {
    slice1 <- cbind(3:1, 6:4, 9:7)
    slice2 <- cbind(12:10, 15:13, 18:16)
    slice3 <- cbind(21:19, 24:22, 27:25)
    all.equal(rev(array(1:27, c(3,3,3))), abind(slice1, slice2, slice3, along=3))
}
RUnit_rev_zts_copy <- function() {
    z <- z1
    rev(z)
    all.equal(z, z1)
}
RUnit_rev_zts_inplace <- function() {
    z <- z1
    rev(--z)
    exp <- z1
    exp[,1] <- 9:1
    exp[,2] <- 18:10
    exp[,3] <- 27:19
    all.equal(z, exp)
}
## list and zts are coded differently so test these fully
RUnit_rev_list <- function() {
    all.equal(rev(list(1,2,3)), list(3,2,1))
}
RUnit_rev_list_copy <- function() {
    a <- list(1,2,3,4)
    b <- rev(a)
    all.equal(b, list(4,3,2,1)) & all.equal(a, list(1,2,3,4))
}
RUnit_rev_list_inplace <- function() {
    a <- list(1,2,3,4)
    b <- a
    rev(--a)
    all.equal(a, list(4,3,2,1)) & all.equal(b, list(1,2,3,4))
}
## of course it's not that clear how useful a rev is on a zts, but here goes:
source("zts.R")
RUnit_rev_zts <- function() {
    z1_rev <- rev(z1)
    expected <- z1
    expected[,1] <- 9:1
    expected[,2] <- 18:10
    expected[,3] <- 27:19
    all.equal(z1_rev, expected)
}
RUnit_rev_zts_copy <- function() {
    z1_rev <- rev(z1)
    expected <- z1
    expected[,1] <- 9:1
    expected[,2] <- 18:10
    expected[,3] <- 27:19
    all.equal(z1_rev, expected) & all.equal(rev(expected), z1)
}
RUnit_rev_zts_copy <- function() {
    z1_copy <- z1
    rev(--z1_copy)
    expected <- z1
    expected[,1] <- 9:1
    expected[,2] <- 18:10
    expected[,3] <- 27:19
    all.equal(z1_copy, expected) & all.equal(rev(expected), z1)
}
## not exhaustively, because they share the same code as double, test other types
RUnit_rev_vector_character <- function() {
    all.equal(rev(as.character(1:3)), as.character(3:1))
}
RUnit_rev_vector_copy_character <- function() {
    a <- as.character(1:4)
    b <- rev(a)
    all.equal(b, as.character(4:1)) & all.equal(a, as.character(1:4))
}
RUnit_rev_vector_inplace_character <- function() {
    a <- as.character(1:4)
    b <- a
    rev(--a)
    all.equal(a, as.character(4:1)) & all.equal(b, as.character(1:4))
}
RUnit_rev_matrix_character <- function() {
    all.equal(rev(as.character(matrix(1:9, 3, 3))), as.character(cbind(3:1, 6:4, 9:7)))
}
RUnit_rev_array_character <- function() {
    slice1 <- as.character(cbind(3:1, 6:4, 9:7))
    slice2 <- as.character(cbind(12:10, 15:13, 18:16))
    slice3 <- as.character(cbind(21:19, 24:22, 27:25))
    all.equal(rev(as.character(array(1:27, c(3,3,3)))), abind(slice1, slice2, slice3, along=3))
}
RUnit_rev_vector_bool <- function() {
    all.equal(rev(c(T,T,F)), c(F,T,T))
}

## move
RUnit_move_vector <- function() {
    all.equal(move(1:10, 1), c(NaN, 1:9))
}
RUnit_move_vector_zero <- function() {
    all.equal(move(1:10, 0), 1:10)
}
RUnit_move_vector_copy <- function() {
    a <- 1:10
    b <- move(a, 2)
    all.equal(b, c(NaN,NaN,1:8)) & all.equal(a, 1:10)
}
RUnit_move_vector_inplace <- function() {
    a <- 1:10
    b <- a
    move(--b, 2)
    all.equal(b, c(NaN,NaN,1:8)) & all.equal(a, 1:10)
}
RUnit_move_vector_out_of_bound <- function() {
    tryCatch(move(1:10, 11), .Last.error=="subscript out of bounds")
}
RUnit_move_vector_negative <- function() {
    tryCatch(move(1:10, -1), .Last.error=="argument cannot be negative")
}
RUnit_move_matrix <- function() {
    all.equal(move(matrix(1:9, 3, 3), 1), matrix(c(NaN,1,2,NaN,4,5,NaN,7,8), 3, 3))
}
RUnit_move_matrix_not_integer <- function() {
    tryCatch(move(matrix(1:9, 3, 3), 1.1), .Last.error=="argument is not an integer")
}
RUnit_move_zts <- function() {
    n <- 3
    exp <- z1
    exp[4:nrow(z1), ] <- zts.data(z1)[1:(nrow(z1)-n), ]
    exp[1:3, ] <- NaN
    all.equal(move(z1, n), exp)
}
RUnit_move_zts_copy <- function() {
    z <- z1
    move(z, 3)
    all.equal(z, z1)
}

## rotate
RUnit_rotate_vector <- function() {
    all.equal(rotate(1:10, 1), c(10, 1:9))
}
RUnit_rotate_vector_zero <- function() {
    all.equal(rotate(1:10, 0), 1:10)
}
RUnit_rotate_vector_copy <- function() {
    a <- 1:10
    b <- rotate(a, 2)
    all.equal(b, c(9:10,1:8)) & all.equal(a, 1:10)
}
RUnit_rotate_vector_inplace <- function() {
    a <- 1:10
    b <- a
    rotate(--b, 2)
    all.equal(b, c(9:10,1:8)) & all.equal(a, 1:10)
}
RUnit_rotate_vector_out_of_bound <- function() {
    tryCatch(rotate(1:10, 11), .Last.error=="subscript out of bounds")
}
RUnit_rotate_vector_negative <- function() {
    tryCatch(rotate(1:10, -1), .Last.error=="argument cannot be negative")
}
RUnit_rotate_matrix <- function() {
    all.equal(rotate(matrix(1:9, 3, 3), 1), matrix(c(3,1,2,6,4,5,9,7,8), 3, 3))
}

## rollmean --------------------------------------
RUnit_rollmean_vector_n10 <- function() {
    expected <- vector("double", 10)
    expected[1:9] <- NaN
    expected[10]  <- cumsum(1:10)[10]/10
    all.equal(rollmean(1:10, 10), expected)
}
RUnit_rollmean_vector_n2 <- function() {
    expected <- c(NaN, 3, 5, 7, 9, 11, 13, 15, 17, 19)
    all.equal(rollmean(seq(2,20,by=2), 2), expected)
}
RUnit_rollmean_array_n2 <- function() {
    a <- array(seq(2,by=2,length.out=27), c(3,3,3))
    a_m <- move(a, 1)
    expected <- (a + a_m) / 2
    all.equal(rollmean(a, 2), expected)
}

## rollvar

## locf
RUnit_locf_vector <- function() {
    a <- c(1:3, NaN, NaN, 6:10)
    all.equal(locf(a, 1), c(1:3, 3, NaN, 6:10))
}
RUnit_locf_vector_n2 <- function() {
    a <- c(1:3, NaN, NaN, 6:10)
    all.equal(locf(a, 2), c(1:3, 3, 3, 6:10))
}
RUnit_locf_not_integer <- function() {
    tryCatch(locf(1:10, 1.1), .Last.error=="argument is not an integer")
}
RUnit_locf_zts <- function() {
    z <- z1
    z[4:5, ] <- NaN
    exp <- z1
    exp[4, ] <- zts.data(exp)[3, ]
    exp[5, ] <- NaN
    all.equal(locf(z, 1), exp)
}

## cumsum
RUnit_cumsum_vector <- function() {
    res <- 1:10
    for (i in 2:10) res[i] <- res[i] + res[i-1]
    all.equal(cumsum(1:10), res)
}
RUnit_cumsum_vector_rev <- function() {
    res <- 1:10
    for (i in 9:1) res[i] <- res[i] + res[i+1]
    all.equal(cumsum(1:10, rev=TRUE), res)
}
RUnit_cumsum_vector_NaN <- function() {
    c1 <- c(1,2,3,NaN,2,3,4,1,2)
    exp <- c(1,3,6,NaN,NaN,NaN,NaN,NaN,NaN)
    all.equal(cumsum(c1), exp)
}
RUnit_cumsum_zts <- function() {
    res <- z1
    for (j in 1:ncol(res)) {
        for (i in 2:nrow(res)) {
            res[i,j] <- zts.data(res)[i,j] + zts.data(res)[i-1,j]
        }
    }
    all.equal(cumsum(z1), res)
}

## cumprod
RUnit_cumprod_vector <- function() {
    res <- 1:10
    for (i in 2:10) res[i] <- res[i] * res[i-1]
    all.equal(cumprod(1:10), res)
}
RUnit_cumprod_vector_rev <- function() {
    res <- 1:10
    for (i in 9:1) res[i] <- res[i] * res[i+1]
    all.equal(cumprod(1:10, rev=TRUE), res)
}
RUnit_cumprod_vector_NaN <- function() {
    c1 <- c(1,2,3,NaN,2,3,4,1,2)
    exp <- c(1,2,6,NaN,NaN,NaN,NaN,NaN,NaN)
    all.equal(cumprod(c1), exp)
}
RUnit_cumprod_zts <- function() {
    res <- z1
    for (j in 1:ncol(res)) {
        for (i in 2:nrow(res)) {
            res[i,j] <- zts.data(res)[i,j] * zts.data(res)[i-1,j]
        }
    }
    all.equal(cumprod(z1), res)
}

## cumdiv
RUnit_cumdiv_vector <- function() {
    c1 <- 1:10
    res <- c1
    for (i in 2:10) res[i] <- res[i] / res[i-1]
    all.equal(cumdiv(c1), res)
}
RUnit_cumdiv_vector_rev <- function() {
    c1 <- 1:10
    res <- c1
    for (i in 9:1) res[i] <- res[i] / res[i+1]
    all.equal(cumdiv(c1, rev=TRUE), res)
}
RUnit_cumdiv_vector_NaN <- function() {
    c1 <- c(1,2,3,NaN,2,3,4,1,2)
    exp <- c(1,2,1.5,NaN,NaN,NaN,NaN,NaN,NaN)
    all.equal(cumdiv(c1), exp)
}
RUnit_cumdiv_vector_zero <- function() {
    c1 <- -1:10
    res <- c1
    for (i in 2:12) res[i] <- res[i] / res[i-1]
    all.equal(cumdiv(c1), res)
}

## cummax
RUnit_cummax_vector <- function() {
    all.equal(cummax(1:10), 1:10) &
    all.equal(cummax(10:1), c(10,10,10,10,10,10,10,10,10,10)) &
    all.equal(cummax(c(1,2,3,1,2,3,4,1,2)), c(1,2,3,3,3,3,4,4,4))        
}
RUnit_cummax_NaN <- function() {
    c1 <- c(1,2,3,NaN,2,3,4,1,2)
    c1_exp <- c(1,2,3,NaN,NaN,NaN,NaN,NaN,NaN)
    all.equal(cummax(c1), c1_exp)
}
RUnit_cummax_zts <- function() {
    all.equal(cummax(z1), z1)
}

## cummin
RUnit_cummin_vector <- function() {
    all.equal(cummin(1:10), c(1,1,1,1,1,1,1,1,1,1)) &
    all.equal(cummin(10:1), 10:1) &
    all.equal(cummin(c(10,9,8,10,9,8,2,5,-5,1)), c(10,9,8,8,8,8,2,2,-5,-5))
}
RUnit_cummin_NaN <- function() {
    c1 <- c(1,2,3,NaN,2,3,4,0,2)
    c1_exp <- c(1,1,1,NaN,NaN,NaN,NaN,NaN,NaN)
    all.equal(cummin(c1), c1_exp)
}
RUnit_cummin_zts <- function() {
    exp <- z1
    exp[,"one"] <- 1
    exp[,"two"] <- 10
    exp[,"three"] <- 19
    all.equal(cummin(z1), exp)
}

## sum/prod
RUnit_sum_vector <- function() {
    sum(1:10) == 55
}
RUnit_sum_matrix <- function() {
    sum(matrix(1, 10, 10)) == 100
}
RUnit_sum_array <- function() {
    sum(array(1, c(10,11,12))) == 1320
}
RUnit_sum_zts <- function() {
    sum(z1) == 378
}
RUnit_sum_duration <- function() {
    sum(as.duration(c(1:10))) == as.duration(55)
}
RUnit_sum_0_array <- function() {
    sum(as.double(NULL)) == 0           # like in R
}
RUnit_sum_NaN <- function() {
    all.equal(sum(as.double(NaN)), NaN)
}
RUnit_sum_vector_invalid_arg <- function() {
    tryCatch(sum(NULL),
             .Last.error == "invalid argument type: 'x' should be any of 'double','duration','zts' but is 'NULL'")
}
RUnit_prod_vector <- function() {
    prod(1:10) == 3628800
}
RUnit_prod_matrix <- function() {
    prod(matrix(2, 3, 4)) == 2^12
}
RUnit_prod_array <- function() {
    prod(array(2, c(2, 3, 4))) == 2^24
}
RUnit_prod_zts <- function() {
    prod(z1[,1]) == 362880
}
RUnit_prod_0_array <- function() {
    prod(as.double(NULL)) == 1           # like in R
}
RUnit_prod_NaN <- function() {
    all.equal(prod(as.double(NaN)), NaN)
}
RUnit_prod_vector_invalid_arg <- function() {
    tryCatch(prod(NULL),
             .Last.error == "invalid argument type: 'x' should be any of 'double','zts' but is 'NULL'")
}
