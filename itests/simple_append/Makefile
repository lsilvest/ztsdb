include ../../src/Makefile.header


COREDIR =../../src
CPPFLAGS += -I$(COREDIR)

%.o : $(COREDIR)/%.cpp
	@$(MAKEDEPEND); \
	  cp $(notdir $*).d $(notdir $*).P; \
	  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	      -e '/^$$/ d' -e 's/$$/ :/' < $(notdir $*).d >> $(notdir $*).P; \
	  rm -f $(notdir $*).d
	$(CPP) $(CPPFLAGS) -c -o $(notdir $@) $<

.cpp.o :
	@$(MAKEDEPEND); \
	  cp $(notdir $*).d $(notdir $*).P; \
	  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	      -e '/^$$/ d' -e 's/$$/ :/' < $(notdir $*).d >> $(notdir $*).P; \
	  rm -f $(notdir $*).d
	$(CPP) $(CPPFLAGS) -c -o $(notdir $@) $<

CPPFLAGS += -I$(COREDIR)

SRCS = zcpp.cpp misc.cpp

BASEOBJS = $(SRCS:.cpp=.o)

OBJS = simple_append.o $(BASEOBJS)

simple_append: $(OBJS)
	$(CPP) $(CPPFLAGS) -o $@ $(OBJS) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f *.o simple_append

-include simple_append.P
-include $(SRCS:.cpp=.P)
