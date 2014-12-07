/**
 * @author: Paul Kirth
 * @file: server.c
 * Comp 429
 * Project 1 Phase IV
 */

#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

/**
 * @param num_msg the number of messages to expect
 * @param sockfd the socket file descriptor to expect communication from
 * @param client a data structure to hold client information
 *
 * @return the time spent gathering packets in the train
 */
clock_t procs_msg(size_t num_msg, int sockfd, struct sockaddr_in client);

/**
 * sets up a server whose clients will send a series of data packages so that
 * compression along the transmission path can be detected.
 */
int main(int argc, char* argv[])
{
	printf("Starting setup\n");
	clock_t diff1, diff2; /*transmission times for each data train*/

	int sockfd, tcpfd, udpfd; /*file descriptors for sockets*/
	char tcp_msg[1024]; /*buffer for tcp communications*/

	struct timeval tv;
	tv.tv_sec = 5; /* timeout = 5 seconds*/
	tv.tv_usec = 0;

	/* set up client and server data structures*/
	struct sockaddr_in client, server;
	socklen_t len = sizeof(client);
	bzero(&client, sizeof(client));
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(9876);
	server.sin_addr.s_addr = INADDR_ANY;

	/*initialize socket*/
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	//Free port
	int on = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	if(sockfd == -1){
		perror("Socket call failed. \n");
		return EXIT_FAILURE;
	}

	/*bind address*/
	if(bind(sockfd, (struct sockaddr*) &server, sizeof(server)) == -1){
		perror("Bind call failed. \n");
		return EXIT_FAILURE;
	}

	/*listen*/
	if(listen(sockfd, 5) == -1){
		perror("Listen call failed. \n");
		return EXIT_FAILURE;
	}

	printf("Now Listening ... \n");

	/*Main loop gets new tcp connections and spawns child processes
	 * to deal with them */
	while(1){

		/* accept tcp connection*/
		if((tcpfd = accept(sockfd, (struct sockaddr *) &client, &len))
				== -1){
			perror("Accept call failed. \n");
			return EXIT_FAILURE;
		}

		/*spawn child process to deal with new connection*/
		if(fork() == 0){
			//open udp connection
			if((udpfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
				perror("Socket call failed. \n");
				return EXIT_FAILURE;
			}

			/*set socket options for timeout.*/
			setsockopt(udpfd, SOL_SOCKET, SO_RCVTIMEO, &tv,
					sizeof(tv));

			recv(tcpfd, tcp_msg, sizeof(tcp_msg), 0);

			/*bind address*/
			if((bind(udpfd, (struct sockaddr*) &server,
					sizeof(server))) == -1){
				perror("Bind call failed. \n");
				return EXIT_FAILURE;
			}

			/*finish preparations before close() to avoid delay
			 * between receiving*/
			size_t num_msg = atoi(tcp_msg);
			printf("number of expected packets = %zu\n", num_msg);
			printf("Setup complete\n");
			printf("Starting up. \n");

			/* process data trains and record transmission times */
			diff1 = procs_msg(num_msg, udpfd, client);
			diff2 = procs_msg(num_msg, udpfd, client);

			long int ms1 = diff1 * 1000 / CLOCKS_PER_SEC;
			long int ms2 = diff2 * 1000 / CLOCKS_PER_SEC;

			printf("\nClocks per sec %ld\n", CLOCKS_PER_SEC);
			printf("Low Entropy Time:%ld ms\n", ms1);
			printf("High Entropy Time: %ld ms\n", ms2);
			printf("diff1 = %lu\ndiff2 = %lu\n", diff1, diff2);
			close(sockfd);
			close(tcpfd);
			close(udpfd);
			return EXIT_SUCCESS;
		}
	}
	close(sockfd);
	close(tcpfd);
	close(udpfd);

	return EXIT_SUCCESS;
}

clock_t procs_msg(size_t num_msg, int sockfd, struct sockaddr_in client)
{

	socklen_t len = sizeof(client);
	size_t i; /*iterator*/
	int n; /*number of bytes received*/
	clock_t start, diff; /* start time and time elapsed*/
	start = clock();
	char msg[1100]; /*message buffer*/

	for(i = 0; i < num_msg; ++i){
		n = recvfrom(sockfd, msg, sizeof(msg), 0,
				(struct sockaddr *) &client, &len);
		if(n < 0){
			perror("Timeout reached");
			break;
		}
		printf("-------------------------------------------------\n");
		printf("Received %zu/%zu Packets\n", (i + 1), num_msg);
		printf("size of message was %d\n", n);
		printf("-------------------------------------------------\n");
	}
	diff = clock() - start;
	return diff;
}
