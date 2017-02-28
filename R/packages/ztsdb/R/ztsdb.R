setGeneric("?", function(e1, e2) standardGeneric("?"))
setMethod("?",
          signature("Rcpp_Zconnection"),
          function (e1, e2) {
              e1$query(paste(deparse(substitute(e2)), collapse="\n"), substitute(e2), parent.frame())
          })

connection <- function(ip, port) {
    new(Zconnection, ip, port)
}
