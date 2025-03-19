CC = gcc
CFLAGS = -Wall -Wextra -std=c99

outlier.o: outlier.c
	$(CC) $(CFLAGS) -c $< -o $@

outlier: outlier.o
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f outlier outlier.o