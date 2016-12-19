## To run a particular test (or a particulra set of tests) the PATTERN
## variable can be set on the command line like this:
## 'make rtest PATTERN=runit_closure.R'   

ZTSDB = ../src/ztsdb
PATTERN = runit*.R

l := (
r := )
q := '
qq := "
s := ;
RUNIT_FILE_DIR := $(shell ls $(PATTERN))
RUNIT_FILES_P := $(addprefix $(q)source$(l)$(qq)RUnitMini.R$(qq)$(r)$(s)dummy<-runTestFile$(l)$(qq), ${RUNIT_FILE_DIR})
RUNIT_FILES   := $(addsuffix $(qq)$(r)$(q), ${RUNIT_FILES_P})
.PHONY: ${RUNIT_FILES}

${RUNIT_FILES}: ; $(foreach file, $@, @${ZTSDB} -e $(file);)

.PHONY: rtest
rtest: ${RUNIT_FILES} ; @echo rtest finished


