## 
## 
runTestFile <- function(absFileName, verbose=FALSE)
{
    cat(paste0("------ ", absFileName, "\n"))

    ## LLL make these lists and test that each test returns a boolean scalar, if not, flag!!!
    testResults <- list(tname=NULL, result=NULL)

    source(absFileName)
    env <- ls()
    
    ## run .setUp() if it exists
    i <- 1
    while (i <= length(env)) {
        if (env[[i]] == ".setUp") {
            do.call(env[[i]], list())
        }
        i <- i + 1
    }

    ## for each item in env:
    i <- 1
    while (i <= length(env)) {
        f <- env[[i]]
        ##  if it's a function that starts with 'RUnit' put function
        ##  name and result of evaluation in testResults:
        if (substr(f, 1, 5) == "RUnit") {
            ## print(f)
            testResults$tname  <- c(testResults$tname, f)
            runResult <- tryCatch(do.call(f, list()), "ERROR")
            if (length(runResult) != 1 |
                (runResult != "ERROR" && runResult != TRUE && runResult != FALSE)) {
                print(paste(f, "has incorrect return type:"))
                print(runResult)
                runResult <- "ERROR"
            }
            testResults$result <- c(testResults$result, as.character(runResult))
        }
        i <- i + 1
    }
    if (is.null(testResults$tname)) {
        print("couldn't find any tests to run")
    }
    else {
        failed <- 0
        error  <- 0
        passed <- 0
        i <- 1
        while (i <= length(testResults$tname)) {
            if (testResults$result[i] == "ERROR") {
                error <- error + 1
                cat(paste0(testResults$tname[i], ": ", testResults$result[i], "\n"))
            }
            else if (testResults$result[i] == "FALSE") {
                failed <- failed + 1
                cat(paste0(testResults$tname[i], ": ", testResults$result[i], "\n"))
            }
            else {
                passed <- passed + 1
                if (verbose) {
                    cat(paste0(testResults$tname[i], ": ", testResults$result[i], "\n"))
                }
            }
            i <- i + 1
        }
        dummy <- cat(paste0("TOTAL:  ",  length(testResults$tname),  ", ",
                            "PASSED: ", passed, ", ",
                            "ERRORS: ", error,  ", ",
                            "FAILED: ", failed, "\n"))
    }

    ## run .tearDown() if it exists
    i <- 1
    while (i <= length(env)) {
        if (env[[i]] == ".tearDown") {
            do.call(env[[i]], list())
        }
        i <- i + 1
    }
}
