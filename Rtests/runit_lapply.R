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


RUnit_lapply_array <- function() {
  all.equal(lapply(1:5, function(x) x*2), list(2,4,6,8,10))
}

RUnit_lapply_array_builtin <- function() {
  all.equal(lapply(1:5, sin), list(sin(1),sin(2),sin(3),sin(4),sin(5)))
}

RUnit_lapply_list <- function() {
  l <- list(1,2,3,4,5)
  all.equal(lapply(l, function(x) x*2), list(2,4,6,8,10))
}

RUnit_lapply_list_builtin <- function() {
  l <- list(1,2,3,4,5)
  all.equal(lapply(l, sin), list(sin(1),sin(2),sin(3),sin(4),sin(5)))
}

RUnit_lapply_incorrect_type <- function() {
  tryCatch(lapply(function(x) x, sin), .Last.error=="function cannot be double subsetted")
}
