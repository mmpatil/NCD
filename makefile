CC=gcc
#CFLAGS=-DDEBUG


all: ncd

ncd: ncd.h ncd.c
	$(CC) ncd.c -o ncd $(CFLAGS)

clean:
	rm *.o 
