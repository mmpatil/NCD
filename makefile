CC=gcc
CFLAGS=
#CFLAGS+=-DDEBUG
#CFLAGS+=-DNCD_NO_KILL
CFLAGS+=-pthread


all: ncd tracert ping_ip ping #ncd_no_raw

ncd: ncd.h ncd.c
	$(CC) ncd.c -o ncd $(CFLAGS)

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
