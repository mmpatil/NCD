CC=gcc
#CFLAGS=-DDEBUG


all: tracrt

tracrt: tracert.c
	$(CC) tracert.c -o tracert $(CFLAGS)

clean:
	rm *.o 
