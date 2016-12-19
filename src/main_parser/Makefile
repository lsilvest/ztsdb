# -*- mode: makefile -*-

include ../Makefile.header
include ../Makefile.rules

CPPFLAGS += -I .. -I . -Wno-deprecated-register -Wno-unused-function -Wno-unneeded-internal-declaration

# the following are needed so that a direct build of parser.o and
# lexer.o does not invoke the default rules:
.PHONY: lexer
lexer: lexer.hpp lexer.cpp lexer.o

.PHONY: parser
parser: parser.hpp parser.cpp parser.o

.PHONY: clean
clean:
	rm -f *.o parser.[hc]pp lexer.[hc]pp *.hh

-include lexer.P
-include parser.P
