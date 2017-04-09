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


## scalar-scalar --------------------
## double binary ops
RUnit_double_add <- function() {
    1 +  2 ==  3   &
    1 + -2 == -1
}
RUnit_double_sub <- function() {
    3 - 2 ==  1   &
    1 - 2 == -1
}
RUnit_double_mul <- function() {
    3 * 2  ==  6   &
    2 * -3 == -6 
}
RUnit_double_div <- function() {
    3 /  2  == 1.5  &
    3 / -1 == -3
}
RUnit_double_div_byzero <- function() {
    3 /  0 ==  Inf   &
   -3 /  0 == -Inf   &
    3 / -0 == -Inf
}
RUnit_double_NaN <- function() {
    is.nan(NaN)        &
    is.nan(NaN * 0)    &
    is.nan(-3 /  NaN)  &
    is.nan(NaN - NaN)
}
RUnit_double_pow <- function() {
    3 ^  2   == 9      &
    4 ^  1.5 == 8      &
    4 ^ -1.5 == 0.125    
}
RUnit_double_pow_NaN <- function() {
    is.nan(NaN ^ NaN)  &
    is.nan(1   ^ NaN)
}
RUnit_double_le <- function() {
    (1 <= 2)               &
    (2 <= 1) == FALSE      &
    (2 <= 2)               &
    (-1 <= -0.5)           &
    (1 <= Inf)             &
    (1 <= -Inf)  == FALSE  &
    (-Inf <= Inf)          &
    (Inf <= Inf)           &
    (1 <= NaN)   == FALSE  &
    (NaN <= NaN) == FALSE  &
    (NaN <= 1)   == FALSE
}
RUnit_double_lt <- function() {
    (1 < 2)               &
    (2 < 1) == FALSE      &
    (2 < 2) == FALSE      &
    (-1 < -0.5)           &
    (1 < Inf)             &
    (1 < -Inf)  == FALSE  &
    (-Inf < Inf)          &
    (Inf < Inf) == FALSE  &
    (1 < NaN)   == FALSE  &
    (NaN < NaN) == FALSE  &
    (NaN < 1)   == FALSE
} 
RUnit_double_eq <- function() {
    (1 == 2) == FALSE       &
    (2 == 2)                &
    (1 == Inf)   == FALSE   &
    (1 == -Inf)  == FALSE   &
    (-Inf == -Inf)          &
    (-Inf == Inf) == FALSE  &
    (Inf == Inf)            &
    (1 == NaN)   == FALSE   &
    (NaN == NaN) == FALSE   &
    (NaN == 1)   == FALSE
} 
RUnit_double_ne <- function() {
    (1 != 2)                 &
    (2 != 2) == FALSE        &
    (1 != Inf)               &
    (1 != -Inf)              &
    (-Inf != -Inf) == FALSE  &
    (-Inf != Inf)            &
    (Inf != Inf)   == FALSE  &
    (1 != NaN)               &
    (NaN != NaN)             &
    (NaN != 1)  
} 
RUnit_double_ge <- function() {
    (1 >= 2) == FALSE      &
    (2 >= 1)               &
    (2 >= 2)               &
    (-1 >= -0.5) == FALSE  &
    (1 >= Inf) == FALSE    &
    (1 >= -Inf)            &
    (-Inf >= Inf) == FALSE &
    (Inf >= Inf)           &
    (1 >= NaN)   == FALSE  &
    (NaN >= NaN) == FALSE  &
    (NaN >= 1)   == FALSE
}
RUnit_double_gt <- function() {
    (1 > 2) == FALSE      &
    (2 > 1)               &
    (2 > 2) == FALSE      &
    (-1 > -0.5) == FALSE  &
    (1 > Inf) == FALSE    &
    (1 > -Inf)            &
    (-Inf > Inf) == FALSE &
    (Inf > Inf)  == FALSE &
    (1 > NaN)   == FALSE  &
    (NaN > NaN) == FALSE  &
    (NaN > 1)   == FALSE
} 
RUnit_double_and <- function() {
    (0 & 0) == FALSE &
    (0 & 1) == FALSE &
    (1 & 0) == FALSE &
    (1 & 1) == TRUE
}
RUnit_double_and2 <- function() {
    (0 && 0) == FALSE &
    (0 && 1) == FALSE &
    (1 && 0) == FALSE &
    (1 && 1) == TRUE
}
RUnit_double_or <- function() {
    (0 | 0) == FALSE &
    (0 | 1) == TRUE  &
    (1 | 0) == TRUE  &
    (1 | 1) == TRUE
}
RUnit_double_or2 <- function() {
    (0 || 0) == FALSE &
    (0 || 1) == TRUE  &
    (1 || 0) == TRUE  &
    (1 || 1) == TRUE
}

## bool
RUnit_bool_le <- function() {
    (TRUE  <= FALSE) == FALSE &
    (FALSE <= TRUE)           &
    (FALSE <= FALSE)          &
    (TRUE  <= TRUE)
}
RUnit_bool_lt <- function() {
    (TRUE  < FALSE) == FALSE &
    (FALSE < TRUE)           &
    (FALSE < FALSE) == FALSE &
    (TRUE  < TRUE)  == FALSE
} 
RUnit_bool_eq <- function() {
    (TRUE  == FALSE) == FALSE &
    (FALSE == FALSE)          &
    (TRUE  == TRUE)
} 
RUnit_bool_ne <- function() {
    (TRUE  != FALSE)          &
    (FALSE != FALSE) == FALSE &
    (TRUE  != TRUE)  == FALSE
} 
RUnit_bool_ge <- function() {
    (TRUE  >= FALSE)         &
    (FALSE >= TRUE) == FALSE &
    (FALSE >= FALSE)         &
    (TRUE  >= TRUE)
}
RUnit_bool_gt <- function() {
    (TRUE  > FALSE)          &
    (FALSE > TRUE)  == FALSE &
    (FALSE > FALSE) == FALSE &
    (TRUE  > TRUE)  == FALSE
} 
RUnit_bool_and <- function() {
    (TRUE  & TRUE)  == TRUE  &
    (TRUE  & FALSE) == FALSE &
    (FALSE & TRUE)  == FALSE &
    (FALSE & FALSE) == FALSE
}
RUnit_bool_and2 <- function() {
    (TRUE  && TRUE)  == TRUE  &
    (TRUE  && FALSE) == FALSE &
    (FALSE && TRUE)  == FALSE &
    (FALSE && FALSE) == FALSE
}
RUnit_bool_or <- function() {
    (TRUE  | TRUE)  == TRUE  &
    (TRUE  | FALSE) == TRUE  &
    (FALSE | TRUE)  == TRUE  &
    (FALSE | FALSE) == FALSE
}
RUnit_bool_or2 <- function() {
    (TRUE  || TRUE)  == TRUE  &
    (TRUE  || FALSE) == TRUE  &
    (FALSE || TRUE)  == TRUE  &
    (FALSE || FALSE) == FALSE
}

## string
RUnit_string_le <- function() {
    ("abcd" <= "abc")  == FALSE &
    ("abc"  <= "abcd")          &
    ("abc"  <= "abc")           &
    ("abcd" <= "abcd")
}
RUnit_string_lt <- function() {
    ("abcd" < "abc")  == FALSE &
    ("abc"  < "abcd")          &
    ("abc"  < "abc")  == FALSE &
    ("abcd" < "abcd") == FALSE
} 
RUnit_string_eq <- function() {
    ("abcd" == "abc") == FALSE &
    ("abc"  == "abc")          &
    ("abcd" == "abcd")
} 
RUnit_string_ne <- function() {
    ("abcd" != "abc")            &
    ("abc"  != "abc")  == FALSE  &
    ("abcd" != "abcd") == FALSE
} 
RUnit_string_ge <- function() {
    ("abcd" >= "abc")           &
    ("abc"  >= "abcd") == FALSE &
    ("abc"  >= "abc")           &
    ("abcd" >= "abcd")
}
RUnit_string_gt <- function() {
    ("abcd" > "abc")           &
    ("abc"  > "abcd") == FALSE &
    ("abc"  > "abc")  == FALSE &
    ("abcd" > "abcd") == FALSE
} 
RUnit_string_undefined_ops <- function() {
    tryCatch("a" +  "b", "error") == "error" &
    tryCatch("a" -  "b", "error") == "error" &
    tryCatch("a" *  "b", "error") == "error" &
    tryCatch("a" /  "b", "error") == "error" &
    tryCatch("a" ^  "b", "error") == "error" &
    tryCatch("a" &  "b", "error") == "error" &
    tryCatch("a" && "b", "error") == "error" &
    tryCatch("a" |  "b", "error") == "error" &
    tryCatch("a" || "b", "error") == "error" &
    tryCatch("a" :  "b", "error") == "error"
}

