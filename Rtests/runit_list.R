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


## basic list functionality
RUnit_list_constructor <- function() {
  l1 <- list(1,2,3)
  l2 <- c(list(1), list(2), list(3))
  length(l1) == 3 &
  length(l2) == 3 &
  all.equal(l1, l2)  
}
RUnit_list_subset_int <- function() {
  l <- list(1,2,3)
  all.equal(l[1], list(1)) &
  all.equal(l[1:2], list(1,2)) &
  all.equal(l[c(1,3)], list(1,3)) &
  all.equal(l[], l)
}
RUnit_list_subset_logical <- function() {
  l <- list(1,2,3)
  all.equal(l[c(T,F,F)], list(1)) &
  all.equal(l[c(T,T,F)], list(1,2)) &
  all.equal(l[c(T,F,T)], list(1,3))
}
RUnit_named_list_constructor <- function() {
  l1 <- list(a=1,b=2,c=3)
  l2 <- c(a=list(1), b=list(2), c=list(3))
  length(l1) == 3 &
  length(l2) == 3 &
  all.equal(rownames(l1), c("a","b","c")) &
  all.equal(rownames(l2),  c("a","b","c")) &
  all.equal(l1, l2)
}
RUnit_list_subset_names <- function() {
  l <- list(a=1,b=2,c=3)
  all.equal(l["a"], list(a=1)) &
  all.equal(l[c("a","b")], list(a=1,b=2)) &
  all.equal(l[c("a","c")], list(a=1,c=3))
}

## recursive [[]] indexing, do we want to implement that??? LLL
##
##
RUnit_list_constructor_mixed <- function() {
  l1 <- list(list(1),"a", matrix(1:9, 3, 3))
  all.equal(l1[1], list(list(1))) &
  all.equal(l1[2], list("a")) &
  all.equal(l1[3], list(matrix(1:9, 3, 3)))
}
RUnit_list_subset_assign <- function() {
  l <- list(1,2,3)
  all.equal(l[1]      <- list(2),     list(2,2,3)) &
  all.equal(l[1:2]    <- list(3,4),   list(3,4,3)) &
  all.equal(l[c(1,3)] <- list(-1, 7), list(-1,4,7)) &
  all.equal(l[]       <- list(2),     list(2,2,2))
}
RUnit_list_dblsubset <- function() {
  l <- list("a", 3, list(1,2,3))
  all.equal(l[[1]], "a") &
  all.equal(l[[2]],   3) &
  all.equal(l[[3]], list(1,2,3))
}
RUnit_list_dblsubset_names <- function() {
  l <- list(a="a", b=3, c=list(1,2,3))
  all.equal(l[["a"]], "a") &
  all.equal(l[["b"]],   3) &
  all.equal(l[["c"]], list(1,2,3))
}
RUnit_list_dblsubset_dollar <- function() {
  l <- list("11"="a", b=3, c=list(1,2,3))
  all.equal(l$"11", "a") &
  all.equal(l$b,   3) &
  all.equal(l[["c"]], list(1,2,3))
}
RUnit_list_dblsubassign <- function() {
  l <- list(1,2,3)
  all.equal({l[[1]] <- list(1); l}, list(list(1), 2, 3)) &
  all.equal({l[[2]] <-       4; l}, list(list(1), 4, 3)) &
  all.equal({l[[3]] <-     "b"; l}, list(list(1), 4, "b"))
}
RUnit_list_dblsubassign_names <- function() {
  l <- list("11"=1,b=2,c=3)
  all.equal({l[["11"]] <- list(1); l}, list("11"=list(1), b=2, c=3)) &
  all.equal({l[["b"]]  <-       4; l}, list("11"=list(1), b=4, c=3)) &
  all.equal({l[["c"]]  <-     "b"; l}, list("11"=list(1), b=4, c="b"))
}
RUnit_list_dblsubassign_dollar <- function() {
  l <- list("11"=1,b=2,c=3)
  all.equal({l$"11" <- list(1); l}, list("11"=list(1), b=2, c=3)) &
  all.equal({l$b    <-       4; l}, list("11"=list(1), b=4, c=3)) &
  all.equal({l$c    <-     "b"; l}, list("11"=list(1), b=4, c="b"))
}
RUnit_list_c_append <- function() {
  l1 <- list(1)
  l2 <- c(l1, list(2,3))
  all.equal(l2, list(1,2,3))
}
RUnit_list_c_append_named_list <- function() {
  l1 <- list(a=1)
  l2 <- c(l1, list(b=2, c=3))
  all.equal(l2, list(a=1, b=2, c=3)) 
}
RUnit_list_append_inplace <- function() {
  l <- list(a=1)
  c(--l, list(b=2, c=3))
  all.equal(l, list(a=1, b=2, c=3))
}
RUnit_list_named_append_inplace <- function() {
  l <- list(a=1)
  l[["b"]] <- 2
  l[["c"]] <- 3
  all.equal(l, list(a=1, b=2, c=3))
}

