/**
 * @author: Paul Kirth
 * @file: ncd.c
 * Comp 429
 * Project 2 Phase III
 */

#include "ncd.h"

/*  Global Variables  */
int size, num_packets, num_tail, time_wait, done = 0;
u_int16_t port;
char entropy;
char* dst_ip;
u_int8_t ttl;

int icmp_fd, send_fd, recv_fd;
char packet_send[SIZE] = { 0 };
char icmp_send[84] = { 0 };
char packet_rcv[SIZE] = { 0 };
size_t send_len, icmp_ip_len, icmp_len, icmp_data_len, rcv_len;
struct addrinfo *res;

struct proto proto;

/*  Just returns current time as double, with most possible precision...  */
double get_time(void)
{
	struct timeval tv;
	double d;
	gettimeofday(&tv, NULL);
	d = ((double) tv.tv_usec) / 1000000. + (unsigned long) tv.tv_sec;
	return d;
}

int comp_det(char* address, u_int16_t port, char hl, size_t data_size,
		size_t num_packets, unsigned short ttl, size_t time_wait,
		int n_tail)
{
	/*size of udp data*/
	int udp_data_len = data_size;

	/* size of udp packet */
	int udp_len = udp_data_len + sizeof(struct udphdr);

	/* size of IP packet (IP header +  udp packet size*/
	send_len = udp_len + sizeof(struct ip);

	/* size of ICMP Echo message */
	icmp_data_len = 56;

	/*size of icmp packet*/
	icmp_len = sizeof(struct icmp) + icmp_data_len;

	/* size of ICMP reply + ip header */
	icmp_ip_len = sizeof(struct ip) + icmp_len;
	struct icmp *icmp; /* ICMP header */

	/*number of bytes sent*/
	int n;

	struct addrinfo hints = { 0 }; /* for get addrinfo */

	/* set up hints for getaddrinfo() */
	hints.ai_flags = AI_CANONNAME;
	hints.ai_protocol = IPPROTO_UDP;

	int err = getaddrinfo(address, NULL, &hints, &res);

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

	send_fd = socket(res->ai_family, SOCK_RAW, IPPROTO_UDP);
	if(send_fd == -1){
		perror("call to socket() failed");
		exit(EXIT_FAILURE);
	}

	/* set up our own ip header */
	int hdrincl = 1;

	int r = res->ai_family == AF_INET ? IPPROTO_IP : IPPROTO_IPV6;
	if(setsockopt(send_fd, r, IP_HDRINCL, &hdrincl, sizeof(hdrincl)) == -1){
		perror("setsockopt() failed send");
		exit(EXIT_FAILURE);
	}/**/

	/* acquire socket for icmp messages*/

	int l = res->ai_family == AF_INET ? IPPROTO_ICMP : IPPROTO_ICMPV6;
	icmp_fd = socket(res->ai_family, SOCK_RAW, l);

	if(icmp_fd == -1){
		perror("call to socket() failed");
		exit(EXIT_FAILURE);    // consider doing something better here
	}

	/* set up our own IP header*/
	int icmp_hdrincl = 1;
	if(setsockopt(icmp_fd, r, IP_HDRINCL, &icmp_hdrincl,
			sizeof(icmp_hdrincl)) == -1){
		perror("setsockopt() failed icmp");
		exit(EXIT_FAILURE);
	}

	err = setuid(getuid());/*give up privileges */

	if(err < 0){
		perror("Elevated privliges not released");
		return EXIT_FAILURE;
	}

	struct udphdr *udp;
	if(res->ai_family == AF_INET){
		udp = (struct udphdr *) (packet_send + sizeof(struct ip));
		mkipv4(packet_send, send_len, res, IPPROTO_UDP);
		mkudphdr(udp, data_size, IPPROTO_UDP);
		mkipv4(icmp_send, send_len, res, IPPROTO_ICMP);
		mkicmpv4(icmp_send + (sizeof(struct ip)), icmp_len);

	}else if(res->ai_family == AF_INET6){
		udp = (struct udphdr *) (packet_send + sizeof(struct ip6_hdr));
		mkipv6(packet_send, send_len, res, IPPROTO_UDP);
		mkudphdr(udp, data_size, IPPROTO_UDP);
		mkipv6(icmp_send, send_len, res, IPPROTO_ICMPV6);
		mkicmpv6(icmp_send + (sizeof(struct ip6_hdr)), icmp_data_len);

	}else{
		errno = EPROTONOSUPPORT;
		perror("Protocol not supported");
		return EXIT_FAILURE;
	}

	double time;
	pthread_t threads[2];
	int rc;
	register int i;
	int ret = 0;
	void *status[2];
	if(entropy == 'b' || entropy == 'l'){
		/* Acquire raw socket to listen for ICMP replies */
		recv_fd = socket(res->ai_family, SOCK_RAW, IPPROTO_ICMP);
		if(recv_fd == -1){
			perror("call to socket() failed");
			return EXIT_FAILURE;
		}

		/*increase size of receive buffer*/
		int size = 60 * 1024;
		setsockopt(recv_fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));

		rc = pthread_create(&threads[0], NULL, recv_data, &time);
		if(rc){
			printf(
					"ERROR; return code from pthread_create() is %d\n",
					rc);
			exit(-1);
		}

		int rc = pthread_create(&threads[1], NULL, send_train,
				status[1]);
		if(rc){
			printf(
					"ERROR; return code from pthread_create() is %d\n",
					rc);
			exit(-1);
		}

		for(i = 0; i < 2; ++i){
			rc = pthread_join(threads[i], &status[i]);
			if(rc){
				printf(
						"ERROR; return code from pthread_create() is %d\n",
						rc);
				exit(-1);
			}
			ret = (int) status[i] == NULL ? ret : (int) status[i];
		}

		printf("%c %f sec\n", 'L', time);
		close(recv_fd);
	}

	sleep(3);    // sloppy replace with better metric

	if(entropy == 'b' || entropy == 'h'){

		done = 0;

		fill_data((udp + 1), data_size);
		mkudphdr(udp, data_size, IPPROTO_UDP);

		/* Acquire raw socket to listen for ICMP replies */
		recv_fd = socket(res->ai_family, SOCK_RAW, IPPROTO_ICMP);
		if(recv_fd == -1){
			perror("call to socket() failed");
			return EXIT_FAILURE;
		}

		/*increase size of receive buffer*/
		int size = 60 * 1024;
		setsockopt(recv_fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));

		rc = pthread_create(&threads[0], NULL, recv_data, &time);
		if(rc){
			printf(
					"ERROR; return code from pthread_create() is %d\n",
					rc);
			exit(-1);
		}

		int rc = pthread_create(&threads[1], NULL, send_train,
				(void *) &n_tail);
		if(rc){
			printf(
					"ERROR; return code from pthread_create() is %d\n",
					rc);
			exit(-1);
		}

		for(i = 0; i < 2; ++i){
			rc = pthread_join(threads[i], &status[i]);
			if(rc){
				printf(
						"ERROR; return code from pthread_create() is %d\n",
						rc);
				exit(-1);
			}
			ret = (int) status == 0 ? ret : (int) status[i];
		}
		printf("%c %f sec\n", 'H', time);
		close(recv_fd);
	}

	if(ret != 0)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

