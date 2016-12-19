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


## index type:
## null, double, double neg, bool, name, time, interval
##
## array types are:
## double, bool, string, time, interval, duration, period, zts


## double -----------------------------------------------------------
## 1d_0
RUnit_subset_double_0_vector_null_idx <- function() {
    a <- vector(mode="double", length=0)
    all.equal(a[], a)
}
RUnit_subset_double_0_vector_double_idx <- function() {
    a <- vector(mode="double", length=0)
    tryCatch(a[1], "error") == "error" &
    tryCatch(a[1:10], "error") == "error"
}
RUnit_subset_double_0_vector_double_neg_idx <- function() {
    a <- vector(mode="double", length=0)
    tryCatch(a[-1], "error") == "error" &
    tryCatch(a[-(1:10)], "error") == "error"
}
RUnit_subset_double_0_vector_bool_idx <- function() {
    a <- vector(mode="double", length=0)
    tryCatch(a[T], "error") == "error"
}
RUnit_subset_double_0_vector_name_idx <- function() {
    a <- vector(mode="double", length=0)
    tryCatch(a["a"], "error") == "error"
}
RUnit_subset_double_0_vector_time_idx <- function() {
    a <- vector(mode="double", length=0)
    tryCatch(a[|.2012-12-12 12:12:12 UTC.|], "error") == "error"
}
RUnit_subset_double_0_vector_interval_idx <- function() {
    a <- vector(mode="double", length=0)
    idx <- |+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC-|
    tryCatch(a[idx], "error") == "error"
}
## 1d
RUnit_subset_double_vector_null_idx <- function() {
    a <- 1:10
    all(a[] == a)
}
RUnit_subset_double_vector_double_idx <- function() {
    a <- 1:10
    all(a[2:9] == 2:9) &
    all(a[c(1,10)] == c(1,10))
}
RUnit_subset_double_vector_double_neg_idx <- function() {
    a <- 1:10
    all(a[-(2:9)] == c(1,10)) &
    all(a[-(1:10)] == vector(mode="double", length=0)) &
    all(a[-c(1,3,7,9)] == c(2,4,5,6,8,10))        
}
RUnit_subset_double_vector_bool_idx <- function() {
    a <- 1:5
    all(a[c(T,T,F,F,T)] == c(1,2,5))
}
RUnit_subset_double_vector_name_idx <- function() {
    a <- c(a=1,b=2,c=3,d=4,e=5)
    all(a[c("a","b","c")] == 1:3)
}
RUnit_subset_double_vector_time_idx <- function() {
    a <- 1:10
    tryCatch(a[|.2012-12-12 12:12:12 UTC.|], "error") == "error"
}
RUnit_subset_double_vector_interval_idx <- function() {
    a <- 1:10
    idx <- |+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC-|
    tryCatch(a[idx], "error") == "error"
}
## 2d
RUnit_subset_double_2d_null_idx <- function() {
    a <- matrix(1:6, 2, 3)
    all(a[] == a) &
    all(a[,] == a) &
    tryCatch(a[,,], "error") == "error"        
}
RUnit_subset_double_2d_double_1d_idx <- function() {
    a <- matrix(1:6, 2, 3)
    all(a[1:2] == 1:2) &
    all(a[1:6] == 1:6) &
    all(a[c(1,4,6)] == c(1,4,6))
}
RUnit_subset_double_2d_double_idx <- function() {
    a <- matrix(1:6, 2, 3)
    all(a[1,] == c(1,3,5)) &
    all(a[,1:3] == a) &
    all(a[1:2, 1:3] == a) &
    all(a[1:2, 1] == 1:2) &
    all(a[1, 1] == 1) &
    all(a[1, c(1,3)] == c(1,5))
}
RUnit_subset_double_2d_double <- neg_idx <- function() {
    a <- matrix(1:6, 2, 3)
    all(a[-1,] == c(2,4,6)) &
    all(a[,-(1:3)] == matrix(0, 2, 0)) &
    all.equal(a[-(1:2), -(1:3)], matrix(0, 0, 0))
    all(a[, -2] == matrix(c(1,2,5,6), 2, 2)) &
    all(a[-2, -(2:3)] == 1) &
    all(a[-2, -2] == c(1,5))
}
RUnit_subset_double_2d_bool_1d_idx <- function() {
    a <- matrix(1:6, 2, 3)
    all(a[c(T,T,F,F,F,F)] == 1:2) &
    all(a[c(T,F,F,F,F,T)] == c(1,6))
}
RUnit_subset_double_2d_bool_idx <- function() {
    a <- matrix(1:6, 2, 3)
    all(a[c(T,T), c(T,T,T)] == a) &
    all(a[c(T,T), ] == a) &
    all(a[, c(T,T,T)] == a) &
    all(a[c(T,F), c(T,F,F)] == 1) &
    all(a[c(T,F), c(T,T,T)] == c(1,3,5))
}
RUnit_subset_double_2d_name_idx <- function() {
    a <- matrix(1:6, 2, 3, dimnames=list(c("1","2"), c("a","b","c")))
    all(a[c("1","2"), ] == a) &
    all(a[, c("a","b","c")] == a) &
    all(a[c("1","2"), c("a","b","c")] == a) &
    all(a[c("1"), c("a")] == 1) &
    all(a[c("2"), c("c")] == 6) &
    all(a[c("2"), ] == c(2,4,6))
}
RUnit_subset_double_2d_time_idx <- function() {
    a <- matrix(1:6, 2, 3)
    tryCatch(a[|.2012-12-12 12:12:12 UTC.|, ], "error") == "error"
}
RUnit_subset_double_2d_interval_idx <- function() {
    a <- matrix(1:6, 2, 3)
    idx <- |+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC-|
    tryCatch(a[idx, ], "error") == "error"
}
## 3d
RUnit_subset_double_3d_null_idx <- function() {
    a <- array(1:24, c(2, 3, 4))
    all(a[] == a) &
    all(a[,,] == a) &
    tryCatch(a[,], "error") == "error" &
    tryCatch(a[,,,], "error") == "error"
}
RUnit_subset_double_3d_double_1d_idx <- function() {
    a <- array(1:24, c(2, 3, 4))
    all(a[1:2] == 1:2) &
    all(a[1:24] == 1:24) &
    all(a[c(1,5,24)] == c(1,5,24))
}
RUnit_subset_double_3d_double_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- array(1:24, c(2, 3, 4), dimnames=list(d1,d2,d3))
    all(a[1,,] == matrix(seq(1,23,by=2), 3, 4, dimnames=list(d2,d3))) &
    all(a[,1,] == matrix(c(1,2,7,8,13,14,19,20), 2, 4, dimnames=list(d1,d3))) &
    all(a[,,1] == matrix(1:6, 2, 3, dimnames=list(d1,d2))) &
    all(a[,1:3,] == a) &
    all(a[1:2, 1:3, 1:4] == a) &
    all(a[1:2, 1, 1] == c("1"=1,"2"=2)) &
    all(a[2,3,4] == 24) &
    all(a[1, c(1,3), 1] == c(1,5))
}
RUnit_subset_double_3d_double_neg_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- array(1:24, c(2, 3, 4), dimnames=list(d1,d2,d3))
    all(a[-2,,] == matrix(seq(1,23,by=2), 3, 4, dimnames=list(d2,d3))) &
    all(a[,-(2:3),] == matrix(c(1,2,7,8,13,14,19,20), 2, 4, dimnames=list(d1,d3))) &
    all(a[,,-(2:4)] == matrix(1:6, 2, 3, dimnames=list(d1,d2))) &
    all(a[,-(1:3),] == array(0, c(2,0,4), dimnames=list(d1,NULL,d3))) &
    all(a[, -(2:3), -(2:4)] == c("1"=1,"2"=2)) &
    all(a[-1,-(1:2),-(1:3)] == 24) &
    all(a[-2, -2, -(2:4)] == c(1,5))
}
RUnit_subset_double_3d_bool_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- array(1:24, c(2, 3, 4), dimnames=list(d1,d2,d3))
    all(a[c(T,F),,] == matrix(seq(1,23,by=2), 3, 4, dimnames=list(d2,d3))) &
    all(a[,c(T,F,F),] == matrix(c(1,2,7,8,13,14,19,20), 2, 4, dimnames=list(d1,d3))) &
    all(a[,,c(T,F,F,F)] == matrix(1:6, 2, 3, dimnames=list(d1,d2))) &
    all(a[c(T,T),,] == a) &
    all(a[,c(T,T,T),] == a) &
    all(a[,,c(T,T,T,T)] == a) &
    all(a[c(T,T), c(T,T,T), c(T,T,T,T)] == a) &
    all(a[, c(T,T,T),] == a) &
    all(a[c(F,T), c(F,F,T), c(F,F,F,T)] == 24)
}
RUnit_subset_double_3d_name_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- array(1:24, c(2, 3, 4), dimnames=list(d1,d2,d3))
    all(a["1",, ] == matrix(seq(1,23,by=2), 3, 4, dimnames=list(d2,d3))) &
    all(a[,"a",] == matrix(c(1,2,7,8,13,14,19,20), 2, 4, dimnames=list(d1,d3))) &
    all(a[,,"i"] == matrix(1:6, 2, 3, dimnames=list(d1,d2))) &
    all(a[d1,, ] == a) &
    all(a[,d2,] == a) &
    all(a[,,d3] == a) &
    all(a[, d2,] == a) &
    all(a[d1,d2,d3] == a) &
    all(a["1", "a", "i"] == 1) &
    all(a["2","c","iv"] == 24)
}
RUnit_subset_double_3d_time_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- array(1:24, c(2, 3, 4), dimnames=list(d1,d2,d3))
    tryCatch(a[|.2012-12-12 12:12:12 UTC.|,,], "error") == "error"
}
RUnit_subset_double_3d_interval_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- array(1:24, c(2, 3, 4), dimnames=list(d1,d2,d3))
    idx <- |+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC-|
    tryCatch(a[idx,,], "error") == "error"
}

