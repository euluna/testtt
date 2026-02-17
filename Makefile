

# Makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -pthread
SRC = test.c spinlock.c mymutex.c mysemaphore.c mycondvar.c mybarrier.c
EXEC = test

all: $(EXEC)

$(EXEC): $(SRC)
	$(CC) $(CFLAGS) -o $(EXEC) $(SRC)

clean:
	rm -f $(EXEC)

.PHONY: all clean