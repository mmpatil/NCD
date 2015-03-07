/**
 * @author: Paul Kirth
 * @file: ncd.c
 */

#include "ncd.h"
#include "bitset.h"

/*  Global Variables  */

/* input arguments */
int data_size = 996;        	// size of udp data payload -- set to 996 by default
int num_packets = 1000;       	// number of packets in udp data train -- set to 1000 by default
int num_tail = 20;        	// number of tail icmp messages sent tail_wait apart -- set to 20 by default
int tail_wait = 10;        	// time between ICMP tail messages -- set to 10 by default
u_int16_t port = 33434; 	// port number -- set to 33434 by default
char* dst_ip = NULL; 		// destination ip address
char* file = "/dev/urandom";    //name of file to read from -- set to /dev/urandom by default
u_int8_t ttl = 255;		// time to live -- set to 255 by default
int lflag = 1;    		// default option for low entropy -- set to on
int hflag = 1;   		// default option for high entropy -- set to on
int fflag = 0;        		// file flag <------- do we need this or is this redundant?

int done = 0;        		// boolean for sending packets -- set to false by default

int icmp_fd; 			//icmp socket file descriptor
int send_fd; 			//udp socket file descriptor
int recv_fd; 			//reply receiving socket file descriptor
char packet_send[SIZE] = { 0 };        // buffer for sending data
uint16_t* packet_id = (uint16_t*) packet_send;        //sequence/ID number of udp msg
char icmp_send[128] = { 0 };        // buffer for ICMP messages
char packet_rcv[SIZE] = { 0 };        // buffer for receiving replies
size_t send_len;		// length of data to be sent
size_t icmp_ip_len;		// length of IP icmp packet including payload
size_t icmp_len;		// length of ICMP packet
size_t icmp_data_len;		// length of ICMP data
size_t rcv_len;			// length of data to be received
struct addrinfo *res = NULL;        // addrinfo struct for getaddrinfo()
void *(*recv_data)(void*) = NULL;        // function pointer so we can select properly for IPV4 or IPV6

/*  Just returns current time as double, with most possible precision...  */
double get_time(void)
{
	struct timeval tv;
	double d;
	gettimeofday(&tv, NULL);
	d = ((double) tv.tv_usec) / 1000000. + (unsigned long) tv.tv_sec;
	return d;
}

