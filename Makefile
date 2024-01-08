CC = gcc
CFLAGS = -Wall -Werror -pedantic -std=c17 -O0
CFLAGS += -g -fsanitize=address -fno-omit-frame-pointer

SRCDIR = ./src
LIBSRC = $(wildcard $(SRCDIR)/*.c)
LIBOBJ = $(LIBSRC:.c=.o)
LIBHEADERS = $(wildcard $(SRCDIR)/*.h)

.PHONY: all clean final

all: test-debug

test-debug: test.o $(LIBOBJ)
	$(CC) $(CFLAGS) -o $@ $^

test.o: test.c $(LIBHEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBOBJ): $(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

final: CFLAGS := $(filter-out -O0,$(CFLAGS))
final: CFLAGS := $(filter-out -g,$(CFLAGS))
final: CFLAGS := $(filter-out -fsanitize=address,$(CFLAGS))
final: CFLAGS := $(filter-out -fno-omit-frame-pointer,$(CFLAGS))
final: CFLAGS += -O2

final: test-debug
	mv test-debug test

clean:
	rm -f test test-debug *.o $(SRCDIR)/*.o
