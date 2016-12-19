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


## equality between different types must return FALSE
##
## array types are:
## double
## bool
## string
## time
## interval
## duration
## period
## zts
## list (Value)

source("zts.R")

#### ---------------------------------------------
## EQ
## double
RUnit_eq_double_double <- function() {
    1 == 1 &
    all(c(1:3) == c(1:3))
}
RUnit_eq_double_double <- function() {
    1 == 1 &
    all(c(1:3) == c(1:3))
}
RUnit_eq_double_bool <- function() {
    # R says TRUE, but we don't do type conversion
    !(1 == TRUE) &
    !(1 == FALSE)
}
RUnit_eq_double_string <- function() {
    # R says TRUE, but we don't do type conversion
    !(1 == "1") &
    !("1" == 1)
}
RUnit_eq_double_time <- function() {
    !(1 == |.2012-12-12 12:12:12 UTC.|) &
    !(|.2012-12-12 12:12:12 UTC.| == 1)
}
RUnit_eq_double_interval <- function() {
    i <- |-2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|
    !(1 == i) &
    !(i == 1)
}
RUnit_eq_double_duration <- function() {
    x <- as.duration(1)
    !(1 == x) &
    !(x == 1)
}
RUnit_eq_double_period <- function() {
    x <- as.period("1d")
    !(1 == x) &
    !(x == 1)
}
RUnit_eq_double_zts <- function() {
    all(zts.data(z1) == z1) &
    all(z1 == zts.data(z1))
}
RUnit_eq_elt_double_list <- function() {
    list(1) == 1 &
    1 == list(1)
}
RUnit_eq_elt_double_list_multiple <- function() {
    all((list(1,2,3) == 1) == c(TRUE, FALSE, FALSE))  &
    all((1 == list(1,2,3)) == c(TRUE, FALSE, FALSE))
}
## bool
RUnit_eq_bool_bool <- function() {
    TRUE  == TRUE &
    FALSE == FALSE
}
RUnit_eq_bool_string <- function() {
    !(TRUE == "1") &
    !("1" == TRUE)
}
RUnit_eq_bool_time <- function() {
    !(TRUE == |.2012-12-12 12:12:12 UTC.|) &
    !(|.2012-12-12 12:12:12 UTC.| == FALSE)
}
RUnit_eq_bool_interval <- function() {
    i <- |-2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|
    !(TRUE == i) &
    !(i == FALSE)
}
RUnit_eq_bool_duration <- function() {
    x <- as.duration(1)
    !(TRUE == x) &
    !(x == FALSE)
}
RUnit_eq_bool_period <- function() {
    x <- as.period("1d")
    !(TRUE == x) &
    !(x == FALSE) 
}
RUnit_eq_bool_zts <- function() {
    !all(FALSE == z1) &
    !all(z1 == TRUE)
}
RUnit_eq_elt_bool_list <- function() {
    list(TRUE) == TRUE &
    TRUE == list(TRUE)
}
RUnit_eq_elt_bool_list_multiple <- function() {
    all((list(T, F, F) == T) == c(TRUE, FALSE, FALSE))  &
    all((F == list(F,T,T)) == c(TRUE, FALSE, FALSE))
}
## string
RUnit_eq_string_string <- function() {
    "1" == "1" &
    !("1" == "12") &
    !("a" == "")
}
RUnit_eq_string_time <- function() {
    !("a" == |.2012-12-12 12:12:12 UTC.|) &
    !(|.2012-12-12 12:12:12 UTC.| == "b")
}
RUnit_eq_string_interval <- function() {
    i <- |-2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|
    !("a" == i) &
    !(i == "b")
}
RUnit_eq_string_duration <- function() {
    x <- as.duration(1)
    !("a" == x) &
    !(x == "b")
}
RUnit_eq_string_period <- function() {
    x <- as.period("1d")
    !("a" == x) &
    !(x == "b") 
}
RUnit_eq_string_zts <- function() {
    !all("b" == z1) &
    !all(z1 == "a")
}
RUnit_eq_elt_string_list <- function() {
    list("a") == "a" &
    "a" == list("a")
}
RUnit_eq_elt_string_list_multiple <- function() {
    all((list("a", "b", "b") == "a") == c(T, F, F))  &
    all(("a" == list("b", "a", "a")) == c(F, T, T))
}
## time
RUnit_eq_time_time <- function() {
    a <- |.2012-12-12 12:12:12 UTC.|
    a == |.2012-12-12 12:12:12 UTC.| &
    |.2012-12-12 12:12:12 UTC.| == a
}
RUnit_eq_time_interval <- function() {
    a <- |.2012-12-12 12:12:12 UTC.|
    i <- |-2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|
    !(a == i) &
    !(i == a)
}
RUnit_eq_time_duration <- function() {
    x <- as.duration(1)
    a <- |.2012-12-12 12:12:12 UTC.|
    !(a == x) &
    !(x == a)
}
RUnit_eq_time_period <- function() {
    x <- as.period("1d")
    a <- |.2012-12-12 12:12:12 UTC.|
    !(a == x) &
    !(x == a) 
}
RUnit_eq_time_zts <- function() {
    a <- |.2012-12-12 12:12:12 UTC.|
    !(a == z1) &
    !(z1 == a)
}
RUnit_eq_elt_time_list <- function() {
    a <- |.2012-12-12 12:12:12 UTC.|
    list(a) == a &
    a == list(a)
}
RUnit_eq_elt_time_list_multiple <- function() {
    a <- |.2012-12-12 12:12:12 UTC.|
    b <- |.2016-12-12 12:12:12 UTC.|
    all((list(a,b,b) == a) == c(T, F, F))  &
    all((b == list(b,a,a)) == c(T, F, F))
}
## interval
RUnit_eq_interval_interval <- function() {
    a <- |-2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|
    i <- |-2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|
    a == i & i == a
}
RUnit_eq_interval_duration <- function() {
    x <- as.duration(1)
    a <- |-2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|
    !(a == x) &
    !(x == a)
}
RUnit_eq_interval_period <- function() {
    x <- as.period("1d")
    a <- |-2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|
    !(a == x) &
    !(x == a) 
}
RUnit_eq_interval_zts <- function() {
    a <- |-2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|
    !(a == z1) &
    !(z1 == a)
}
RUnit_eq_elt_interval_list <- function() {
    a <- |-2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|
    list(a) == a &
    a == list(a)
}
RUnit_eq_elt_interval_list_multiple <- function() {
    a <- |-2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|
    b <- |-2012-12-12 12:12:12 UTC -> 2017-12-12 12:12:12 UTC+|
    all((list(a,b,b) == a) == c(T, F, F))  &
    all((b == list(b,a,a)) == c(T, F, F))
}
## duration
RUnit_eq_duration_duration <- function() {
    x <- as.duration(1)
    a <- as.duration(1)
    a == x & x == a
}
RUnit_eq_duration_period <- function() {
    x <- as.period("1d")
    a <- as.duration(1)
    !(a == x) &
    !(x == a) 
}
RUnit_eq_duration_zts <- function() {
    a <- as.duration(1)
    !(a == z1) &
    !(z1 == a)
}
RUnit_eq_elt_duration_list <- function() {
    a <- as.duration(1)
    list(a) == a &
    a == list(a)
}
RUnit_eq_elt_duration_list_multiple <- function() {
    a <- as.duration(1)
    b <- as.duration(2)
    all((list(a,b,b) == a) == c(T, F, F))  &
    all((b == list(b,a,a)) == c(T, F, F))
}
## period
RUnit_eq_period_period <- function() {
    x <- as.period("1w")
    a <- as.period("7d")
    a == x & x == a
}
RUnit_eq_period_zts <- function() {
    a <- as.period("1d")
    !(a == z1) &
    !(z1 == a)
}
RUnit_eq_elt_period_list <- function() {
    a <- as.period("1d")
    list(a) == a &
    a == list(a)
}
RUnit_eq_elt_period_list_multiple <- function() {
    a <- as.period("1d")
    b <- as.period("2d")
    all((list(a,b,b) == a) == c(T, F, F))  &
    all((b == list(b,a,a)) == c(T, F, F))
}
## zts
RUnit_eq_zts_zts <- function() {
    all(z1 == z1)
}
RUnit_eq_elt_zts_list <- function() {
    list(z1) == z1 &
    z1 == list(z1)
}
RUnit_eq_elt_zts_list_multiple <- function() {
    all((list(z1,z2,z2) == z1) == c(T, F, F))  &
    all((z2 == list(z2,z1,z1)) == c(T, F, F))
}

