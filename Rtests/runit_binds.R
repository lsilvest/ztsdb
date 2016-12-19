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

source("utils.R")


## cbinds
RUnit_cbind_matrices <- function() {
    a <- matrix(1:9,   3, 3, dimnames=list(c("1","2","3"), c("a","b","c")))
    b <- matrix(10:18, 3, 3, dimnames=list(c("un","deux","trois"), c("unu","doi","trei")))
    r <- cbind(a, b)
    all.equal(r, matrix(1:18, 3, 6, dimnames=list(c("1","2","3"), c("a","b","c","unu","doi","trei"))))
}
RUnit_cbind_matrices_name_in_bind <- function() {
    a <- matrix(1:9,   3, 3, dimnames=list(c("1","2","3"), c("a","b","c")))
    b <- matrix(10:18, 3, 3, dimnames=list(c("un","deux","trois"), c("unu","doi","trei")))
    r <- cbind(a=a, b=b)
    all.equal(r, matrix(1:18, 3, 6, dimnames=list(c("1","2","3"),
                                        c("a.a","a.b","a.c","b.unu","b.doi","b.trei"))))
}
RUnit_cbind_vectors <- function() {
    r <- cbind(a=1:3, b=4:6)
    all.equal(r, matrix(1:6, 3, 2, dimnames=list(NULL, c("a","b")))) &&
    all.equal(cbind(r, c=7:9), matrix(1:9, 3, 3, dimnames=list(NULL, c("a","b","c"))))
}
RUnit_cbind_matrices_ref <- function() {
    a <- matrix(1:9,   3, 3, dimnames=list(c("1","2","3"), c("a","b","c")))
    b <- matrix(10:18, 3, 3, dimnames=list(c("un","deux","trois"), c("unu","doi","trei")))
    cbind(--a, b)
    all.equal(a, matrix(1:18, 3, 6, dimnames=list(c("1","2","3"), c("a","b","c","unu","doi","trei"))))
}
RUnit_cbind_matrices_name_in_bind_ref <- function() {
    a <- matrix(1:9,   3, 3, dimnames=list(c("1","2","3"), c("a","b","c")))
    b <- matrix(10:18, 3, 3, dimnames=list(c("un","deux","trois"), c("unu","doi","trei")))
    cbind(a=--a, b=b)
    all.equal(a, matrix(1:18, 3, 6, dimnames=list(c("1","2","3"),
                                        c("a.a","a.b","a.c","b.unu","b.doi","b.trei"))))
}
RUnit_cbind_matrices_0_dim <- function() {
    a <- matrix(1:9,   3, 3, dimnames=list(c("1","2","3"), c("a","b","c")))
    all.equal(a, cbind(matrix(0,3,0), a))
}
RUnit_cbind_matrices_0_dim_0_dim  <- function() {
    all.equal(cbind(matrix(0, 0, 0), matrix(0, 0, 4)), matrix(0, 0, 4))
}
RUnit_cbind_matrices_0_dim_0_dim_named <- function() {
    dn <- list(NULL, as.character(1:4))
    all.equal(cbind(matrix(0, 0, 0), matrix(0, 0, 4, dimnames=dn)), matrix(0, 0, 4, dimnames=dn))
}

## rbinds
RUnit_rbind_vectors <- function() {
    r <- rbind(a=1:3, b=4:6)
    all.equal(r, matrix(c(1,4,2,5,3,6), 2, 3, dimnames=list(c("a","b"), NULL))) &&
    all.equal(rbind(r, c=7:9), matrix(c(1,4,7,2,5,8,3,6,9), 3, 3, dimnames=list(c("a","b","c"),NULL)))
}
RUnit_rbind_matrices_0_dim <- function() {
    a <- matrix(1:9, 3, 3, dimnames=list(c("1","2","3"), c("a","b","c")))
    all.equal(a, rbind(matrix(0,0,3), a)) &&
    all.equal(a, rbind(a, matrix(0,0,3)))
}
RUnit_rbind_matrices_0_dim_0_dim_named <- function() {
    all.equal(rbind(matrix(0, 0, 0), matrix(0, 4, 0)), matrix(0, 4, 0))
}
RUnit_rbind_matrices_0_dim_0_dim_named <- function() {
    dn <- list(as.character(1:4), NULL)
    all.equal(rbind(matrix(0, 0, 0), matrix(0, 4, 0, dimnames=dn)), matrix(0, 4, 0, dimnames=dn))
}

