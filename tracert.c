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

#define SIZE 1500    //maximum ip packet size
//#define DEBUG

struct pseudo_header {
	u_int32_t source;
	u_int32_t dest;
	u_int8_t zero;
	u_int8_t proto;
	uint16_t len;
};

int main(int arc, char *argv[])
{
	//int port = 33434;
	int port = atoi(argv[2]);

	int my_pipe[2];
	pipe(my_pipe);
	int udp_data_len = atoi(argv[3]);
	int udp_len = udp_data_len + 8;
	int packet_size = udp_len + 20;

	if(SIZE <= udp_data_len + 40 + sizeof(struct udphdr)){
		perror("Maximum packet size exceeded");
		exit(EXIT_FAILURE);
	}

	int send_fd, recv_fd, n;
	size_t datalen = 56; /* data for ICMP msg */
	size_t len = sizeof(struct icmp) + datalen; /*size of icmp packet*/
	size_t icmp_len = sizeof(struct ip) + len; /* size of ICMP reply + ip header */
	struct icmp *icmp; /* ICMP header */
	struct addrinfo *res; /* for get addrinfo */
	struct addrinfo hints = { 0 }; /* for get addrinfor */
	struct sockaddr_in addr; /* to recieve data with*/
	socklen_t adrlen = sizeof(addr); /* length of address */

	char packet_send[SIZE] = { 0 }; /* buffer to send data with */
	char packet_rcv[SIZE] = { 0 }; /* buffer to recieve data into */
	char str[INET_ADDRSTRLEN]; /* buffer to debug with -- remove, or wrap with #ifdef*/

	send_fd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
	recv_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	int size = 60 * 1024;
	setsockopt(recv_fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));

	if(send_fd == -1){
		perror("call to socket() failed");
		exit(EXIT_FAILURE);
	}

	/* set up our own ip header */
	int hdrincl = 1;
	if(setsockopt(send_fd, IPPROTO_IP, IP_HDRINCL, &hdrincl,
			sizeof(hdrincl)) == -1){
		perror("setsockopt() failed");
		exit(EXIT_FAILURE);
	}/**/

	setuid(getuid());/*give up privileges */

	/* not used, -- remove*/
	addr.sin_addr.s_addr = inet_addr(argv[0]);
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;

	/* set up hints for getaddrinfo() */
	hints.ai_flags = AI_CANONNAME;
	hints.ai_family = AF_INET;
	hints.ai_socktype = 0;
	hints.ai_protocol = IPPROTO_UDP;

	int err = getaddrinfo(argv[1], NULL, &hints, &res);

	/*USE BETTER ERROR MESSAGES HERE!!!! taken from _____ --find site*/
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
	ip->ip_len = htons(packet_size);
	ip->ip_id = htons(1234);
	ip->ip_src.s_addr = inet_addr("192.168.1.100");
	ip->ip_dst = ((struct sockaddr_in*) res->ai_addr)->sin_addr;
	//ip->ip_ttl = atoi(argv[2]);
	ip->ip_ttl = 255;
	ip->ip_p = IPPROTO_UDP;

	/*create udp packet*/
	int offset = sizeof(struct udphdr) + sizeof(struct ip);
	//int udp_len = SIZE - sizeof(struct ip);

	struct udphdr *udp = (struct udphdr *) (ip + 1);
	udp->uh_sport = htons(port); /* set source port*/
	udp->uh_dport = htons(port); /* set destination port */
	udp->uh_ulen = htons(udp_len); /* set udp length */

	/* fill with random data from /dev/urandom */
	/*get random data for high entropy datagrams*/
	int random = open("/dev/urandom", O_RDONLY);
	read(random, (udp + 1), udp_data_len);
	close(random);

	void* data = udp + 1;

	struct pseudo_header *ps;
	ps = (struct pseudo_header *) packet_rcv;
	ps->source = ip->ip_src.s_addr;
	ps->dest = ip->ip_dst.s_addr;
	ps->zero = 0;
	ps->proto = IPPROTO_UDP;
	ps->len = htons(udp_len);

	memcpy(ps + 1, udp, udp_len);
	udp->check = ip_checksum(ps, udp_len + sizeof(struct pseudo_header)); /* set udp checksum */
	ip->ip_sum = ip_checksum(ip, packet_size);/**/

	bzero(packet_rcv, SIZE);

	double d = get_time();
	if(fork() == 0){
		ip = (struct ip *) packet_rcv;
		icmp = (struct icmp *) (ip + 1);

		for(;;){

			if((n = recvfrom(recv_fd, packet_rcv, icmp_len, 0,
					(struct sockaddr *) &addr, &adrlen))
					< 0){
				if(errno == EINTR)
					continue;
				perror("tracert: recvfrom");
				continue;
			}else if(icmp->icmp_type != 3 && icmp->icmp_code != 3)
				continue;
			else{
				d = get_time() - d;
				break;
			}
		}

#ifdef DEBUG
		printf("\nIn Child Process\n");
		debug_print(icmp, n);
		printf("ip header:\n");
		inet_ntop(AF_INET, &(ip->ip_dst.s_addr), str, INET_ADDRSTRLEN);
		printf("destination address: %s\n", str);
		inet_ntop(AF_INET, &(ip->ip_src.s_addr), str, INET_ADDRSTRLEN);
		printf("source address: %s\n", str);
		printf("ip header length:%d\n", ip->ip_hl);
		printf("TTL: %d\n", ip->ip_ttl);
		printf("Protocol: %d\n", ip->ip_p);
		printf("version: %d\n", ip->ip_v);
#endif

		write(my_pipe[1], &d, sizeof(d));

		return EXIT_SUCCESS;
	}    // end fork()

	n = sendto(send_fd, packet_send, packet_size, 0, res->ai_addr,
			res->ai_addrlen);
	if(n == -1){
		perror("Send error");
		return EXIT_FAILURE;
	}

