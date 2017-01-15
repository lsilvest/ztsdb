include ../../src/Makefile.header

CPP = clang++-3.6
CPPFLAGS = -O3 -std=c++1y -I $(DOUBLE-CONVERSION-INCLUDE) -Wall -stdlib=libstdc++ -Wno-deprecated-register


COREDIR =../../src
COREDIR_TZ =../../src/timezone
CPPFLAGS += -I$(COREDIR)

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

.cpp.o :
	@$(MAKEDEPEND); \
	  cp $(notdir $*).d $(notdir $*).P; \
	  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	      -e '/^$$/ d' -e 's/$$/ :/' < $(notdir $*).d >> $(notdir $*).P; \
	  rm -f $(notdir $*).d
	$(CPP) $(CPPFLAGS) -c -o $(notdir $@) $<

CPPFLAGS += -I$(COREDIR)

SRCS = array.cpp dname.cpp zts.cpp zcpp.cpp period.cpp misc.cpp
SRCS_TZ = ztime.cpp ztime_vector.cpp zone.cpp localtime.cpp

BASEOBJS = $(SRCS:.cpp=.o) $(SRCS_TZ:.cpp=.o)

OBJS = append.o $(BASEOBJS)

append: $(OBJS)
	$(CPP) $(CPPFLAGS) -o $@ $(OBJS) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f *.o append

-include append.P
-include $(SRCS:.cpp=.P)
-include $(SRCS_TZ:.cpp=.P)
