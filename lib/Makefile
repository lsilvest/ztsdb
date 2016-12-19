#CC = clang++ 
#CFLAGS = -O3 -std=c++1y -stdlib=libstdc++
LDFLAGS = -lstdc++ -shared
CC = g++
CFLAGS = -g -std=c++1y -fPIC -I../interp
MAKEDEPEND = g++ -M $(CFLAGS) -o $*.d $<


# following is needed to generate the dependencies with make depend
SRCS = sotest.cpp

.cpp.o:
	@$(MAKEDEPEND); \
	  cp $*.d $*.P; \
	  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	      -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
	  rm -f $*.d
	$(CC) $(CFLAGS) -c -o $@ $<

libtest: $(SRCS:.cpp=.o)
	$(CC) $(CFLAGS) -o $@.so $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f *.o libtest.so