## time
RUnit_time_minus <- function() {
    |.2015-03-09 06:38:02 America/New_York.| - |.2015-03-09 06:38:01 America/New_York.| ==
    as.duration(1e9)
}
RUnit_time_le <- function() {
    (|.2015-03-09 06:38:01 America/New_York.| <= |.2015-03-09 06:38:02 America/New_York.|)          &
    (|.2015-03-09 06:38:02 America/New_York.| <= |.2015-03-09 06:38:01 America/New_York.|) == FALSE &
    (|.2015-03-09 06:38:02 America/New_York.| <= |.2015-03-09 06:38:02 America/New_York.|)          &
    (|.1956-03-09 06:38:02 America/New_York.| <= |.2015-03-09 06:38:02 America/New_York.|)
}
RUnit_time_lt <- function() {
    (|.2015-03-09 06:38:01 America/New_York.| < |.2015-03-09 06:38:02 America/New_York.|)          &
    (|.2015-03-09 06:38:02 America/New_York.| < |.2015-03-09 06:38:01 America/New_York.|) == FALSE &
    (|.2015-03-09 06:38:02 America/New_York.| < |.2015-03-09 06:38:02 America/New_York.|) == FALSE &
    (|.1956-03-09 06:38:02 America/New_York.| < |.2015-03-09 06:38:02 America/New_York.|)
} 
RUnit_time_eq <- function() {
    (|.2015-03-09 06:38:01 America/New_York.| == |.2015-03-09 06:38:01 America/New_York.|) &
    (|.2015-03-09 06:38:01.11 America/New_York.| == |.2015-03-09 06:38:01 America/New_York.|) == FALSE
} 
RUnit_time_ne <- function() {
    (|.2015-03-09 06:38:01 America/New_York.| != |.2015-03-09 06:38:01 America/New_York.|) == FALSE &
    (|.2015-03-09 06:38:01.11 America/New_York.| != |.2015-03-09 06:38:01 America/New_York.|)
} 
RUnit_time_ge <- function() {
    (|.2015-03-09 06:38:02 America/New_York.| >= |.2015-03-09 06:38:01 America/New_York.|)    &
    (|.2015-03-09 06:38:01 America/New_York.| >= |.2015-03-09 06:38:02 America/New_York.|) == FALSE &
    (|.2015-03-09 06:38:02 America/New_York.| >= |.2015-03-09 06:38:02 America/New_York.|)    &
    (|.2015-03-09 06:38:02 America/New_York.| >= |.1956-03-09 06:38:02 America/New_York.|)
}
RUnit_time_gt <- function() {
    (|.2015-03-09 06:38:02 America/New_York.| > |.2015-03-09 06:38:01 America/New_York.|)    &
    (|.2015-03-09 06:38:01 America/New_York.| > |.2015-03-09 06:38:02 America/New_York.|) == FALSE &
    (|.2015-03-09 06:38:02 America/New_York.| > |.2015-03-09 06:38:02 America/New_York.|) == FALSE &
    (|.2015-03-09 06:38:02 America/New_York.| > |.1956-03-09 06:38:02 America/New_York.|)
}

## interval
RUnit_interval_le <- function() {
    a <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York-|;
    b <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|;
    c <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York-|;
    d <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|;
      (a <= b) &  (b <= c) &  (c <= d) &  (a <= c) &  (b <= d) &  (a <= d) &
     !(b <= a) & !(c <= b) & !(d <= c) & !(c <= a) & !(d <= b) & !(d <= a) &
     (a <= a) & (b <= b) & (c <= c) & (d <= d)       
}
RUnit_interval_lt <- function() {
    a <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York-|;
    b <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|;
    c <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York-|;
    d <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|;
      (a < b) &  (b < c) &  (c < d) &  (a < c) &  (b < d) &  (a < d) &
     !(b < a) & !(c < b) & !(d < c) & !(c < a) & !(d < b) & !(d < a) &
     !(a < a) & !(b < b) & !(c < c) & !(d < d)       
} 
RUnit_interval_eq <- function() {
    a <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York-|;
    b <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|;
    c <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York-|;
    d <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|;
     !(a == b) & !(b == c) & !(c == d) & !(a == c) & !(b == d) & !(a == d) &
     !(b == a) & !(c == b) & !(d == c) & !(c == a) & !(d == b) & !(d == a) &
      (a == a) &  (b == b) &  (c == c) &  (d == d)       
} 
RUnit_interval_ne <- function() {
    a <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York-|;
    b <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|;
    c <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York-|;
    d <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|;
      (a != b) &  (b != c) &  (c != d) &  (a != c) &  (b != d) &  (a != d) &
      (b != a) &  (c != b) &  (d != c) &  (c != a) &  (d != b) &  (d != a) &
     !(a != a) & !(b != b) & !(c != c) & !(d != d)       
} 
RUnit_interval_ge <- function() {
    a <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York-|;
    b <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|;
    c <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York-|;
    d <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|;
     !(a >= b) & !(b >= c) & !(c >= d) & !(a >= c) & !(b >= d) & !(a >= d) &
      (b >= a) &  (c >= b) &  (d >= c) &  (c >= a) &  (d >= b) &  (d >= a) &
     (a >= a) & (b >= b) & (c >= c) & (d >= d)       
}
RUnit_interval_gt <- function() {
    a <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York-|;
    b <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|;
    c <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York-|;
    d <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|;
     !(a > b) & !(b > c) & !(c > d) & !(a > c) & !(b > d) & !(a > d) &
      (b > a) &  (c > b) &  (d > c) &  (c > a) &  (d > b) &  (d > a) &
     !(a > a) & !(b > b) & !(c > c) & !(d > d)       
}

## duration
RUnit_duration_add <- function() {
    as.duration(1) +  as.duration(2) ==  as.duration(3)  &
    as.duration(1) + as.duration(-2) == as.duration(-1)
}
RUnit_duration_sub <- function() {
    as.duration(3) - as.duration(2) ==  as.duration(1)   &
    as.duration(1) - as.duration(2) == as.duration(-1)
}
RUnit_duration_mul <- function() {
    as.duration(3) * 2  ==  as.duration(6)   &
    as.duration(2) * -3 == as.duration(-6)   &
    3 * as.duration(2)  ==  as.duration(6)   &
    2 * as.duration(-3) == as.duration(-6)   &
    tryCatch(as.duration(1) *  NaN, "error") == "error"   &
    tryCatch(as.duration(1) * -Inf, "error") == "error"
}
RUnit_duration_div <- function() {
    as.duration(3) / 2  ==  as.duration(1)   &
    as.duration(30) / -3 == as.duration(-10)
    tryCatch(as.duration(2) / 0, "error") == "error"      & # can't divide by 0!
    tryCatch(2 / as.duration(1), "error") == "error"      & # can't divide by duration!
    tryCatch(as.duration(1) /  NaN, "error") == "error"   &
    tryCatch(as.duration(1) / -Inf, "error") == "error"
}
RUnit_duration_le <- function() {
    (as.duration(1) <= as.duration(2))          &
    (as.duration(2) <= as.duration(1)) == FALSE &
    (as.duration(2) <= as.duration(2))
}
RUnit_duration_lt <- function() {
    (as.duration(1) < as.duration(2))           &
    (as.duration(2) < as.duration(1)) == FALSE  &
    (as.duration(2) < as.duration(2)) == FALSE
} 
RUnit_duration_eq <- function() {
    (as.duration(1) == as.duration(2)) == FALSE &
    (as.duration(2) == as.duration(2))
} 
RUnit_duration_ne <- function() {
    (as.duration(1) != as.duration(2))          &
    (as.duration(2) != as.duration(2)) == FALSE
} 
RUnit_duration_ge <- function() {
    (as.duration(1) >= as.duration(2)) == FALSE &
    (as.duration(2) >= as.duration(1))          &
    (as.duration(2) >= as.duration(2))
}
RUnit_duration_gt <- function() {
    (as.duration(1) > as.duration(2)) == FALSE  &
    (as.duration(2) > as.duration(1))           &
    (as.duration(2) > as.duration(2)) == FALSE
} 
RUnit_duration_undefined_ops <- function() {
    tryCatch(as.duration(1) ^ -as.duration(1), "error") == "error"  &
    tryCatch(as.duration(1) & -as.duration(1), "error") == "error"  &
    tryCatch(as.duration(1) && -as.duration(1), "error") == "error" &
    tryCatch(as.duration(1) | -as.duration(1), "error") == "error"  &
    tryCatch(as.duration(1) || -as.duration(1), "error") == "error" &
    tryCatch(as.duration(1) : as.duration(2), "error") == "error"
}

## time op duration
RUnit_time_duration_add <- function() {
    t <- |.2015-03-09 06:38:01 America/New_York.|
    t + as.duration(1e9)  == |.2015-03-09 06:38:02 America/New_York.|  &
    as.duration(1e9) + t  == |.2015-03-09 06:38:02 America/New_York.|  &
    t + as.duration(-2e9) == |.2015-03-09 06:37:59 America/New_York.|  &
    as.duration(-2e9) + t == |.2015-03-09 06:37:59 America/New_York.|
}
RUnit_time_duration_sub <- function() {
    t <- |.2015-03-09 06:38:01 America/New_York.|
    t - as.duration(1e9)  == |.2015-03-09 06:38:00 America/New_York.|  &
    tryCatch(as.duration(1e9) - t, "error")  == "error"                &
    t - as.duration(-2e9) == |.2015-03-09 06:38:03 America/New_York.|
}
RUnit_time_duration_undefined_ops <- function() {
    t <- |.2015-03-09 06:38:01 America/New_York.|
    tryCatch(as.duration(1e9) *  t, "error")  == "error"  &
    tryCatch(as.duration(1e9) /  t, "error")  == "error"  &
    tryCatch(as.duration(1e9) && t, "error")  == "error"  &
    tryCatch(as.duration(1e9) &  t, "error")  == "error" 
}

## interval op duration
RUnit_time_duration_add <- function() {
    a <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    b <-|-2015-03-09 06:38:02 America/New_York -> 2015-03-09 06:38:03 America/New_York+|
    c <-|-2015-03-09 06:38:00 America/New_York -> 2015-03-09 06:38:01 America/New_York+|
    a + as.duration(1e9)  == b  &
    as.duration(1e9) + a  == b  &
    a + as.duration(-1e9) == c  &
    as.duration(-1e9) + a == c 
}
RUnit_time_duration_sub <- function() {
    a <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    b <-|-2015-03-09 06:38:02 America/New_York -> 2015-03-09 06:38:03 America/New_York+|
    c <-|-2015-03-09 06:38:00 America/New_York -> 2015-03-09 06:38:01 America/New_York+|
    a - as.duration(1e9)  == c  &
    tryCatch(as.duration(1e9) - a, "error")  == "error"  &
    a - as.duration(-1e9) == b
}
RUnit_time_duration_undefined_ops <- function() {
    a <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    b <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    tryCatch(a * b, "error")  == "error"   &
    tryCatch(a / b, "error")  == "error"   &
    tryCatch(a & b, "error")  == "error"   &
    tryCatch(a || b, "error")  == "error"  
}

