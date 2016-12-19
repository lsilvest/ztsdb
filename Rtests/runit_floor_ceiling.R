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



RUnit_floor_double <- function() {
    all.equal(floor(5.5), 5)
}
RUnit_floor_double_neg <- function() {
    all.equal(floor(-5.5), -6)
}
RUnit_floor_double_multi <- function() {
    all.equal(floor(matrix(c(-1.1,-1.9,-2, 0, 5.2, 5.9), 3, 2)),
              matrix      (c(-2,  -2,  -2, 0, 5,   5),   3, 2))
}
