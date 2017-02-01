\name{connection}
\alias{connection}
\title{Create a connection to a \code{ztsdb} instance}
\description{
Establish a TCP connection to a \code{ztsdb} instance. The TCP
connection will stay open as long as the connection object exists and
the remote instance is reachable.
}
\usage{
connection(ip, port)
}
\arguments{
\item{ip}{IP address of the \code{ztsdb} instance to connect to.}
\item{port}{port port number of the \code{ztsdb} instance to connect to.}
}
\value{
  \code{Zconnection} - S4 class connection handle.
}
\examples{
c1 <- connection("127.0.0.1", 20001)    # establish the connection
c1 ? tail(z)                            # ask the remote to evaluate tail(z)
}
\keyword{ztsdb}
\keyword{connection}
