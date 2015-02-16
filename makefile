CC=gcc
#CC=clang
CFLAGS=
#CFLAGS+=-DDEBUG
#CFLAGS+=-DNCD_NO_KILL
CFLAGS+=-pthread -lm


all: test ncd_main 

test: unit_test.h unit_test.cpp
	g++ unit_test.cpp  -pthread -L/usr/lib -lgtest -lgtest_main -o test
	
ncd_main: ncd.o ncd_main.c
	$(CC) ncd_main.c ncd.o -o ncd_main $(CFLAGS)

ncd.o: ncd.h ncd.c bitset.h
	$(CC) -c ncd.c $(CFLAGS)

clean:
	rm ncd.o test ncd_main
