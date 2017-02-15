## Copyright (C) 2016 Leonardo Silvestri
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


## here we test the environment related functions: 'ls', 'rm', 'get', 'assign'

## rm
RUnit_rm_current_default <- function() {
    a <- 1
    rm(a)
    tryCatch(a, .Last.error == "object 'a' not found") 
}
RUnit_rm_current_explicit <- function() {
    a <- 1
    rm(a, envir="current")
    tryCatch(a, .Last.error == "object 'a' not found") 
}
RUnit_rm_base_not_inherit <- function() {
    tryCatch(rm(sin), .Last.error == "object 'sin' not found") 
}
RUnit_rm_base <- function() {
    rm(cos, envir="base")
    tryCatch(cos, .Last.error == "object 'cos' not found") 
}
RUnit_rm_base_inherits <- function() {
    rm(tan, inherits = TRUE)
    tryCatch(tan, .Last.error == "object 'tan' not found") 
}
## ls
RUnit_ls_current <- function() {
    a <- 1; b <- 2; c <- 3;
    length(ls()) == 3
}
RUnit_ls_global <- function() {
    n <- length(ls("global"))
    global_a <<- 1; global_b <<- 2; global_c <<- 3;
    n3 <- length(ls("global"))
    n3 == n + 3
}
RUnit_ls_name <- function() {
    any(ls("base") == "sin")
}
RUnit_ls_m1 <- function() {
    a <- 1; b <- 2; c <- 3;
    length(ls(-1)) == 3
}
RUnit_ls_0 <- function() {
    tryCatch(ls(0), .Last.error == "environment position out of bounds")
}
RUnit_ls_1 <- function() {
    a <- 1; b <- 2; c <- 3;
    length(ls(1)) == 3
}
RUnit_ls_2 <- function() {
    a <- 1; b <- 2; c <- 3;
    f <- function() length(ls(2))
    f() == 4                            # 4 with f()
}
## get
RUnit_get_current <- function() {
    a <- 2200
    get("a") == a
}
RUnit_get_base <- function() {
    get("sin", "base")(3) == sin(3)
}
RUnit_get_global <- function() {
    a_get_global <<- 2300
    get("a_get_global", "global") == 2300
}
## assign
RUnit_assign_current <- function() {
    assign("a", 2200)
    a == 2200
}
RUnit_assing_base <- function() {
    assign("sin2", sin, "base")
    get("sin2", "base")(3) == sin(3)
}
RUnit_assign_global <- function() {
    assign("a_get_global", 2300, "global")
    get("a_get_global", "global") == 2300
}

## test that the same variable name can reference both an object and a
## function at the same time
RUnit_access_builtin <- function() {
    t <- 2
    t(3) == 3 & t == 2
}
RUnit_access_function <- function() {
    f <- function(x) x
    g <- function() { f <- 2; f(3) }
    g() == 3
}
