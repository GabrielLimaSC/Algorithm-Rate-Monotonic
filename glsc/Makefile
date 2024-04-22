CC=gcc
CFLAGS=-Wall -Wextra -std=c11

all: rate edf

rate: real.c
	$(CC) $(CFLAGS) -o rate real.c -DRATE_MONOTONIC

edf: real.c
	$(CC) $(CFLAGS) -o edf real.c -DEARLIEST_DEADLINE_FIRST

clean:
	rm -f rate edf