## bool -----------------------------------------------------------
## 1d_0
RUnit_subset_bool_0_vector_null_idx <- function() {
    a <- vector(mode="logical", length=0)
    all.equal(a[], a)
}
RUnit_subset_bool_0_vector_double_idx <- function() {
    a <- vector(mode="logical", length=0)
    tryCatch(a[1], "error") == "error" &
    tryCatch(a[1:10], "error") == "error"
}
RUnit_subset_bool_0_vector_double_neg_idx <- function() {
    a <- vector(mode="logical", length=0)
    tryCatch(a[-1], "error") == "error" &
    tryCatch(a[-(1:10)], "error") == "error"
}
RUnit_subset_bool_0_vector_bool_bool_idx <- function() {
    a <- vector(mode="logical", length=0)
    tryCatch(a[T], "error") == "error"
}
RUnit_subset_bool_0_vector_bool_name_idx <- function() {
    a <- vector(mode="logical", length=0)
    tryCatch(a["a"], "error") == "error"
}
RUnit_subset_bool_0_vector_bool_time_idx <- function() {
    a <- vector(mode="logical", length=0)
    tryCatch(a[|.2012-12-12 12:12:12 UTC.|], "error") == "error"
}
RUnit_subset_bool_0_vector_bool_interval_idx <- function() {
    a <- vector(mode="logical", length=0)
    idx <- |+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC-|
    tryCatch(a[idx], "error") == "error"
}
## 1d
RUnit_subset_bool_vector_null_idx <- function() {
    a <- c(T,T,F,F,T,T)
    all(a[] == a)
}
RUnit_subset_bool_vector_double_idx <- function() {
    a <- c(T,T,F,F,T,T)
    all(a[2:5] == c(T,F,F,T)) &
    all(a[c(1:6)] == a)
}
RUnit_subset_bool_vector_double_neg_idx <- function() {
    a <- c(T,T,F,F,T,T)
    all(a[-c(1,6)] == c(T,F,F,T)) &
    all(a[-c(1:6)] == vector(mode="logical", length=0))
}
RUnit_subset_bool_vector_bool_idx <- function() {
    a <- c(T,T,F,F,T)
    all(a[c(T,T,F,F,T)] == c(T,T,T))
}
RUnit_subset_bool_vector_name_idx <- function() {
    a <- c(a=T,b=T,c=F,d=F,e=F)
    all(a[c("a","b","c")] == c(T,T,F))
}
RUnit_subset_bool_vector_time_idx <- function() {
    a <- c(T,T,T)
    tryCatch(a[|.2012-12-12 12:12:12 UTC.|], "error") == "error"
}
RUnit_subset_bool_vector_interval_idx <- function() {
    a <- c(F,T,T)
    idx <- |+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC-|
    tryCatch(a[idx], "error") == "error"
}
## 2d
RUnit_subset_bool_2d_null_idx <- function() {
    a <- matrix(c(T,T,F,F,T,T), 2, 3)
    all(a[] == a) &
    all(a[,] == a) &
    tryCatch(a[,,], "error") == "error"        
}
RUnit_subset_bool_2d_double_1d_idx <- function() {
    a <- matrix(c(T,T,F,F,T,T), 2, 3)
    a[1] &
    all(a[1:3] == c(T,T,F)) &
    all(a[4:6] == c(F,T,T)) &
    all(a[c(1,3,5)] == c(T,F,T)) &
    all(a[1:6] == c(T,T,F,F,T,T))
}
RUnit_subset_bool_2d_double_idx <- function() {
    a <- matrix(c(T,T,F,F,T,T), 2, 3)
    all(a[1,] == c(T,F,T)) &
    all(a[,1:3] == a) &
    all(a[1:2, 1:3] == a) &
    all(a[1:2, 1] == c(T,T)) &
    all(a[1, 1] == T) &
    all(a[1, c(1,3)] == c(T,T))
}
RUnit_subset_bool_2d_bool_idx <- function() {
    a <- matrix(c(T,T,F,F,T,T), 2, 3)
    all(a[c(T,T), c(T,T,T)] == a) &
    all(a[c(T,T), ] == a) &
    all(a[, c(T,T,T)] == a) &
    all(a[c(T,F), c(T,F,F)] == T) &
    all(a[c(T,F), c(T,T,T)] == c(T,F,T))
}
RUnit_subset_bool_2d_name_idx <- function() {
    a <- matrix(c(T,T,F,F,T,T), 2, 3, dimnames=list(c("1","2"), c("a","b","c")))
    all(a[c("1","2"), ] == a) &
    all(a[, c("a","b","c")] == a) &
    all(a[c("1","2"), c("a","b","c")] == a) &
    all(a[c("1"), c("a")] == T) &
    all(a[c("2"), c("c")] == T) &
    all(a[c("2"), ] == c(T,F,T))
}
RUnit_subset_bool_2d_time_idx <- function() {
    a <- matrix(c(T,T,F,F,T,T), 2, 3)
    tryCatch(a[|.2012-12-12 12:12:12 UTC.|, ], "error") == "error"
}
RUnit_subset_bool_2d_interval_idx <- function() {
    a <- matrix(c(T,T,F,F,T,T), 2, 3)
    idx <- |+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC-|
    tryCatch(a[idx, ], "error") == "error"
}
## 3d
RUnit_subset_bool_3d_null_idx <- function() {
    d <- c(T,T,T,F,F,F, T,T,F,F,T,F, T,F,T,F,T,F, F,F,F,T,T,T)
    a <- array(d, c(2, 3, 4))
    all(a[] == a) &
    all(a[,,] == a) &
    tryCatch(a[,], "error") == "error" &
    tryCatch(a[,,,], "error") == "error"
}
RUnit_subset_double_3d_double_1d_idx <- function() {
    d <- c(T,T,T,F,F,F, T,T,F,F,T,F, T,F,T,F,T,F, F,F,F,T,T,T)
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- array(d, c(2, 3, 4), dimnames=list(d1,d2,d3))
    all(a[1:2] == c(T,T)) &
    all(a[1:24] == d) &
    all(a[c(1,5,24)] == d[c(1,5,24)])
}
RUnit_subset_bool_3d_double_idx <- function() {
    d <- c(T,T,T,F,F,F, T,T,F,F,T,F, T,F,T,F,T,F, F,F,F,T,T,T)
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- array(d, c(2, 3, 4), dimnames=list(d1,d2,d3))
    all(a[1,,] == matrix(c(T,T,F,T,F,T,T,T,T,F,F,T), 3, 4, dimnames=list(d2,d3))) &
    all(a[,1,] == matrix(c(T,T,T,T,T,F,F,F), 2, 4, dimnames=list(d1,d3))) &
    all(a[,,1] == matrix(c(T,T,T,F,F,F), 2, 3, dimnames=list(d1,d2))) &
    all(a[,1:3,] == a) &
    all(a[1:2, 1:3, 1:4] == a) &
    all(a[1:2, 1, 1] == c("1"=T,"2"=T)) &
    all(a[2,3,4] == T) &
    all(a[1, c(1,3), 1] == c(T,F))
}
RUnit_subset_bool_3d_double_neg_idx <- function() {
    d <- c(T,T,T,F,F,F, T,T,F,F,T,F, T,F,T,F,T,F, F,F,F,T,T,T)
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- array(d, c(2, 3, 4), dimnames=list(d1,d2,d3))
    all(a[-2,,] == matrix(c(T,T,F,T,F,T,T,T,T,F,F,T), 3, 4, dimnames=list(d2,d3))) &
    all(a[,-c(2:3),] == matrix(c(T,T,T,T,T,F,F,F), 2, 4, dimnames=list(d1,d3))) &
    all(a[,,-c(2:4)] == matrix(c(T,T,T,F,F,F), 2, 3, dimnames=list(d1,d2))) &
    all(a[,-c(1:3),] == array(F, c(2,0,4), dimnames=list(d1,NULL,d3))) &
    all(a[, -c(2:3), -c(2:4)] == c("1"=T,"2"=T)) &
    all(a[-1,-c(1:2),-c(1:3)] == T) &
    all(a[-2, -2, -c(2:4)] == c(T,F))
}
RUnit_subset_bool_3d_bool_idx <- function() {
    d <- c(T,T,T,F,F,F, T,T,F,F,T,F, T,F,T,F,T,F, F,F,F,T,T,T)
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- array(d, c(2, 3, 4), dimnames=list(d1,d2,d3))
    all(a[c(T,F),,] == matrix(c(T,T,F,T,F,T,T,T,T,F,F,T), 3, 4, dimnames=list(d2,d3))) &
    all(a[,c(T,F,F),] == matrix(c(T,T,T,T,T,F,F,F), 2, 4, dimnames=list(d1,d3))) &
    all(a[,,c(T,F,F,F)] == matrix(c(T,T,T,F,F,F), 2, 3, dimnames=list(d1,d2))) &
    all(a[c(T,T),,] == a) &
    all(a[,c(T,T,T),] == a) &
    all(a[,,c(T,T,T,T)] == a) &
    all(a[c(T,T), c(T,T,T), c(T,T,T,T)] == a) &
    all(a[, c(T,T,T),] == a) &
    all(a[c(F,T), c(F,F,T), c(F,F,F,T)] == T)
}
RUnit_subset_bool_3d_name_idx <- function() {
    d <- c(T,T,T,F,F,F, T,T,F,F,T,F, T,F,T,F,T,F, F,F,F,T,T,T)
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- array(d, c(2, 3, 4), dimnames=list(d1,d2,d3))
    all(a["1",, ] == matrix(c(T,T,F,T,F,T,T,T,T,F,F,T), 3, 4, dimnames=list(d2,d3))) &
    all(a[,"a",] == matrix(c(T,T,T,T,T,F,F,F), 2, 4, dimnames=list(d1,d3))) &
    all(a[,,"i"] == matrix(c(T,T,T,F,F,F), 2, 3, dimnames=list(d1,d2))) &
    all(a[d1,, ] == a) &
    all(a[,d2,] == a) &
    all(a[,,d3] == a) &
    all(a[, d2,] == a) &
    all(a[d1,d2,d3] == a) &
    all(a["1", "a", "i"] == T) &
    all(a["2","c","iv"] == T)
}
RUnit_subset_bool_3d_time_idx <- function() {
    d <- c(T,T,T,F,F,F, T,T,F,F,T,F, T,F,T,F,T,F, F,F,F,T,T,T)
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- array(d, c(2, 3, 4), dimnames=list(d1,d2,d3))
    tryCatch(a[|.2012-12-12 12:12:12 UTC.|,,], "error") == "error"
}
RUnit_subset_bool_3d_interval_idx <- function() {
    d <- c(T,T,T,F,F,F, T,T,F,F,T,F, T,F,T,F,T,F, F,F,F,T,T,T)
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- array(d, c(2, 3, 4), dimnames=list(d1,d2,d3))
    idx <- |+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC-|
    tryCatch(a[idx,,], "error") == "error"
}

