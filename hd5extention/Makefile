INC_DIR=src
CC := h5cc
CFLAGS := -g -Wall -I $(INC_DIR)
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


$(TEST)/bin/%: $(TEST)/bin/%.o
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ -lcriterion

$(TEST)/bin/%.o: $(TEST)/%.c
	$(CC) $(CFLAGS) $< $(OBJS) -c -o $@ -lcriterion

tests: $(LIB) $(TEST)/bin $(TESTBINS)
	for test in $(TESTBINS) ; do ./$$test --jobs 0; done

testdebug: $(LIB) $(TEST)/bin $(TESTBINS)
	./tests/bin/test_H5FileIO --filter "*read_array_exists*"

clean:
	$(RM) -r $(LIBDIR) $(OBJ) $(TEST)/bin