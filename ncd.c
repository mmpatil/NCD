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
		size_t num_packets, ushort ttl, size_t time_wait, int n_tail)
{
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

	if(fork() == 0){
		send_data(address, port, hl, data_size, num_packets, ttl,
				time_wait, n_tail);
	}

	double time = recv_data();
	printf("Time elapsed was: %f s", time);

	return 0;
}

int send_data(char* address, char * port_name, char hl, size_t data_size,
		size_t num_packets, ushort ttl, size_t time_wait, int n_tail)
{
	size_t nsent = (size_t) rand();/*get random number for seq #*/
	int port = atoi(port_name);

	int udp_data_len = data_size;
	int udp_len = udp_data_len + 8;
	int packet_size = udp_len + 20;
	size_t datalen = 56; /* data for ICMP msg */
	size_t len = sizeof(struct icmp) + datalen; /*size of icmp packet*/
	size_t icmp_len = sizeof(struct ip) + len; /* size of ICMP reply + ip header */
	struct icmp *icmp; /* ICMP header */

	int send_fd, icmp_fd, n;

	struct addrinfo *res; /* for get addrinfo */
	struct addrinfo hints = { 0 }; /* for get addrinfor */

	char packet_send[SIZE] = { 0 }; /* buffer to send data with */
	char pseudo[SIZE] = { 0 }; /* buffer for pseudo header */

	char icmp_packet[64];

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

	icmp_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

	if(icmp_fd == -1){
		perror("call to socket() failed");
		exit(EXIT_FAILURE);    // consider doing something better here
	}

	int icmp_hdrincl = 1;
	if(setsockopt(icmp_fd, IPPROTO_IP, IP_HDRINCL, &icmp_hdrincl,
			sizeof(icmp_hdrincl)) == -1){
		perror("setsockopt() failed");
		exit(EXIT_FAILURE);
	}
	/**/

	setuid(getuid());/*give up privileges */

	/* set up hints for getaddrinfo() */
	hints.ai_flags = AI_CANONNAME;
	hints.ai_family = AF_INET;
	hints.ai_socktype = 0;
	hints.ai_protocol = IPPROTO_UDP;

	int err = getaddrinfo(address, NULL, &hints, &res);

	/*USE BETTER ERROR MESSAGES HERE!!!! taken from http://stackoverflow.com/questions/17914550/getaddrinfo-error-success*/
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
	ip->ip_off |= ntohs(IP_DF);
	//ip->ip_ttl = atoi(argv[2]);
	ip->ip_ttl = ttl;
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
	if('h' == tolower(hl)){
		int random = open("/dev/urandom", O_RDONLY);
		read(random, (udp + 1), udp_data_len);
		close(random);
	}

	struct pseudo_header *ps;
	ps = (struct pseudo_header *) pseudo;
	ps->source = ip->ip_src.s_addr;
	ps->dest = ip->ip_dst.s_addr;
	ps->zero = 0;
	ps->proto = IPPROTO_UDP;
	ps->len = htons(udp_len);

	memcpy(ps + 1, udp, udp_len);
	udp->check = ip_checksum(ps, udp_len + sizeof(struct pseudo_header)); /* set udp checksum */
	ip->ip_sum = ip_checksum(ip, packet_size);/**/

	/*Create ICMP Packets*/
	/* create IP header*/
	struct ip *ip_icmp = (struct ip *) icmp_packet;
	ip_icmp->ip_v = 4;
	ip_icmp->ip_hl = 5;
	ip_icmp->ip_len = sizeof(struct ip) + len;
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
		n = sendto(icmp_fd, icmp_packet, icmp_len, 0, res->ai_addr,
				res->ai_addrlen);
		if(n == -1){
			perror("Send error");
			return EXIT_FAILURE;
		}
		sleep(time_wait);	//fix this !!!!!!!!!!!!!!!!!!!!!!!!!!!!
	}

	printf("\nfinished sending\n");
	int ret = 0;

	return ret;
}

double recv_data()
{

	int n;
	size_t datalen = 56; /* data for ICMP msg */
	size_t len = sizeof(struct icmp) + datalen; /*size of icmp packet*/
	size_t icmp_len = sizeof(struct ip) + len; /* size of ICMP reply + ip header */
	struct icmp *icmp; /* ICMP header */

	struct sockaddr_in addr; /* to recieve data with*/
	socklen_t adrlen = sizeof(addr); /* length of address */

	char packet_rcv[SIZE] = { 0 }; /* buffer to recieve data into */

	int recv_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

	if(recv_fd == -1){
		perror("call to socket() failed");
		exit(EXIT_FAILURE);
	}

	int size = 60 * 1024;
	setsockopt(recv_fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));

	/*Recieve initial ICMP echo response && Timestamp*/
	struct ip *ip = (struct ip *) packet_rcv;
	icmp = (struct icmp *) (ip + 1);
	int count = 0;
	double d;
	for(;;){

		if((n = recvfrom(recv_fd, packet_rcv, icmp_len, 0,
				(struct sockaddr *) &addr, &adrlen)) < 0){
			if(errno == EINTR)
				continue;
			perror("tracert: recvfrom");
			continue;
		}else if(icmp->icmp_type == 8 && count == 0){
			d = get_time();
			count= 1;
			printf("Received first reply\n");

		}else if(icmp->icmp_type == 8 && count == 1){
			d = get_time() - d;
			printf("Received last reply\n");
			break;
		}
	}


	return d;
}

/*
 struct icmphdr* make_icmp(char *address, unsigned char code, void * data, ssize_t data_len){

 }
 */

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

int main(int argc, char *argv[])
{
	return comp_det(argv[1], argv[2], argv[3][0], atoi(argv[4]),
			atoi(argv[5]), atoi(argv[6]), atoi(argv[7]), atoi(argv[8]));

}
