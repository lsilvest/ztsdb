\name{ztsdb-package}
\alias{ztsdb-package}
\alias{ztsdb}
\docType{package}
\title{
ztsdb connectivity
}
\description{
  Provides connectivity with the 'ztsdb' database management system. This
  connectivity is provided with the S4 class \code{connection} and the
  overload of the \code{?} operator for this new class.
}
\details{
  
}
\author{
  Leonardo Silvestri
  
  Maintainer: lsilvestri@ztsdb.org
}
\references{
  Additional information about this package and 'ztsdb' can be found at
  \url{https://www.ztsdb.org}
}
\keyword{ package }
\keyword{ timeseries }
\examples{
## create a connection:
c1 <- connection("127.0.0.1", port=18000)
## get the tail of remote object 'a'
c1 ? tail(a)
}
