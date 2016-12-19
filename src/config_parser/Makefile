# -*- mode: makefile -*-

include ../Makefile.header
include ../Makefile.rules

CPPFLAGS +=  -Wno-deprecated-register -Wno-unused-function -Wno-unneeded-internal-declaration

.PHONY: cfglexer
cfglexer: cfglexer.hpp cfglexer.cpp cfglexer.o

.PHONY: cfgparser
cfgparser: cfgparser.hpp cfgparser.cpp cfgparser.o

.PHONY: clean
clean:
	rm -f *.o *.[hc]pp *.hh

-include cfglexer.P
-include cfgparser.P
