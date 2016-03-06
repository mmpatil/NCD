IDIR=include
SDIR=src
ODIR=obj
TESTDIR=test

#CC=gcc
#CXX=g++

#CC=clang
#CXX=clang++

#GCOV=--coverage -lgcov

ASAN=-fsanitize=address -fno-omit-frame-pointer
MSAN=-fsanitize=memory -fsanitize-memory-track-origins  -fno-omit-frame-pointer -fPIE
TSAN=-fsanitize=thread

SANITIZER=

ARCH?=$(shell getconf LONG_BIT)
#DEBUG_RELEASE=-DDEBUG

CLIBS =-lm -lpthread
CFLAGS= -m$(ARCH) -O2 -g -I$(IDIR) -I$(SDIR) $(GCOV) $(SANITIZER)

CXXFLAGS=$(CFLAGS) -std=c++14  #-stdlib=libc++
CXXLIBS=-lgtest -lgtest_main $(CLIBS)

_DEPS=detector.hpp simple_bitset.h ip_checksum.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ=detector_main.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

all:detector

$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(CLIBS)

detector: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ) $(CLIBS)

unit_test: $(DEPS) $(OBJ) $(TESTDIR)/detector*.*pp
	$(CXX) $(TESTDIR)/detector_unit_test.cpp -o $@ $(CXXFLAGS) $(CXXLIBS)

.PHONY: clean

clean:
	rm $(ODIR)/*.o detector

#IDIR=ncd
#SDIR=ncd
#ODIR=ncd/obj
#TESTDIR=ncd/Test
#
#C_DEPS=simple_bitset.h ip_checksum.h ncd.h
#DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))
#
#_OBJ=ncd.o ncd_main.o
#OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))
#
#
#all: ncd_main
#
#$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
#	$(CC) $(CFLAGS) -c -o $@ $< $(CLIBS)
#
#ncd_main: $(OBJ)
#	$(CC) $(CFLAGS) -o $@ $(OBJ) $(CLIBS)
#
#unit_test: $(DEPS) $(OBJ) $(TESTDIR)/*.*pp
#	$(CXX) obj/ncd.o $(TESTDIR)/*.cpp -o $@ $(CXXFLAGS) $(CXXLIBS)
#
#.PHONY: clean
#
#clean:
#	rm $(ODIR)/*.o ncd_main unit_test
