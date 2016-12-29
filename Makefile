ztsdb:
	$(MAKE) -C ./src ztsdb


.PHONY: anf
anf: ztsdb
	cd ./tests/anf           && $(MAKE) -s test
.PHONY: ast
ast: ztsdb
	cd ./tests/ast           && $(MAKE) -s test
.PHONY: config
config: ztsdb
	cd ./tests/config        && $(MAKE) -s test
.PHONY: cow_ptr
cow_ptr: ztsdb
	cd ./tests/cow_ptr       && $(MAKE) -s test
.PHONY: zts
zts: ztsdb
	cd ./tests/zts           && $(MAKE) -s test
.PHONY: duration
duration: ztsdb
	cd ./tests/duration      && $(MAKE) -s test
.PHONY: time
time: ztsdb
	cd ./tests/time          && $(MAKE) -s test
.PHONY: period
period: ztsdb
	cd ./tests/period        && $(MAKE) -s test
.PHONY: zstring
zstring: ztsdb
	cd ./tests/zstring       && $(MAKE) -s test
.PHONY: vector
vector: ztsdb
	cd ./tests/vector        && $(MAKE) -s test
.PHONY: vector_set
vector_set: ztsdb
	cd ./tests/vector_set    && $(MAKE) -s test
.PHONY: vector_bool
vector_bool: ztsdb
	cd ./tests/vector_bool   && $(MAKE) -s test
.PHONY: array
array: ztsdb
	cd ./tests/array         && $(MAKE) -s test
.PHONY: array_time
array_time: ztsdb
	cd ./tests/array_time    && $(MAKE) -s test
.PHONY: array_bool
array_bool: ztsdb
	cd ./tests/array_bool    && $(MAKE) -s test
.PHONY: align
align: ztsdb
	cd ./tests/align         && $(MAKE) -s test
.PHONY: encode
encode: ztsdb
	cd ./tests/encode        && $(MAKE) -s test
.PHONY: interp_error
interp_error: ztsdb
	cd ./tests/interp_error  && $(MAKE) -s test
.PHONY: interp
interp: ztsdb
	cd ./tests/interp        && $(MAKE) -s test
.PHONY: interp_time
interp_time: ztsdb
	cd ./tests/interp_time   && $(MAKE) -s test
.PHONY: binds
binds: ztsdb
	cd ./tests/binds         && $(MAKE) -s test
.PHONY: control
control: ztsdb
	cd ./tests/control       && $(MAKE) -s test
.PHONY: mmap
mmap: ztsdb
	cd ./tests/mmap          && $(MAKE) -s test
.PHONY: csv
csv: ztsdb
	cd ./tests/csv           && $(MAKE) -s test
.PHONY: comm_append
comm_append: ztsdb
	cd ./tests/comm_append   && $(MAKE) -s test
.PHONY: comm
comm: ztsdb
	cd ./tests/comm          && $(MAKE) -s test
.PHONY: display
display: ztsdb
	cd ./tests/display          && $(MAKE) -s test


.PHONY: test
test: anf ast config period cow_ptr zts duration time period zstring vector vector_set vector_bool array array_time array_bool align encode interp_error interp interp_time binds control mmap csv comm_append comm display


.PHONY: rtest
rtest:
	@$(MAKE) -s -C ./Rtests rtest

.PHONY: clean
clean:
	rm -rf ./build/*
	$(MAKE) -C ./src clean
	$(MAKE) -C ./src/shlib clean
	$(MAKE) -C ./lib clean
	$(MAKE) -C ./itests/append clean
	$(MAKE) -C ./itests/client clean
	find ./tests -type f -name test -exec rm {} \;
	find ./tests -type f -name test.o -exec rm {} \;
	find ./tests -type f -name test.P -exec rm {} \;