## string -----------------------------------------------------------
## 1d_0
RUnit_subset_string_0_vector_double_null_idx <- function() {
    a <- vector(mode="character", length=0)
    all.equal(a[], a)
}
RUnit_subset_string_0_vector_double_idx <- function() {
    a <- vector(mode="character", length=0)
    tryCatch(a[1], "error") == "error" &
    tryCatch(a[1:10], "error") == "error"
}
RUnit_subset_string_0_vector_double_neg_idx <- function() {
    a <- vector(mode="character", length=0)
    tryCatch(a[-1], "error") == "error" &
    tryCatch(a[-(1:10)], "error") == "error"
}
RUnit_subset_string_0_vector_bool_idx <- function() {
    a <- vector(mode="character", length=0)
    tryCatch(a[T], "error") == "error"
}
RUnit_subset_string_0_vector_name_idx <- function() {
    a <- vector(mode="character", length=0)
    tryCatch(a["a"], "error") == "error"
}
RUnit_subset_string_0_vector_time_idx <- function() {
    a <- vector(mode="character", length=0)
    tryCatch(a[|.2012-12-12 12:12:12 UTC.|], "error") == "error"
}
RUnit_subset_string_0_vector_interval_idx <- function() {
    a <- vector(mode="character", length=0)
    idx <- |+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC-|
    tryCatch(a[idx], "error") == "error"
}
## 1d
RUnit_subset_string_vector_null_idx <- function() {
    a <- as.character(1:10)
    all(a[] == a)
}
RUnit_subset_string_vector_double_idx <- function() {
    a <- as.character(1:10)
    all(a[2:9] == as.character(2:9)) &
    all(a[c(1,10)] == as.character(c(1,10)))
}
RUnit_subset_string_vector_double_neg_idx <- function() {
    a <- as.character(1:10)
    all(a[-c(1,10)] == as.character(2:9)) &
    all(a[-(2:9)] == as.character(c(1,10)))
}
RUnit_subset_string_vector_bool_idx <- function() {
    a <- as.character(1:5)
    all(a[c(T,T,F,F,T)] == as.character(c(1,2,5)))
}
RUnit_subset_string_vector_name_idx <- function() {
    a <- as.character(c(a=1,b=2,c=3,d=4,e=5))
    all(a[c("a","b","c")] == as.character(1:3))
}
RUnit_subset_string_vector_time_idx <- function() {
    a <- 1:10
    tryCatch(a[|.2012-12-12 12:12:12 UTC.|], "error") == "error"
}
RUnit_subset_string_vector_interval_idx <- function() {
    a <- 1:10
    idx <- |+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC-|
    tryCatch(a[idx], "error") == "error"
}
## 2d
RUnit_subset_string_2d_null_idx <- function() {
    a <- as.character(matrix(1:6, 2, 3))
    all(a[] == a) &
    all(a[,] == a) &
    tryCatch(a[,,], "error") == "error"        
}
RUnit_subset_string_2d_double_1d_idx <- function() {
    a <- as.character(matrix(1:6, 2, 3))
    all(a[1] == as.character(1)) &
    all(a[1:3] == as.character(1:3)) &
    all(a[4:6] == as.character(4:6)) &
    all(a[1:6] == as.character(1:6))
}
RUnit_subset_string_2d_double_idx <- function() {
    a <- as.character(matrix(1:6, 2, 3))
    all(a[1,] == as.character(c(1,3,5))) &
    all(a[,1:3] == a) &
    all(a[1:2, 1:3] == a) &
    all(a[1:2, 1] == as.character(1:2)) &
    all(a[1, 1] == "1") &
    all(a[1, c(1,3)] == as.character(c(1,5)))
}
RUnit_subset_string_2d_double_neg_idx <- function() {
    a <- as.character(matrix(1:6, 2, 3))
    all(a[-2,] == as.character(c(1,3,5))) &
    all(a[, -(2:3)] == as.character(1:2)) &
    all(a[-2, -(2:3)] == "1") &
    all(a[-2, -2] == as.character(c(1,5)))
}
RUnit_subset_string_2d_bool_idx <- function() {
    a <- as.character(matrix(1:6, 2, 3))
    all(a[c(T,T), c(T,T,T)] == a) &
    all(a[c(T,T), ] == a) &
    all(a[, c(T,T,T)] == a) &
    all(a[c(T,F), c(T,F,F)] == "1") &
    all(a[c(T,F), c(T,T,T)] == as.character(c(1,3,5)))
}
RUnit_subset_string_2d_name_idx <- function() {
    a <- as.character(matrix(1:6, 2, 3, dimnames=list(c("1","2"), c("a","b","c"))))
    all(a[c("1","2"), ] == a) &
    all(a[, c("a","b","c")] == a) &
    all(a[c("1","2"), c("a","b","c")] == a) &
    all(a[c("1"), c("a")] == "1") &
    all(a[c("2"), c("c")] == "6") &
    all(a[c("2"), ] == as.character(c(2,4,6)))
}
RUnit_subset_string_2d_time_idx <- function() {
    a <- as.character(matrix(1:6, 2, 3))
    tryCatch(a[|.2012-12-12 12:12:12 UTC.|, ], "error") == "error"
}
RUnit_subset_string_2d_interval_idx <- function() {
    a <- as.character(matrix(1:6, 2, 3))
    idx <- |+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC-|
    tryCatch(a[idx, ], "error") == "error"
}
## 3d
RUnit_subset_string_3d_null_idx <- function() {
    a <- as.character(array(1:24, c(2, 3, 4)))
    all(a[] == a) &
    all(a[,,] == a) &
    tryCatch(a[,], "error") == "error" &
    tryCatch(a[,,,], "error") == "error"
}
RUnit_subset_string_3d_double_1d_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- as.character(array(1:24, c(2, 3, 4), dimnames=list(d1,d2,d3)))
    all(a[1] == as.character(1)) &
    all(a[1:24] == as.character(1:24)) &
    all(a[c(2,23)] == as.character(c(2,23)))
}
RUnit_subset_string_3d_double_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- as.character(array(1:24, c(2, 3, 4), dimnames=list(d1,d2,d3)))
    all(a[1,,] == as.character(matrix(seq(1,23,by=2), 3, 4, dimnames=list(d2,d3)))) &
    all(a[,1,] == as.character(matrix(c(1,2,7,8,13,14,19,20), 2, 4, dimnames=list(d1,d3)))) &
    all(a[,,1] == as.character(matrix(1:6, 2, 3, dimnames=list(d1,d2)))) &
    all(a[,1:3,] == a) &
    all(a[1:2, 1:3, 1:4] == a) &
    all(a[1:2, 1, 1] == c("1"="1","2"="2")) &
    all(a[2,3,4] == "24") &
    all(a[1, c(1,3), 1] == c("1","5"))
}
RUnit_subset_string_3d_double_neg_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- as.character(array(1:24, c(2, 3, 4), dimnames=list(d1,d2,d3)))
    all(a[-2,,] == as.character(matrix(seq(1,23,by=2), 3, 4, dimnames=list(d2,d3)))) &
    all(a[,-(2:3),] == as.character(matrix(c(1,2,7,8,13,14,19,20), 2, 4, dimnames=list(d1,d3)))) &
    all(a[,,-(2:4)] == as.character(matrix(1:6, 2, 3, dimnames=list(d1,d2)))) &
    all(a[, -(2:3), -(2:4)] == c("1"="1","2"="2")) &
    all(a[-1,-(1:2),-(1:3)] == "24") &
    all(a[-2, -2, -(2:4)] == c("1","5"))
}
RUnit_subset_string_3d_bool_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- as.character(array(1:24, c(2, 3, 4), dimnames=list(d1,d2,d3)))
    all(a[c(T,F),,] == as.character(matrix(seq(1,23,by=2), 3, 4, dimnames=list(d2,d3)))) &
    all(a[,c(T,F,F),] == as.character(matrix(c(1,2,7,8,13,14,19,20), 2, 4, dimnames=list(d1,d3)))) &
    all(a[,,c(T,F,F,F)] == as.character(matrix(1:6, 2, 3, dimnames=list(d1,d2)))) &
    all(a[c(T,T),,] == a) &
    all(a[,c(T,T,T),] == a) &
    all(a[,,c(T,T,T,T)] == a) &
    all(a[c(T,T), c(T,T,T), c(T,T,T,T)] == a) &
    all(a[, c(T,T,T),] == a) &
    all(a[c(F,T), c(F,F,T), c(F,F,F,T)] == "24")
}
RUnit_subset_string_3d_name_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- as.character(array(1:24, c(2, 3, 4), dimnames=list(d1,d2,d3)))
    all(a["1",, ] == as.character(matrix(seq(1,23,by=2), 3, 4, dimnames=list(d2,d3)))) &
    all(a[,"a",] == as.character(matrix(c(1,2,7,8,13,14,19,20), 2, 4, dimnames=list(d1,d3)))) &
    all(a[,,"i"] == as.character(matrix(1:6, 2, 3, dimnames=list(d1,d2)))) &
    all(a[d1,, ] == a) &
    all(a[,d2,] == a) &
    all(a[,,d3] == a) &
    all(a[, d2,] == a) &
    all(a[d1,d2,d3] == a) &
    all(a["1", "a", "i"] == "1") &
    all(a["2","c","iv"] == "24")
}
RUnit_subset_string_3d_time_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- as.character(array(1:24, c(2, 3, 4), dimnames=list(d1,d2,d3)))
    tryCatch(a[|.2012-12-12 12:12:12 UTC.|,,], "error") == "error"
}
RUnit_subset_string_3d_interval_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- as.character(array(1:24, c(2, 3, 4), dimnames=list(d1,d2,d3)))
    idx <- |+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC-|
    tryCatch(a[idx,,], "error") == "error"
}

