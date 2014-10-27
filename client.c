/**
 * @author: Paul Kirth
 * @file: client.c
 * Comp 429
 * Project Phase I
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
#include <fcntl.h>
/**
 * Detects compression by sending a stream of low entropy and high entropy
 * packets, and comparing their transmission time to determine if
 * compression occurs along the transmission path.
 *
 * @argv[1] Destination IP address
 * @argv[2] Number of data packets to send in each train
 * @argv[3] TIME_WAIT
 *
 */
int main(int argc, char* argv[])
{
	if(argc != 4)
		return EXIT_FAILURE;

	printf("Starting setup\n");
	int sockfd; /* socket file descriptor*/
	struct sockaddr_in server; /* socket addr info	*/
	char tcp_msg[1024]; /* buffer for tcp messages*/

	size_t num_msg = atoi(argv[2]);
	size_t TIME_WAIT = atoi(argv[3]);
	strcpy(tcp_msg, argv[2]);

	char low_data[1100] = { 0 }; /* low entropy data*/
	char high_data[1100]; /* high entropy data*/

	/*get random data for high entropy datagrams*/
	int random = open("/dev/urandom", O_RDONLY);
	read(random, high_data, sizeof(high_data));
	close(random);

	printf("size of tcp_msg : %zu\nnumber of msgs: %zu\nTIME_WAIT %Zu\n\n",
			sizeof(low_data), num_msg, TIME_WAIT);
	printf("Address %s\n\n", argv[1]);

	/* initialize the server address info*/
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(argv[1]);
	server.sin_port = htons(9876);

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("Socket call failed");
		return EXIT_FAILURE;
	}

	if(connect(sockfd, (struct sockaddr *) &server, sizeof(server)) == -1){
		perror("Socket call failed");
		return EXIT_FAILURE;
	}

	if((send(sockfd, tcp_msg, sizeof(tcp_msg), 0)) == -1){
		perror("Send error");
		return EXIT_FAILURE;
	}

	close(sockfd); /*close tcp socket */

	/*sleep to wait for server set up --remove later*/
	sleep(3);
	printf("Setup complete\n");

	/*open UDP SOCKET*/
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		perror("Socket call failed");
		return EXIT_FAILURE;
	}

	/* send the message*/
	size_t n; /* iterator*/
	for(n = 0; n < num_msg; ++n){
		if((sendto(sockfd, low_data, sizeof(low_data), 0,
				(struct sockaddr*) &server, sizeof(server)))
				== -1){
			perror("Send error");
			return EXIT_FAILURE;
		}
	}
	sleep(TIME_WAIT); /* long wait between data trains ...*/
	for(n = 0; n < num_msg; ++n){
		if((sendto(sockfd, high_data, sizeof(high_data), 0,
				(struct sockaddr*) &server, sizeof(server)))
				== -1){
			perror("Send error");
			return EXIT_FAILURE;
		}
	}
	printf("\nTask Complete ...");
	close(sockfd);
	printf("Socket Closed ... Now Closing...\n");
	return EXIT_SUCCESS;
}