int comp_det()
{
	send_len = data_size + sizeof(uint16_t);

	/* size of ICMP Echo message */
	icmp_data_len = 56;

	/*size of icmp packet*/
	icmp_len = sizeof(struct icmp) + icmp_data_len;

	/* size of ICMP reply + ip header */
	icmp_ip_len = sizeof(struct ip) + icmp_len;

	/* set up hints for getaddrinfo() */
	struct addrinfo hints = { 0 }; /* for get addrinfo */
	hints.ai_flags = AI_CANONNAME;
	hints.ai_protocol = IPPROTO_UDP;

	/* FIX THIS !!!!!!!!!!!!!!!*/
	char str[32] = { 0 };
	snprintf(str, 32, "%d", port);

	int err = getaddrinfo(dst_ip, str, &hints, &res);

	/*taken from http://stackoverflow.com/questions/17914550/getaddrinfo-error-success*/
	if(err != 0){
		if(err == EAI_SYSTEM)
			fprintf(stderr, "looking up %s: %s\n", dst_ip,
					strerror(errno));
		else
			fprintf(stderr, "looking up %s: %s\n", dst_ip,
					gai_strerror(err));
		exit(EXIT_FAILURE);
	}

	/* setup socket for UDP train */
	send_fd = socket(res->ai_family, SOCK_DGRAM, IPPROTO_UDP);
	if(send_fd == -1){
		perror("call to socket() failed");
		exit(EXIT_FAILURE);
	}

	int r = (res->ai_family == AF_INET) ? IPPROTO_IP : IPPROTO_IPV6;

	setsockopt(send_fd, r, IP_TTL, &ttl, sizeof(ttl));

	/* acquire socket for icmp messages*/
	int l = res->ai_family == AF_INET ? IPPROTO_ICMP : IPPROTO_ICMPV6;
	icmp_fd = socket(res->ai_family, SOCK_RAW, l);

	if(icmp_fd == -1){
		perror("call to socket() failed");
		exit(EXIT_FAILURE);        // consider doing something better here
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

	//make ICMP packets
	if(res->ai_family == AF_INET){
		mkipv4(icmp_send, icmp_len, res, IPPROTO_ICMP);
		mkicmpv4(icmp_send + (sizeof(struct ip)), icmp_len);
		recv_data = recv4;

	}else if(res->ai_family == AF_INET6){
		mkipv6(icmp_send, icmp_len, res, IPPROTO_ICMPV6);
		mkicmpv6(icmp_send + (sizeof(struct ip6_hdr)), icmp_data_len);
		recv_data = recv6;
	}else{
		errno = EPROTONOSUPPORT;
		perror("Protocol not supported");
		return EXIT_FAILURE;
	}

	double time;
	pthread_t threads[2];
	int rc;
	register int i;
	void *status[2];
	if(lflag == 1){
		done = 0;        //boolean false

		/* Acquire raw socket to listen for ICMP replies */
		recv_fd = socket(res->ai_family, SOCK_RAW, IPPROTO_ICMP);
		if(recv_fd == -1){
			perror("call to socket() failed");
			return EXIT_FAILURE;
		}

		/*increase size of receive buffer*/
		int size = 1500 * num_packets;
		int buffsize;
		socklen_t bufflen = sizeof(buffsize);
		setsockopt(recv_fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
		getsockopt(recv_fd, SOL_SOCKET, SO_RCVBUF, (void*) &buffsize,
				&bufflen);
		printf("Receive Buffer size: %d\n", buffsize);
		rc = pthread_create(&threads[0], NULL, recv_data, &time);
		if(rc){
			printf("ERROR CODE from pthread_create() is %d\n", rc);
			exit(-1);
		}

		int rc = pthread_create(&threads[1], NULL, send_train,
				status[1]);
		if(rc){
			printf("ERROR CODE from pthread_create() is %d\n", rc);
			exit(-1);
		}

		for(i = 0; i < 2; ++i){
			rc = pthread_join(threads[i], &status[i]);
			if(rc){
				printf("ERROR CODE from pthread_create()"
						" is %d\n",
						rc);
				exit(-1);
			}

			if(status[i] != NULL)
				return EXIT_FAILURE;

		}        //end for

		printf("%c %f sec\n", 'L', time);
		close(recv_fd);
	}

	sleep(3);        // sloppy replace with better metric

	if(hflag == 1){

		done = 0;        //boolean false

		fill_data(packet_send, data_size);

		/* Acquire raw socket to listen for ICMP replies */
		recv_fd = socket(res->ai_family, SOCK_RAW, l);
		if(recv_fd == -1){
			perror("call to socket() failed");
			return EXIT_FAILURE;
		}

		/*increase size of receive buffer*/
		int size = 60 * 1024;
		setsockopt(recv_fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));

		rc = pthread_create(&threads[0], NULL, recv_data, &time);
		if(rc){
			printf("ERROR CODE from pthread_create() is %d\n", rc);
			exit(-1);
		}

		int rc = pthread_create(&threads[1], NULL, send_train,
				(void *) &num_tail);
		if(rc){
			printf("ERROR CODE from pthread_create() is %d\n", rc);
			exit(-1);
		}

		for(i = 0; i < 2; ++i){
			rc = pthread_join(threads[i], &status[i]);
			if(rc){
				printf("ERROR CODE from"
						" pthread_create() is %d\n",
						rc);
				exit(-1);
			}
			if(status[i] != NULL)
				return EXIT_FAILURE;
		}
		printf("%c %f sec\n", 'H', time);
		close(recv_fd);
	}
	freeaddrinfo(res);
	return EXIT_SUCCESS;
}

int mkipv4(void* buff, size_t size, struct addrinfo *res, u_int8_t proto)
{
	/* create IP header*/
	struct ip *ip = (struct ip *) buff;
	ip->ip_v = 4;
	ip->ip_hl = 5;
	ip->ip_len = htons(size);
	ip->ip_id = htons(getpid());

	/* get a better way to assign my IP address!!!*/
	//inet_pton(AF_INET, "192.168.1.101", &ip->ip_src.s_addr);
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
	//inet_pton(AF_INET6, "192.168.1.101", &ip->ip6_src);
	ip->ip6_ctlun.ip6_un1.ip6_un1_flow = 0;
	ip->ip6_ctlun.ip6_un1.ip6_un1_hlim = ttl;
	ip->ip6_ctlun.ip6_un1.ip6_un1_nxt = htons(sizeof(struct ip6_hdr));
	ip->ip6_ctlun.ip6_un1.ip6_un1_plen = htons(size);

	return 0;
}

int mkudphdr(void* buff, size_t udp_data_len, u_int8_t proto)
{
	struct ip* ip = (struct ip *) buff;
	ip--;
	int udp_len = udp_data_len + sizeof(struct udphdr);
	char pseudo[SIZE] = { 0 }; /* buffer for pseudo header */
	struct udphdr *udp = (struct udphdr *) buff;
	udp->uh_sport = htons(port); /* set source port*/
	udp->uh_dport = htons(port); /* set destination port */
	udp->uh_ulen = htons(udp_len); /* set udp length */
	udp->uh_sum = 0;/* zero out the udp checksum */

	/* pseudo header for udp checksum */
	struct pseudo_header *ps = (struct pseudo_header *) pseudo;
	ps->source = ip->ip_src.s_addr;
	ps->dest = ip->ip_dst.s_addr;
	ps->zero = 0;
	ps->proto = proto;
	ps->len = htons(udp_len);

	/*copy udp packet into pseudo header buffer to calculate checksum*/
	memcpy(ps + 1, udp, udp_len);

	/* set udp checksum */

	udp->uh_sum = ip_checksum(ps, udp_len + sizeof(struct pseudo_header));

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
	icmphdr->icmp6_id= htons (getpid());
	icmphdr->icmp6_seq= htons (rand());
	memset(&icmphdr->icmp6_dataun, 0xa5, datalen);
	gettimeofday((struct timeval *) &icmphdr->icmp6_dataun, NULL);
	icmphdr->icmp6_cksum = 0;
	icmphdr->icmp6_cksum = ip_checksum(icmphdr,
			datalen + sizeof(struct icmp6_hdr));
	return 0;
}

void *send_train(void* num)
{
	/*send Head ICMP Packet*/
	int n = sendto(icmp_fd, icmp_send, icmp_ip_len, 0, res->ai_addr,
			res->ai_addrlen);
	if(n == -1){
		perror("Send error ICMP head");
		exit(EXIT_FAILURE);
	}

	*packet_id = 0;
	struct udphdr *udp = (struct udphdr *) (send_fd + sizeof(struct ip));
	int x = port;
	/*send data train*/
	int i = 0;
	for(i = 0; i < num_packets; ++i){
		n = sendto(send_fd, packet_send, send_len, 0, res->ai_addr,
				res->ai_addrlen);
		if(n == -1){
			perror("Send error udp train");
			exit(EXIT_FAILURE);
		}
		(*packet_id)++;
		//udp->uh_dport = htons(++x);    // maybe we should increment port directly rather than reuse port numbers????
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
			exit(EXIT_FAILURE);
		}
		usleep(tail_wait * 1000);
	}
	return NULL;
}

