CC = gcc
CFLAGS = -Wall -g -std=c99 

SRC = $(wildcard *.c)
EXEC = $(SRC:.c=)

all: $(EXEC)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(EXEC)
