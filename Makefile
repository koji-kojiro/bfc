CC ?= gcc
CFLAGS = -o2 -Wall -std=gnu11
LDFLAGS = -lgccjit
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)
PROG = ./bfc

.phony: clean all

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS)

clean:
	@rm -fv $(PROG)
	@rm -fv $(OBJS)