void fill_data(void *buff, size_t size)
{
	/* fill with random data from /dev/urandom */
	/* get random data for high entropy datagrams */
	int fd = open(file, O_RDONLY);
	if(fd < 0){
		perror("Error opening file");
		exit(-1);
	}
	int err = read(fd, buff, size);
	if(err < 0){
		perror("Error reading file");
		exit(-1);
	}
	close(fd);
}

void *recv4(void *t)
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
	struct udphdr* udp = (struct udphdr*) (&(icmp->icmp_data)
			+ sizeof(struct ip));

	uint32_t* bitset = make_bs_32(num_packets);
	uint16_t *id = (uint16_t *) (udp + 1);
	;
	for(;;){

		if((n = recvfrom(recv_fd, packet_rcv, icmp_len, 0,
				(struct sockaddr *) &addr, &adrlen)) < 0){
			if(errno == EINTR)
				continue;
			perror("recvfrom failed");
			continue;
		}else if(icmp->icmp_type == 3 && icmp->icmp_code == 3){
			ack++;
			//id = *(uint16_t *) (udp + 1);
			//printf("Packet #%d\n", id);
			set_bs_32(bitset, *id, num_packets);
			continue;
		}else if(icmp->icmp_type == 0){
			if(count == 0){
				*time = get_time();
				count = 1;
			}else{
				*time = get_time() - *time;
				done = 1;
				break;
			}        //end if
		}else if(icmp->icmp_type == 11){
			errno = ENETUNREACH;
			perror("TTL Exceeded");
			exit(EXIT_FAILURE);
		}        // end if

	}        // end for
	printf("UDP Packets received: %d/%d\n", ack, num_packets);
	printf("Missing Packets:  ");
	register int i = 0;
	for(i = 0; i < num_packets; ++i){
		if(get_bs_32(bitset, i, num_packets) == 0){
			int start = i;
			while(i < num_packets
					&& get_bs_32(bitset, i, num_packets)
							== 0)
				i++;
			int end = i;
			if(start - end == 0)
				printf("%d, ", start + 1);
			else
				printf("%d-%d, ", start + 1, end);
		}
	}
	printf("\b\b \n");
	printf("Echo reply from IP: %s\n", inet_ntoa(ip->ip_src));

	free(bitset);
	return NULL;
}

