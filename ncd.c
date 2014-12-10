/**
 * @author: Paul Kirth
 * @file: ncd.c
 * Comp 429
 * Project 2 Phase III
 */

#include "ncd.h"
/*  Just returns current time as double, with most possible precision...  */
double get_time(void)
{
	struct timeval tv;
	double d;
	gettimeofday(&tv, NULL);
	d = ((double) tv.tv_usec) / 1000000. + (unsigned long) tv.tv_sec;
	return d;
}

int comp_det(char* address, char * port, char hl, size_t data_size,
		size_t num_packets, unsigned short ttl, size_t time_wait,
		int n_tail)
{
	pid_t child;
	char c = tolower(hl);
	if(c != 'h' && c != 'l'){
		perror(
				"Invalid setting for High or Low entropy data, use 'H', or 'L'");
		exit(EXIT_FAILURE);
	}

	if(SIZE <= data_size + 40 + sizeof(struct udphdr)){
		perror("Maximum packet size exceeded");
		exit(EXIT_FAILURE);
	}

	if((child = fork()) == 0){
		return send_data(address, port, hl, data_size, num_packets, ttl,
				time_wait, n_tail);

	} //end fork()

	double time;
	int ret = recv_data(&time);
#ifndef NCD_NO_KILL
	kill(child, SIGKILL);
#endif
	if(ret != 0)
		return EXIT_FAILURE;

	printf("%c %f sec\n", hl, time);

	return EXIT_SUCCESS;
}

int send_data(char* address, char * port_name, char hl, size_t data_size,
		size_t num_packets, unsigned short ttl, size_t time_wait,
		int n_tail)
{
	size_t nsent = (size_t) rand();/*get random number for seq #*/
	int port = atoi(port_name);

	/*size of udp data*/
	int udp_data_len = data_size;

	/* size of udp packet */
	int udp_len = udp_data_len + 8;

	/* size of IP packet (IP header +  udp packet size*/
	int packet_size = udp_len + 20;

	/* size of ICMP Echo message */
	size_t datalen = 56;

	/*size of icmp packet*/
	size_t len = sizeof(struct icmp) + datalen;

	/* size of ICMP reply + ip header */
	size_t icmp_len = sizeof(struct ip) + len;
	struct icmp *icmp; /* ICMP header */

	/*file descriptors for udp and icmp sockets */
	int send_fd, icmp_fd;

	/*number of bytes sent*/
	int n;

	struct addrinfo *res; /* for get addrinfo */
	struct addrinfo hints = { 0 }; /* for get addrinfor */

	char packet_send[SIZE] = { 0 }; /* buffer to send data with */
	char pseudo[SIZE] = { 0 }; /* buffer for pseudo header */

	char icmp_packet[84];/*icmp packet buffer*/

	send_fd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);

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

	/* acquire socket for icmp messages*/
	icmp_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

	if(icmp_fd == -1){
		perror("call to socket() failed");
		exit(EXIT_FAILURE);    // consider doing something better here
	}

	/* set up our own IP header*/
	int icmp_hdrincl = 1;
	if(setsockopt(icmp_fd, IPPROTO_IP, IP_HDRINCL, &icmp_hdrincl,
			sizeof(icmp_hdrincl)) == -1){
		perror("setsockopt() failed");
		exit(EXIT_FAILURE);
	}

	int err = setuid(getuid());/*give up privileges */

	if(err < 0){
		perror("Elevated privliges not released");
		exit(EXIT_FAILURE);
	}

	/* set up hints for getaddrinfo() */
	hints.ai_flags = AI_CANONNAME;
	hints.ai_family = AF_INET;
	hints.ai_socktype = 0;
	hints.ai_protocol = IPPROTO_UDP;

	err = getaddrinfo(address, NULL, &hints, &res);

	/*taken from http://stackoverflow.com/questions/17914550/getaddrinfo-error-success*/
	if(err != 0){
		if(err == EAI_SYSTEM)
			fprintf(stderr, "looking up %s: %s\n", address,
					strerror(errno));
		else
			fprintf(stderr, "looking up %s: %s\n", address,
					gai_strerror(err));
		exit(EXIT_FAILURE);
	}

	/* create IP header*/
	struct ip *ip = (struct ip *) packet_send;
	ip->ip_v = 4;
	ip->ip_hl = 5;
	ip->ip_len = htons(packet_size);
	ip->ip_id = htons(1234);

	/* get a better way to assign my IP address!!!*/
	ip->ip_src.s_addr = inet_addr("192.168.1.100");
	ip->ip_dst = ((struct sockaddr_in*) res->ai_addr)->sin_addr;
	ip->ip_off |= ntohs(IP_DF);
	ip->ip_ttl = ttl;
	ip->ip_p = IPPROTO_UDP;

	/*create udp packet*/
	struct udphdr *udp = (struct udphdr *) (ip + 1);
	udp->uh_sport = htons(port); /* set source port*/
	udp->uh_dport = htons(port); /* set destination port */
	udp->uh_ulen = htons(udp_len); /* set udp length */

	/* fill with random data from /dev/urandom */
	/*get random data for high entropy datagrams*/
	if('h' == tolower(hl)){
		int random = open("/dev/urandom", O_RDONLY);
		read(random, (udp + 1), udp_data_len);
		close(random);
	}

	/* pseudo header for udp checksum */
	struct pseudo_header *ps = (struct pseudo_header *) pseudo;
	ps->source = ip->ip_src.s_addr;
	ps->dest = ip->ip_dst.s_addr;
	ps->zero = 0;
	ps->proto = IPPROTO_UDP;
	ps->len = htons(udp_len);

	/*copy udp packet into pseudo header buffer to calculate checksum*/
	memcpy(ps + 1, udp, udp_len);

	/* set udp checksum */
	udp->check = ip_checksum(ps, udp_len + sizeof(struct pseudo_header));
	ip->ip_sum = ip_checksum(ip, packet_size);/**/

	/*Create ICMP Packets*/
	/* create IP header for icmp packet */
	struct ip *ip_icmp = (struct ip *) icmp_packet;
	ip_icmp->ip_v = 4;
	ip_icmp->ip_hl = 5;
	ip_icmp->ip_len = htons(icmp_len);
	ip_icmp->ip_off |= ntohs(IP_DF);
	ip_icmp->ip_id = htons(1234);
	ip_icmp->ip_dst = ((struct sockaddr_in*) res->ai_addr)->sin_addr;
	ip_icmp->ip_ttl = ttl;
	ip_icmp->ip_p = IPPROTO_ICMP;

	/* set up icmp message header*/
	icmp = (struct icmp *) (ip_icmp + 1);
	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	icmp->icmp_id = (u_int16_t) getpid();
	icmp->icmp_seq = (u_int16_t) nsent++;
	memset(icmp->icmp_data, 0xa5, datalen);
	gettimeofday((struct timeval *) icmp->icmp_data, NULL);
	icmp->icmp_cksum = 0;

	icmp->icmp_cksum = ip_checksum(icmp, len);
	ip_icmp->ip_sum = ip_checksum(ip_icmp, icmp_len);

	/*send Head ICMP Packet*/

	n = sendto(icmp_fd, icmp_packet, icmp_len, 0, res->ai_addr,
			res->ai_addrlen);
	if(n == -1){
		perror("Send error");
		return EXIT_FAILURE;
	}

	/*send data train*/
	int i = 0;
	for(i = 0; i < num_packets; ++i){
		n = sendto(send_fd, packet_send, packet_size, 0, res->ai_addr,
				res->ai_addrlen);
		if(n == -1){
			perror("Send error");
			return EXIT_FAILURE;
		}
	}

	/*send tail ICMP Packets w/ timer*/
	for(i = 0; i < n_tail; ++i){
		/*not sure if changing the sequence number will help*/
		icmp->icmp_cksum = 0;
		icmp->icmp_seq += 1;
		icmp->icmp_cksum = ip_checksum(icmp, len);
		ip_icmp->ip_sum = ip_checksum(ip_icmp, icmp_len);

		n = sendto(icmp_fd, icmp_packet, icmp_len, 0, res->ai_addr,
				res->ai_addrlen);
		if(n == -1){
			perror("Send error");
			return EXIT_FAILURE;
		}
		usleep(time_wait * 1000);
	}

	freeaddrinfo(res);/* give back the memory from getaddrinfo()*/
	return EXIT_SUCCESS;
}

