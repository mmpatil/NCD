all: server client

server: server.c
	gcc server.c -o server 

client: client.c icmp.c icmp.h
	gcc client.c icmp.c -o client 

clean:
	rm client server
