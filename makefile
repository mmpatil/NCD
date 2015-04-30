IDIR=include
SDIR=src
ODIR=obj

#CC=gcc
#CPP=g++

CC=clang
LIBS=-lm
CLINKFLAGS=-pthread
CFLAGS=-O2 -g -I$(IDIR)

_DEPS=ncd.h bitset.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ=ncd_main.o ncd.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(echo $(OBJ))

#CFLAGS+=-fsanitize=thread #-fPIE -fsanitize-memory-track-origins -fno-omit-frame-pointer
#CFLAGS+=-DDEBUG
#CFLAGS+=-DNCD_NO_KILL

CPP=clang++
CPPFLAGS=-std=c++11 -stdlib=libc++ -O2
CPPLINKFLAGS=-pthread -L/usr/lib -lgtest -lgtest_main


all: ncd_main #test

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(CLINKFLAGS)

ncd_main: $(OBJ)
	$(CC) -o $@  $(OBJ) $(CFLAGS) $(CLINKFLAGS) $(LIBS)

test: unit_test.h unit_test.cpp
	$(CPP) unit_test.cpp  $(CPPFLAGS) $(CPPLINKFLAGS) -o $@


.PHONY: clean

clean:
	rm $(ODIR)/*.o ncd_main #test
