## Copyright (C) 2015-2017 Leonardo Silvestri
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


RUnit_control_if <- function() {
    (if (T) T) &
    (if (F) F else T) &
    (if (1) T else F) &
     is.null(if (0) F) # when the branch is not present, it is NULL by default
}
RUnit_control_if_error <- function() {
    tryCatch(if (1:2) T, "error") == "error" &
    tryCatch(if ("a") T, "error") == "error" &
    tryCatch(if (as.duration(1)) T, "error") == "error" &
    tryCatch(if (as.period("1 day")) T, "error") == "error" &
    tryCatch(if (|.2012-12-12 12:12:12 UTC.|) T, "error") == "error" &
    tryCatch(if (|+2012-12-12 12:12:12 UTC -> 2012-12-12 12:12:13 UTC-|) T, "error") == "error"
}
RUnit_while_false <- function() {
    is.null(while (F) T) &
    is.null(while (0) F) &
    is.null(while (F) T)
}
RUnit_while_true <- function() {
    { count <- 1; while (count < 10) count <- count + 1; T } &
    { count <- 10; while (count) count <- count - 1; T } &
    { count <- 10; while (as.logical(count)) count <- count - 1; T } &
    { count <- 10; while ({count}) count <- count - 1; T }
}
RUnit_for_vector_double <- function() {
    count <- 0
    is.null(for (i in 1:10) count <- count + 1) &
    count == 10
}
RUnit_for_vector_bool <- function() {
    count <- 0
    is.null(for (i in c(T,F,T,F,F,T)) if (i) count <- count + 1) &
    count == 3
}
RUnit_for_vector_char <- function() {
    count <- 0
    is.null(for (i in as.character(c(T,F,T,F,F,T))) if (i == "TRUE") count <- count + 1) &
    count == 3
}
RUnit_for_matrix_double <- function() {
    sum <- 0
    is.null(for (i in matrix(1:10, 2, 5)) sum <- sum + i) &
    sum == 1+2+3+4+5+6+7+8+9+10
}
