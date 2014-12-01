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

#include <fcntl.h>

#define SIZE 1500    //maximum ip packet size
struct pseudo_header {
	u_int32_t source;
	u_int32_t dest;
	u_int8_t zero;
	u_int8_t proto;
	uint16_t len;
};

// maybe not this either....
char* host; /*the host address stored as a string*/

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
 * @return returns 0 for no compression detection, 1 for compression, -1 for error
 * */
int comp_det(char* address, char * port, char hl, size_t data_size,
		size_t num_packets, ushort ttl, size_t time_wait, int n_tail);

/**
 * sends data train to the end host with leading and trailing icmp timestamps
 */
int send_data(char* address, char * port, char hl, size_t data_size,
		size_t num_packets, ushort ttl, size_t time_wait, int n_tail);

/**
 * recieves ICMP responses from end host and records times
 */
double recv_data();

uint16_t ip_checksum(void* vdata, size_t length);

#endif
