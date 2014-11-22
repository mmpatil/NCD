#include "icmp.h"

uint16_t ip_checksum(void* vdata, size_t length)
{
	// Cast the data pointer to one that can be indexed.
	char* data = (char*) vdata;

	// Initialise the accumulator.
	uint64_t acc = 0xffff;

	// Handle any partial block at the start of the data.
	unsigned int offset = ((uintptr_t) data) & 3;
	if(offset){
		size_t count = 4 - offset;
		if(count > length)
			count = length;
		uint32_t word = 0;
		memcpy(offset + (char*) &word, data, count);
		acc += ntohl(word);
		data += count;
		length -= count;
	}

	// Handle any complete 32-bit blocks.
	char* data_end = data + (length & ~3);
	while(data != data_end){
		uint32_t word;
		memcpy(&word, data, 4);
		acc += ntohl(word);
		data += 4;
	}
	length &= 3;

	// Handle any partial block at the end of the data.
	if(length){
		uint32_t word = 0;
		memcpy(&word, data, length);
		acc += ntohl(word);
	}

	// Handle deferred carries.
	acc = (acc & 0xffffffff) + (acc >> 32);
	while(acc >> 16){
		acc = (acc & 0xffff) + (acc >> 16);
	}

	// If the data began at an odd byte address
	// then reverse the byte order to compensate.
	if(offset & 1){
		acc = ((acc & 0xff00) >> 8) | ((acc & 0x00ff) << 8);
	}

	// Return the checksum in network byte order.
	return htons(~acc);
}

void make_icmp(char *address, unsigned char code, void * data, ssize_t data_len)
{
	struct icmphdr req;

	int fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if(fd == -1)
		exit(EXIT_FAILURE);    // consider doing something better here

	int hdrincl = 1;

	if(setsockopt(fd, IPPROTO_IP, IP_HDRINCL, &hdrincl, sizeof(hdrincl))
			== -1){
		perror("setsockopt failed");
		exit(EXIT_FAILURE);
	}

	req.type = ICMP_TIMESTAMP;
	req.code = 0;
	req.checksum = 0;
	req.un.echo.id = htons(rand());
	req.un.echo.sequence = htons(1);
	req.checksum = ip_checksum(&req, sizeof(req));

}