## array-array --------------------
## double
RUnit_arr_arr_double_add <- function() {
    all(matrix(1, 2, 2) + matrix( 2, 2, 2)  == matrix( 3, 2, 2) &
        matrix(1, 2, 2) + matrix(-2, 2, 2)  == matrix(-1, 2, 2))
}
RUnit_arr_arr_double_sub <- function() {
    all(matrix(1, 2, 2) - matrix( 2, 2, 2) == matrix(-1, 2, 2) &
        matrix(1, 2, 2) - matrix(-2, 2, 2) == matrix( 3, 2, 2))
}
RUnit_arr_arr_double_mul <- function() {
    all(matrix(3, 2, 2) * matrix( 2, 2, 2) == matrix( 6, 2, 2) &
        matrix(2, 2, 2) * matrix(-3, 2, 2) == matrix(-6, 2, 2))
}
RUnit_arr_arr_double_div <- function() {
    all(matrix( 3, 2, 2) / matrix(2, 2, 2) == matrix(1.5, 2, 2) &
        matrix(-3, 2, 2) / matrix(1, 2, 2) == matrix( -3, 2, 2))
}
RUnit_arr_arr_double_div_byzero <- function() {
    all(matrix( 3, 2, 2) / matrix(0, 2, 2) == matrix( Inf, 2, 2) &
        matrix(-3, 2, 2) / matrix(0, 2, 2) == matrix(-Inf, 2, 2))
}
RUnit_arr_arr_double_NaN <- function() {
    all(is.nan(array(NaN, c(3,3,3))))
}
RUnit_arr_arr_double_le <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(1, c(2,3,4), dimnames=dnames)
    b <- array(0, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a <= b) == f  &
        (b <= a) == t  &
        (b <= b) == t  &
        (a <= a) == t)
}
RUnit_arr_arr_double_lt <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(1, c(2,3,4), dimnames=dnames)
    b <- array(0, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a < b) == f  &
        (b < a) == t  &
        (b < b) == f  &
        (a < a) == f)
} 
RUnit_arr_arr_double_eq <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(1, c(2,3,4), dimnames=dnames)
    b <- array(0, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a == b) == f  &
        (b == b) == t  &
        (a == a) == t)
} 
RUnit_arr_arr_double_ne <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(1, c(2,3,4), dimnames=dnames)
    b <- array(0, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a != b) == t  &
        (b != b) == f  &
        (a != a) == f)
} 
RUnit_arr_arr_double_ge <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(1, c(2,3,4), dimnames=dnames)
    b <- array(0, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a >= b) == t  &
        (b >= a) == f  &
        (b >= b) == t  &
        (a >= a) == t)
}
RUnit_arr_arr_double_gt <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(1, c(2,3,4), dimnames=dnames)
    b <- array(0, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a > b) == t  &
        (b > a) == f  &
        (b > b) == f  &
        (a > a) == f)
} 
RUnit_arr_arr_double_and <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(1, c(2,3,4), dimnames=dnames)
    b <- array(0, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a & a) == t  &
        (a & b) == f  &
        (b & a) == f  &
        (b & b) == f)
}
RUnit_arr_arr_double_and2 <- function() {
    ## '&&' only looks at the first element
    (c(0, 1, 1) && c(0, 1, 1)) == FALSE &
    (c(0, 0, 0) && c(1, 0, 0)) == FALSE &
    (c(1, 1, 1) && c(0, 1, 1)) == FALSE &
    (c(1, 0, 0) && c(1, 0, 0)) == TRUE
}
RUnit_arr_arr_double_or <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(1, c(2,3,4), dimnames=dnames)
    b <- array(0, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a | a) == t  &
        (a | b) == t  &
        (b | a) == t  &
        (b | b) == f)
}
RUnit_arr_arr_double_or2 <- function() {
    ## '||' only looks at the first element
    (c(0, 1, 1) || c(0, 1, 1)) == FALSE &
    (c(0, 0, 0) || c(1, 0, 0)) == TRUE  &
    (c(1, 1, 1) || c(0, 1, 1)) == TRUE  &
    (c(1, 0, 0) || c(1, 0, 0)) == TRUE
}

## zstring
RUnit_arr_arr_string_le <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array("abs", c(2,3,4), dimnames=dnames)
    b <- array("123", c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a <= b) == f  &
        (b <= a) == t  &
        (b <= b) == t  &
        (a <= a) == t)
}
RUnit_arr_arr_string_lt <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array("abs", c(2,3,4), dimnames=dnames)
    b <- array("123", c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a < b) == f  &
        (b < a) == t  &
        (b < b) == f  &
        (a < a) == f)
} 
RUnit_arr_arr_string_eq <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array("abs", c(2,3,4), dimnames=dnames)
    b <- array("123", c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a == b) == f  &
        (b == b) == t  &
        (a == a) == t)
} 
RUnit_arr_arr_string_ne <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array("abs", c(2,3,4), dimnames=dnames)
    b <- array("123", c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a != b) == t  &
        (b != b) == f  &
        (a != a) == f)
} 
RUnit_arr_arr_string_ge <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array("abs", c(2,3,4), dimnames=dnames)
    b <- array("123", c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a >= b) == t  &
        (b >= a) == f  &
        (b >= b) == t  &
        (a >= a) == t)
}
RUnit_arr_arr_string_gt <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array("abs", c(2,3,4), dimnames=dnames)
    b <- array("123", c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a > b) == t  &
        (b > a) == f  &
        (b > b) == f  &
        (a > a) == f)
} 
RUnit_arr_arr_string_undefined_ops <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array("abs", c(2,3,4), dimnames=dnames)
    b <- array("123", c(2,3,4), dimnames=dnames)
    tryCatch(a +  b, "error") == "error" &
    tryCatch(a -  b, "error") == "error" &
    tryCatch(a *  b, "error") == "error" &
    tryCatch(a /  b, "error") == "error" &
    tryCatch(a ^  b, "error") == "error" &
    tryCatch(a &  b, "error") == "error" &
    tryCatch(a && b, "error") == "error" &
    tryCatch(a |  b, "error") == "error" &
    tryCatch(a || b, "error") == "error" &
    tryCatch(a :  b, "error") == "error"
}

## bool
RUnit_arr_arr_bool_le <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((t <= f) == f  &
        (f <= t) == t  &
        (f <= f) == t  &
        (t <= t) == t)
}
RUnit_arr_arr_bool_lt <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((t < f) == f  &
        (f < t) == t  &
        (f < f) == f  &
        (t < t) == f)
} 
RUnit_arr_arr_bool_eq <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((t == f) == f  &
        (f == f) == t  &
        (t == t) == t)
} 
RUnit_arr_arr_bool_ne <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((t != f) == t  &
        (f != f) == f  &
        (t != t) == f)
} 
RUnit_arr_arr_bool_ge <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((t >= f) == t  &
        (f >= t) == f  &
        (f >= f) == t  &
        (t >= t) == t)
}
RUnit_arr_arr_bool_gt <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((t > f) == t  &
        (f > t) == f  &
        (f > f) == f  &
        (t > t) == f)
} 
RUnit_arr_arr_bool_and <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    t <- array(TRUE, c(2,3,4), dimnames=dnames)
    f <- !t
    all((t & t) == t  &
        (t & f) == f  &
        (f & t) == f  &
        (f & f) == f)
}
RUnit_arr_arr_bool_and2 <- function() {
    ## '&&' only looks at the first element
    (c(F, T, T) && c(F, T, T)) == FALSE &
    (c(F, F, F) && c(T, F, F)) == FALSE &
    (c(T, T, T) && c(F, T, T)) == FALSE &
    (c(T, F, F) && c(T, F, F)) == TRUE
}
RUnit_arr_arr_bool_or <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    t <- array(TRUE, c(2,3,4), dimnames=dnames)
    f <- !t
    all((t | t) == t  &
        (t | f) == t  &
        (f | t) == t  &
        (f | f) == f)
}
RUnit_arr_arr_bool_or2 <- function() {
    ## '||' only looks at the first element
    (c(F, T, T) || c(F, T, T)) == FALSE &
    (c(F, F, F) || c(T, F, F)) == TRUE  &
    (c(T, T, T) || c(F, T, T)) == TRUE  &
    (c(T, F, F) || c(T, F, F)) == TRUE
}
RUnit_arr_arr_bool_undefined_ops <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(TRUE, c(2,3,4), dimnames=dnames)
    b <- array(FALSE, c(2,3,4), dimnames=dnames)
    tryCatch(a +  b, "error") == "error" &
    tryCatch(a -  b, "error") == "error" &
    tryCatch(a *  b, "error") == "error" &
    tryCatch(a /  b, "error") == "error" &
    tryCatch(a ^  b, "error") == "error" &
    tryCatch(a :  b, "error") == "error"
}

