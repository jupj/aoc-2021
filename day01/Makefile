SOURCES = day01.c
BIN = $(BUILD)day01
CC = gcc
CFLAGS = -Wall
BUILD = ../build/

default: run

run: $(BIN)
	@$(BIN)

$(BIN): $(SOURCES)
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	@rm -f $(BIN)