## time -----------------------------------------------------------
## 1d_0
RUnit_subset_time_0_vector_null_idx <- function() {
    a <- vector(mode="time", length=0)
    all.equal(a[], a)
}
RUnit_subset_time_0_vector_double_idx <- function() {
    a <- vector(mode="time", length=0)
    tryCatch(a[1], "error") == "error" &
    tryCatch(a[1:10], "error") == "error"
}
RUnit_subset_time_0_vector_double_neg_idx <- function() {
    a <- vector(mode="time", length=0)
    tryCatch(a[-1], "error") == "error" &
    tryCatch(a[-(1:10)], "error") == "error"
}
RUnit_subset_time_0_vector_bool_idx <- function() {
    a <- vector(mode="time", length=0)
    tryCatch(a[T], "error") == "error"
}
RUnit_subset_time_0_vector_name_idx <- function() {
    a <- vector(mode="time", length=0)
    tryCatch(a["a"], "error") == "error"
}
RUnit_subset_time_0_vector_time_idx <- function() {
    a <- vector(mode="time", length=0)
    tryCatch(a[|.2012-12-12 12:12:12 UTC.|], "error") == "error"
}
## this is perfectly legit:
RUnit_subset_time_0_vector_interval_idx <- function() {
    a <- vector(mode="time", length=0)
    idx <- |+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC-|
    all.equal(a[idx], a)
}
## 1d
RUnit_subset_time_vector_null_idx <- function() {
    a <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=10)
    all(a[] == a)
}
RUnit_subset_time_vector_double_idx <- function() {
    a <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=10)
    all(a[2:9] == seq(|.2012-12-12 12:12:13 UTC.|, length.out=8)) &
    all(a[c(1,10)] == c(a[1],a[10]))
}
RUnit_subset_time_vector_double_neg_idx <- function() {
    a <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=10)
    all(a[-c(1,10)] == seq(|.2012-12-12 12:12:13 UTC.|, length.out=8)) &
    all(a[-(2:9)] == c(a[1],a[10]))
}
RUnit_subset_time_vector_bool_idx <- function() {
    a <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=5)
    all(a[c(F,F,T,T,T)] == seq(|.2012-12-12 12:12:14 UTC.|, length.out=3))
}
RUnit_subset_time_vector_name_idx <- function() {
    x <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=5)
    a <- c(a=x[1],b=x[2],c=x[3],d=x[4],e=x[5])
    all(a[c("a","b","c")] == seq(|.2012-12-12 12:12:12 UTC.|, length.out=3))
}
RUnit_subset_time_vector_time_idx <- function() {
    a <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=10)
    all(a[seq(|.2012-12-12 12:12:12 UTC.|, length.out=3)] ==
        seq(|.2012-12-12 12:12:12 UTC.|, length.out=3))
}
RUnit_subset_time_vector_interval_idx <- function() {
    a <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=10)
    idx <- |+2012-12-12 12:12:12 UTC -> 2012-12-12 12:12:14 UTC-|
