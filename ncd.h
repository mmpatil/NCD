#ifndef _NCD_H_
#define _NCD_H_

#include <stdio.h>			/* for printf */
#include <stdlib.h>			/* for EXIT_SUCCESS, EXIT_FAILURE, */
#include <string.h> 		/* for memcpy */
//#include <time.h> 		/* for struct tv */
#include <sys/time.h>		/* for gettimeofday() */
#include <errno.h>			/* for errno*/
#include <sys/socket.h>		/* for socket(), setsockopt(), etc...*/
#include <netinet/ip.h>		/* for struct ip */
#include <netinet/ip6.h>	/* for struct ip6_hdr */
#include <netinet/ip_icmp.h>/* for struct icmp */
#include <netinet/icmp6.h>	/* for struct icmp */
#include <netinet/udp.h>	/* for struct udphdr */
#include <netdb.h>			/* for getaddrinfo() */
#include <arpa/inet.h>		/* for inet_pton() */
#include <signal.h>			/* for kill() */
#include <fcntl.h>			/* for O_RDONLY */
#include <unistd.h>			/* for _________ */
#include <ctype.h>		/* for inet_pton() */
#include <pthread.h>		/* for pthreads */

/**
 *  maximum ip packet size
 */
#define SIZE 1500

/**
 * struct for udp pseudo header
 */
struct pseudo_header {
	u_int32_t source;
	u_int32_t dest;
	u_int8_t zero;
	u_int8_t proto;
	uint16_t len;
};

struct proto
{
	int udp_prot;
	int icmp_prot;
	int icmp_type;
	void *(*rcv_data)(void*);

};

/**
 * Determines if compression occurs along the current transmission path to host
 * by sending data to a remote location.
 *
 * two data trains are sent each with leading and trailing ICMP timestamp messages
 *
 * the first data train will be low entropy data, to encourage compression
 * the second train will have high entropy data, which should not be compressed
 * if the times are significantly different, we have reasonable evidence
 * compression exists along this path.
 *
 * @param address the address of the end host stored in a char array (cstring)
 * @param port the port number or service name
 * @param num_packets the number of packets in each data train
 * @param time_wait the wait between trains
 * @return 0 success, 1 error/failure
 * */
int comp_det(char* address, u_int16_t port, char hl, size_t data_size,
		size_t num_packets, unsigned short ttl, size_t time_wait,
		int n_tail);



/**
 *
 * @param buff
 * @param size
 * @param res
 * @param proto
 * @return
 */
int mkipv4(void* buff, size_t size, struct addrinfo *res, u_int8_t proto);

int mkipv6(void* buff, size_t size, struct addrinfo *res, u_int8_t proto);

/**
 *
 * @param buff
 * @param udp_data_len
 * @param proto
 * @return
 */
int mkudphdr(void* buff, size_t udp_data_len, u_int8_t proto);

/**
 *
 * @param buff
 * @param datalen
 * @return
 */
int mkicmpv4(void *buff, size_t datalen);

int mkicmpv6(void *buff, size_t datalen);

/**
 *
 * @param buff
 * @param size
 */
void fill_data(void *buff, size_t size);

/**
 * sends data train to the end host with leading and trailing icmp timestamps
 *
 * sets up a connection using raw sockets and sends a head ICMP echo request
 * followed by a UDP data train. After the data train is sent, a series of
 * ICMP echo responses is sent.
 *
 * @param address the address of the end host stored in a char array (cstring)
 * @param port the port number or service name
 * @param hl the entropy of the data (either 'H' for high entropy or 'L'
 * for low entropy
 * @param data_size the size in bytes of the data to be sent in the udp train
 * @param num_packets the number of packets in each data train
 * @param ttl the time to live for each packet (max size 255)
 * @param time_wait the wait between ICMP tail messages
 * @param n_tail the number of ICMP tail messages to be sent
 * @return 0 success, 1 error/failure
 */
int send_data(char* address, u_int16_t port, char hl, size_t data_size,
		size_t num_packets, unsigned short ttl, size_t time_wait,
		int n_tail);

/**
 * @return integer value cast to void*. 0 success, 1 error/failure
 * @param num number of tail icmp messages to send
 */
void *send_train(void* num);

/**
 * Receives ICMP responses from end host and records times
 * @param t pointer to a double. returns the time in ms between head echo response and first
 * processed tail echo response to a resolution of microseconds (10^-6 sec)
 * @return 0 success, 1 error/failure
 */
void *recv4(void *t);

void *recv6(void *t);



/**
 * calculates the ip cheksum for some buffer of size length
 * @param vdata a pointer to the buffer to be checksummed
 * @param length the length in bytes of the data to be checksummed
 * @return the IP checksum of the buffer
 */
uint16_t ip_checksum(void* vdata, size_t length);

#endif