## time
RUnit_arr_arr_time_minus <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(|.2015-03-09 06:38:02 America/New_York.|, c(2,3,4), dimnames=dnames)
    b <- array(|.2015-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    res <- array(as.duration(1e9), c(2,3,4), dimnames=dnames);
    all((a - b) == res  &
        (b - a) == -res)
}
RUnit_arr_arr_time_le <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(|.2015-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    b <- array(|.2014-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a <= b) == f  &
        (b <= a) == t  &
        (b <= b) == t  &
        (a <= a) == t)
}
RUnit_arr_arr_time_lt <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(|.2015-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    b <- array(|.2014-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a < b) == f  &
        (b < a) == t  &
        (b < b) == f  &
        (a < a) == f)
} 
RUnit_arr_arr_time_eq <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(|.2015-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    b <- array(|.2014-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a == b) == f  &
        (b == b) == t  &
        (a == a) == t)
} 
RUnit_arr_arr_time_ne <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(|.2015-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    b <- array(|.2014-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a != b) == t  &
        (b != b) == f  &
        (a != a) == f)
} 
RUnit_arr_arr_time_ge <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(|.2015-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    b <- array(|.2014-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a >= b) == t  &
        (b >= a) == f  &
        (b >= b) == t  &
        (a >= a) == t)
}
RUnit_arr_arr_time_gt <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(|.2015-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    b <- array(|.2014-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a > b) == t  &
        (b > a) == f  &
        (b > b) == f  &
        (a > a) == f)
} 
RUnit_arr_arr_time_undefined_ops <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(|.2015-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    b <- array(|.2014-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    tryCatch(a +  b, "error") == "error" &
    tryCatch(a *  b, "error") == "error" &
    tryCatch(a /  b, "error") == "error" &
    tryCatch(a ^  b, "error") == "error" &
    tryCatch(a &  b, "error") == "error" &
    tryCatch(a && b, "error") == "error" &
    tryCatch(a |  b, "error") == "error" &
    tryCatch(a || b, "error") == "error" &
    tryCatch(a :  b, "error") == "error"
}

## interval
RUnit_arr_arr_interval_le <- function() {
    i1 <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    i0 <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(i1, c(2,3,4), dimnames=dnames)
    b <- array(i0, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a <= b) == f  &
        (b <= a) == t  &
        (b <= b) == t  &
        (a <= a) == t)
}
RUnit_arr_arr_interval_lt <- function() {
    i1 <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    i0 <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(i1, c(2,3,4), dimnames=dnames)
    b <- array(i0, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a < b) == f  &
        (b < a) == t  &
        (b < b) == f  &
        (a < a) == f)
} 
RUnit_arr_arr_interval_eq <- function() {
    i1 <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    i0 <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(i1, c(2,3,4), dimnames=dnames)
    b <- array(i0, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a == b) == f  &
        (b == b) == t  &
        (a == a) == t)
} 
RUnit_arr_arr_interval_ne <- function() {
    i1 <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    i0 <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(i1, c(2,3,4), dimnames=dnames)
    b <- array(i0, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a != b) == t  &
        (b != b) == f  &
        (a != a) == f)
} 
RUnit_arr_arr_interval_ge <- function() {
    i1 <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    i0 <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(i1, c(2,3,4), dimnames=dnames)
    b <- array(i0, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a >= b) == t  &
        (b >= a) == f  &
        (b >= b) == t  &
        (a >= a) == t)
}
RUnit_arr_arr_interval_gt <- function() {
    i1 <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    i0 <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(i1, c(2,3,4), dimnames=dnames)
    b <- array(i0, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a > b) == t  &
        (b > a) == f  &
        (b > b) == f  &
        (a > a) == f)
} 
RUnit_arr_arr_interval_undefined_ops <- function() {
    i1 <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    i0 <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(i1, c(2,3,4), dimnames=dnames)
    b <- array(i0, c(2,3,4), dimnames=dnames)
    tryCatch(a +  b, "error") == "error" &
    tryCatch(a -  b, "error") == "error" &
    tryCatch(a *  b, "error") == "error" &
    tryCatch(a /  b, "error") == "error" &
    tryCatch(a ^  b, "error") == "error" &
    tryCatch(a &  b, "error") == "error" &
    tryCatch(a && b, "error") == "error" &
    tryCatch(a |  b, "error") == "error" &
    tryCatch(a || b, "error") == "error" &
    tryCatch(a :  b, "error") == "error"
}

## duration
RUnit_arr_arr_duration_add <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(1),  c(2,3,4), dimnames=dnames)
    b <- array(as.duration(-1), c(2,3,4), dimnames=dnames)
    d <- array(as.duration(2),  c(2,3,4), dimnames=dnames)
    e <- array(as.duration(3),  c(2,3,4), dimnames=dnames)
    all((a + d) == e  &
        (e + b) == d)
}
RUnit_arr_arr_duration_sub <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(1),  c(2,3,4), dimnames=dnames)
    b <- array(as.duration(-1), c(2,3,4), dimnames=dnames)
    d <- array(as.duration(2),  c(2,3,4), dimnames=dnames)
    e <- array(as.duration(3),  c(2,3,4), dimnames=dnames)
    all((e - d) == a  &
        (d - b) == e)
}
RUnit_arr_arr_duration_mul <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(2),  c(2,3,4), dimnames=dnames)
    b <- array(3,  c(2,3,4), dimnames=dnames)
    d <- array(as.duration(6),  c(2,3,4), dimnames=dnames)
    all((a * b) == d  &
        (b * a) == d)
}
RUnit_arr_arr_duration_div <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(2),  c(2,3,4), dimnames=dnames)
    b <- array(3,  c(2,3,4), dimnames=dnames)
    d <- array(as.duration(6),  c(2,3,4), dimnames=dnames)
    all((d / b) == a) & tryCatch(b / d, "error") == "error"
}
RUnit_arr_arr_duration_div_byzero<- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(2),  c(2,3,4), dimnames=dnames)
    b <- array(0, c(2,3,4), dimnames=dnames)
    tryCatch(d / b, "error") == "error"
}
RUnit_arr_arr_duration_div_bynan <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(2),  c(2,3,4), dimnames=dnames)
    b <- array(NaN,  c(2,3,4), dimnames=dnames)
    tryCatch(d / b, "error") == "error"
}
RUnit_arr_arr_duration_div_byinf <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(2),  c(2,3,4), dimnames=dnames)
    b <- array(Inf,  c(2,3,4), dimnames=dnames)
    tryCatch(d / b, "error") == "error"
}
RUnit_arr_arr_duration_le <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(1), c(2,3,4), dimnames=dnames)
    b <- array(as.duration(0), c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a <= b) == f  &
        (b <= a) == t  &
        (b <= b) == t  &
        (a <= a) == t)
}
RUnit_arr_arr_duration_lt <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(1), c(2,3,4), dimnames=dnames)
    b <- array(as.duration(0), c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a < b) == f  &
        (b < a) == t  &
        (b < b) == f  &
        (a < a) == f)
} 
RUnit_arr_arr_duration_eq <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(1), c(2,3,4), dimnames=dnames)
    b <- array(as.duration(0), c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a == b) == f  &
        (b == b) == t  &
        (a == a) == t)
} 
RUnit_arr_arr_duration_ne <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(1), c(2,3,4), dimnames=dnames)
    b <- array(as.duration(0), c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a != b) == t  &
        (b != b) == f  &
        (a != a) == f)
} 
RUnit_arr_arr_duration_ge <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(1), c(2,3,4), dimnames=dnames)
    b <- array(as.duration(0), c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a >= b) == t  &
        (b >= a) == f  &
        (b >= b) == t  &
        (a >= a) == t)
}
RUnit_arr_arr_duration_gt <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(1), c(2,3,4), dimnames=dnames)
    b <- array(as.duration(0), c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a > b) == t  &
        (b > a) == f  &
        (b > b) == f  &
        (a > a) == f)
}
RUnit_arr_arr_duration_undefined_ops <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(1), c(2,3,4), dimnames=dnames)
    b <- array(as.duration(0), c(2,3,4), dimnames=dnames)
    tryCatch(a *  b, "error") == "error" &
    tryCatch(a /  b, "error") == "error" &
    tryCatch(a ^  b, "error") == "error" &
    tryCatch(a &  b, "error") == "error" &
    tryCatch(a && b, "error") == "error" &
    tryCatch(a |  b, "error") == "error" &
    tryCatch(a || b, "error") == "error" &
    tryCatch(a :  b, "error") == "error"
}

## scalar - array --------------------
## double
RUnit_scalar_arr_double_add <- function() {
    all(2 +  matrix(1, 2, 2) == matrix( 3, 2, 2) &
        1 + -matrix(2, 2, 2) == matrix(-1, 2, 2))
}
RUnit_scalar_arr_double_sub <- function() {
    all(3 - matrix(2, 2, 2) == matrix( 1, 2, 2) &
        1 - matrix(2, 2, 2) == matrix(-1, 2, 2))
}
RUnit_scalar_arr_double_mul <- function() {
    all(3 * matrix( 2, 2, 2) == matrix( 6, 2, 2) &
        2 * matrix(-3, 2, 2) == matrix(-6, 2, 2))
}
RUnit_scalar_arr_double_div <- function() {
    all( 3 / matrix(2, 2, 2) == matrix(1.5, 2, 2) &
        -3 / matrix(1, 2, 2) == matrix( -3, 2, 2))
}
RUnit_scalar_arr_double_div_byzero <- function() {
    all( 3 / matrix(0, 2, 2) == matrix( Inf, 2, 2) &
        -3 / matrix(0, 2, 2) == matrix(-Inf, 2, 2))
}
RUnit_scalar_arr_double_le <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(1, c(2,3,4), dimnames=dnames)
    b <- array(0, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((1 <= b) == f  &
        (0 <= a) == t  &
        (0 <= b) == t  &
        (1 <= a) == t)
}
RUnit_scalar_arr_double_lt <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(1, c(2,3,4), dimnames=dnames)
    b <- array(0, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((1 < b) == f  &
        (0 < a) == t  &
        (0 < b) == f  &
        (1 < a) == f)
} 
RUnit_scalar_arr_double_eq <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(1, c(2,3,4), dimnames=dnames)
    b <- array(0, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((1 == b) == f  &
        (0 == b) == t  &
        (1 == a) == t)
} 
RUnit_scalar_arr_double_ne <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(1, c(2,3,4), dimnames=dnames)
    b <- array(0, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((1 != b) == t  &
        (0 != b) == f  &
        (1 != a) == f)
} 
RUnit_scalar_arr_double_ge <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(1, c(2,3,4), dimnames=dnames)
    b <- array(0, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((1 >= b) == t  &
        (0 >= a) == f  &
        (0 >= b) == t  &
        (1 >= a) == t)
}
RUnit_scalar_arr_double_gt <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(1, c(2,3,4), dimnames=dnames)
    b <- array(0, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((1 > b) == t  &
        (0 > a) == f  &
        (0 > b) == f  &
        (1 > a) == f)
} 
RUnit_scalar_arr_double_and <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(1, c(2,3,4), dimnames=dnames)
    b <- array(0, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((1 & a) == t  &
        (1 & b) == f  &
        (0 & a) == f  &
        (0 & b) == f)
}
RUnit_scalar_arr_double_and2 <- function() {
    ## '&&' only looks at the first element
    (0 && c(0, 1, 1)) == FALSE &
    (0 && c(1, 0, 0)) == FALSE &
    (1 && c(0, 1, 1)) == FALSE &
    (1 && c(1, 0, 0)) == TRUE
}
RUnit_scalar_arr_double_or <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(1, c(2,3,4), dimnames=dnames)
    b <- array(0, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((1 | a) == t  &
        (1 | b) == t  &
        (0 | a) == t  &
        (0 | b) == f)
}
RUnit_scalar_arr_double_or2 <- function() {
    ## '||' only looks at the first element
    (0 || c(0, 1, 1)) == FALSE &
    (0 || c(1, 0, 0)) == TRUE  &
    (1 || c(0, 1, 1)) == TRUE  &
    (1 || c(1, 0, 0)) == TRUE
}

