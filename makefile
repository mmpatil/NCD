C=gcc
#CFLAGS=-DDEBUG

all: ncd tracert ping_ip ping

ncd: ncd.h ncd.c
	$(CC) ncd.c -o ncd $(CFLAGS)

ping_ip: ping_ip.c
	$(CC) ping_ip.c -o ping_ip $(CFLAGS)


ping: ping.c
	$(CC) ping.c -o ping $(CFLAGS)


tracert: tracert.c
	$(CC) tracert.c -o tracert $(CFLAGS)

clean:
	rm ncd tracert ping ping_ip 
