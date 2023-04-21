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

# this puts object files into the root I thought they were "intermediate"
$(TEST)/bin/%: $(TEST)/%.c
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ -lcriterion

tests: $(LIB) $(TEST)/bin $(TESTBINS)
	for test in $(TESTBINS) ; do ./$$test ; done


clean:
	$(RM) -r $(LIBDIR) $(OBJ) $(TEST)/bin
	find . -type f -iname 'file*.h5' -exec rm -f {} \;