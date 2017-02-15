include ../src/Makefile.header

COREDIR =../src
COREDIR_TZ =$(COREDIR)/timezone
COREDIR_MAIN_PARSER =$(COREDIR)/main_parser
CPPFLAGS += -I$(COREDIR) -I$(COREDIR_MAIN_PARSER) -I. -fPIC -DZTSDB_CLIENT
LDFLAGS += -shared



# we want to compile in this directory in particular because we want
# the -fPIC option

%.o : $(COREDIR)/%.cpp
	@$(MAKEDEPEND); \
	  cp $(notdir $*).d $(notdir $*).P; \
	  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	      -e '/^$$/ d' -e 's/$$/ :/' < $(notdir $*).d >> $(notdir $*).P; \
	  rm -f $(notdir $*).d
	$(CPP) $(CPPFLAGS) -c -o $(notdir $@) $<

%.o : $(COREDIR_TZ)/%.cpp
	@$(MAKEDEPEND); \
	  cp $(notdir $*).d $(notdir $*).P; \
	  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	      -e '/^$$/ d' -e 's/$$/ :/' < $(notdir $*).d >> $(notdir $*).P; \
	  rm -f $(notdir $*).d
	$(CPP) $(CPPFLAGS) -c -o $(notdir $@) $<

%.o : $(COREDIR_MAIN_PARSER)/%.cpp
	@$(MAKEDEPEND); \
	  cp $(notdir $*).d $(notdir $*).P; \
	  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	      -e '/^$$/ d' -e 's/$$/ :/' < $(notdir $*).d >> $(notdir $*).P; \
	  rm -f $(notdir $*).d
	$(CPP) $(CPPFLAGS) -c -o $(notdir $@) $<

SRCS = parser_ctx.cpp anf.cpp array.cpp ast.cpp valuevar.cpp		\
	dname.cpp net_handler.cpp encode.cpp misc.cpp zts.cpp		\
	display.cpp conversion_funcs.cpp string.cpp base_types.cpp	\
	unop_binop_funcs.cpp config.cpp zcpp.cpp zcpp_zts.cpp		\
	period.cpp net_client.cpp

SRCS_TZ = ztime.cpp ztime_vector.cpp zone.cpp localtime.cpp
SRCS_MAIN_PARSER = lexer.cpp parser.cpp

OBJS = $(SRCS:.cpp=.o) $(SRCS_TZ:.cpp=.o) $(SRCS_MAIN_PARSER:.cpp=.o)

libztsdb: $(OBJS)
	$(CPP) $(CPPFLAGS) -o $@.so $(OBJS) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f *.o libztsdb.so


-include $(SRCS:.cpp=.P)
-include $(SRCS_TZ:.cpp=.P)
-include $(SRCS_MAIN_PARSER:.cpp=.P)
