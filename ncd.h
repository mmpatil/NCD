#ifndef _NCD_H_
#define _NCD_H_

#include <ctype.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/udp.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>


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
int comp_det(char* address, char * port, char hl, size_t data_size,
		size_t num_packets, ushort ttl, size_t time_wait, int n_tail);

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
 * @return 0 success, -1 error/failure
 */
int send_data(char* address, char * port, char hl, size_t data_size,
		size_t num_packets, ushort ttl, size_t time_wait, int n_tail);

/**
 * Receives ICMP responses from end host and records times
 * @param time returns the time in ms between head echo response and first
 * processed tail echo response to a resolution of microseconds (10^-6 sec)
 * @return 0 success, -1 error/failure
 */
int recv_data(double *time);

/**
 * calculates the ip cheksum for some buffer of size length
 * @param vdata a pointer to the buffer to be checksummed
 * @param length the length in bytes of the data to be checksummed
 * @return the IP checksum of the buffer
 */
uint16_t ip_checksum(void* vdata, size_t length);

#endif
