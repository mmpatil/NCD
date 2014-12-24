CC=gcc
#CC=clang
CFLAGS=
#CFLAGS+=-DDEBUG
#CFLAGS+=-DNCD_NO_KILL
CFLAGS+=-pthread


all: test ncd_main tracert ping_ip ping #ncd_no_raw

test: unit_test.h unit_test.cpp
	g++ unit_test.cpp  -pthread -L/usr/lib -lgtest -lgtest_main -o test
	
ncd_main: ncd.o ncd_main.c
	$(CC) ncd_main.c ncd.o -o ncd_main $(CFLAGS)

ncd.o: ncd.h ncd.c
	$(CC) -c ncd.c $(CFLAGS)

ncd_no_raw: ncd.h ncd_no_raw.c
	$(CC) ncd_no_raw.c -o ncd_no_raw $(CFLAGS)

ping_ip: ping_ip.c
	$(CC) ping_ip.c -o ping_ip $(CFLAGS)


ping: ping.c
	$(CC) ping.c -o ping $(CFLAGS)


tracert: tracert.c
	$(CC) tracert.c -o tracert $(CFLAGS)

clean:
	rm ncd tracert ping ping_ip 
