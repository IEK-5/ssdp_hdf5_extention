# There might be an easier way but at least this works
# This is a makefile for example programs where each program just requires one c file to produce an executable
# Source code is in SRC_DIR, temporary objects files are in OBJ_DIR and the executables are in BIN_DIR

CC = h5cc
CFLAGS = -g -Wall
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin



names := $(shell find $(SRC_DIR) -type f -iname "*.c" -print | sed 's/\.c//g' | sed 's/src\///g')
SRCS := $(patsubst %,$(SRC_DIR)/%.c,$(names))
OBJS := $(patsubst %,$(OBJ_DIR)/%.o,$(names))
BINS := $(patsubst %,$(BIN_DIR)/%,$(names))


all : $(BINS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

$(BIN_DIR)/%: $(OBJ_DIR)/%.o
	$(CC) $(CFLAGS) $< -o $@


clean::
	find bin/ -type f -exec rm -f {} \;
	find . -type f -iname 'file*.h5' -exec rm -f {} \;