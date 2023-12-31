INC_DIR=src
CC := h5cc
CFLAGS := -g -Wall -Wextra -Wpedantic -I $(INC_DIR) -I /usr/include/hdf5/serial # second -I is for Ubuntu
SRC=src
OBJ=obj
SRCS=$(wildcard $(SRC)/*.c)
OBJS=$(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))

TEST=tests
TESTS=$(wildcard $(TEST)/*.c)
TESTBINS=$(patsubst $(TEST)/%.c, $(TEST)/bin/%, $(TESTS))

LIBDIR=lib
LIB = $(LIBDIR)/hdf5extention.a

all:$(LIB)

release: CFLAGS=-Wall -O2 -DNDEBUG
release: clean
release: $(LIB)

$(LIB): $(LIBDIR) $(OBJ) $(OBJS)
	# this makes a static library
	$(RM) $(LIB)
	ar -cvrs $(LIB) $(OBJS)

$(OBJ)/%.o: $(SRC)/%.c $(SRC)/%.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ):
	mkdir $@

$(LIBDIR):
	mkdir $@

$(TEST)/bin:
	mkdir $@
	mkdir -p compressed_files


$(TEST)/bin/%: $(TEST)/bin/%.o
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ -lcriterion

$(TEST)/bin/%.o: $(TEST)/%.c
	$(CC) $(CFLAGS) $< $(OBJS) -c -o $@ -lcriterion

tests: $(LIB) $(TEST)/bin $(TESTBINS)
	for test in $(TESTBINS) ; do ./$$test 100000 --jobs 0; done

testscompile: $(LIB) $(TEST)/bin $(TESTBINS)
	@echo "Compiled all tests"


testvalgrind: $(LIB) $(TEST)/bin $(TESTBINS)
	valgrind --tool=callgrind --dump-instr=yes --simulate-cache=yes --collect-jumps=yes --collect-atstart=no --instr-atstart=no --trace-children=yes --callgrind-out-file=callgrind_%p.out ./tests/bin/test_compression #--filter "*WriteZerosDifferentChunkSizes*" --jobs 1

testdebug: $(LIB) $(TEST)/bin $(TESTBINS)
	for chunk in 100 10000 1000000; do valgrind --tool=massif --massif-out-file=massif_chunksize_$$chunk.%p.out ./tests/bin/test_compression $$chunk ;done

testdatatype: $(LIB) $(TEST)/bin $(TESTBINS)
	./tests/bin/test_H5Datatype 

clean:
	$(RM) -r $(LIBDIR) $(OBJ) $(TEST)/bin callgrind_* massif* testfiles compressed_files
