CC ?= gcc
CFLAGS = -o2 -Wall -std=c11
LDFLAGS = -lgccjit
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)
BIN = ./bfc

.phony: clean all run

all: $(BIN)

run: all
	@$(BIN)

$(BIN): $(OBJS)
	$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS)

clean:
	@rm -fv $(BIN)
	@rm -fv $(OBJS)
