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


RUnit_system_intern <- function() {
  output <- system("echo 123", intern=TRUE)
  output == "123"
}
RUnit_system_not_intern <- function() {
  exit_status <- system("echo 123", ignore.stdout=TRUE, ignore.stderr=TRUE)
  exit_status == 0
}
RUnit_system_not_intern_exit_non_zero <- function() {
  command <- "asfdasdsadas-293373739810192834610394871"
  exit_status <- system(command, ignore.stdout=TRUE, ignore.stderr=TRUE)
  exit_status == 127
}
RUnit_system_not_intern_exit_non_zero_2 <- function() {
  exit_status <- system("exit 101", ignore.stdout=TRUE, ignore.stderr=TRUE)
  exit_status == 101
}
