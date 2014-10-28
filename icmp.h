#ifndef _ICMP_H_
#define _ICMP_H_

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

uint16_t ip_checksum(void* vdata,size_t length);
void make_icmp(char *address, unsigned char code, void * data, ssize_t data_len);


#endif
