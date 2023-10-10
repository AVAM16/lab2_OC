CC = gcc
CFLAGS=-Wall -Wextra 
TARGET=SimpleCache
TARGET2=L1Cache

all:
	$(CC) $(CFLAGS) SimpleProgram.c SimpleCache.c -o $(TARGET)
	$(CC) $(CFLAGS) SimpleProgram.c L1Cache.c -o $(TARGET2) -g

clean:
	rm $(TARGET)
	rm $(TARGET2)