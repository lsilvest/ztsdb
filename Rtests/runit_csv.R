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


## create a unique temporary directory in which all tests will write:
dir <- system("mktemp -d", intern=T)

## double
RUnit_double_to_and_fro_header <- function() {
    a <- matrix((1:3000)/3, 1000, 3, dimnames=list(NULL, c("un","deux","trois")))
    csvfile <- paste0(dir, "/RUnit_double_to_and_fro_header")
    write.csv(a, csvfile, TRUE)
    ar <- read.csv(csvfile, type="double", header=TRUE)
    all.equal(a, ar)
}
RUnit_double_to_and_fro_header_sep_space <- function() {
    a <- matrix((1:3000)/3, 1000, 3, dimnames=list(NULL, c("un","deux","trois")))
    csvfile <- paste0(dir, "/RUnit_double_to_and_fro_header_sep_space")
    write.csv(a, csvfile, TRUE, sep=" ")
    ar <- read.csv(csvfile, type="double", header=TRUE, sep=" ")
    all.equal(a, ar)
}
RUnit_double_to_and_fro_no_header <- function() {
    a <- matrix((1:3000)/3, 1000, 3)
    csvfile <- paste0(dir, "/RUnit_double_to_and_fro_no_header")
    write.csv(a, csvfile, header=FALSE)
    ar <- read.csv(csvfile, type="double", header=FALSE)
    all.equal(a, ar)
}
RUnit_double_vector_to_and_fro_header <- function() {
    a <- c(x=(1:3000)/3)
    csvfile <- paste0(dir, "/RUnit_double_vector_to_and_fro_header")
    write.csv(a, csvfile, TRUE)
    ar <- read.csv(csvfile, type="double", header=TRUE)
    all.equal(matrix(a, length(a), 1, dimnames=list(NULL, "")), ar)
}
RUnit_double_vector_to_and_fro_no_header <- function() {
    a <- c(1:3000)/3
    csvfile <- paste0(dir, "/RUnit_double_vector_to_and_fro_no_header")
    write.csv(a, csvfile, FALSE)
    ar <- read.csv(csvfile, type="double",  header=FALSE)
    all.equal(matrix(a, length(a), 1), ar)
}
RUnit_double_quoted <- function() {    
    csvfile <- paste0(dir, "/RUnit_double_quoted")
    system(paste0("echo '",
                  "unu,\"doi\",trei\n",
                  "1,4,7\n",
                  "2,\"5\",\"8\"\n",
                  "3,\"6\",9\n",
                  "' >", csvfile))
    ar <- read.csv(csvfile, type="double",  header=TRUE)
    all.equal(matrix(1:9, 3, 3, dimnames=list(NULL, c("unu","doi","trei"))), ar)
}

## time
RUnit_time_to_and_fro_header <- function() {
    a <- matrix(seq(|.2012-12-12 00:00:00 America/New_York.|, length.out=3000),
                1000, 3, dimnames=list(NULL, c("un","deux","trois")))
    csvfile <- paste0(dir, "/RUnit_time_to_and_fro_header")
    write.csv(a, csvfile, TRUE)
    ar <- read.csv(csvfile, type="time", header=TRUE)
    all.equal(a, ar)
}
RUnit_time_to_and_fro_no_header <- function() {
    a <- matrix(seq(|.2012-12-12 00:00:00 America/New_York.|, length.out=3000), 1000, 3)
    csvfile <- paste0(dir, "/RUnit_time_to_and_fro_no_header")
    write.csv(a, csvfile, FALSE)
    ar <- read.csv(csvfile, type="time", header=FALSE)
    all.equal(a, ar)
}

## logical
RUnit_logical_to_and_fro_header <- function() {
    a <- as.logical(matrix((1:3000) %% 3, 1000, 3, dimnames=list(NULL, c("un","deux","trois"))))
    csvfile <- paste0(dir, "/RUnit_logical_to_and_fro_header")
    write.csv(a, csvfile, TRUE)
    ar <- read.csv(csvfile, type="logical", header=TRUE)
    all.equal(a, ar)
}
RUnit_logical_to_and_fro_no_header <- function() {
    a <- as.logical(matrix((1:3000) %% 3, 1000, 3))
    csvfile <- paste0(dir, "/RUnit_logical_to_and_fro_no_header")
    write.csv(a, csvfile, FALSE)
    ar <- read.csv(csvfile, type="logical", header=FALSE)
    all.equal(a, ar)
}

## character
RUnit_character_to_and_fro_header <- function() {
    a <- as.character(matrix((1:3000)/3, 1000, 3, dimnames=list(NULL, c("un","deux","trois"))))
    csvfile <- paste0(dir, "/RUnit_character_to_and_fro_header")
    write.csv(a, csvfile, TRUE)
    ar <- read.csv(csvfile, type="character", header=TRUE)
    all.equal(a, ar)
}
RUnit_character_to_and_fro_no_header <- function() {
    a <- as.character(matrix((1:3000)/3, 1000, 3))
    csvfile <- paste0(dir, "/RUnit_character_to_and_fro_no_header")
    write.csv(a, csvfile, FALSE)
    ar <- read.csv(csvfile, type="character", header=FALSE)
    all.equal(a, ar)
}

## errors
RUnit_double_incorrect_header_columns <- function() {    
    csvfile <- paste0(dir, "/RUnit_double_incorrect_columns")
    system(paste0("echo '",
                 "unu,trei\n",          # the function will think there are two columns
                 "1,2,3\n",
                 "4,5,6\n",
                 "7,8,9\n",
                 "' >", csvfile))
    tryCatch(read.csv(csvfile, type="double", header=TRUE),
             .Last.error == "incorrect number of elements in row 2")
}
RUnit_double_incorrect_columns <- function() {    
    csvfile <- paste0(dir, "/RUnit_double_incorrect_columns")
    system(paste0("echo '",
                 "unu,doi,trei\n",
                 "1,2,3\n",
                 "4,6\n",               # missing column!
                 "7,8,9\n",
                 "' >", csvfile))
    tryCatch(read.csv(csvfile, type="double", header=TRUE),
             .Last.error == "incorrect number of elements in row 3")
}
RUnit_double_malformed_elt <- function() {    
    csvfile <- paste0(dir, "/RUnit_double_malformed_elt")
    system(paste0("echo '",
                 "unu,doi,trei\n",
                 "1,2,3\n",
                 "4,5,6af\n",           # malformed elt!
                 "7,8,9\n",
                 "' >", csvfile))
    tryCatch(read.csv(csvfile, type="double", header=TRUE),
             .Last.error == "can't parse '6af' on row 3, col 3")
}
RUnit_logical_malformed_elt <- function() {    
    csvfile <- paste0(dir, "/RUnit_double_malformed_elt")
    system(paste0("echo '",
                 "unu,doi,trei\n",
                 "0,0,1\n",
                 "1,0,0af\n",           # malformed elt!
                 "1,1,0\n",
                 "' >", csvfile))
    tryCatch(read.csv(csvfile, type="logical", header=TRUE),
             .Last.error == "can't parse '0af' on row 3, col 3")
}
