include ../../src/Makefile.header

LDFLAGS += -L/home/lsilvest/repos/ztsdb/shlib_client -lztsdb_client

COREDIR =../../src
CPPFLAGS += -I$(COREDIR)


.c.o :
	@$(MAKEDEPEND); \
	  cp $(notdir $*).d $(notdir $*).P; \
	  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	      -e '/^$$/ d' -e 's/$$/ :/' < $(notdir $*).d >> $(notdir $*).P; \
	  rm -f $(notdir $*).d
	$(CC) $(CFLAGS) -c -o $(notdir $@) $<

CFLAGS += -I$(COREDIR)

OBJS = simple_append.o $(BASEOBJS)

simple_append: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f *.o simple_append

-include simple_append.P
