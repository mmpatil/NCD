/*
 *	COMP429 Fall 2014 Project Skeleton
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netdb.h>
#include <fcntl.h>
//#include <sys/types.h>
//#include <sys/socket.h>
//#include <netinet/in.h>

/*  Just returns current time as double, with most possible precision...  */
double get_time(void)
{
	struct timeval tv;
	double d;
	gettimeofday(&tv, NULL);
	d = ((double) tv.tv_usec) / 1000000. + (unsigned long) tv.tv_sec;
	return d;
}

double sub_time(struct timeval *out, struct timeval *in)
{
	struct timeval tv;
	double d;
	tv.tv_usec = out->tv_usec - in->tv_usec;
	tv.tv_sec = out->tv_sec - in->tv_sec;
	d = ((double) tv.tv_usec) / 1000000. + (unsigned long) tv.tv_sec;
	return d;
}

uint16_t ip_checksum(void* vdata, size_t length)
{
	// Cast the data pointer to one that can be indexed.
	char* data = (char*) vdata;

	// Initialize the accumulator.
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

size_t nsent;    //global sequence number for IP

void debug_print(struct icmp *icmp, ssize_t n);

#define SIZE 1024
#define DEBUG

int main(int arc, char *argv[])
{
	int port = 9876;

	int my_pipe[2];
	pipe(my_pipe);

	int fd, fd2;
	size_t datalen = 56; /* data for echo msg */
	size_t len = 8 + datalen;
	struct icmp *icmp;
	struct addrinfo *res;
	struct addrinfo hints = { 0 };
	struct sockaddr_in adr;
	socklen_t adrlen = sizeof(adr);
	char packet[SIZE] = { 0 };
	char packet_rcv[SIZE] = { 0 };
	char str[INET_ADDRSTRLEN];
	nsent = (size_t) rand();/*get random number for seq #*/
	struct timeval *tp, tv;

	fd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
	fd2 = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

	if(fd == -1){
		perror("call to socket() failed");
		exit(EXIT_FAILURE);    // consider doing something better here
	}

	int hdrincl = 1;
	if(setsockopt(fd, IPPROTO_IP, IP_HDRINCL, &hdrincl, sizeof(hdrincl))
			== -1){
		perror("setsockopt() failed");
		exit(EXIT_FAILURE);
	}/**/

	setuid(getuid());/*give up privileges */

	/* set up hints for getaddrinfo() */
	hints.ai_flags = AI_CANONNAME;
	hints.ai_family = AF_INET;
	hints.ai_socktype = 0;
	int err = getaddrinfo(argv[1], NULL, &hints, &res);

	if(err != 0){
		if(err == EAI_SYSTEM)
			fprintf(stderr, "looking up www.example.com: %s\n",
					strerror(errno));
		else
			fprintf(stderr, "looking up www.example.com: %s\n",
					gai_strerror(err));
		exit(EXIT_FAILURE);
	}

	/* create IP header*/
	struct ip *ip = (struct ip *) packet;
	ip->ip_v = AF_INET;
	ip->ip_hl = 5;
	ip->ip_len = htons(1024);
	ip->ip_id = htonl(getpid());
	ip->ip_tos = 0;
	//ip->ip_src = adr.sin_addr;
	ip->ip_dst =  ((struct sockaddr_in*)res->ai_addr)->sin_addr;
	ip->ip_ttl = 4;
	ip->ip_p = IPPROTO_UDP;

	/*create udp packet*/
	int offset = sizeof(struct udphdr) + sizeof(struct ip);
	struct udphdr *udp = (struct udphdr *) (ip + sizeof(struct ip));
	udp->source = htons(port); /* set source port*/
	udp->dest = htons(port); /* set destination port */
	udp->len = htons(sizeof(packet) - sizeof(*ip)); /* set udp length */



	/* fill with random data from /dev/urandom */
	/*get random data for high entropy datagrams*/
	int random = open("/dev/urandom", O_RDONLY);
	int udp_len = sizeof(packet) - offset;
	read(random, packet + offset, udp_len);
	close(random);

	/* insert current time */
	gettimeofday((struct timeval *) udp + 1, NULL);

	udp->check = ip_checksum(udp, udp_len); /* set udp checksum */

	double d = get_time();
	if(fork() == 0)
	{
		ssize_t n;
		int hlen1 = ip->ip_hl << 2;
		tp = (struct timeval *) icmp->icmp_data;
		icmp = (struct icmp *) (packet_rcv + hlen1);
		ip = (struct ip *)packet_rcv;
		udp = (struct udphdr *) (ip + sizeof(*ip));

		for(;;){

			if((n = recvfrom(fd2, packet_rcv, len, 0,
					(struct sockaddr *) &adr, &adrlen)) < 0 ){
				if(errno == EINTR )
					continue;
				perror("tracert: recvfrom");
				continue;
			}//else if(icmp->icmp_type != 3)continue;
			else{
				gettimeofday(&tv, NULL);
				d = get_time() - d;
				break;
			}
		}
		close(fd2);

	#ifdef DEBUG
		debug_print(icmp, n);
		printf("ip header:\n");
		inet_ntop(AF_INET, &(ip->ip_dst.s_addr), str, INET_ADDRSTRLEN );
		printf("destination address: %s\n", str);
		inet_ntop(AF_INET, &(ip->ip_src.s_addr), str, INET_ADDRSTRLEN );
		printf("source address: %s\n", str);
		printf("ip header length:%d\n", ip->ip_hl);
		printf("TTL: %d\n", ip->ip_ttl);
	#endif
		//double elapsed = sub_time(&tv, tp);

		//printf("Total elapsed time = %f seconds\nBut d = %f\n", elapsed, d);
		printf("d = %f\n", d * 1000);
		write(my_pipe[1], &d, sizeof(d));

		return EXIT_SUCCESS;
	}// end fork()


	sendto(fd, packet, SIZE, 0, res->ai_addr, res->ai_addrlen);

	read(my_pipe[0], &d, sizeof(d));


#ifdef DEBUG
	printf("ip header:\n");
	inet_ntop(AF_INET, &(ip->ip_dst.s_addr), str, INET_ADDRSTRLEN );
	printf("destination address: %s\n", str);
	inet_ntop(AF_INET, &(ip->ip_src.s_addr), str, INET_ADDRSTRLEN );
	printf("source address: %s\n", str);
	printf("ip header length:%d\n", ip->ip_hl);
	printf("TTL: %d\n", ip->ip_ttl);

	printf("msg sent\n\n");
#endif


	close(fd);
	printf("Time elapsed: %f\n", d);

	return 0;
}

void debug_print(struct icmp *icmp, ssize_t n)
{
	printf("Receved: %zu bytes.\n", n);
	printf("Process ID: %d\n", getpid());
	printf("ICMP ECHO type: %d\n", ICMP_ECHO);
	printf("ICMP reply type: %d\n", icmp->icmp_type);

	printf("ICMP reply ID: %d\n", icmp->icmp_id);
}
