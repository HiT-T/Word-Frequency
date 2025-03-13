CC = gcc
CFLAGS = -Wall -Wextra -std=c99

outlier: outlier.c
	$(CC) $(CFLAGS) -o outlier outlier.c

clean:
	rm -f outlier