## bool
RUnit_scalar_arr_bool_le <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((TRUE <= f)  == f  &
        (FALSE <= t) == t  &
        (FALSE <= f) == t  &
        (TRUE <= t)  == t)
}
RUnit_scalar_arr_bool_lt <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((TRUE < f)  == f  &
        (FALSE < t) == t  &
        (FALSE < f) == f  &
        (TRUE < t)  == f)
} 
RUnit_scalar_arr_bool_eq <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((TRUE == f)  == f  &
        (FALSE == f) == t  &
        (TRUE == t)  == t)
} 
RUnit_scalar_arr_bool_ne <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((TRUE != f)  == t  &
        (FALSE != f) == f  &
        (TRUE != t)  == f)
} 
RUnit_scalar_arr_bool_ge <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((TRUE >= f)  == t  &
        (FALSE >= t) == f  &
        (FALSE >= f) == t  &
        (TRUE >= t)  == t)
}
RUnit_scalar_arr_bool_gt <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((TRUE > f)  == t  &
        (FALSE > t) == f  &
        (FALSE > f) == f  &
        (TRUE > t)  == f)
} 
RUnit_scalar_arr_bool_and <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    t <- array(TRUE, c(2,3,4), dimnames=dnames)
    f <- !t
    all((TRUE & t)  == t  &
        (TRUE & f)  == f  &
        (FALSE & t) == f  &
        (FALSE & f) == f)
}
RUnit_scalar_arr_bool_and2 <- function() {
    ## '&&' only looks at the first element
    (F && c(F, T, T)) == FALSE &
    (F && c(T, F, F)) == FALSE &
    (T && c(F, T, T)) == FALSE &
    (T && c(T, F, F)) == TRUE
}
RUnit_scalar_arr_bool_or <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    t <- array(TRUE, c(2,3,4), dimnames=dnames)
    f <- !t
    all((TRUE | t)  == t  &
        (TRUE | f)  == t  &
        (FALSE | t) == t  &
        (FALSE | f) == f)
}
RUnit_scalar_arr_bool_or2 <- function() {
    ## '||' only looks at the first element
    (F || c(F, T, T)) == FALSE &
    (F || c(T, F, F)) == TRUE  &
    (T || c(F, T, T)) == TRUE  &
    (T || c(T, F, F)) == TRUE
}
RUnit_scalar_arr_bool_undefined_ops <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- TRUE
    b <- array(FALSE, c(2,3,4), dimnames=dnames)
    tryCatch(a + b, "error") == "error" &
    tryCatch(a - b, "error") == "error" &
    tryCatch(a * b, "error") == "error" &
    tryCatch(a / b, "error") == "error" &
    tryCatch(a ^ b, "error") == "error" &
    tryCatch(a : b, "error") == "error"
}

# string
RUnit_scalar_arr_string_le <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array("abc", c(2,3,4), dimnames=dnames)
    b <- array("123", c(2,3,4), dimnames=dnames)
    as <- "abc"
    bs <- "123"
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((as <= b) == f  &
        (bs <= a) == t  &
        (bs <= b) == t  &
        (as <= a) == t)
}
RUnit_scalar_arr_string_lt <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array("abc", c(2,3,4), dimnames=dnames)
    b <- array("123", c(2,3,4), dimnames=dnames)
    as <- "abc"
    bs <- "123"
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((as < b) == f  &
        (bs < a) == t  &
        (bs < b) == f  &
        (as < a) == f)
} 
RUnit_scalar_arr_string_eq <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array("abc", c(2,3,4), dimnames=dnames)
    b <- array("123", c(2,3,4), dimnames=dnames)
    as <- "abc"
    bs <- "123"
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((as == b) == f  &
        (bs == b) == t  &
        (as == a) == t)
}
RUnit_scalar_arr_string_ne <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array("abc", c(2,3,4), dimnames=dnames)
    b <- array("123", c(2,3,4), dimnames=dnames)
    as <- "abc"
    bs <- "123"
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((as != b) == t  &
        (bs != b) == f  &
        (as != a) == f)
}
RUnit_scalar_arr_string_ge <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array("abc", c(2,3,4), dimnames=dnames)
    b <- array("123", c(2,3,4), dimnames=dnames)
    as <- "abc"
    bs <- "123"
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((as >= b) == t  &
        (bs >= a) == f  &
        (bs >= b) == t  &
        (as >= a) == t)
}
RUnit_scalar_arr_string_gt <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array("abc", c(2,3,4), dimnames=dnames)
    b <- array("123", c(2,3,4), dimnames=dnames)
    as <- "abc"
    bs <- "123"
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((as > b) == t  &
        (bs > a) == f  &
        (bs > b) == f  &
        (as > a) == f)
} 
RUnit_scalar_arr_string_undefined_ops <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    b <- array("123", c(2,3,4), dimnames=dnames)
    as <- "abc"
    tryCatch(as +  b, "error") == "error" &
    tryCatch(as -  b, "error") == "error" &
    tryCatch(as *  b, "error") == "error" &
    tryCatch(as /  b, "error") == "error" &
    tryCatch(as ^  b, "error") == "error" &
    tryCatch(as &  b, "error") == "error" &
    tryCatch(as && b, "error") == "error" &
    tryCatch(as |  b, "error") == "error" &
    tryCatch(as || b, "error") == "error" &
    tryCatch(as :  b, "error") == "error"
}