all(a[idx] == seq(|.2012-12-12 12:12:12 UTC.|, length.out=2))
}
## 2d
RUnit_subset_time_2d_null_idx <- function() {
    d <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=6)
    a <- matrix(d, 2, 3)
    all(a[] == a) &
    all(a[,] == a) &
    tryCatch(a[,,], "error") == "error"        
}
RUnit_subset_time_2d_double_1d_idx <- function() {
    d <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=6)
    a <- matrix(d, 2, 3)
    all(a[1] == d[1]) &
    all(a[1:3] == d[1:3]) &
    all(a[4:6] == d[4:6]) &
    all(a[c(1,6)] == d[c(1,6)])
}
RUnit_subset_time_2d_double_neg_1d_idx <- function() {
    d <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=6)
    a <- matrix(d, 2, 3)
    all(a[-1] == d[2:6]) &
    all(a[-(1:3)] == d[4:6]) &
    all(a[-(4:6)] == d[1:3])
}
RUnit_subset_time_2d_double_idx <- function() {
    d <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=6)
    a <- matrix(d, 2, 3)
    all(a[1,] == d[c(1,3,5)]) &
    all(a[,1:3] == a) &
    all(a[1:2, 1:3] == a) &
    all(a[1:2, 1] == d[1:2]) &
    all(a[1, 1] == d[1]) &
    all(a[1, c(1,3)] == d[c(1,5)])
}
RUnit_subset_time_2d_double_neg_idx <- function() {
    d <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=6)
    a <- matrix(d, 2, 3)
    all(a[-2,] == d[c(1,3,5)]) &
    all(a[, -(2:3)] == d[1:2]) &
    all(a[-2, -(2:3)] == d[1]) &
    all(a[-2, -2] == d[c(1,5)])
}
RUnit_subset_time_2d_bool_idx <- function() {
    d <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=6)
    a <- matrix(d, 2, 3)
    all(a[c(T,T), c(T,T,T)] == a) &
    all(a[c(T,T), ] == a) &
    all(a[, c(T,T,T)] == a) &
    all(a[c(T,F), c(T,F,F)] == d[1]) &
    all(a[c(T,F), c(T,T,T)] == d[c(1,3,5)])
}
RUnit_subset_time_2d_name_idx <- function() {
    d <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=6)
    a <- matrix(d, 2, 3, dimnames=list(c("1","2"), c("a","b","c")))
    all(a[c("1","2"), ] == a) &
    all(a[, c("a","b","c")] == a) &
    all(a[c("1","2"), c("a","b","c")] == a) &
    all(a[c("1"), c("a")] == d[1]) &
    all(a[c("2"), c("c")] == d[6]) &
    all(a[c("2"), ] == d[c(2,4,6)])
}
RUnit_subset_time_2d_time_idx <- function() {
    d <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=6)
    a <- matrix(d, 2, 3)
    ## error: time point index into non-vector
    tryCatch(a[|.2012-12-12 12:12:12 UTC.|,], "error") == "error"
}
RUnit_subset_time_2d_interval_idx <- function() {
    d <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=6)
    a <- matrix(d, 2, 3)
    idx <- |+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC-|
    tryCatch(a[idx], "error") == "error"
}
## 3d
RUnit_subset_time_3d_double_null_idx <- function() {
    d <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=24)
    a <- array(d, c(2, 3, 4))
    all(a[] == a) &
    all(a[,,] == a) &
    tryCatch(a[,], "error") == "error" &
    tryCatch(a[,,,], "error") == "error"
}
RUnit_subset_time_3d_double_1d_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    d <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=24)
    a <- array(d, c(2, 3, 4), dimnames=list(d1,d2,d3))
    all(a[1] == d[1]) &
    all(a[1:2] == d[1:2]) &
    all(a[c(1,24)] == d[c(1,24)])
}
RUnit_subset_time_3d_double_neg_1d_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    d <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=24)
    a <- array(d, c(2, 3, 4), dimnames=list(d1,d2,d3))
    all(a[-1] == d[2:24]) &
    all(a[-(1:2)] == d[3:24]) &
    all(a[-c(2:23)] == d[c(1,24)])
}
RUnit_subset_time_3d_double_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    d <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=24)
    a <- array(d, c(2, 3, 4), dimnames=list(d1,d2,d3))
    all(a[1,,] == matrix(seq(|.2012-12-12 12:12:12 UTC.|,
             length.out=12, by=as.duration(2e9)), 3, 4, dimnames=list(d2,d3))) &
    all(a[,1,] == matrix(c(d[1],d[2],d[7],d[8],d[13],d[14],d[19],d[20]),
             2, 4, dimnames=list(d1,d3))) &
    all(a[,,1] == matrix(d[1:6], 2, 3, dimnames=list(d1,d2))) &
    all(a[,1:3,] == a) &
    all(a[1:2, 1:3, 1:4] == a) &
    all(a[1:2, 1, 1] == c("1"=d[1],"2"=d[2])) &
    all(a[2,3,4] == d[24]) &
    all(a[1, c(1,3), 1] == d[c(1,5)])
}
RUnit_subset_time_3d_double_neg_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    d <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=24)
    a <- array(d, c(2, 3, 4), dimnames=list(d1,d2,d3))
    all(a[-2,,] == matrix(seq(|.2012-12-12 12:12:12 UTC.|,
             length.out=12, by=as.duration(2e9)), 3, 4, dimnames=list(d2,d3))) &
    all(a[,-(2:3),] == matrix(c(d[1],d[2],d[7],d[8],d[13],d[14],d[19],d[20]),
             2, 4, dimnames=list(d1,d3))) &
    all(a[,,-(2:4)] == matrix(d[1:6], 2, 3, dimnames=list(d1,d2))) &
    all.equal(a[,-(1:3),], array(d[1], c(2,0,4), dimnames=list(d1, NULL, d3))) &
    all.equal(a[-(1:2), -(1:3), -(1:4)], array(d[1], c(0,0,0))) &
    all(a[, -(2:3), -(2:4)] == c("1"=d[1],"2"=d[2])) &
    all(a[-1,-(1:2),-(1:3)] == d[24]) &
    all(a[-2, -2, -(2:4)] == d[c(1,5)])
}
RUnit_subset_time_3d_bool_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    d <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=24)
    a <- array(d, c(2, 3, 4), dimnames=list(d1,d2,d3))
    all(a[c(T,F),,] == matrix(d[seq(1,23,by=2)], 3, 4, dimnames=list(d2,d3))) &
    all(a[,c(T,F,F),] == matrix(d[c(1,2,7,8,13,14,19,20)], 2, 4, dimnames=list(d1,d3))) &
    all(a[,,c(T,F,F,F)] == matrix(d[1:6], 2, 3, dimnames=list(d1,d2))) &
    all(a[c(T,T),,] == a) &
    all(a[,c(T,T,T),] == a) &
    all(a[,,c(T,T,T,T)] == a) &
    all(a[c(T,T), c(T,T,T), c(T,T,T,T)] == a) &
    all(a[, c(T,T,T),] == a) &
    all(a[c(F,T), c(F,F,T), c(F,F,F,T)] == d[24])
}
RUnit_subset_time_3d_name_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    d <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=24)
    a <- array(d, c(2, 3, 4), dimnames=list(d1,d2,d3))
    all(a["1",, ] == matrix(d[seq(1,23,by=2)], 3, 4, dimnames=list(d2,d3))) &
    all(a[,"a",] == matrix(d[c(1,2,7,8,13,14,19,20)], 2, 4, dimnames=list(d1,d3))) &
    all(a[,,"i"] == matrix(d[1:6], 2, 3, dimnames=list(d1,d2))) &
    all(a[d1,, ] == a) &
    all(a[,d2,] == a) &
    all(a[,,d3] == a) &
    all(a[, d2,] == a) &
    all(a[d1,d2,d3] == a) &
    all(a["1", "a", "i"] == d[1]) &
    all(a["2","c","iv"] == d[24])
}
RUnit_subset_time_3d_time_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    d <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=24)
    a <- array(d, c(2, 3, 4), dimnames=list(d1,d2,d3))
    tryCatch(a[|.2012-12-12 12:12:12 UTC.|,,], "error") == "error"
}
RUnit_subset_time_3d_interval_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    d <- seq(|.2012-12-12 12:12:12 UTC.|, length.out=24)
    a <- array(d, c(2, 3, 4), dimnames=list(d1,d2,d3))
    idx <- |+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC-|
    tryCatch(a[idx,,], "error") == "error"
}


