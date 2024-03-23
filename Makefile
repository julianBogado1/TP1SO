CC = gcc
CFLAGS = -Wall -g -lc

SRC = $(wildcard *.c)
EXEC = $(SRC:.c=)

all: $(EXEC)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(EXEC)
