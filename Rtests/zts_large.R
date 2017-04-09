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

get_large_zts <- function(n=527041, mmapped=FALSE) {
  if (mmapped) {
    dir <- system("mktemp -d", intern=T)
    system(paste("rmdir", dir))         # remove it as it will be recreated by 'matrix'
  }
  else {
    dir <- ""
  }
  
  if (n==0) {
    zts(as.time(NULL), matrix(0,0,3, dimnames=list(NULL, c("one", "two", "three"))))
  }
  else {
    start <- |.2015-08-06 06:38:01 America/New_York.|
    idx <- seq(from=start, length.out=n, by=as.duration("00:01:00"))
    data <- matrix(1:(length(idx)*3), length(idx), 3, dimnames=list(NULL, c("one", "two", "three")))
    zts(idx, data, file=dir)
  }
}