int mkipv4(void* buff, size_t size, struct addrinfo *res, u_int8_t proto)
{
	/* create IP header*/
	struct ip *ip = (struct ip *) buff;
	ip->ip_v = 4;
	ip->ip_hl = 5;
	ip->ip_len = htons(size);
	ip->ip_id = htons(1234);

	/* get a better way to assign my IP address!!!*/
	ip->ip_src.s_addr = inet_pton("192.168.1.100");
	ip->ip_dst = ((struct sockaddr_in*) res->ai_addr)->sin_addr;
	ip->ip_off |= ntohs(IP_DF);
	ip->ip_ttl = ttl;
	ip->ip_p = proto;
	return 0;
}

int mkipv6(void* buff, size_t size, struct addrinfo *res, u_int8_t proto)
{
	struct ip6_hdr *ip = (struct ip6_hdr *) buff;
	ip->ip6_dst = ((struct sockaddr_in6*) res->ai_addr)->sin6_addr;
	inet_pton(AF_INET6, "192.168.1.100", &ip->ip6_src);
	ip->ip6_ctlun.ip6_un1.ip6_un1_flow = 0;
	ip->ip6_ctlun.ip6_un1.ip6_un1_hlim = ttl;
	ip->ip6_ctlun.ip6_un1.ip6_un1_nxt = htons(sizeof(struct ip6_hdr));
	ip->ip6_ctlun.ip6_un1.ip6_un1_plen = htons(send_len);

	return 0;
}

