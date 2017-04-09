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


d1 <- |.2016-08-06 06:38:01 America/New_York.|

idx1 <- seq(d1, by=as.duration(3600*1e9), length.out=9)
z1 <- zts(idx1, matrix(1:27, 9, 3, dimnames=list(NULL, c("one", "two", "three"))))

idx2 <- seq(tail(idx1,1)+as.duration(3600*1e9), by=as.duration(3600*1e9), length.out=9)
z2 <- zts(idx2, matrix(1:27, 9, 3, dimnames=list(NULL, c("one", "two", "three"))))