## time
RUnit_scalar_arr_time_minus <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(|.2015-03-09 06:38:02 America/New_York.|, c(2,3,4), dimnames=dnames)
    b <- array(|.2015-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    as <- |.2015-03-09 06:38:02 America/New_York.|
    bs <- |.2015-03-09 06:38:01 America/New_York.|
    res <- array(as.duration(1e9), c(2,3,4), dimnames=dnames);
    all((as - b) == res  &
        (bs - a) == -res)
}
RUnit_scalar_arr_time_le <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(|.2015-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    b <- array(|.2014-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    as <- |.2015-03-09 06:38:01 America/New_York.|
    bs <- |.2014-03-09 06:38:01 America/New_York.|
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((as <= b) == f  &
        (bs <= a) == t  &
        (bs <= b) == t  &
        (as <= a) == t)
}
RUnit_scalar_arr_time_lt <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(|.2015-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    b <- array(|.2014-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    as <- |.2015-03-09 06:38:01 America/New_York.|
    bs <- |.2014-03-09 06:38:01 America/New_York.|
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((as < b) == f  &
        (bs < a) == t  &
        (bs < b) == f  &
        (as < a) == f)
} 
RUnit_scalar_arr_time_eq <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(|.2015-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    b <- array(|.2014-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    as <- |.2015-03-09 06:38:01 America/New_York.|
    bs <- |.2014-03-09 06:38:01 America/New_York.|
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((as == b) == f  &
        (bs == b) == t  &
        (as == a) == t)
}
RUnit_scalar_arr_time_ne <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(|.2015-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    b <- array(|.2014-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    as <- |.2015-03-09 06:38:01 America/New_York.|
    bs <- |.2014-03-09 06:38:01 America/New_York.|
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((as != b) == t  &
        (bs != b) == f  &
        (as != a) == f)
}
RUnit_scalar_arr_time_ge <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(|.2015-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    b <- array(|.2014-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    as <- |.2015-03-09 06:38:01 America/New_York.|
    bs <- |.2014-03-09 06:38:01 America/New_York.|
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((as >= b) == t  &
        (bs >= a) == f  &
        (bs >= b) == t  &
        (as >= a) == t)
}
RUnit_scalar_arr_time_gt <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(|.2015-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    b <- array(|.2014-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    as <- |.2015-03-09 06:38:01 America/New_York.|
    bs <- |.2014-03-09 06:38:01 America/New_York.|
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((as > b) == t  &
        (bs > a) == f  &
        (bs > b) == f  &
        (as > a) == f)
} 
RUnit_scalar_arr_time_undefined_ops <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(|.2015-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    b <- array(|.2014-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    as <- |.2015-03-09 06:38:01 America/New_York.|
    bs <- |.2014-03-09 06:38:01 America/New_York.|
    tryCatch(as +  b, "error") == "error" &
    tryCatch(as *  b, "error") == "error" &
    tryCatch(as /  b, "error") == "error" &
    tryCatch(as ^  b, "error") == "error" &
    tryCatch(as &  b, "error") == "error" &
    tryCatch(as && b, "error") == "error" &
    tryCatch(as |  b, "error") == "error" &
    tryCatch(as || b, "error") == "error" &
    tryCatch(as :  b, "error") == "error"
}

## interval
RUnit_scalar_arr_interval_le <- function() {
    as <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    bs <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as, c(2,3,4), dimnames=dnames)
    b <- array(bs, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((as <= b) == f  &
        (bs <= a) == t  &
        (bs <= b) == t  &
        (as <= a) == t)
}
RUnit_scalar_arr_interval_lt <- function() {
    as <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    bs <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as, c(2,3,4), dimnames=dnames)
    b <- array(bs, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((as < b) == f  &
        (bs < a) == t  &
        (bs < b) == f  &
        (as < a) == f)
} 
RUnit_scalar_arr_interval_eq <- function() {
    as <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    bs <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as, c(2,3,4), dimnames=dnames)
    b <- array(bs, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((as == b) == f  &
        (bs == b) == t  &
        (as == a) == t)
}
RUnit_scalar_arr_interval_ne <- function() {
    as <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    bs <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as, c(2,3,4), dimnames=dnames)
    b <- array(bs, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((as != b) == t  &
        (bs != b) == f  &
        (as != a) == f)
}
RUnit_scalar_arr_interval_ge <- function() {
    as <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    bs <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as, c(2,3,4), dimnames=dnames)
    b <- array(bs, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((as >= b) == t  &
        (bs >= a) == f  &
        (bs >= b) == t  &
        (as >= a) == t)
}
RUnit_scalar_arr_interval_gt <- function() {
    as <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    bs <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as, c(2,3,4), dimnames=dnames)
    b <- array(bs, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((as > b) == t  &
        (bs > a) == f  &
        (bs > b) == f  &
        (as > a) == f)
} 
RUnit_scalar_arr_interval_undefined_ops <- function() {
    as <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    bs <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as, c(2,3,4), dimnames=dnames)
    b <- array(bs, c(2,3,4), dimnames=dnames)
    tryCatch(as +  b, "error") == "error" &
    tryCatch(as -  b, "error") == "error" &
    tryCatch(as *  b, "error") == "error" &
    tryCatch(as /  b, "error") == "error" &
    tryCatch(as ^  b, "error") == "error" &
    tryCatch(as &  b, "error") == "error" &
    tryCatch(as && b, "error") == "error" &
    tryCatch(as |  b, "error") == "error" &
    tryCatch(as || b, "error") == "error" &
    tryCatch(as :  b, "error") == "error"
}

## duration
RUnit_scalar_arr_duration_add <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(1),  c(2,3,4), dimnames=dnames)
    b <- array(as.duration(-1), c(2,3,4), dimnames=dnames)
    d <- array(as.duration(2),  c(2,3,4), dimnames=dnames)
    e <- array(as.duration(3),  c(2,3,4), dimnames=dnames)
    as <- as.duration(1)
    es <- as.duration(3)
    all((as + d) == e  &
        (es + b) == d)
}
RUnit_scalar_arr_duration_sub <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(1),  c(2,3,4), dimnames=dnames)
    b <- array(as.duration(-1), c(2,3,4), dimnames=dnames)
    d <- array(as.duration(2),  c(2,3,4), dimnames=dnames)
    e <- array(as.duration(3),  c(2,3,4), dimnames=dnames)
    ds <- as.duration(2)
    es <- as.duration(3)
    all((es - d) == a  &
        (ds - b) == e)
}
RUnit_scalar_arr_duration_mul <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(2),  c(2,3,4), dimnames=dnames)
    b <- array(3,  c(2,3,4), dimnames=dnames)
    d <- array(as.duration(6),  c(2,3,4), dimnames=dnames)
    as <- as.duration(2)
    all((a * b) == d  &
        (3 * a) == d)
}
RUnit_scalar_arr_duration_div <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(2),  c(2,3,4), dimnames=dnames)
    b <- array(3,  c(2,3,4), dimnames=dnames)
    d <- array(as.duration(6),  c(2,3,4), dimnames=dnames)
    all((as.duration(6) / b) == a) & tryCatch(3 / d, "error") == "error"
}
RUnit_scalar_arr_duration_div_byzero<- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- as.duration(2)
    b <- array(0, c(2,3,4), dimnames=dnames)
    tryCatch(a / b, "error") == "error"
}
RUnit_scalar_arr_duration_div_bynan <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <-as.duration(2)
    b <- array(NaN,  c(2,3,4), dimnames=dnames)
    tryCatch(a / b, "error") == "error"
}
RUnit_scalar_arr_duration_div_byinf <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- as.duration(2)
    b <- array(Inf,  c(2,3,4), dimnames=dnames)
    tryCatch(a / b, "error") == "error"
}
RUnit_scalar_arr_duration_le <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(1), c(2,3,4), dimnames=dnames)
    b <- array(as.duration(0), c(2,3,4), dimnames=dnames)
    as <- as.duration(1)
    bs <- as.duration(0)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((as <= b) == f  &
        (bs <= a) == t  &
        (bs <= b) == t  &
        (as <= a) == t)
}
RUnit_scalar_arr_duration_lt <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(1), c(2,3,4), dimnames=dnames)
    b <- array(as.duration(0), c(2,3,4), dimnames=dnames)
    as <- as.duration(1)
    bs <- as.duration(0)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((as < b) == f  &
        (bs < a) == t  &
        (bs < b) == f  &
        (as < a) == f)
} 
RUnit_scalar_arr_duration_eq <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(1), c(2,3,4), dimnames=dnames)
    b <- array(as.duration(0), c(2,3,4), dimnames=dnames)
    as <- as.duration(1)
    bs <- as.duration(0)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((as == b) == f  &
        (bs == b) == t  &
        (as == a) == t)
}
RUnit_scalar_arr_duration_ne <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(1), c(2,3,4), dimnames=dnames)
    b <- array(as.duration(0), c(2,3,4), dimnames=dnames)
    as <- as.duration(1)
    bs <- as.duration(0)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((as != b) == t  &
        (bs != b) == f  &
        (as != a) == f)
}
RUnit_scalar_arr_duration_ge <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(1), c(2,3,4), dimnames=dnames)
    b <- array(as.duration(0), c(2,3,4), dimnames=dnames)
    as <- as.duration(1)
    bs <- as.duration(0)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((as >= b) == t  &
        (bs >= a) == f  &
        (bs >= b) == t  &
        (as >= a) == t)
}
RUnit_scalar_arr_duration_gt <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(1), c(2,3,4), dimnames=dnames)
    b <- array(as.duration(0), c(2,3,4), dimnames=dnames)
    as <- as.duration(1)
    bs <- as.duration(0)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((as > b) == t  &
        (bs > a) == f  &
        (bs > b) == f  &
        (as > a) == f)
}
RUnit_scalar_arr_duration_undefined_ops <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- as.duration(1)
    b <- array(as.duration(0), c(2,3,4), dimnames=dnames)
    tryCatch(a *  b, "error") == "error" &
    tryCatch(a /  b, "error") == "error" &
    tryCatch(a ^  b, "error") == "error" &
    tryCatch(a &  b, "error") == "error" &
    tryCatch(a && b, "error") == "error" &
    tryCatch(a |  b, "error") == "error" &
    tryCatch(a || b, "error") == "error" &
    tryCatch(a :  b, "error") == "error"
}


## array-scalar --------------------
## double
RUnit_arr_scalar_double_add <- function() {
    all(matrix(1, 2, 2) +  2  == matrix( 3, 2, 2) &
        matrix(1, 2, 2) + -2  == matrix(-1, 2, 2))
}
RUnit_arr_scalar_double_sub <- function() {
    all(matrix(1, 2, 2) -  2 == matrix(-1, 2, 2) &
        matrix(1, 2, 2) - -2 == matrix( 3, 2, 2))
}
RUnit_arr_scalar_double_mul <- function() {
    all(matrix(3, 2, 2) *  2 == matrix( 6, 2, 2) &
        matrix(2, 2, 2) * -3 == matrix(-6, 2, 2))
}
RUnit_arr_scalar_double_div <- function() {
    all(matrix( 3, 2, 2) / 2 == matrix(1.5, 2, 2) &
        matrix(-3, 2, 2) / 1 == matrix( -3, 2, 2))
}
RUnit_arr_scalar_double_div_byzero <- function() {
    all(matrix( 3, 2, 2) / 0 == matrix( Inf, 2, 2) &
        matrix(-3, 2, 2) / 0 == matrix(-Inf, 2, 2))
}
RUnit_arr_scalar_double_le <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(1, c(2,3,4), dimnames=dnames)
    b <- array(0, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a <= 0) == f  &
        (b <= 1) == t  &
        (b <= 0) == t  &
        (a <= 1) == t)
}
RUnit_arr_scalar_double_lt <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(1, c(2,3,4), dimnames=dnames)
    b <- array(0, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a < 0) == f  &
        (b < 1) == t  &
        (b < 0) == f  &
        (a < 1) == f)
} 
RUnit_arr_scalar_double_eq <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(1, c(2,3,4), dimnames=dnames)
    b <- array(0, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a == 0) == f  &
        (b == 0) == t  &
        (a == 1) == t)
} 
RUnit_arr_scalar_double_ne <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(1, c(2,3,4), dimnames=dnames)
    b <- array(0, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a != 0) == t  &
        (b != 0) == f  &
        (a != 1) == f)
} 
RUnit_arr_scalar_double_ge <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(1, c(2,3,4), dimnames=dnames)
    b <- array(0, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a >= 0) == t  &
        (b >= 1) == f  &
        (b >= 0) == t  &
        (a >= 1) == t)
}
RUnit_arr_scalar_double_gt <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(1, c(2,3,4), dimnames=dnames)
    b <- array(0, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a > 0) == t  &
        (b > 1) == f  &
        (b > 0) == f  &
        (a > 1) == f)
} 
RUnit_arr_scalar_double_and <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(1, c(2,3,4), dimnames=dnames)
    b <- array(0, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a & 1) == t  &
        (a & 0) == f  &
        (b & 1) == f  &
        (b & 0) == f)
}
RUnit_arr_scalar_double_and2 <- function() {
    ## '&&' only looks at the first element
    (c(0, 1, 1) && 0) == FALSE &
    (c(0, 0, 0) && 1) == FALSE &
    (c(1, 1, 1) && 0) == FALSE &
    (c(1, 0, 0) && 1) == TRUE
}
RUnit_arr_scalar_double_or <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(1, c(2,3,4), dimnames=dnames)
    b <- array(0, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a | 1) == t  &
        (a | 0) == t  &
        (b | 1) == t  &
        (b | 0) == f)
}
RUnit_arr_scalar_double_or2 <- function() {
    ## '||' only looks at the first element
    (c(0, 1, 1) || 0) == FALSE &
    (c(0, 0, 0) || 1) == TRUE  &
    (c(1, 1, 1) || 0) == TRUE  &
    (c(1, 0, 0) || 1) == TRUE
}