int mkudphdr(void* buff, size_t udp_data_len, u_int8_t proto)
{
	struct ip* ip = buff;
	ip--;
	int udp_len = udp_data_len + sizeof(struct udphdr);
	char pseudo[SIZE] = { 0 }; /* buffer for pseudo header */
	struct udphdr *udp = (struct udphdr *) buff;
	udp->uh_sport = htons(port); /* set source port*/
	udp->uh_dport = htons(port); /* set destination port */
	udp->uh_ulen = htons(udp_len); /* set udp length */
	udp->check = 0;/* zero out the udp checksum */

	/*printf("ip: %d\n", ip);
	 printf("udp, %d, udp+1: %d\n", udp, udp + 1);
	 fill_data(udp + 1, udp_data_len);/**/

	/* pseudo header for udp checksum */
	struct pseudo_header *ps = (struct pseudo_header *) pseudo;
	ps->source = ip->ip_src.s_addr;
	ps->dest = ip->ip_dst.s_addr;
	ps->zero = 0;
	ps->proto = proto;
	ps->len = htons(udp_len);

	/*printf("udp_data: %d, udp_len: %d, ps: %d, ps_len %d\n", udp_data_len,
	 udp_len, sizeof(struct pseudo_header),
	 udp_len + sizeof(struct pseudo_header));
	 printf("ps: %d, ps+1: %d\n", ps, ps+1);/**/

	/*copy udp packet into pseudo header buffer to calculate checksum*/
	memcpy(ps + 1, udp, udp_len);

	/* set udp checksum */

	udp->check = ip_checksum(ps, udp_len + sizeof(struct pseudo_header));

	return 0;
}

int mkicmpv4(void *buff, size_t datalen)
{
	/* set up icmp message header*/
	struct icmp *icmp = (struct icmp *) buff;
	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	icmp->icmp_id = (u_int16_t) getpid();
	icmp->icmp_seq = (u_int16_t) rand();
	memset(icmp->icmp_data, 0xa5, datalen);
	gettimeofday((struct timeval *) icmp->icmp_data, NULL);
	icmp->icmp_cksum = 0;
	icmp->icmp_cksum = ip_checksum(icmp, datalen + sizeof(struct icmp));
	return 0;
}

int mkicmpv6(void *buff, size_t datalen)
{
	struct icmp6_hdr *icmphdr = (struct icmp6_hdr *) buff;
	icmphdr->icmp6_type = ICMP6_ECHO_REQUEST;
	icmphdr->icmp6_code = 0;
	icmphdr->icmp6_cksum = 0;
	icmphdr->icmp6_id= htons (getpid());
	icmphdr->icmp6_seq= htons (rand());
	memset(&icmphdr->icmp6_dataun, 0xa5, datalen);
	gettimeofday((struct timeval *) &icmphdr->icmp6_dataun, NULL);
	return 0;
}

