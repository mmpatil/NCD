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

/*  consider not using this ....*/
struct trans_info{
	char * address;
	size_t num_packets;
	size_t time_wait;
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
 * @return returns 0 for no compressin detection, 1 for compression, -1 for error
 * */
int comp_det(char* address, char * port,size_t num_packets, size_t time_wait);


/**
 * sends data to the end host with leading and trailing icmp timestamps
 */
int send_data();


/**
 * recieves timestamps from end host to detect compression
 */
int recv_data();

/**
 * creates an ICMP packet header
 * @return returns a pointer to a new icmp header
 */
struct icmphdr* make_icmp(char *address, unsigned char code, void * data, ssize_t data_len);// maybe make this different...

uint16_t ip_checksum(void* vdata,size_t length);




#endif