## interval -----------------------------------------------------------
## 1d_0
RUnit_subset_interval_0_vector_null_idx <- function() {
    a <- vector(mode="interval", length=0)
    all.equal(a[], a)
}
RUnit_subset_interval_0_vector_double_idx <- function() {
    a <- vector(mode="interval", length=0)
    tryCatch(a[1], "error") == "error" &
    tryCatch(a[1:10], "error") == "error"
}
RUnit_subset_interval_0_vector_double_neg_idx <- function() {
    a <- vector(mode="interval", length=0)
    tryCatch(a[-1], "error") == "error" &
    tryCatch(a[-(1:10)], "error") == "error"
}
RUnit_subset_interval_0_vector_bool_idx <- function() {
    a <- vector(mode="interval", length=0)
    tryCatch(a[T], "error") == "error"
}
RUnit_subset_interval_0_vector_name_idx <- function() {
    a <- vector(mode="interval", length=0)
    tryCatch(a["a"], "error") == "error"
}
RUnit_subset_interval_0_vector_time_idx <- function() {
    a <- vector(mode="interval", length=0)
    tryCatch(a[|.2012-12-12 12:12:12 UTC.|], "error") == "error"
}
RUnit_subset_interval_0_vector_interval_idx <- function() {
    a <- vector(mode="interval", length=0)
    idx <- |+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC-|
    tryCatch(a[idx], "error") == "error"
}
## 1d
RUnit_subset_interval_vector_null_idx <- function() {
    a <- seq(|+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|, length.out=10)
    all(a[] == a)
}
RUnit_subset_interval_vector_double_idx <- function() {
    a <- seq(|+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|, length.out=10)
    all(a[2:9] == seq(|+2012-12-12 12:12:13 UTC -> 2016-12-12 12:12:13 UTC+|, length.out=8)) &
    all(a[c(1,10)] == c(a[1],a[10]))
}
RUnit_subset_interval_vector_double_neg_idx <- function() {
    a <- seq(|+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|, length.out=10)
    all(a[-c(1,10)] == seq(|+2012-12-12 12:12:13 UTC -> 2016-12-12 12:12:13 UTC+|, length.out=8)) &
    all(a[-(2:9)] == c(a[1],a[10]))
}
RUnit_subset_interval_vector_bool_idx <- function() {
    a <- seq(|+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|, length.out=5)
    all(a[c(F,F,T,T,T)] == seq(|+2012-12-12 12:12:14 UTC -> 2016-12-12 12:12:14 UTC+|, length.out=3))
}
RUnit_subset_interval_vector_name_idx <- function() {
    x <- seq(|+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|, length.out=5)
    a <- c(a=x[1],b=x[2],c=x[3],d=x[4],e=x[5])
    all(a[c("a","b","c")] == seq(|+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|, length.out=3))
}
## time/interval is tested in another unit
## interval/interval is tested in another unit
## 2d
RUnit_subset_interval_2d_null_idx <- function() {
    d <- seq(|+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|, length.out=6)
    a <- matrix(d, 2, 3)
    all(a[] == a) &
    all(a[,] == a) &
    tryCatch(a[,,], "error") == "error"        
}
RUnit_subset_interval_2d_double_1d_idx <- function() {
    d <- seq(|+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|, length.out=6)
    a <- matrix(d, 2, 3)
    all(a[1] == d[c(1)]) &
    all(a[1:3] == d[1:3]) &
    all(a[4:6] == d[4:6]) &
    all(a[1:6] == d[1:6])
}
RUnit_subset_interval_2d_double_idx <- function() {
    d <- seq(|+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|, length.out=6)
    a <- matrix(d, 2, 3)
    all(a[1,] == d[c(1,3,5)]) &
    all(a[,1:3] == a) &
    all(a[1:2, 1:3] == a) &
    all(a[1:2, 1] == d[1:2]) &
    all(a[1, 1] == d[1]) &
    all(a[1, c(1,3)] == d[c(1,5)])
}
RUnit_subset_interval_2d_double_neg_idx <- function() {
    d <- seq(|+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|, length.out=6)
    a <- matrix(d, 2, 3)
    all(a[-2,] == d[c(1,3,5)]) &
    all(a[, -(2:3)] == d[1:2]) &
    all(a[-2, -(2:3)] == d[1]) &
    all(a[-2, -2] == d[c(1,5)])
}
RUnit_subset_interval_2d_bool_idx <- function() {
    d <- seq(|+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|, length.out=6)
    a <- matrix(d, 2, 3)
    all(a[c(T,T), c(T,T,T)] == a) &
    all(a[c(T,T), ] == a) &
    all(a[, c(T,T,T)] == a) &
    all(a[c(T,F), c(T,F,F)] == d[1]) &
    all(a[c(T,F), c(T,T,T)] == d[c(1,3,5)])
}
RUnit_subset_interval_2d_name_idx <- function() {
    d <- seq(|+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|, length.out=6)
    a <- matrix(d, 2, 3, dimnames=list(c("1","2"), c("a","b","c")))
    all(a[c("1","2"), ] == a) &
    all(a[, c("a","b","c")] == a) &
    all(a[c("1","2"), c("a","b","c")] == a) &
    all(a[c("1"), c("a")] == d[1]) &
    all(a[c("2"), c("c")] == d[6]) &
    all(a[c("2"), ] == d[c(2,4,6)])
}
RUnit_subset_interval_2d_time_idx <- function() {
    d <- seq(|+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|, length.out=6)
    a <- matrix(d, 2, 3)
    tryCatch(a[|+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|,], "error") == "error"
}
RUnit_subset_interval_2d_interval_idx <- function() {
    d <- seq(|+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|, length.out=6)
    a <- matrix(d, 2, 3)
    idx <- |+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC-|
    tryCatch(a[idx], "error") == "error"
}
## 3d
RUnit_subset_interval_3d_double_null_idx <- function() {
    d <- seq(|+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|, length.out=24)
    a <- array(d, c(2, 3, 4))
    all(a[] == a) &
    all(a[,,] == a) &
    tryCatch(a[,], "error") == "error" &
    tryCatch(a[,,,], "error") == "error"
}
RUnit_subset_interval_3d_double_1d_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    d <- seq(|+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|, length.out=24)
    a <- array(d, c(2, 3, 4), dimnames=list(d1,d2,d3))
    all(a[1,,] == matrix(seq(|+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|,
             length.out=12, by=as.duration(2e9)), 3, 4, dimnames=list(d2,d3))) &
    all(a[,1,] == matrix(c(d[1],d[2],d[7],d[8],d[13],d[14],d[19],d[20]),
             2, 4, dimnames=list(d1,d3))) &
    all(a[,,1] == matrix(d[1:6], 2, 3, dimnames=list(d1,d2))) &
    all(a[,1:3,] == a) &
    all(a[1:2, 1:3, 1:4] == a) &
    all(a[1:2, 1, 1] == c("1"=d[1],"2"=d[2])) &
    all(a[2,3,4] == d[24]) &
    all(a[1, c(1,3), 1] == d[c(1,5)])
}
RUnit_subset_interval_3d_double_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    d <- seq(|+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|, length.out=24)
    a <- array(d, c(2, 3, 4), dimnames=list(d1,d2,d3))
    all(a[1,,] == matrix(seq(|+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|,
             length.out=12, by=as.duration(2e9)), 3, 4, dimnames=list(d2,d3))) &
    all(a[,1,] == matrix(c(d[1],d[2],d[7],d[8],d[13],d[14],d[19],d[20]),
             2, 4, dimnames=list(d1,d3))) &
    all(a[,,1] == matrix(d[1:6], 2, 3, dimnames=list(d1,d2))) &
    all(a[,1:3,] == a) &
    all(a[1:2, 1:3, 1:4] == a) &
    all(a[1:2, 1, 1] == c("1"=d[1],"2"=d[2])) &
    all(a[2,3,4] == d[24]) &
    all(a[1, c(1,3), 1] == d[c(1,5)])
}
RUnit_subset_interval_3d_double_neg_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    d <- seq(|+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|, length.out=24)
    a <- array(d, c(2, 3, 4), dimnames=list(d1,d2,d3))
    all(a[-2,,] == matrix(seq(|+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|,
             length.out=12, by=as.duration(2e9)), 3, 4, dimnames=list(d2,d3))) &
    all(a[,-(2:3),] == matrix(c(d[1],d[2],d[7],d[8],d[13],d[14],d[19],d[20]),
             2, 4, dimnames=list(d1,d3))) &
    all(a[,,-(2:4)] == matrix(d[1:6], 2, 3, dimnames=list(d1,d2))) &
    all(a[, -(2:3), -(2:4)] == c("1"=d[1],"2"=d[2])) &
    all(a[-1,-(1:2),-(1:3)] == d[24]) &
    all(a[-2, -2, -(2:4)] == d[c(1,5)])
}
RUnit_subset_interval_3d_bool_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    d <- seq(|+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|, length.out=24)
    a <- array(d, c(2, 3, 4), dimnames=list(d1,d2,d3))
    all(a[c(T,F),,] == matrix(d[seq(1,23,by=2)], 3, 4, dimnames=list(d2,d3))) &
    all(a[,c(T,F,F),] == matrix(d[c(1,2,7,8,13,14,19,20)], 2, 4, dimnames=list(d1,d3))) &
    all(a[,,c(T,F,F,F)] == matrix(d[1:6], 2, 3, dimnames=list(d1,d2))) &
    all(a[c(T,T),,] == a) &
    all(a[,c(T,T,T),] == a) &
    all(a[,,c(T,T,T,T)] == a) &
    all(a[c(T,T), c(T,T,T), c(T,T,T,T)] == a) &
    all(a[, c(T,T,T),] == a) &
    all(a[c(F,T), c(F,F,T), c(F,F,F,T)] == d[24])
}
RUnit_subset_interval_3d_name_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    d <- seq(|+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|, length.out=24)
    a <- array(d, c(2, 3, 4), dimnames=list(d1,d2,d3))
    all(a["1",, ] == matrix(d[seq(1,23,by=2)], 3, 4, dimnames=list(d2,d3))) &
    all(a[,"a",] == matrix(d[c(1,2,7,8,13,14,19,20)], 2, 4, dimnames=list(d1,d3))) &
    all(a[,,"i"] == matrix(d[1:6], 2, 3, dimnames=list(d1,d2))) &
    all(a[d1,, ] == a) &
    all(a[,d2,] == a) &
    all(a[,,d3] == a) &
    all(a[, d2,] == a) &
    all(a[d1,d2,d3] == a) &
    all(a["1", "a", "i"] == d[1]) &
    all(a["2","c","iv"] == d[24])
}
RUnit_subset_interval_3d_time_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    d <- seq(|+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|, length.out=24)
    a <- array(d, c(2, 3, 4), dimnames=list(d1,d2,d3))
    tryCatch(a[|+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|,,], "error") == "error"
}
RUnit_subset_interval_3d_interval_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    d <- seq(|+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|, length.out=24)
    a <- array(d, c(2, 3, 4), dimnames=list(d1,d2,d3))
    idx <- |+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC-|
    tryCatch(a[idx,,], "error") == "error"
}

