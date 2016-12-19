include ../../src/Makefile.header

CPP = clang++-3.6
#CPPFLAGS = -g -std=c++1y -I $(DOUBLE-CONVERSION-INCLUDE) -Wall -stdlib=libstdc++ -Wno-deprecated-register -fPIC
LDFLAGS += -L/home/lsilvest/repos/ztsdb/src/shlib -lztsdb

COREDIR =../../src
CPPFLAGS += -I$(COREDIR)

.cpp.o :
	@$(MAKEDEPEND); \
	  cp $(notdir $*).d $(notdir $*).P; \
	  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	      -e '/^$$/ d' -e 's/$$/ :/' < $(notdir $*).d >> $(notdir $*).P; \
	  rm -f $(notdir $*).d
	$(CPP) $(CPPFLAGS) -c -o $(notdir $@) $<

OBJS = client.o

client: $(OBJS)
	$(CPP) $(CPPFLAGS) -o $@ $(OBJS) $(LDFLAGS)
#	valgrind ./$@ -t -v
#	./$@

.PHONY: clean
clean:
	rm -f *.o client

-include client.P
