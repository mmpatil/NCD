IDIR=include
SDIR=src
ODIR=obj
TESTDIR=test

#CC=gcc
#CXX=g++

#CC=clang
LIBS=-lm
CLINKFLAGS=-pthread
CFLAGS=-O2 -g -I$(IDIR) -Wall -Wextra

_DEPS=ncd.h bitset.h ncd_global.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ=ncd_main.o ncd.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


#CFLAGS+=-fsanitize=address -fno-omit-frame-pointer
#CFLAGS+=-fsanitize=memory -fsanitize-memory-track-origins  -fno-omit-frame-pointer -fPIE
#CFLAGS+=-fsanitize=thread
#CFLAGS+=-DDEBUG
#CFLAGS+=-DNCD_NO_KILL

#CXX=clang++
CXXFLAGS=-std=c++1y -O2 -I$(IDIR) -I$(SDIR) #-stdlib=libc++
CXXLINKFLAGS=-pthread -lgtest -lgtest_main


all: ncd_main unittest

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC)  $(CFLAGS) $(CLINKFLAGS) -c -o $@ $<

ncd_main: $(OBJ)
	$(CC) -o $@  $(OBJ) $(CFLAGS) $(CLINKFLAGS) $(LIBS)

unittest: $(DEPS) $(OBJ)  $(TESTDIR)/unit_test.cpp
	$(CXX) obj/ncd.o $(TESTDIR)/unit_test.cpp   -o $@ $(CXXFLAGS) $(CXXLINKFLAGS)


.PHONY: clean

clean:
	rm $(ODIR)/*.o ncd_main unittest
