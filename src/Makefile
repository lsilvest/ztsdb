# -*- mode: makefile -*-

include Makefile.header
include Makefile.rules

# following is needed to generate the dependencies with make depend
SRCS = main_parser/lexer.cpp main_parser/parser.cpp			\
	config_parser/cfglexer.cpp config_parser/cfgparser.cpp		\
	parser_ctx.cpp anf.cpp array.cpp interp.cpp ast.cpp		\
	load_builtin.cpp valuevar.cpp valuevar_ic.cpp dname.cpp		\
	net_handler.cpp msg_handler.cpp interp_ctx.cpp ztsdb.cpp	\
	encode.cpp misc.cpp zts.cpp display.cpp base_funcs.cpp		\
	base_funcs_math.cpp base_funcs_array.cpp			\
	base_funcs_array_idx.cpp base_funcs_set.cpp base_funcs_ic.cpp	\
	base_funcs_roll.cpp conversion_funcs.cpp csv.cpp string.cpp	\
	base_types.cpp timezone/ztime.cpp timezone/zone.cpp		\
	timezone/ztime_vector.cpp timezone/localtime.cpp		\
	unop_binop_funcs.cpp config_ctx.cpp config.cpp			\
	interp_error.cpp zcpp.cpp period.cpp
CSRCS = cmdline.c
OBJS =  $(CSRCS:.c=.o) $(SRCS:.cpp=.o)

GIT_VERSION := $(shell git describe --abbrev=6 --dirty --always --tags)


.PHONY: ztsdb_inner
ztsdb_inner: $(OBJS)
	$(CPP) $(CPPFLAGS) -o ztsdb $^ $(LDFLAGS)


.PHONY: gengetopt
gengetopt: cmdline.ggo
	gengetopt --set-version=$(GIT_VERSION) -i cmdline.ggo --long-help --show-required

.PHONY: mparser
mparser:
	$(MAKE) -C ./main_parser parser
	$(MAKE) -C ./main_parser lexer

.PHONY: cparser
cparser:
	$(MAKE) -C ./config_parser cfgparser
	$(MAKE) -C ./config_parser cfglexer

ztsdb: mparser cparser gengetopt
	$(MAKE) ztsdb_inner

libztsdb: 
	$(MAKE) -C ./shlib libztsdb


.PHONY: clean
clean:
	rm -f *.o *.P ztsdb
	rm -f timezone/*.o
	$(MAKE) -C ./main_parser clean
	$(MAKE) -C ./config_parser clean
	$(MAKE) -C ./shlib clean

-include $(CSRCS:.c=.P)
-include $(SRCS:.cpp=.P)