## duration -----------------------------------------------------------
## 1d_0
RUnit_subset_duration_0_vector_null_idx <- function() {
    a <- vector(mode="duration", length=0)
    all.equal(a[], a)
}
RUnit_subset_duration_0_vector_double_idx <- function() {
    a <- vector(mode="duration", length=0)
    tryCatch(a[1], "error") == "error" &
    tryCatch(a[1:10], "error") == "error"
}
RUnit_subset_duration_0_vector_double_neg_idx <- function() {
    a <- vector(mode="duration", length=0)
    tryCatch(a[-1], "error") == "error" &
    tryCatch(a[-(1:10)], "error") == "error"
}
RUnit_subset_duration_0_vector_bool_idx <- function() {
    a <- vector(mode="duration", length=0)
    tryCatch(a[T], "error") == "error"
}
RUnit_subset_duration_0_vector_name_idx <- function() {
    a <- vector(mode="duration", length=0)
    tryCatch(a["a"], "error") == "error"
}
RUnit_subset_duration_0_vector_time_idx <- function() {
    a <- vector(mode="duration", length=0)
    tryCatch(a[|.2012-12-12 12:12:12 UTC.|], "error") == "error"
}
RUnit_subset_duration_0_vector_interval_idx <- function() {
    a <- vector(mode="duration", length=0)
    idx <- |+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC-|
    tryCatch(a[idx], "error") == "error"
}
## 1d
RUnit_subset_duration_vector_null_idx <- function() {
    a <- as.duration(1:10)
    all(a[] == a)
}
RUnit_subset_duration_vector_double_idx <- function() {
    a <- as.duration(1:10)
    all(a[2:9] == as.duration(2:9)) &
    all(a[c(1,10)] == as.duration(c(1,10)))
}
RUnit_subset_duration_vector_double_neg_idx <- function() {
    a <- as.duration(1:10)
    all(a[-c(1,10)] == as.duration(2:9)) &
    all(a[-(2:9)] == as.duration(c(1,10)))
}
RUnit_subset_duration_vector_bool_idx <- function() {
    a <- as.duration(1:5)
    all(a[c(T,T,F,F,T)] == as.duration(c(1,2,5)))
}
RUnit_subset_duration_vector_name_idx <- function() {
    a <- as.duration(c(a=1,b=2,c=3,d=4,e=5))
    all(a[c("a","b","c")] == as.duration(1:3))
}
RUnit_subset_duration_vector_time_idx <- function() {
    a <- as.duration(1:10)
    tryCatch(a[|.2012-12-12 12:12:12 UTC.|], "error") == "error"
}
RUnit_subset_duration_vector_interval_idx <- function() {
    a <- as.duration(1:10)
    idx <- |+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC-|
    tryCatch(a[idx], "error") == "error"
}
## 2d
RUnit_subset_duration_2d_null_idx <- function() {
    a <- as.duration(matrix(1:6, 2, 3))
    all(a[] == a) &
    all(a[,] == a) &
    tryCatch(a[,,], "error") == "error"        
}
RUnit_subset_duration_2d_double_1d_idx <- function() {
    a <- as.duration(matrix(1:6, 2, 3))
    all(a[1:2] == as.duration(1:2)) &
    all(a[1:6] == as.duration(1:6)) &
    all(a[c(1,4,6)] == as.duration(c(1,4,6)))
}
RUnit_subset_duration_2d_double_idx <- function() {
    a <- as.duration(matrix(1:6, 2, 3))
    all(a[1,] == as.duration(c(1,3,5))) &
    all(a[,1:3] == a) &
    all(a[1:2, 1:3] == a) &
    all(a[1:2, 1] == as.duration(1:2)) &
    all(a[1, 1] == as.duration(1)) &
    all(a[1, c(1,3)] == as.duration(c(1,5)))
}
RUnit_subset_duration_2d_double_neg_idx <- function() {
    a <- as.duration(matrix(1:6, 2, 3))
    all(a[-2,] == as.duration(c(1,3,5))) &
    all(a[, -(2:3)] == as.duration(1:2)) &
    all(a[-2, -(2:3)] == as.duration(1)) &
    all(a[-2,-2] == as.duration(c(1,5)))
}
RUnit_subset_duration_2d_bool_1d_idx <- function() {
    a <- as.duration(matrix(1:6, 2, 3))
    all(a[c(T,T,F,F,F,F)] == as.duration(1:2)) &
    all(a[c(T,F,F,F,F,T)] == as.duration(c(1,6)))
}
RUnit_subset_duration_2d_bool_idx <- function() {
    a <- as.duration(matrix(1:6, 2, 3))
    all(a[c(T,T), c(T,T,T)] == a) &
    all(a[c(T,T), ] == a) &
    all(a[, c(T,T,T)] == a) &
    all(a[c(T,F), c(T,F,F)] == as.duration(1)) &
    all(a[c(T,F), c(T,T,T)] == as.duration(c(1,3,5)))
}
RUnit_subset_duration_2d_name_idx <- function() {
    a <- as.duration(matrix(1:6, 2, 3, dimnames=list(c("1","2"), c("a","b","c"))))
    all(a[c("1","2"), ] == a) &
    all(a[, c("a","b","c")] == a) &
    all(a[c("1","2"), c("a","b","c")] == a) &
    all(a[c("1"), c("a")] == as.duration(1)) &
    all(a[c("2"), c("c")] == as.duration(6)) &
    all(a[c("2"), ] == as.duration(c(2,4,6)))
}
RUnit_subset_duration_2d_time_idx <- function() {
    a <- as.duration(matrix(1:6, 2, 3))
    tryCatch(a[|.2012-12-12 12:12:12 UTC.|, ], "error") == "error"
}
RUnit_subset_duration_2d_interval_idx <- function() {
    a <- as.duration(matrix(1:6, 2, 3))
    idx <- |+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC-|
    tryCatch(a[idx, ], "error") == "error"
}
## 3d
RUnit_subset_duration_3d_null_idx <- function() {
    a <- as.duration(array(1:24, c(2, 3, 4)))
    all(a[] == a) &
    all(a[,,] == a) &
    tryCatch(a[,], "error") == "error" &
    tryCatch(a[,,,], "error") == "error"
}
RUnit_subset_duration_3d_double_1d_idx <- function() {
    a <- as.duration(array(1:24, c(2, 3, 4)))
    all(a[1:2] == as.duration(1:2)) &
    all(a[1:24] == as.duration(1:24)) &
    all(a[c(1,5,24)] == as.duration(c(1,5,24)))
}
RUnit_subset_duration_3d_double_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- as.duration(array(1:24, c(2, 3, 4), dimnames=list(d1,d2,d3)))
    all(a[1,,] == as.duration(matrix(seq(1,23,by=2), 3, 4, dimnames=list(d2,d3)))) &
    all(a[,1,] == as.duration(matrix(c(1,2,7,8,13,14,19,20), 2, 4, dimnames=list(d1,d3)))) &
    all(a[,,1] == as.duration(matrix(1:6, 2, 3, dimnames=list(d1,d2)))) &
    all(a[,1:3,] == a) &
    all(a[1:2, 1:3, 1:4] == a) &
    all(a[1:2, 1, 1] == as.duration(c("1"=1,"2"=2))) &
    all(a[2,3,4] == as.duration(24)) &
    all(a[1, c(1,3), 1] == as.duration(c(1,5)))
}
RUnit_subset_duration_3d_double_neg_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- as.duration(array(1:24, c(2, 3, 4), dimnames=list(d1,d2,d3)))
    all(a[-2,,] == as.duration(matrix(seq(1,23,by=2), 3, 4, dimnames=list(d2,d3)))) &
    all(a[,-(2:3),] == as.duration(matrix(c(1,2,7,8,13,14,19,20), 2, 4, dimnames=list(d1,d3)))) &
    all(a[,,-(2:4)] == as.duration(matrix(1:6, 2, 3, dimnames=list(d1,d2)))) &
    all(a[, -(2:3), -(2:4)] == as.duration(c("1"=1,"2"=2))) &
    all(a[-1,-(1:2),-(1:3)] == as.duration(24)) &
    all(a[-2, -2, -(2:4)] == as.duration(c(1,5)))
}
RUnit_subset_duration_3d_bool_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- as.duration(array(1:24, c(2, 3, 4), dimnames=list(d1,d2,d3)))
    all(a[c(T,F),,] == as.duration(matrix(seq(1,23,by=2), 3, 4, dimnames=list(d2,d3)))) &
    all(a[,c(T,F,F),] == as.duration(matrix(c(1,2,7,8,13,14,19,20), 2, 4, dimnames=list(d1,d3)))) &
    all(a[,,c(T,F,F,F)] == as.duration(matrix(1:6, 2, 3, dimnames=list(d1,d2)))) &
    all(a[c(T,T),,] == a) &
    all(a[,c(T,T,T),] == a) &
    all(a[,,c(T,T,T,T)] == a) &
    all(a[c(T,T), c(T,T,T), c(T,T,T,T)] == a) &
    all(a[, c(T,T,T),] == a) &
    all(a[c(F,T), c(F,F,T), c(F,F,F,T)] == as.duration(24))
}
RUnit_subset_duration_3d_name_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- as.duration(array(1:24, c(2, 3, 4), dimnames=list(d1,d2,d3)))
    all(a["1",, ] == as.duration(matrix(seq(1,23,by=2), 3, 4, dimnames=list(d2,d3)))) &
    all(a[,"a",] == as.duration(matrix(c(1,2,7,8,13,14,19,20), 2, 4, dimnames=list(d1,d3)))) &
    all(a[,,"i"] == as.duration(matrix(1:6, 2, 3, dimnames=list(d1,d2)))) &
    all(a[d1,, ] == a) &
    all(a[,d2,] == a) &
    all(a[,,d3] == a) &
    all(a[, d2,] == a) &
    all(a[d1,d2,d3] == a) &
    all(a["1", "a", "i"] == as.duration(1)) &
    all(a["2","c","iv"] == as.duration(24))
}
RUnit_subset_duration_3d_time_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- as.duration(array(1:24, c(2, 3, 4), dimnames=list(d1,d2,d3)))
    tryCatch(a[|.2012-12-12 12:12:12 UTC.|,,], "error") == "error"
}
RUnit_subset_duration_3d_interval_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- as.duration(array(1:24, c(2, 3, 4), dimnames=list(d1,d2,d3)))
    idx <- |+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC-|
    tryCatch(a[idx,,], "error") == "error"
}

