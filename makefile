all: server client ncd

ncd: ncd_main.c ncd.o
	gcc ncd_main.c ncd.o -o ncd

ncd.o: ncd.h ncd.c
	gcc -c ncd.c 

server: server.c
	gcc server.c -o server 

client: client.c icmp.c icmp.h
	gcc client.c icmp.c -o client 

clean:
	rm client server ncd *.o
