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
struct __attribute__((__packed__))pseudo_header
{
        u_int32_t source;
        u_int32_t dest;
        u_int8_t zero;
        u_int8_t proto;
        uint16_t len;
};

/**
 * @brief Determines if compression occurs along the current transmission path
 * to host by sending data to a remote location.
 *
 * @details Determines if compression occurs along the current transmission path
 * to host by sending data to a remote location.
 * Two data trains are sent each with leading and trailing ICMP timestamp
 * messages The first data train will be low entropy data, to encourage compression
 * the second train will have high entropy data, which should not be compressed
 * if the times are significantly different, we have reasonable evidence
 * compression exists along this path.
 *
 * @return Returns an integer value for success(0), failure(1), or error(-1)
 * */
int comp_det();

/**
 * @brief Sends and measures a single data train, setup and called from
 * comp_det()
 *
 * @details Sends and measures a single data train, setup and called from
 * comp_det(). It does most of the work in NCD, by sending a single data
 * train and taking all relevant measurements.
 * @return Returns an integer value for success(0), failure(1), or error(-1)
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
 * @brief Formats an ICMP packet beginning at buff with a payload of length datalen
 * @param buff Address of the starting location for the ICMP packet
 * @param datalen The length of the ICMP payload
 * @return Returns an integer value for success(0), failure(1), or error(-1)
 */
int mkicmpv4(void *buff, size_t datalen);

int mkicmpv6(void *buff, size_t datalen);

/**
 * @brief Fills the data portion of a packet with size bytes data from a file (char* file)
 * @param[out] buff Address of the starting location for the data to be filled
 * @param[in] size The length of the data region to be filled
 */
void fill_data(void *buff, size_t size);

/**
 * @brief Sends the UDP data train with leading and trailing ICMP messages
 * @return An unsigned integer value cast to void*. 0 success, 1 error/failure
 * @param[out] status returns the status/return code
 */
void *send_udp(void* status);

/**
 * @brief Sends a tcp data train with leading and trailing ICMP messages
 * @return unsigned integer value cast to void*. 0 success, 1 error/failure
 * @param[out] status returns the status/return code
 */
void *send_tcp(void* status);

/**
 * @brief Receives ICMP responses from end host and records times
 * @param[out] t Pointer to a double. Returns the time in ms between head echo
 * response and first processed tail echo response to a resolution of
 * microseconds (10^-6 sec)
 * @return Returns an integer value for success(0), failure(1), or error(-1) -- pthreads
 */
void *recv4(void *t);

void *recv6(void *t);

/**
 * @brief Calculates the ip cheksum for some buffer of size length
 * @param vdata a pointer to the buffer to be checksummed
 * @param length the length in bytes of the data to be checksummed
 * @return the IP checksum of the buffer
 */
uint16_t ip_checksum(void* vdata, size_t length);

/**
 * @brief Checks arguments given on command line and stores values in global variables
 * @param argc number of command line args
 * @param argv array of commandline args
 * @return Returns an integer value for success(0), failure(1), or error(-1)
 */
int check_args(int argc, char* argv[]);

/**
 * @brief sets up arrays of TCP packets. Can be extended to UDP if necessary
 * @return Returns an integer value for success(0), failure(1), or error(-1)
 */
int setup_tcp_packets();

/**
 * @brief Sets up syn packets for use in tcp_send
 */
void setup_syn_packets();

/**
 * @brief Sets up a single syn packet for use in tcp_send.
 * @param buff address of tcp header start for syn packet
 * @param port the source port the syn packet should use.
 */
void setup_syn_packet(void* buff, uint16_t port);

#endif// end ncd.h
