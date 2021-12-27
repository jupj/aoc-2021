SOURCES = day22.c utils.c
BIN = $(BUILD)day22

HEADERS = utils.h
INPUTFILE = input.txt
TESTFILE = test_input.txt
EXPETCTED = test_expected.txt
CC = gcc
CFLAGS = -Wall -Werror -g
BUILD = ../build/

default: run

.PHONY: test
test: $(BIN)
	@bash -c 'diff -s $(EXPETCTED) <($(BIN) $(TESTFILE))'

.PHONY: run
run: $(BIN)
	@$(BIN) $(INPUTFILE)

.PHONY: check
check: $(BIN)
	valgrind --leak-check=yes $(BIN) $(INPUTFILE)

$(BIN): $(SOURCES) $(HEADERS)
	@mkdir -p $(BUILD)
	@$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean
clean:
	@rm -f $(BIN)