## period -----------------------------------------------------------
## double to period
d2p <- function(x) {
    p <- as.period(paste0(as.character(x), "d"))
    array(p, dim(x), dimnames(x))
}
## 1d_0
RUnit_subset_period_0_vector_null_idx <- function() {
    a <- vector(mode="period", length=0)
    all.equal(a[], a)
}
RUnit_subset_period_0_vector_double_idx <- function() {
    a <- vector(mode="period", length=0)
    tryCatch(a[1], "error") == "error" &
    tryCatch(a[1:10], "error") == "error"
}
RUnit_subset_period_0_vector_double_neg_idx <- function() {
    a <- vector(mode="period", length=0)
    tryCatch(a[-1], "error") == "error" &
    tryCatch(a[-(1:10)], "error") == "error"
}
RUnit_subset_period_0_vector_bool_idx <- function() {
    a <- vector(mode="period", length=0)
    tryCatch(a[T], "error") == "error"
}
RUnit_subset_period_0_vector_name_idx <- function() {
    a <- vector(mode="period", length=0)
    tryCatch(a["a"], "error") == "error"
}
RUnit_subset_period_0_vector_time_idx <- function() {
    a <- vector(mode="period", length=0)
    tryCatch(a[|.2012-12-12 12:12:12 UTC.|], "error") == "error"
}
RUnit_subset_period_0_vector_interval_idx <- function() {
    a <- vector(mode="period", length=0)
    idx <- |+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC-|
    tryCatch(a[idx], "error") == "error"
}
## 1d
RUnit_subset_period_vector_null_idx <- function() {
    a <- d2p(1:10)
    all(a[] == a)
}
RUnit_subset_period_vector_double_idx <- function() {
    a <- d2p(1:10)
    all(a[2:9] == d2p(2:9)) &
    all(a[c(1,10)] == d2p(c(1,10)))
}
RUnit_subset_period_vector_double_idx <- function() {
    a <- d2p(1:10)
    all(a[-c(1,10)] == d2p(2:9)) &
    all(a[-(2:9)] == d2p(c(1,10)))
}
RUnit_subset_period_vector_bool_idx <- function() {
    a <- d2p(1:5)
    all(a[c(T,T,F,F,T)] == d2p(c(1,2,5)))
}
RUnit_subset_period_vector_name_idx <- function() {
    a <- d2p(c(a=1,b=2,c=3,d=4,e=5))
    all(a[c("a","b","c")] == d2p(1:3))
}
RUnit_subset_period_vector_time_idx <- function() {
    a <- d2p(1:10)
    tryCatch(a[|.2012-12-12 12:12:12 UTC.|], "error") == "error"
}
RUnit_subset_period_vector_interval_idx <- function() {
    a <- d2p(1:10)
    idx <- |+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC-|
    tryCatch(a[idx], "error") == "error"
}
## 2d
RUnit_subset_period_2d_null_idx <- function() {
    a <- d2p(matrix(1:6, 2, 3))
    all(a[] == a) &
    all(a[,] == a) &
    tryCatch(a[,,], "error") == "error"        
}
RUnit_subset_period_2d_double_1d_idx <- function() {
    a <- d2p(matrix(1:6, 2, 3))
    all(a[1:2] == d2p(1:2)) &
    all(a[1:6] == d2p(1:6)) &
    all(a[c(1,4,6)] == d2p(c(1,4,6)))
}
RUnit_subset_period_2d_double_idx <- function() {
    a <- d2p(matrix(1:6, 2, 3))
    all(a[1,] == d2p(c(1,3,5))) &
    all(a[,1:3] == a) &
    all(a[1:2, 1:3] == a) &
    all(a[1:2, 1] == d2p(1:2)) &
    all(a[1, 1] == d2p(1)) &
    all(a[1, c(1,3)] == d2p(c(1,5)))
}
RUnit_subset_period_2d_double_neg_idx <- function() {
    a <- d2p(matrix(1:6, 2, 3))
    all(a[-2,] == d2p(c(1,3,5))) &
    all(a[, -(2:3)] == d2p(1:2)) &
    all(a[-2, -(2:3)] == d2p(1)) &
    all(a[-2, -2] == d2p(c(1,5)))
}
RUnit_subset_period_2d_bool_1d_idx <- function() {
    a <- d2p(matrix(1:6, 2, 3))
    all(a[c(T,T,F,F,F,F)] == d2p(1:2)) &
    all(a[c(T,F,F,F,F,T)] == d2p(c(1,6)))
}
RUnit_subset_period_2d_bool_idx <- function() {
    a <- d2p(matrix(1:6, 2, 3))
    all(a[c(T,T), c(T,T,T)] == a) &
    all(a[c(T,T), ] == a) &
    all(a[, c(T,T,T)] == a) &
    all(a[c(T,F), c(T,F,F)] == d2p(1)) &
    all(a[c(T,F), c(T,T,T)] == d2p(c(1,3,5)))
}
RUnit_subset_period_2d_name_idx <- function() {
    a <- d2p(matrix(1:6, 2, 3, dimnames=list(c("1","2"), c("a","b","c"))))
    all(a[c("1","2"), ] == a) &
    all(a[, c("a","b","c")] == a) &
    all(a[c("1","2"), c("a","b","c")] == a) &
    all(a[c("1"), c("a")] == d2p(1)) &
    all(a[c("2"), c("c")] == d2p(6)) &
    all(a[c("2"), ] == d2p(c(2,4,6)))
}
RUnit_subset_period_2d_time_idx <- function() {
    a <- d2p(matrix(1:6, 2, 3))
    tryCatch(a[|.2012-12-12 12:12:12 UTC.|, ], "error") == "error"
}
RUnit_subset_period_2d_interval_idx <- function() {
    a <- d2p(matrix(1:6, 2, 3))
    idx <- |+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC-|
    tryCatch(a[idx, ], "error") == "error"
}
## 3d
RUnit_subset_period_3d_null_idx <- function() {
    a <- d2p(array(1:24, c(2, 3, 4)))
    all(a[] == a) &
    all(a[,,] == a) &
    tryCatch(a[,], "error") == "error" &
    tryCatch(a[,,,], "error") == "error"
}
RUnit_subset_period_3d_double_1d_idx <- function() {
    a <- d2p(array(1:24, c(2, 3, 4)))
    all(a[1:2] == d2p(1:2)) &
    all(a[1:24] == d2p(1:24)) &
    all(a[c(1,5,24)] == d2p(c(1,5,24)))
}
RUnit_subset_period_3d_double_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- d2p(array(1:24, c(2, 3, 4), dimnames=list(d1,d2,d3)))
    all(a[1,,] == d2p(matrix(seq(1,23,by=2), 3, 4, dimnames=list(d2,d3)))) &
    all(a[,1,] == d2p(matrix(c(1,2,7,8,13,14,19,20), 2, 4, dimnames=list(d1,d3)))) &
    all(a[,,1] == d2p(matrix(1:6, 2, 3, dimnames=list(d1,d2)))) &
    all(a[,1:3,] == a) &
    all(a[1:2, 1:3, 1:4] == a) &
    all(a[1:2, 1, 1] == d2p(c("1"=1,"2"=2))) &
    all(a[2,3,4] == d2p(24)) &
    all(a[1, c(1,3), 1] == d2p(c(1,5)))
}
RUnit_subset_period_3d_double_neg_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- d2p(array(1:24, c(2, 3, 4), dimnames=list(d1,d2,d3)))
    all(a[-2,,] == d2p(matrix(seq(1,23,by=2), 3, 4, dimnames=list(d2,d3)))) &
    all(a[,-(2:3),] == d2p(matrix(c(1,2,7,8,13,14,19,20), 2, 4, dimnames=list(d1,d3)))) &
    all(a[,,-(2:4)] == d2p(matrix(1:6, 2, 3, dimnames=list(d1,d2)))) &
    all(a[, -(2:3), -(2:4)] == d2p(c("1"=1,"2"=2))) &
    all(a[-1,-(1:2),-(1:3)] == d2p(24)) &
    all(a[-2, -2, -(2:4)] == d2p(c(1,5)))
}
RUnit_subset_period_3d_bool_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- d2p(array(1:24, c(2, 3, 4), dimnames=list(d1,d2,d3)))
    all(a[c(T,F),,] == d2p(matrix(seq(1,23,by=2), 3, 4, dimnames=list(d2,d3)))) &
    all(a[,c(T,F,F),] == d2p(matrix(c(1,2,7,8,13,14,19,20), 2, 4, dimnames=list(d1,d3)))) &
    all(a[,,c(T,F,F,F)] == d2p(matrix(1:6, 2, 3, dimnames=list(d1,d2)))) &
    all(a[c(T,T),,] == a) &
    all(a[,c(T,T,T),] == a) &
    all(a[,,c(T,T,T,T)] == a) &
    all(a[c(T,T), c(T,T,T), c(T,T,T,T)] == a) &
    all(a[, c(T,T,T),] == a) &
    all(a[c(F,T), c(F,F,T), c(F,F,F,T)] == d2p(24))
}
RUnit_subset_period_3d_name_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- d2p(array(1:24, c(2, 3, 4), dimnames=list(d1,d2,d3)))
    all(a["1",, ] == d2p(matrix(seq(1,23,by=2), 3, 4, dimnames=list(d2,d3)))) &
    all(a[,"a",] == d2p(matrix(c(1,2,7,8,13,14,19,20), 2, 4, dimnames=list(d1,d3)))) &
    all(a[,,"i"] == d2p(matrix(1:6, 2, 3, dimnames=list(d1,d2)))) &
    all(a[d1,, ] == a) &
    all(a[,d2,] == a) &
    all(a[,,d3] == a) &
    all(a[, d2,] == a) &
    all(a[d1,d2,d3] == a) &
    all(a["1", "a", "i"] == d2p(1)) &
    all(a["2","c","iv"] == d2p(24))
}
RUnit_subset_period_3d_time_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- d2p(array(1:24, c(2, 3, 4), dimnames=list(d1,d2,d3)))
    tryCatch(a[|.2012-12-12 12:12:12 UTC.|,,], "error") == "error"
}
RUnit_subset_period_3d_interval_idx <- function() {
    d1 <- c("1","2"); d2 <- c("a","b","c"); d3 <- c("i","ii","iii","iv")
    a <- d2p(array(1:24, c(2, 3, 4), dimnames=list(d1,d2,d3)))
    idx <- |+2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC-|
    tryCatch(a[idx,,], "error") == "error"
}