int recv_data(double *time)
{

	/*number of bytes received*/
	int n;

	/* number of echo replies*/
	int count = 0;

	/*number of port unreachable replies processed and ignored*/
	int ack = 0;

	/* data for ICMP msg */
	size_t datalen = 56;

	/*size of icmp packet*/
	size_t len = sizeof(struct icmp) + datalen;

	/* size of ICMP reply + ip header */
	size_t icmp_len = sizeof(struct ip) + len;

	/* ICMP header */
	struct icmp *icmp;

	/* to receive data with*/
	struct sockaddr_in addr;

	/* length of address */
	socklen_t adrlen = sizeof(addr);

	/* buffer to receive data into */
	char packet_rcv[SIZE] = { 0 };

	/*Acquire raw socket to listen for ICMP replies*/
	int recv_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

	if(recv_fd == -1){
		perror("call to socket() failed");
		return (EXIT_FAILURE);
	}

	/*increase size of receive buffer*/
	int size = 60 * 1024;
	setsockopt(recv_fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));

	/*Receive initial ICMP echo response && Timestamp*/
	struct ip *ip = (struct ip *) packet_rcv;
	icmp = (struct icmp *) (ip + 1);

	for(;;){

		if((n = recvfrom(recv_fd, packet_rcv, icmp_len, 0,
				(struct sockaddr *) &addr, &adrlen)) < 0){
			if(errno == EINTR)
				continue;
			perror("recvfrom failed");
			continue;
		}else if(icmp->icmp_type == 3 && icmp->icmp_code == 3){
			ack++;
			continue;
		}else if(icmp->icmp_type == 0){
			if(count == 0){
				*time = get_time();
				count = 1;
			}else{
				*time = get_time() - *time;
				break;
			}    //end if
		}else if(icmp->icmp_type == 11){
			errno = ENETUNREACH;
			perror("TTL Exceeded");
			return EXIT_FAILURE;
		}    // end if
	}    // end for

	printf("\nUDP Packets received: %d\n", ack);
	return EXIT_SUCCESS;
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

/**
 * Main function
 * only calls comp_detection()
 *
 * @argv[1] Destination IP address
 * @argv[2] Port Number
 * @argv[3] High or low entropy data 'H' or 'L'
 * @argv[4] Size of udp data
 * @argv[5] Number of packets in UDP Data Train
 * @argv[6] Time to Live
 * @argv[7] Wait time in milliseconds
 * @argv[8] Number of tail ICMP messages to send
 *
 */
int main(int argc, char *argv[])
{
	return comp_det(argv[1], argv[2], argv[3][0], atoi(argv[4]),
			atoi(argv[5]), atoi(argv[6]), atoi(argv[7]),
			atoi(argv[8]));
}