## bool
RUnit_arr_scalar_bool_le <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((t <= FALSE) == f  &
        (f <= TRUE)  == t  &
        (f <= FALSE) == t  &
        (t <= TRUE)  == t)
}
RUnit_arr_scalar_bool_lt <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((t < FALSE) == f  &
        (f < TRUE)  == t  &
        (f < FALSE) == f  &
        (t < TRUE)  == f)
} 
RUnit_arr_scalar_bool_eq <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((t == FALSE) == f  &
        (f == FALSE) == t  &
        (t == TRUE)  == t)
} 
RUnit_arr_scalar_bool_ne <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((t != FALSE) == t  &
        (f != FALSE) == f  &
        (t != TRUE)  == f)
} 
RUnit_arr_scalar_bool_ge <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((t >= FALSE) == t  &
        (f >= TRUE)  == f  &
        (f >= FALSE) == t  &
        (t >= TRUE)  == t)
}
RUnit_arr_scalar_bool_gt <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((t > FALSE) == t  &
        (f > TRUE)  == f  &
        (f > FALSE) == f  &
        (t > TRUE)  == f)
} 
RUnit_arr_scalar_bool_and <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    t <- array(TRUE, c(2,3,4), dimnames=dnames)
    f <- !t
    all((t & TRUE)  == t  &
        (t & FALSE) == f  &
        (f & TRUE)  == f  &
        (f & FALSE) == f)
}
RUnit_arr_scalar_bool_and2 <- function() {
    ## '&&' only looks at the first element
    (c(F, T, T) && F) == FALSE &
    (c(F, F, F) && T) == FALSE &
    (c(T, T, T) && F) == FALSE &
    (c(T, F, F) && T) == TRUE
}
RUnit_arr_scalar_bool_or <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    t <- array(TRUE, c(2,3,4), dimnames=dnames)
    f <- !t
    all((t | TRUE)  == t  &
        (t | FALSE) == t  &
        (f | TRUE)  == t  &
        (f | FALSE) == f)
}
RUnit_arr_scalar_bool_or2 <- function() {
    ## '||' only looks at the first element
    (c(F, T, T) || F) == FALSE &
    (c(F, F, F) || T) == TRUE  &
    (c(T, T, T) || F) == TRUE  &
    (c(T, F, F) || T) == TRUE
}
RUnit_arr_scalar_bool_undefined_ops <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(TRUE, c(2,3,4), dimnames=dnames)
    b <- FALSE
    tryCatch(a +  b, "error") == "error" &
    tryCatch(a -  b, "error") == "error" &
    tryCatch(a *  b, "error") == "error" &
    tryCatch(a /  b, "error") == "error" &
    tryCatch(a ^  b, "error") == "error" &
    tryCatch(a :  b, "error") == "error"
}

## string
RUnit_arr_scalar_string_le <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array("abc", c(2,3,4), dimnames=dnames)
    b <- array("123", c(2,3,4), dimnames=dnames)
    as <- "abc"
    bs <- "123"
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a <= bs) == f  &
        (b <= as) == t  &
        (b <= bs) == t  &
        (a <= as) == t)
}
RUnit_arr_scalar_string_lt <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array("abc", c(2,3,4), dimnames=dnames)
    b <- array("123", c(2,3,4), dimnames=dnames)
    as <- "abc"
    bs <- "123"
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a < b) == f  &
        (b < a) == t  &
        (b < b) == f  &
        (a < a) == f)
} 
RUnit_arr_scalar_string_eq <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array("abc", c(2,3,4), dimnames=dnames)
    b <- array("123", c(2,3,4), dimnames=dnames)
    as <- "abc"
    bs <- "123"
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a == bs) == f  &
        (b == bs) == t  &
        (a == as) == t)
}
RUnit_arr_scalar_string_ne <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array("abc", c(2,3,4), dimnames=dnames)
    b <- array("123", c(2,3,4), dimnames=dnames)
    as <- "abc"
    bs <- "123"
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a != bs) == t  &
        (b != bs) == f  &
        (a != as) == f)
}
RUnit_arr_scalar_string_ge <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array("abc", c(2,3,4), dimnames=dnames)
    b <- array("123", c(2,3,4), dimnames=dnames)
    as <- "abc"
    bs <- "123"
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a >= bs) == t  &
        (b >= as) == f  &
        (b >= bs) == t  &
        (a >= as) == t)
}
RUnit_arr_scalar_string_gt <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array("abc", c(2,3,4), dimnames=dnames)
    b <- array("123", c(2,3,4), dimnames=dnames)
    as <- "abc"
    bs <- "123"
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a > bs) == t  &
        (b > as) == f  &
        (b > bs) == f  &
        (a > as) == f)
} 
RUnit_arr_scalar_string_undefined_ops <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array("abc", c(2,3,4), dimnames=dnames)
    b <- "123"
    tryCatch(a +  b, "error") == "error" &
    tryCatch(a -  b, "error") == "error" &
    tryCatch(a *  b, "error") == "error" &
    tryCatch(a /  b, "error") == "error" &
    tryCatch(a ^  b, "error") == "error" &
    tryCatch(a &  b, "error") == "error" &
    tryCatch(a && b, "error") == "error" &
    tryCatch(a |  b, "error") == "error" &
    tryCatch(a || b, "error") == "error" &
    tryCatch(a :  b, "error") == "error"
}

## time
RUnit_arr_scalar_time_minus <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(|.2015-03-09 06:38:02 America/New_York.|, c(2,3,4), dimnames=dnames)
    b <- array(|.2015-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    as <- |.2015-03-09 06:38:02 America/New_York.|
    bs <- |.2015-03-09 06:38:01 America/New_York.|
    res <- array(as.duration(1e9), c(2,3,4), dimnames=dnames);
    all((a - bs) == res  &
        (b - as) == -res)
}
RUnit_arr_scalar_time_lt <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(|.2015-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    b <- array(|.2014-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    as <- |.2015-03-09 06:38:01 America/New_York.|
    bs <- |.2014-03-09 06:38:01 America/New_York.|
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a < b) == f  &
        (b < a) == t  &
        (b < b) == f  &
        (a < a) == f)
} 
RUnit_arr_scalar_time_eq <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(|.2015-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    b <- array(|.2014-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    as <- |.2015-03-09 06:38:01 America/New_York.|
    bs <- |.2014-03-09 06:38:01 America/New_York.|
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a == bs) == f  &
        (b == bs) == t  &
        (a == as) == t)
}
RUnit_arr_scalar_time_ne <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(|.2015-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    b <- array(|.2014-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    as <- |.2015-03-09 06:38:01 America/New_York.|
    bs <- |.2014-03-09 06:38:01 America/New_York.|
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a != bs) == t  &
        (b != bs) == f  &
        (a != as) == f)
}
RUnit_arr_scalar_time_ge <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(|.2015-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    b <- array(|.2014-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    as <- |.2015-03-09 06:38:01 America/New_York.|
    bs <- |.2014-03-09 06:38:01 America/New_York.|
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a >= bs) == t  &
        (b >= as) == f  &
        (b >= bs) == t  &
        (a >= as) == t)
}
RUnit_arr_scalar_time_gt <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(|.2015-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    b <- array(|.2014-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    as <- |.2015-03-09 06:38:01 America/New_York.|
    bs <- |.2014-03-09 06:38:01 America/New_York.|
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a > bs) == t  &
        (b > as) == f  &
        (b > bs) == f  &
        (a > as) == f)
} 
RUnit_arr_scalar_time_undefined_ops <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(|.2015-03-09 06:38:01 America/New_York.|, c(2,3,4), dimnames=dnames)
    b <- |.2014-03-09 06:38:01 America/New_York.|
    tryCatch(a +  b, "error") == "error" &
    tryCatch(a *  b, "error") == "error" &
    tryCatch(a /  b, "error") == "error" &
    tryCatch(a ^  b, "error") == "error" &
    tryCatch(a &  b, "error") == "error" &
    tryCatch(a && b, "error") == "error" &
    tryCatch(a |  b, "error") == "error" &
    tryCatch(a || b, "error") == "error" &
    tryCatch(a :  b, "error") == "error"
}

## interval
RUnit_arr_scalar_interval_le <- function() {
    as <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    bs <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as, c(2,3,4), dimnames=dnames)
    b <- array(bs, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a <= bs) == f  &
        (b <= as) == t  &
        (b <= bs) == t  &
        (a <= as) == t)
}
RUnit_arr_scalar_interval_lt <- function() {
    as <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    bs <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as, c(2,3,4), dimnames=dnames)
    b <- array(bs, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a < bs) == f  &
        (b < as) == t  &
        (b < bs) == f  &
        (a < as) == f)
} 
RUnit_arr_scalar_interval_eq <- function() {
    as <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    bs <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as, c(2,3,4), dimnames=dnames)
    b <- array(bs, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a == bs) == f  &
        (b == bs) == t  &
        (a == as) == t)
}
RUnit_arr_scalar_interval_ne <- function() {
    as <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    bs <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as, c(2,3,4), dimnames=dnames)
    b <- array(bs, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a != bs) == t  &
        (b != bs) == f  &
        (a != as) == f)
}
RUnit_arr_scalar_interval_ge <- function() {
    as <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    bs <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as, c(2,3,4), dimnames=dnames)
    b <- array(bs, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a >= bs) == t  &
        (b >= as) == f  &
        (b >= bs) == t  &
        (a >= as) == t)
}
RUnit_arr_scalar_interval_gt <- function() {
    as <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    bs <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as, c(2,3,4), dimnames=dnames)
    b <- array(bs, c(2,3,4), dimnames=dnames)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a > bs) == t  &
        (b > as) == f  &
        (b > bs) == f  &
        (a > as) == f)
} 
RUnit_arr_scalar_interval_undefined_ops <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    as <-|-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    a <- array(as, c(2,3,4), dimnames=dnames)
    b <-|+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|
    tryCatch(a +  b, "error") == "error" &
    tryCatch(a -  b, "error") == "error" &
    tryCatch(a *  b, "error") == "error" &
    tryCatch(a /  b, "error") == "error" &
    tryCatch(a ^  b, "error") == "error" &
    tryCatch(a &  b, "error") == "error" &
    tryCatch(a && b, "error") == "error" &
    tryCatch(a |  b, "error") == "error" &
    tryCatch(a || b, "error") == "error" &
    tryCatch(a :  b, "error") == "error"
}