## abinds
RUnit_abind_vectors_a1 <- function() {
    r <- abind(a=1:3, b=4:6, along=1)
    all.equal(r, matrix(c(1,4,2,5,3,6), 2, 3, dimnames=list(c("a","b"), NULL)))
}
RUnit_abind_vectors_a2 <- function() {
    r <- abind(a=1:3, b=4:6, along=2)
    all.equal(r, matrix(1:6, 3, 2, dimnames=list(NULL, c("a","b"))))
}
RUnit_abind_vectors_a3 <- function() {
    r <- abind(a=1:3, b=4:6, along=3)
    all.equal(r, array(c(1:6), c(3,1,2), dimnames=list(NULL,NULL,c("a","b"))))
}
RUnit_abind_array_a1 <- function() {
    a <- matrix(1:6, 3, 2)
    r <- matrix(c(1,2,3,1,2,3,4,5,6,4,5,6), 6, 2)
    all.equal(abind(a, a, along=1), r)
}
RUnit_abind_array_a2 <- function() {
    a <- matrix(1:6, 3, 2)
    r <- matrix(c(1:6, 1:6), 3, 4)
    all.equal(abind(a, a, along=2), r)
}
RUnit_abind_array_a3 <- function() {
    a <- matrix(1:6, 3, 2)
    r <- array(c(1:6, 1:6), c(3, 2, 2))
    all.equal(abind(a, a, along=3), r)
}
RUnit_abind_array_a3_multiple <- function() {
    a <- matrix(1:6, 3, 2)
    r <- array(c(1:6, 1:6, 1:6), c(3, 2, 3))
    all.equal(abind(a, a, a, along=3), r)
}
RUnit_abind_array_a3_multiple_dim0 <- function() {
    u <- array(0,c(0,0,1))
    all.equal(abind(array(0,c(0,0,0)), u, u, u, along=3), array(0, c(0,0,3)))
}


## ## zts
source("zts.R")
## zts rbind
RUnit_zts_rbind_matrix <- function() {
    data <- matrix(1:27, 9, 3)
    all.equal(rbind(z1, z2), zts(c(idx1,idx2), rbind(data,data), dim=c(18,3), dimnames=dimnames(z1)))
}
RUnit_zts_rbind_matrix_0dim <- function() {
    z <- zts(vector("time", 0), (matrix(0,0,3)))
    all.equal(rbind(z, z1), z1)
}
RUnit_zts_rbind_matrix_ref <- function() {
    data <- matrix(1:27, 9, 3)
    zp <- z1
    rbind(--zp, z2)
    all.equal(zp, zts(c(idx1,idx2), rbind(data,data), dim=c(18,3), dimnames=dimnames(z1)))
}
## zts cbind
RUnit_zts_cbind <- function() {
    data <- cbind(zts.data(z1), zts.data(z1))
    all.equal(cbind(z1, z1), zts(idx1, data, dimnames=dimnames(data)))
}
RUnit_zts_cbind_0dim <- function() {
    z <- zts(zts.idx(z1), matrix(0,dim(z1)[1],0))
    all.equal(cbind(z, z1), z1)
}
RUnit_zts_cbind_ref <- function() {
    data <- cbind(zts.data(z1), zts.data(z1))
    zp <- z1
    cbind(--zp, z1)
    all.equal(zp, zts(idx1, data, dimnames=dimnames(data)))
}
RUnit_zts_cbind_matrix <- function() {
    data <- cbind(zts.data(z1), zts.data(z1))
    all.equal(cbind(z1, zts.data(z1)), zts(idx1, data, dimnames=dimnames(data)))
}
RUnit_zts_cbind_matrix_0dim <- function() {
    z <- zts(zts.idx(z1), matrix(0,dim(z1)[1],0))
    all.equal(cbind(z, zts.data(z1)), z1)
}
RUnit_zts_cbind_matrix_ref <- function() {
    data <- cbind(zts.data(z1), zts.data(z1))
    zp <- z1
    cbind(--zp, zts.data(z1))
    all.equal(zp, zts(idx1, data, dimnames=dimnames(data)))
}
## zts abind
RUnit_zts_abind  <- function() {
    data <- abind(zts.data(z1), zts.data(z1), along=3)
    all.equal(abind(z1, z1, along=3), zts(idx1, data, dimnames=dimnames(data)))
}
RUnit_zts_abind_0dim  <- function() {
    z <- zts(zts.idx(z1), array(0, c(dim(z1)[1],3,0)))
    dnames <- list(NULL, dimnames(z1)[[2]], NULL)
    res <- zts(zts.idx(z1), array(zts.data(z1), c(dim(z1)[1],3,1)), dimnames=dnames)
    all.equal(abind(z, z1, along=3), res)
}
RUnit_zts_abind_multiple<- function() {
    data <- abind(zts.data(z1), zts.data(z1), zts.data(z1), along=3)
    all.equal(abind(z1, z1, z1, along=3), zts(idx1, data, dimnames=dimnames(data)))
}
RUnit_zts_abind_ref <- function() {
    data <- abind(zts.data(z1), zts.data(z1), along=3)
    zp <- z1
    abind(--zp, z1, along=3)
    all.equal(zp, zts(idx1, data, dimnames=dimnames(data)))
}