void *send_train(void* num)
{

	//printf("Tail size: %d\n", num_tail);
	/*send Head ICMP Packet*/
	int n = sendto(icmp_fd, icmp_send, icmp_ip_len, 0, res->ai_addr,
			res->ai_addrlen);
	if(n == -1){
		perror("Send error ICMP head");
		pthread_exit((void*) EXIT_FAILURE);
	}

	/*send data train*/
	int i = 0;
	for(i = 0; i < num_packets; ++i){
		n = sendto(send_fd, packet_send, send_len, 0, res->ai_addr,
				res->ai_addrlen);
		if(n == -1){
			perror("Send error udp train");
			pthread_exit((void*) EXIT_FAILURE);
		}
	}

	struct icmp *icmp = (struct icmp *) (icmp_send + sizeof(struct ip));
	/*send tail ICMP Packets w/ timer*/
	for(i = 0; i < num_tail && done == 0; ++i){
		/*not sure if changing the sequence number will help*/
		icmp->icmp_cksum = 0;
		icmp->icmp_seq += 1;
		icmp->icmp_cksum = ip_checksum(icmp, icmp_len);

		n = sendto(icmp_fd, icmp_send, icmp_ip_len, 0, res->ai_addr,
				res->ai_addrlen);
		if(n == -1){
			perror("Send error icmp tail");
			pthread_exit((void*) EXIT_FAILURE);
		}
		usleep(time_wait * 1000);
	}
	pthread_exit((void*) EXIT_SUCCESS);
}

void fill_data(void *buff, size_t size)
{
	/* fill with random data from /dev/urandom */
	/*get random data for high entropy datagrams*/
	int random = open("/dev/urandom", O_RDONLY);
	read(random, buff, size);
	close(random);
}