void *recv6(void *t)
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
				(struct sockaddr *) &addr, &adrlen)) < 0){
			if(errno == EINTR)
				continue;
			perror("recvfrom failed");
			continue;
		}else if(icmp->icmp6_type == 3 && icmp->icmp6_code == 3){
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
			}        //end if
		}else if(icmp->icmp6_type == 11){
			errno = ENETUNREACH;
			perror("TTL Exceeded");
			exit(EXIT_FAILURE);
		}        // end if
	}        // end for
	printf("\nUDP Packets received: %d\n", ack);
	return NULL;
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

void print_use(char* program_name)
{
	printf("%s IPAddress [-p PORT] [-H | -L] [-s DATA_SIZE] "
			"[-n NUM_PACKETS] [-t TTL] [-w TAIL_INTERVAL] "
			"[-t NUM_TAIL] [-f FILENAME_PAYLOAD]\n", program_name);
}

int check_args(int argc, char* argv[])
{
	if(argc < 2){
		errno = EINVAL;
		perror("Too few arguments, see use");
		print_use(argv[0]);
		return EXIT_FAILURE;
	}

	int check;
	int c = 0;
	int err = 0;        // error flag for options
	while((c = getopt(argc, argv, "HLp:f:s:n:t:w:r:")) != -1){
		switch(c){
		case 'H':
			lflag = 0;
			break;
		case 'L':
			hflag = 0;
			break;
		case 'p':
			check = atoi(optarg);
			if(check < (1 << 16) & check > 0){
				port = check;
			}else{
				errno = ERANGE;
				perror("Port range: 1 - 65535");
				return EXIT_FAILURE;
			}
			break;
		case 's':
			data_size = atoi(optarg);
			if(data_size < 1 || data_size > SIZE){
				errno = ERANGE;
				perror("Valid UDP data size: 1-1460");
				return EXIT_FAILURE;
			}
			break;
		case 'n':
			num_packets = atoi(optarg);
			if(num_packets < 1 || num_packets > 10000){
				errno = ERANGE;
				perror("# of packets: 0 - 10000");
				return EXIT_FAILURE;
			}
			break;
		case 't':
			check = atoi(optarg);
			if(check < 0 || check > 255){
				errno = ERANGE;
				perror("TTL range: 0 - 255");
				return EXIT_FAILURE;
			}else
				ttl = check;
			break;
		case 'w':
			tail_wait = atoi(optarg);
			if(tail_wait < 0){
				errno = ERANGE;
				perror("Time wait must be positive");
				return EXIT_FAILURE;
			}
			break;
		case 'r':
			num_tail = atoi(optarg);
			if(num_tail < 1 || num_tail > 1000){
				errno = ERANGE;
				perror("# Tail Packets: 1 - 1,000");
				return EXIT_FAILURE;
			}
			break;
		case 'f':

			fflag = 1;
			file = optarg;
			int fd = open(file, O_RDONLY);
			if(fd < 0){
				fprintf(stderr,"Error opening file: "
						"\"%s\" : %s\n",
						file, strerror(errno));
				return EXIT_FAILURE;
			}
			close(fd);
			break;
		case '?':
			err = 1;        // hmm we don't even use this ...
			printf("Arguments errors ...\n");
			return EXIT_FAILURE;
			break;
		case 'h':
			print_use(argv[0]);
			return EXIT_FAILURE;
			break;
		default:
			errno = ERANGE;
			perror("Invalid options, check use");
			return EXIT_FAILURE;
		}        // end switch
	}        //end while
	/* these are the arguments after the command-line options */
	for(; optind < argc; optind++){
		dst_ip = argv[optind];
		//printf("argument: \"%s\"\n", dst_ip);
	}

	return EXIT_SUCCESS;
}

