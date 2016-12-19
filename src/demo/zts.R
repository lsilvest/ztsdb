t1 <- |.2015-03-09 06:38:01 America/New_York.|
t2 <- |.2015-03-09 06:38:10 America/New_York.|
tidx <- t1:t2
data <- 1.0:30.0
z <<- zts(tidx, data, dimnames=list(NULL, c("a", "b", "c")))
