include ../src/Makefile.header

COREDIR =../src
CPPFLAGS += -I$(COREDIR) -I. -fPIC -DZTSDB_CLIENT
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

SRCS = misc.cpp zcpp.cpp 

OBJS = $(SRCS:.cpp=.o)

libztsdb_client: $(OBJS)
	$(CPP) $(CPPFLAGS) -o $@.so $(OBJS) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f *.o libztsdb_client.so


-include $(SRCS:.cpp=.P)
