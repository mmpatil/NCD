#CC=gcc
#CPP=g++
CC=clang
CLINKFLAGS=-pthread -lm 
CFLAGS=-O2 -g
#CFLAGS+=-fsanitize=thread #-fPIE -fsanitize-memory-track-origins -fno-omit-frame-pointer
#CFLAGS+=-DDEBUG
#CFLAGS+=-DNCD_NO_KILL
 
CPP=clang++
CPPFLAGS=-std=c++11 -stdlib=libc++ -O2
CPPLINKFLAGS=-pthread -L/usr/lib -lgtest -lgtest_main


all: ncd_main #test 

test: unit_test.h unit_test.cpp
	$(CPP) unit_test.cpp  $(CPPFLAGS) $(CPPLINKFLAGS) -o test
	
ncd_main: ncd.o ncd_main.c
	$(CC) ncd_main.c ncd.o -o ncd_main $(CFLAGS) $(CLINKFLAGS)

ncd.o: ncd.h ncd.c bitset.h
	$(CC) -c ncd.c $(CFLAGS) $(CLINKFLAGS)

clean:
	rm ncd.o test ncd_main
