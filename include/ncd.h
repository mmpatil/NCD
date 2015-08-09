/**
 * @author: Paul Kirth
 * @file: ncd.c
 */

#ifndef _NCD_H_
#define _NCD_H_

#include <stdio.h>		/* for printf */
#include <stdlib.h>		/* for EXIT_SUCCESS, EXIT_FAILURE, */
#include <stdio.h>
#include <string.h> 		/* for memcpy */
//#include <time.h> 		/* for struct tv */
#include <sys/time.h>		/* for gettimeofday() */
#include <errno.h>		/* for errno*/
#include <sys/socket.h>		/* for socket(), setsockopt(), etc...*/
#include <netinet/ip.h>		/* for struct ip */
#include <netinet/ip6.h>	/* for struct ip6_hdr */
#include <netinet/ip_icmp.h>	/* for struct icmp */
#include <netinet/icmp6.h>	/* for struct icmp */
#include <netinet/tcp.h>	/* for struct tcphdr */
#include <netinet/udp.h>	/* for struct udphdr */
#include <netdb.h>		/* for getaddrinfo() */
#include <arpa/inet.h>		/* for inet_pton() */
#include <signal.h>		/* for kill() */
#include <fcntl.h>		/* for O_RDONLY */
#include <unistd.h>		/* for _________ */
#include <ctype.h>		/* for inet_pton() */
#include <pthread.h>		/* for pthreads */


/**
 * Favor the BSD style UDP & IP headers
 */
/*
#ifndef __USE_BSD
#define __USE_BSD
#endif
#ifndef __FAVOR_BSD
#define __FAVOR_BSD
#endif
*/
/**
 *  maximum ip packet size
 *  1500 bytes Ethernet max size
 *  40 IPv6 header max size
 *  8 UDP header
 *  2 16-bit packet ID
 *
 */
#define SIZE (1500 - sizeof(struct ip))
#define UDP_DATA_SIZE (SIZE-sizeof(struct udphdr)-sizeof(uint16_t))
#define TCP_DATA_SIZE (SIZE-sizeof(struct tcphdr)-sizeof(uint16_t))



/**
 * struct for udp pseudo header
 */
struct __attribute__((__packed__))pseudo_header {
	u_int32_t source;
	u_int32_t dest;
	u_int8_t zero;
	u_int8_t proto;
	uint16_t len;
};

/**
 * Determines if compression occurs along the current transmission path to host
 * by sending data to a remote location.
 *
 * Two data trains are sent each with leading and trailing ICMP timestamp
 * messages
 *
 * The first data train will be low entropy data, to encourage compression
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
int comp_det();


/**
 * @brief
 * @param c Character flag for determining which set of packet trains ncd
 * should send
 * @return
 */
int detect();
/**
 * Formats an ipv4 header beginning at buff of length size
 * @param buff Address of the starting location for the IP packet
 * @param size The length of the IP packet
 * @param res A pointer to a struct addrinfo -- from getaddrinfo()
 * @param proto The 8-bit protocol
 * @return Returns an integer value for success(0), failure(1), or error(-1)
 */
int mkipv4(void* buff, size_t size, struct addrinfo *res, u_int8_t proto);

int mkipv6(void* buff, size_t size, struct addrinfo *res, u_int8_t proto);

/**
 * Formats a UDP header beginning at buff with a payload of length udp_data_len
 * @param buff Address of the starting location for the UDP packet
 * @param udp_data_len The length of the UDP payload
 * @param proto The 8-bit protocol
 * @return Returns an integer value for success(0), failure(1), or error(-1)
 */
int mkudphdr(void* buff, size_t udp_data_len, u_int8_t proto);
int mktcphdr(void* buff, size_t data_len, u_int8_t proto);
/**
 * Formats an ICMP packet beginning at buff with a payload of length datalen
 * @param buff Address of the starting location for the ICMP packet
 * @param datalen The length of the ICMP payload
 * @return Returns an integer value for success(0), failure(1), or error(-1)
 */
int mkicmpv4(void *buff, size_t datalen);

int mkicmpv6(void *buff, size_t datalen);

/**
 * Fills the data portion of a packet with the size data from a file (char* file)
 * @param buff Address of the starting location for the data
 * @param size The length of the data region
 */
void fill_data(void *buff, size_t size);

/**
 * Sends the UDP data train with leading and trailing ICMP messages
 * @return An unsigned integer value cast to void*. 0 success, 1 error/failure
 * @param status returns the status/return code
 */
void *send_udp(void* status);

/**
 * sends a tcp data train with leading and trailing ICMP messages
 * @return unsigned integer value cast to void*. 0 success, 1 error/failure
 * @param status returns the status/return code
 */
void *send_tcp(void* status);

/**
 * Receives ICMP responses from end host and records times
 * @param t Pointer to a double. Returns the time in ms between head echo response and first
 * processed tail echo response to a resolution of 
 * microseconds (10^-6 sec)
 * @return 0 success, 1 error/failure -- pthreads
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

/**
 * Checks arguments given on command line and stores values in global variables
 * @param argc number of command line args
 * @param argv array of commandline args
 * @return 0 success, 1 Failure
 */
int check_args(int argc, char* argv[]);


/**
 * sets up arrays of tcp packets. Can be extended to udp if neccessary
 * @return
 */
int setup_tcp_packets();

void setup_syn_packets();
void setup_syn_packet(void* buff, uint16_t port);
#endif
