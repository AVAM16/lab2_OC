CC = gcc
CFLAGS=-Wall -Wextra 
TARGET=SimpleCache
TARGET2=L1Cache
TARGET3=L2Cache

all:
	$(CC) $(CFLAGS) SimpleProgram.c SimpleCache.c -o $(TARGET)
	$(CC) $(CFLAGS) SimpleProgram.c L1Cache.c -o $(TARGET2)
	$(CC) $(CFLAGS) SimpleProgram.c L2Cache.c -o $(TARGET3) -g

clean:
	rm $(TARGET)
	rm $(TARGET2)
	rm $(TARGET3)