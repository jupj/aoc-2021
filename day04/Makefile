SOURCES = day04.c
BIN = $(BUILD)day04

INPUTFILE = input.txt
TESTFILE = test_input.txt
EXPETCTED = test_expected.txt
CC = gcc
CFLAGS = -Wall
BUILD = ../build/

default: run

.PHONY: test
test: $(BIN)
	@bash -c 'diff -s $(EXPETCTED) <($(BIN) $(TESTFILE))'

.PHONY: run
run: $(BIN)
	@$(BIN) $(INPUTFILE)

$(BIN): $(SOURCES)
	@mkdir -p $(BUILD)
	@$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	@rm -f $(BIN)