#### ---------------------------------------------
## NE
RUnit_ne_double_double <- function() {
    1 != 2 &
    all(c(1:3) != c(2:4))
}
RUnit_ne_double_bool <- function() {
    # R says TRUE, but we don't do type conversion
    (1 != TRUE) &
    (1 != FALSE)
}
RUnit_ne_double_string <- function() {
    # R says TRUE, but we don't do type conversion
    (1 != "1") &
    ("1" != 1)
}
RUnit_ne_double_time <- function() {
    (1 != |.2012-12-12 12:12:12 UTC.|) &
    (|.2012-12-12 12:12:12 UTC.| != 1)
}
RUnit_ne_double_interval <- function() {
    i <- |-2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|
    (1 != i) &
    (i != 1)
}
RUnit_ne_double_duration <- function() {
    x <- as.duration(1)
    (1 != x) &
    (x != 1)
}
RUnit_ne_double_period <- function() {
    x <- as.period("1d")
    (1 != x) &
    (x != 1)
}
RUnit_ne_double_zts <- function() {
    all(zts.data(z1) != z2+1) &
    all(z1+1 != zts.data(z2))
}
RUnit_ne_elt_double_list <- function() {
    list(1) != 2 &
    2 != list(1)
}
RUnit_ne_elt_double_list_multiple <- function() {
    all((list(1,2,3) != 1) == c(F,T,T))  &
    all((1 != list(1,2,3)) == c(F,T,T))
}
## bool
RUnit_ne_bool_bool <- function() {
    TRUE  != FALSE &
    FALSE != TRUE
}
RUnit_ne_bool_string <- function() {
    (TRUE != "1") &
    ("1" != TRUE)
}
RUnit_ne_bool_time <- function() {
    (TRUE != |.2012-12-12 12:12:12 UTC.|) &
    (|.2012-12-12 12:12:12 UTC.| != FALSE)
}
RUnit_ne_bool_interval <- function() {
    i <- |-2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|
    (TRUE != i) &
    (i != FALSE)
}
RUnit_ne_bool_duration <- function() {
    x <- as.duration(1)
    (TRUE != x) &
    (x != FALSE)
}
RUnit_ne_bool_period <- function() {
    x <- as.period("1d")
    (TRUE != x) &
    (x != FALSE) 
}
RUnit_ne_bool_zts <- function() {
    all(FALSE != z1) &
    all(z1 != TRUE)
}
RUnit_ne_elt_bool_list <- function() {
    list(TRUE) != FALSE &
    FALSE != list(TRUE)
}
RUnit_ne_elt_bool_list_multiple <- function() {
    all((list(T, F, F) != T) == c(F, T, T))  &
    all((F != list(F,T,T)) == c(F, T, T))
}
## string
RUnit_ne_string_string <- function() {
    "1" != "3" &
    "1" != "12" &
    "a" != ""
}
RUnit_ne_string_time <- function() {
    ("a" != |.2012-12-12 12:12:12 UTC.|) &
    (|.2012-12-12 12:12:12 UTC.| != "b")
}
RUnit_ne_string_interval <- function() {
    i <- |-2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|
    ("a" != i) &
    (i != "b")
}
RUnit_ne_string_duration <- function() {
    x <- as.duration(1)
    ("a" != x) &
    (x != "b")
}
RUnit_ne_string_period <- function() {
    x <- as.period("1d")
    ("a" != x) &
    (x != "b") 
}
RUnit_ne_string_zts <- function() {
    all("b" != z1) &
    all(z1 != "a")
}
RUnit_ne_elt_string_list <- function() {
    list("a") != "b" &
    "a" != list("b")
}
RUnit_ne_elt_string_list_multiple <- function() {
    all((list("a", "b", "b") != "a") == c(F, T, T))  &
    all(("b" != list("b", "a", "a")) == c(F, T, T))
}
## time
RUnit_ne_time_time <- function() {
    a <- |.2012-12-12 12:12:12 UTC.|
    a != |.2016-12-12 12:12:12 UTC.| &
    |.2016-12-12 12:12:12 UTC.| != a
}
RUnit_ne_time_interval <- function() {
    a <- |.2012-12-12 12:12:12 UTC.|
    i <- |-2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|
    (a != i) &
    (i != a)
}
RUnit_ne_time_duration <- function() {
    x <- as.duration(1)
    a <- |.2012-12-12 12:12:12 UTC.|
    (a != x) &
    (x != a)
}
RUnit_ne_time_period <- function() {
    x <- as.period("1d")
    a <- |.2012-12-12 12:12:12 UTC.|
    (a != x) &
    (x != a) 
}
RUnit_ne_time_zts <- function() {
    a <- |.2012-12-12 12:12:12 UTC.|
    (a != z1) &
    (z1 != a)
}
RUnit_ne_elt_time_list <- function() {
    a <- |.2012-12-12 12:12:12 UTC.|
    b <- |.1999-12-12 12:12:12 UTC.|
    list(a) != b &
    b != list(a)
}
RUnit_ne_elt_time_list_multiple <- function() {
    a <- |.2012-12-12 12:12:12 UTC.|
    b <- |.2016-12-12 12:12:12 UTC.|
    all((list(a,b,b) != a) == c(F, T, T))  &
    all((b != list(b,a,a)) == c(F, T, T))
}
## interval
RUnit_ne_interval_interval <- function() {
    a <- |-2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|
    i <- |-2011-12-12 12:12:12 UTC -> 2015-12-12 12:12:12 UTC+|
    a != i & i != a
}
RUnit_ne_interval_duration <- function() {
    x <- as.duration(1)
    a <- |-2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|
    (a != x) &
    (x != a)
}
RUnit_ne_interval_period <- function() {
    x <- as.period("1d")
    a <- |-2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|
    (a != x) &
    (x != a) 
}
RUnit_ne_interval_zts <- function() {
    a <- |-2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|
    (a != z1) &
    (z1 != a)
}
RUnit_ne_elt_interval_list <- function() {
    a <- |-2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|
    b <- |+2010-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|
    list(a) != b &
    a != list(b)
}
RUnit_ne_elt_interval_list_multiple <- function() {
    a <- |-2012-12-12 12:12:12 UTC -> 2016-12-12 12:12:12 UTC+|
    b <- |-2012-12-12 12:12:12 UTC -> 2017-12-12 12:12:12 UTC+|
    all((list(a,b,b) != a) == c(F, T, T))  &
    all((b != list(b,a,a)) == c(F, T, T))
}
## duration
RUnit_ne_duration_duration <- function() {
    x <- as.duration(1)
    a <- as.duration(2)
    a != x & x != a
}
RUnit_ne_duration_period <- function() {
    x <- as.period("1d")
    a <- as.duration(1)
    (a != x) &
    (x != a) 
}
RUnit_ne_duration_zts <- function() {
    a <- as.duration(1)
    (a != z1) &
    (z1 != a)
}
RUnit_ne_elt_duration_list <- function() {
    a <- as.duration(1)
    b <- as.duration(2)
    list(a) != b &
    a != list(b)
}
RUnit_ne_elt_duration_list_multiple <- function() {
    a <- as.duration(1)
    b <- as.duration(2)
    all((list(a,b,b) != a) == c(F, T, T))  &
    all((b != list(b,a,a)) == c(F, T, T))
}
## period
RUnit_ne_period_period <- function() {
    x <- as.period("1w")
    a <- as.period("1m")
    a != x & x != a
}
RUnit_ne_period_zts <- function() {
    a <- as.period("1d")
    (a != z1) &
    (z1 != a)
}
RUnit_ne_elt_period_list <- function() {
    a <- as.period("1d")
    b <- as.period("2d")
    list(a) != b &
    a != list(b)
}
RUnit_ne_elt_period_list_multiple <- function() {
    a <- as.period("1d")
    b <- as.period("2d")
    all((list(a,b,b) != a) == c(F, T, T))  &
    all((b != list(b,a,a)) == c(F, T, T))
}
## zts
RUnit_ne_zts_zts <- function() {
    all(z1 != z1+1)
}
RUnit_ne_elt_zts_list <- function() {
    list(z1) != z2 &
    z1 != list(z2)
}
RUnit_ne_elt_zts_list_multiple <- function() {
    all((list(z1,z2,z2) != z1) == c(F, T, T))  &
    all((z2 != list(z2,z1,z1)) == c(F, T, T))
}


## in R:
## > list(1,2,3) == list(1,2,3)
## Error in list(1, 2, 3) == list(1, 2, 3) : 
##   comparison of these types is not implemented