int send_data(char* address, u_int16_t port, char hl, size_t data_size,
		size_t num_packets, unsigned short ttl, size_t time_wait,
		int n_tail)
{
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
	struct addrinfo hints = { 0 }; /* for get addrinfo */

	char packet_send[SIZE] = { 0 }; /* buffer to send data with */
	char icmp_packet[84];/* icmp packet buffer */

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
	mkipv4(packet_send, packet_size, res, IPPROTO_UDP);
	mkudphdr(packet_send + sizeof(struct ip), udp_data_len, IPPROTO_UDP);

	/*Create ICMP Packets*/
	/* create IP header for icmp packet */
	mkipv4(icmp_packet, icmp_len, res, IPPROTO_ICMP);
	mkicmpv4(icmp_packet + sizeof(struct ip), datalen);

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

	icmp = (struct icmp *) (icmp_packet + sizeof(struct ip));
	/*send tail ICMP Packets w/ timer*/
	for(i = 0; i < n_tail; ++i){
		/*not sure if changing the sequence number will help*/
		icmp->icmp_cksum = 0;
		icmp->icmp_seq += 1;
		icmp->icmp_cksum = ip_checksum(icmp, len);

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

void* recv_data(void *t)
{
	double* time = (double *) t;

	/*number of bytes received*/
	int n;

	/* number of echo replies*/
	int count = 0;

	/*number of port unreachable replies processed and ignored*/
	int ack = 0;

	/* data for ICMP msg */
	size_t datalen = 56;

	if(res->ai_family == AF_INET){

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

		/*Receive initial ICMP echo response && Time-stamp*/
		struct ip *ip = (struct ip *) packet_rcv;
		icmp = (struct icmp *) (ip + 1);

		for(;;){

			if((n = recvfrom(recv_fd, packet_rcv, icmp_len, 0,
					(struct sockaddr *) &addr, &adrlen))
					< 0){
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
					done = 1;
					break;
				}    //end if
			}else if(icmp->icmp_type == 11){
				errno = ENETUNREACH;
				perror("TTL Exceeded");
				pthread_exit((void*) EXIT_FAILURE);
			}    // end if
		}    // end for
	}else{
		/*size of icmp packet*/
		size_t len = sizeof(struct icmp6_hdr) + datalen;

		/* size of ICMP reply + ip header */
		size_t icmp_len = sizeof(struct ip6_hdr) + len;

		/* ICMP header */
		struct icmp6_hdr *icmp;

		/* to receive data with*/
		struct sockaddr_in addr;

		/* length of address */
		socklen_t adrlen = sizeof(addr);

		/*Receive initial ICMP echo response && Time-stamp*/
		struct ip6_hdr *ip = (struct ip6_hdr *) packet_rcv;
		icmp = (struct icmp6_hdr *) (ip + 1);

		for(;;){

			if((n = recvfrom(recv_fd, packet_rcv, icmp_len, 0,
					(struct sockaddr *) &addr, &adrlen))
					< 0){
				if(errno == EINTR)
					continue;
				perror("recvfrom failed");
				continue;
			}else if(icmp->icmp6_type == 3
					&& icmp->icmp6_code == 3){
				ack++;
				continue;
			}else if(icmp->icmp6_type == 0){
				if(count == 0){
					*time = get_time();
					count = 1;
				}else{
					*time = get_time() - *time;
					done = 1;
					break;
				}    //end if
			}else if(icmp->icmp6_type == 11){
				errno = ENETUNREACH;
				perror("TTL Exceeded");
				pthread_exit((void*) EXIT_FAILURE);
			}    // end if
		}    // end for
	}
	printf("\nUDP Packets received: %d\n", ack);
	pthread_exit((void*) EXIT_SUCCESS);
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

int check_args(int argc, char* argv[])
{
	dst_ip = NULL;

	if(argc == 2){
		dst_ip = argv[1];
		/* probably change default port from traceroute port */
		port = 33434;
		entropy = 'b';
		size = 996;
		num_packets = 1000;
		ttl = 255;
		time_wait = 10;
		num_tail = 20;
	}else{
		register int i;
		int check;
		char* cp;
		char c;
		for(i = 1; i < argc; ++i){
			cp = argv[i];
			c = *cp;
			if(c == '-'){
				c = tolower(*(cp + 1));
				i++;
				switch(c){
				case 'p':
					check = atoi(argv[i]);
					if(check < (1 << 16) && check > 0)
						port = check;
					else{
						errno = ERANGE;
						perror("Port range: 0 - 65535");
						return EXIT_FAILURE;
					}
					break;
				case 'h':
				case 'l':
					entropy = c;
					i--;
					break;
				case 's':
					size = atoi(argv[i]);
					if(size < 0 || size > 1460){
						errno = ERANGE;
						perror(
								"Valid UDP data size: 1-1460");
						return EXIT_FAILURE;
					}
					break;
				case 'n':
					num_packets = atoi(argv[i]);
					if(num_packets < 1
							|| num_packets > 10000){
						errno = ERANGE;
						perror(
								"# UDP packets: 1 - 10,000");
						return EXIT_FAILURE;
					}
					break;
				case 't':
					check = atoi(argv[i]);
					if(check < 0 || check > 255){
						errno = ERANGE;
						perror("TTL range: 0 - 255");
						return EXIT_FAILURE;
					}
					break;
				case 'w':
					time_wait = atoi(argv[i]);
					if(time_wait < 0){
						errno = ERANGE;
						perror(
								"Time wait must be positive");
						return EXIT_FAILURE;
					}
					break;
				case 'r':
					num_tail = atoi(argv[i]);
					if(num_tail < 1 || num_tail > 1000){
						errno = ERANGE;
						perror(
								"# tail packets: 1 - 1,000");
						return EXIT_FAILURE;
					}
					break;
				default:
					errno = ERANGE;
					perror("Invalid options, check use");
					return EXIT_FAILURE;
				}	//end switch
			}else if(dst_ip == NULL){
				dst_ip = argv[i];
			}else{
				errno = ERANGE;
				perror("Too many IP Addresses, check use");
				return EXIT_FAILURE;
			}	// end if

		}	//end for
	}	//endif

	return EXIT_SUCCESS;
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
	/* Check ARGs */
	check_args(argc, argv);
	return comp_det(dst_ip, port, entropy, size, num_packets, ttl,
			time_wait, num_tail);
}
