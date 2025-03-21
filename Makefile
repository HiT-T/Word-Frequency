CC = gcc
CFLAGS = -Wall -Wextra -std=gnu99

all: outlier

outlier.o: outlier.c
	$(CC) $(CFLAGS) -c $< -o $@

outlier: outlier.o
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f outlier outlier.o