## duration
RUnit_arr_scalar_duration_add <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(1),  c(2,3,4), dimnames=dnames)
    b <- array(as.duration(-1), c(2,3,4), dimnames=dnames)
    d <- array(as.duration(2),  c(2,3,4), dimnames=dnames)
    e <- array(as.duration(3),  c(2,3,4), dimnames=dnames)
    bs <- as.duration(-1)
    ds <- as.duration(2)
    all((a + ds) == e  &
        (e + bs) == d)
}
RUnit_arr_scalar_duration_sub <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(1),  c(2,3,4), dimnames=dnames)
    b <- array(as.duration(-1), c(2,3,4), dimnames=dnames)
    d <- array(as.duration(2),  c(2,3,4), dimnames=dnames)
    e <- array(as.duration(3),  c(2,3,4), dimnames=dnames)
    bs <- as.duration(-1)
    ds <- as.duration(2)
    all((e - ds) == a  &
        (d - bs) == e)
}
RUnit_arr_scalar_duration_mul <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(2),  c(2,3,4), dimnames=dnames)
    b <- array(3,  c(2,3,4), dimnames=dnames)
    d <- array(as.duration(6),  c(2,3,4), dimnames=dnames)
    as <- as.duration(2)
    bs <- 3
    all((a * bs) == d  &
        (b * as) == d)
}
RUnit_arr_scalar_duration_div <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(2),  c(2,3,4), dimnames=dnames)
    b <- array(3,  c(2,3,4), dimnames=dnames)
    d <- array(as.duration(6),  c(2,3,4), dimnames=dnames)
    all((d / 3) == a) & tryCatch(b / as.duration(6), "error") == "error"
}
RUnit_arr_scalar_duration_div_byzero<- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(2),  c(2,3,4), dimnames=dnames)
    tryCatch(d / 0, "error") == "error"
}
RUnit_arr_scalar_duration_div_bynan <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(2),  c(2,3,4), dimnames=dnames)
    tryCatch(d / NaN, "error") == "error"
}
RUnit_arr_scalar_duration_div_byinf <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(2),  c(2,3,4), dimnames=dnames)
    tryCatch(d / Inf, "error") == "error"
}
RUnit_arr_scalar_duration_le <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(1), c(2,3,4), dimnames=dnames)
    b <- array(as.duration(0), c(2,3,4), dimnames=dnames)
    as <- as.duration(1)
    bs <- as.duration(0)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a <= bs) == f  &
        (b <= as) == t  &
        (b <= bs) == t  &
        (a <= as) == t)
}
RUnit_arr_scalar_duration_lt <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(1), c(2,3,4), dimnames=dnames)
    b <- array(as.duration(0), c(2,3,4), dimnames=dnames)
    as <- as.duration(1)
    bs <- as.duration(0)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a < bs) == f  &
        (b < as) == t  &
        (b < bs) == f  &
        (a < as) == f)
} 
RUnit_arr_scalar_duration_eq <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(1), c(2,3,4), dimnames=dnames)
    b <- array(as.duration(0), c(2,3,4), dimnames=dnames)
    as <- as.duration(1)
    bs <- as.duration(0)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a == bs) == f  &
        (b == bs) == t  &
        (a == as) == t)
}
RUnit_arr_scalar_duration_ne <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(1), c(2,3,4), dimnames=dnames)
    b <- array(as.duration(0), c(2,3,4), dimnames=dnames)
    as <- as.duration(1)
    bs <- as.duration(0)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a != bs) == t  &
        (b != bs) == f  &
        (a != as) == f)
}
RUnit_arr_scalar_duration_ge <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(1), c(2,3,4), dimnames=dnames)
    b <- array(as.duration(0), c(2,3,4), dimnames=dnames)
    as <- as.duration(1)
    bs <- as.duration(0)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a >= bs) == t  &
        (b >= as) == f  &
        (b >= bs) == t  &
        (a >= as) == t)
}
RUnit_arr_scalar_duration_gt <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(1), c(2,3,4), dimnames=dnames)
    b <- array(as.duration(0), c(2,3,4), dimnames=dnames)
    as <- as.duration(1)
    bs <- as.duration(0)
    t <- array(TRUE, c(2,3,4), dimnames=dnames);
    f <- !t
    all((a > bs) == t  &
        (b > as) == f  &
        (b > bs) == f  &
        (a > as) == f)
}
RUnit_arr_scalar_duration_undefined_ops <- function() {
    dnames <- list(c("un","deux"), c("uno","due","tre"), c("unu","doi","trei","patru"))
    a <- array(as.duration(1), c(2,3,4), dimnames=dnames)
    b <- as.duration(0)
    tryCatch(a *  b, "error") == "error" &
    tryCatch(a /  b, "error") == "error" &
    tryCatch(a ^  b, "error") == "error" &
    tryCatch(a &  b, "error") == "error" &
    tryCatch(a && b, "error") == "error" &
    tryCatch(a |  b, "error") == "error" &
    tryCatch(a || b, "error") == "error" &
    tryCatch(a :  b, "error") == "error"
}


## zts
## helper function to quickly build a zts:
getts <- function(a) {
    idx <- c(|.2015-03-09 06:38:01 America/New_York.|,
             |.2015-03-09 06:38:02 America/New_York.|,
             |.2015-03-09 06:38:03 America/New_York.|)
    zts(idx, matrix(a, nrow(a), ncol(a), dimnames=list(NULL, c("one", "two"))))
}
RUnit_zts_add <- function() {
    all(getts(matrix(1, 3, 2)) +  2 == getts(matrix(3, 3, 2)) &
        getts(matrix(1, 3, 2)) + -2 == getts(matrix(-1, 3, 2)) &
        2 +  getts(matrix(1, 3, 2)) == getts(matrix( 3, 3, 2)) &
        1 + getts(-matrix(2, 3, 2)) == getts(matrix(-1, 3, 2)) &
        getts(matrix(1, 3, 2)) + matrix( 2, 3, 2)  == getts(matrix( 3, 3, 2)) &
        matrix(1, 3, 2) + getts(matrix(-2, 3, 2))  == getts(matrix(-1, 3, 2)) & 
        getts(matrix(1, 3, 2)) + getts(matrix(-2, 3, 2))  == getts(matrix(-1, 3, 2)))
}
RUnit_zts_sub <- function() {
    all(getts(matrix(1, 3, 2)) -  2 == getts(matrix(-1, 3, 2)) &
        getts(matrix(1, 3, 2)) - -2 == getts(matrix( 3, 3, 2)) &
        2 - getts(matrix(1, 3, 2))  == getts(matrix( 1, 3, 2)) &
        1 - getts(-matrix(2, 3, 2)) == getts(matrix( 3, 3, 2)) &
        getts(matrix(1, 3, 2)) - matrix( 2, 3, 2)  == getts(matrix(-1, 3, 2)) &
        matrix(1, 3, 2) - getts(matrix(-2, 3, 2))  == getts(matrix( 3, 3, 2)) & 
        getts(matrix(1, 3, 2)) - getts(matrix(2, 3, 2))  == getts(matrix(-1, 3, 2)))
}
RUnit_zts_mul <- function() {
    all(getts(matrix(1, 3, 2)) *  2 == getts(matrix( 2, 3, 2)) &
        getts(matrix(1, 3, 2)) * -2 == getts(matrix(-2, 3, 2)) &
        2 * getts(matrix(1, 3, 2))  == getts(matrix( 2, 3, 2)) &
        1 * getts(-matrix(2, 3, 2)) == getts(matrix(-2, 3, 2)) &
        getts(matrix(1, 3, 2)) * matrix( 2, 3, 2)  == getts(matrix( 2, 3, 2)) &
        matrix(1, 3, 2) * getts(matrix(-2, 3, 2))  == getts(matrix(-2, 3, 2)) & 
        getts(matrix(4, 3, 2)) * getts(matrix(2, 3, 2))  == getts(matrix(8, 3, 2)))
}
RUnit_zts_div <- function() {
    all(getts(matrix(1, 3, 2)) /  2 == getts(matrix( 0.5, 3, 2)) &
        getts(matrix(1, 3, 2)) / -2 == getts(matrix(-0.5, 3, 2)) &
        2 / getts(matrix(1, 3, 2))  == getts(matrix( 2  , 3, 2)) &
        1 / getts(-matrix(2, 3, 2)) == getts(matrix(-0.5, 3, 2)) &
        getts(matrix(1, 3, 2)) / matrix( 2, 3, 2)  == getts(matrix( 0.5, 3, 2)) &
        matrix(1, 3, 2) / getts(matrix(-2, 3, 2))  == getts(matrix(-0.5, 3, 2)) & 
        getts(matrix(1, 3, 2)) / getts(matrix(2, 3, 2))  == getts(matrix(0.5, 3, 2)))
}
RUnit_zts_pow <- function() {
    all(getts(matrix(1, 3, 2)) -  2 == getts(matrix(-1, 3, 2)) &
        getts(matrix(1, 3, 2)) - -2 == getts(matrix( 3, 3, 2)) &
        2 - getts(matrix(1, 3, 2))  == getts(matrix( 1, 3, 2)) &
        1 - getts(-matrix(2, 3, 2)) == getts(matrix( 3, 3, 2)) &
        getts(matrix(1, 3, 2)) - matrix( 2, 3, 2)  == getts(matrix(-1, 3, 2)) &
        matrix(1, 3, 2) - getts(matrix(-2, 3, 2))  == getts(matrix( 3, 3, 2)) & 
        getts(matrix(1, 3, 2)) - getts(matrix(2, 3, 2))  == getts(matrix(-1, 3, 2)))
}
## list
RUnit_list_equal <- function() {
    ## the following will take each value in the list on the left and
    ## compare it to the value on the right:
    ## note that in R we get:
    ## Error in list(1, 2) == list(1, 2) : 
    ##   comparison of these types is not implemented
    all((2 == list(list(1,2),2)) == c(FALSE, TRUE) &
        (list(1,2) == 2) == c(FALSE, TRUE) & 
        (list(1,2) == 3) == c(FALSE, FALSE) &
        ## in R the following returns:
        ##   [1] FALSE    NA
        (list(1, list(2)) == 2) == c(FALSE, FALSE))
}
RUnit_list_list_equal <- function() {
    tryCatch(list(1,2) == list(1,2), "error") == "error" &
    tryCatch(list(list(1,2),2) == list(1,2), "error") == "error"
}
RUnit_list_nequal <- function() {
    all((1 != list(1,2)) == c(FALSE, TRUE) &
        (list(1,2) != 2) == c(TRUE, FALSE) &
        (list(1,2) != 3) == c(TRUE, TRUE) &
        ## in R the following returns:
        ##   [1] FALSE    NA
        (list(1, list(2)) != 2) == c(TRUE, TRUE))
}
RUnit_list_list_nequal <- function() {
    tryCatch(list(1,2) != list(1,2), "error") == "error" &
    tryCatch(list(list(1,2),2) != list(1,2), "error") == "error"
}