## nested operations
RUnit_nested_subset <- function() {
  l <- list(list(list(1), 2, 3), 4, list(5))
  all.equal(l[1], list(list(list(1), 2, 3))) &
  all.equal(l[1][1], l[1])
}
RUnit_nested_dlbsubset <- function() {
  l <- list(list(list(1), 2, 3), 4, list(5))
  all.equal(l[[1]], list(list(1), 2, 3)) &
  all.equal(l[[1]][[2]], 2) &
  all.equal(l[[1]][[1]][[1]], 1) &
  all.equal(l[[3]][[1]], 5)
}
RUnit_nested_dlbsubset_names <- function() {
  l <- list(a=list(a=list(a=1), b=2, c=3), b=4, c=list(a=5))
  all.equal(l[["a"]], list(a=list(a=1), b=2, c=3)) &
  all.equal(l[["a"]][["b"]], 2) &
  all.equal(l[["a"]][["a"]][["a"]], 1) &
  all.equal(l[["c"]][["a"]], 5)
}
RUnit_nested_dblsubset_dollar <- function() {
  l <- list(a=list(a=list(a=1), b=2, c=3), b=4, c=list(a=5))
  all.equal(l$a, list(a=list(a=1), b=2, c=3)) &
  all.equal(l$a$b, 2) &
  all.equal(l$a$a$a, 1) &
  all.equal(l$c$a, 5)
}
RUnit_nested_dlbassign <- function() {
  l <- list(list(list(1), 2, 3), 4, list(5))
  all.equal({l[[1]][[2]]      <- 12; l}, list(list(list(1), 12, 3), 4, list(5))) &
  all.equal({l[[1]][[1]][[1]] <- 11; l}, list(list(list(11), 12, 3), 4, list(5))) &
  all.equal({l[[3]][[1]]      <- 15; l}, list(list(list(11), 12, 3), 4, list(15))) &
  all.equal({l[[1]]           <-  1; l}, list(1, 4, list(15)))
}
RUnit_nested_dlbsubassign_names <- function() {
  l <- list(a=list(a=list(a=1), b=2, c=3), b=4, c=list(a=5))
  all.equal({l[["a"]][["b"]]        <- 12; l},
            list(a=list(a=list(a=1), b=12, c=3), b=4, c=list(a=5))) &
  all.equal({l[["a"]][["a"]][["a"]] <- 11; l},
            list(a=list(a=list(a=11), b=12, c=3), b=4, c=list(a=5))) &
  all.equal({l[["c"]][["a"]]        <- 15; l},
            list(a=list(a=list(a=11), b=12, c=3), b=4, c=list(a=15))) &
  all.equal({l[["a"]]               <-  1; l}, list(a=1, b=4, c=list(a=15)))
}
RUnit_nested_dlbsubassign_dollar <- function() {
  l <- list(a=list(a=list(a=1), b=2, c=3), b=4, c=list(a=5))
  all.equal({l$a$b   <- 12; l}, list(a=list(a=list(a=1), b=12, c=3), b=4, c=list(a=5))) &
  all.equal({l$a$a$a <- 11; l}, list(a=list(a=list(a=11), b=12, c=3), b=4, c=list(a=5))) &
  all.equal({l$c$a   <- 15; l}, list(a=list(a=list(a=11), b=12, c=3), b=4, c=list(a=15))) &
  all.equal({l$a     <-  1; l}, list(a=1, b=4, c=list(a=15)))
}
RUnit_nested_named_append_inplace <- function() {
  l <- list(a=list(a=list(a=1), b=2, c=3), b=4, c=list(a=5))
  l[[1]][[1]][["b"]] <- 2
  all.equal(l, list(a=list(a=list(a=1, b=2), b=2, c=3), b=4, c=list(a=5)))
}

## list delete LLL

## subset and subassign beyond range
RUnit_list_subset_out_of_range <- function() {
  l <- list(1,2,3)
  tryCatch(l[4], .Last.error == "subscript out of bounds")
}
RUnit_named_list_subset_out_of_range <- function() {
  l <- list(a=1,b=2,c=3)
  tryCatch(l["d"], .Last.error == "subscript out of bounds")
}
RUnit_list_logical_subset_out_of_range <- function() {
  l <- list(a=1,b=2,c=3)
  tryCatch(l[c(T,T,F,F)], .Last.error == "boolean index not equal to array extent")
}
RUnit_list_subassign_out_of_range <- function() {
  l <- list(1,2,3)
  tryCatch(l[4] <- list(2), .Last.error == "subscript out of bounds")
}
RUnit_list_logical_subset_out_of_range <- function() {
  l <- list(a=1,b=2,c=3)
  tryCatch(l[c(T,T,F,F)] <- 5, .Last.error == "boolean index not equal to array extent")
}
RUnit_list_dblsubset_out_of_range <- function() {
  l <- list(1,2,3)
  tryCatch(l[[4]], .Last.error == "subscript out of bounds")
}
RUnit_list_logical_dblsubset_out_of_range <- function() {
  l <- list(a=1,b=2,c=3)
  tryCatch(l[[c(T,T,F,F)]], .Last.error == "boolean index not equal to array extent")
}
RUnit_list_dblsubassign_out_of_range <- function() {
  l <- list(1,2,3)
  tryCatch(l[[4]] <- 2, .Last.error == "recursive indexing failed at level 1")
}
RUnit_list_logical_dblassign_out_of_range <- function() {
  l <- list(a=1,b=2,c=3)
  tryCatch(l[[c(T,T,F,F)]] <- 5, .Last.error == "boolean index not equal to array extent")
}



## test no copying with locked objects LLL
