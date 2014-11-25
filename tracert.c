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

	int fd, fd2, n;
	size_t datalen = 56; /* data for echo msg */
	size_t len = 8 + datalen;
	size_t total_len = sizeof(struct ip) + len; // sizeof(struct icmp) + datalen;
	struct icmp *icmp;
	struct addrinfo *res;
	struct addrinfo hints = { 0 };
	struct sockaddr_in addr;
	socklen_t adrlen = sizeof(addr);

	char packet_send[SIZE] = { 0 };
	char packet_rcv[SIZE] = { 0 };
	char str[INET_ADDRSTRLEN];

	nsent = (size_t) rand();/*get random number for seq #*/
	//struct timeval *tp, tv;

	fd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
	fd2 = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	int size = 60 * 1024;
	setsockopt(fd2, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));

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
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_RAW;

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
	struct ip *ip = (struct ip *) packet_send;
	ip->ip_v = 4;
	ip->ip_hl = 5;
	ip->ip_len = htons(SIZE);
	ip->ip_id = htons(1234);
	ip->ip_dst = ((struct sockaddr_in*)res->ai_addr)->sin_addr;
	ip->ip_ttl = 255;
	ip->ip_p = IPPROTO_UDP;

	/*create udp packet*/
	int offset = sizeof(struct udphdr) + sizeof(struct ip);
	printf("Offset: %d\n", offset);

	struct udphdr *udp = (struct udphdr *) (ip +1);
	udp->uh_sport = htons(port); /* set source port*/
	udp->uh_dport = htons(port); /* set destination port */
	udp->uh_ulen = htons(sizeof(struct udphdr)); /* set udp length */


	/* fill with random data from /dev/urandom */
	/*get random data for high entropy datagrams*/
	int random = open("/dev/urandom", O_RDONLY);
	int udp_len = SIZE - offset;
	read(random, (udp + 1), udp_len);
	close(random);

	udp->check = ip_checksum(udp, udp_len); /* set udp checksum */
	ip->ip_sum = ip_checksum(ip, SIZE);

	double d = get_time();
	if(fork() == 0)
	{
		ip = (struct ip *)packet_rcv;
		icmp = (struct icmp *) (ip + 1);

		for(;;){

			if((n = recvfrom(fd2, packet_rcv, total_len, 0,
					(struct sockaddr *) &addr, &adrlen)) < 0 ){
				if(errno == EINTR )
					continue;
				perror("tracert: recvfrom");
				continue;
			}//else if(icmp->icmp_type != 3)continue;
			else{
				d = get_time() - d;
				break;
			}
		}
		close(fd2);

	#ifdef DEBUG
		printf("\nIn Child Process\n");
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


	n = sendto(fd, packet_send, SIZE, 0, res->ai_addr, res->ai_addrlen);
	if(n == -1){
		perror("Send error");
		return EXIT_FAILURE;
	}




#ifdef DEBUG
	printf("\nIn Parent Process\n");
	printf("msg sent\n");
	printf("Sent: %d bytes.\n", n);
	printf("ip header:\n");
	inet_ntop(AF_INET, &(ip->ip_dst.s_addr), str, INET_ADDRSTRLEN );
	printf("destination address: %s\n", str);
	inet_ntop(AF_INET, &(ip->ip_src.s_addr), str, INET_ADDRSTRLEN );
	printf("source address: %s\n", str);
	printf("ip header length:%d\n", ip->ip_hl);
	printf("TTL: %d\n", ip->ip_ttl);

	printf("UDP Header:\n");
	printf("Source port: %d\n", ntohs(udp->uh_sport));
	printf("Destination port: %d\n", ntohs(udp->uh_dport));
	printf("UDP Header Length: %d\n", ntohs(udp->uh_ulen));

#endif

	freeaddrinfo(res);
	read(my_pipe[0], &d, sizeof(d));


	close(fd);
	printf("Time elapsed: %f\n", d);

	return 0;
}

void debug_print(struct icmp *icmp, ssize_t n)
{
	printf("Receved: %zu bytes.\n", n);
	printf("Process ID: %d\n", getpid());
	printf("ICMP reply type: %d\n", icmp->icmp_type);
	printf("ICMP CODE: %d\n",icmp->icmp_code);
	printf("ICMP reply ID: %d\n", icmp->icmp_id);

}