#ifdef DEBUG
	printf("\nIn Parent Process\n");
	printf("msg sent\n");
	printf("Sent: %d bytes.\n", n);
	printf("ip header:\n");
	inet_ntop(AF_INET, &(ip->ip_dst.s_addr), str, INET_ADDRSTRLEN);
	printf("destination address: %s\n", str);
	inet_ntop(AF_INET, &(ip->ip_src.s_addr), str, INET_ADDRSTRLEN);
	printf("source address: %s\n", str);
	printf("ip header length:%d\n", ip->ip_hl);
	printf("TTL: %d\n", ip->ip_ttl);
	printf("Protocol: %d\n", ip->ip_p);
	printf("version: %d\n", ip->ip_v);

	printf("UDP Header:\n");
	printf("Source port: %d\n", ntohs(udp->uh_sport));
	printf("Destination port: %d\n", ntohs(udp->uh_dport));
	printf("UDP Header Length: %d\n", ntohs(udp->uh_ulen));
	printf("UDP ck_sum: %d\n", udp->uh_sum);

#endif

	freeaddrinfo(res);
	read(my_pipe[0], &d, sizeof(d));
	printf("Time elapsed TTL = 255: %f sec\n", d);

	ip->ip_ttl = 3;
	ip->ip_sum = ip_checksum(ip, packet_size);/**/

	d = get_time();
	if(fork() == 0){
		ip = (struct ip *) packet_rcv;
		icmp = (struct icmp *) (ip + 1);

		for(;;){

			if((n = recvfrom(recv_fd, packet_rcv, icmp_len, 0,
					(struct sockaddr *) &addr, &adrlen))
					< 0){
				if(errno == EINTR)
					continue;
				perror("tracert: recvfrom");
				continue;
			}else if(icmp->icmp_type != 11)
				continue;
			else{
				d = get_time() - d;
				break;
			}
		}
		close(recv_fd);

#ifdef DEBUG
		printf("\nIn Child Process\n");
		debug_print(icmp, n);
		printf("ip header:\n");
		inet_ntop(AF_INET, &(ip->ip_dst.s_addr), str, INET_ADDRSTRLEN);
		printf("destination address: %s\n", str);
		inet_ntop(AF_INET, &(ip->ip_src.s_addr), str, INET_ADDRSTRLEN);
		printf("source address: %s\n", str);
		printf("ip header length:%d\n", ip->ip_hl);
		printf("TTL: %d\n", ip->ip_ttl);
		printf("Protocol: %d\n", ip->ip_p);
		printf("version: %d\n", ip->ip_v);
#endif
		write(my_pipe[1], &d, sizeof(d));

		return EXIT_SUCCESS;
	}    // end fork()

	n = sendto(send_fd, packet_send, packet_size, 0, res->ai_addr,
			res->ai_addrlen);
	if(n == -1){
		perror("Send error");
		return EXIT_FAILURE;
	}

	read(my_pipe[0], &d, sizeof(d));
	close(send_fd);
	printf("Time elapsed TTL = 3: %f sec\n", d);

	return EXIT_SUCCESS;
}

void debug_print(struct icmp *icmp, ssize_t n)
{
	printf("Receved: %zu bytes.\n", n);
	printf("Process ID: %d\n", getpid());
	printf("ICMP reply type: %d\n", icmp->icmp_type);
	printf("ICMP CODE: %d\n", icmp->icmp_code);
	printf("ICMP reply ID: %d\n", icmp->icmp_id);

}
