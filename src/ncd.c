/**
 * @author: Paul Kirth
 * @file: ncd.c
 */

#include "ncd.h"
#include "bitset.h"

/*  Global Variables  */
int data_size; 			// size of udp data payload
int num_packets;		// number of packets in udp data train
int num_tail;		// number of tail icmp messages sent tail_wait apart
int tail_wait;			// time between ICMP tail messages
//16 bytes

u_int16_t dport; 		// destination port number
u_int16_t sport; 		// source port number
u_int16_t syn_port; 		// source port number
u_int8_t ttl;			// time to live
//16+7=23bytes

char entropy; 			// the entropy of the data High, Low, Both
char* dst_ip = NULL; 		// destination ip address
char* file = NULL;        //name of file to read from /dev/urandom by default
//=23+1+16=40bytes

/* flags */
uint8_t lflag = 1;    		// default option for low entropy -- set to on
uint8_t hflag = 1;   		// default option for high entropy -- set to on
uint8_t fflag = 0;        // file flag <------- do we need this or is this redundant?
// 40+(3*1) = 40+3 = 43

/* file descriptors */
int icmp_fd; 			//icmp socket file descriptor
int send_fd; 			//udp socket file descriptor
int recv_fd; 			//reply receiving socket file descriptor
//= 43+3*4 = 43+12 = 55 Bytes

/* lengths of packets and data, etc. */
size_t send_len;		// length of data to be sent
uint8_t tcp_bool = 0;        //bool for whether to use tcp or udp(1 == true, 0 == false)
//55+8+1=64

//cacheline

size_t seq = 0;
size_t icmp_ip_len;		// length of IP icmp packet including payload
size_t icmp_len;		// length of ICMP packet
size_t icmp_data_len;		// length of ICMP data
size_t rcv_len;			// length of data to be received
struct addrinfo *res = NULL;        // addrinfo struct for getaddrinfo()
void *(*recv_data)(void*) = NULL;        // function pointer so we can select properly for IPV4 or IPV6
void* (*send_train)(void*) = NULL;        // function pointer to send data: UDP or TCP
//(8*8) = 64 bytes

//cacheline
double td;

volatile int done = 0;			// boolean for if the send can stop
volatile int rcv_bool = 0;        // bool for recving SYN packets really a Condition variable, consider replacing
/*pthread_mutex_t stop_mutex;
 pthread_mutex_t recv_ready_mutex;
 pthread_cond_t stop;
 pthread_cond_t recv_ready;*/

//8+4+4+4=20
volatile int second_train = 0;

char pseudo[1500] = { 0 }; /* buffer for pseudo header */
char packet_rcv[1500] = { 0 };			// buffer for receiving replies
char packet_send[SIZE] = { 0 };        		// buffer for sending data
char syn_packet_1[20] = { 0 };			// packet for head SYN
char syn_packet_2[20] = { 0 };			// packet for tail SYN
char icmp_send[128] = { 0 };			// buffer for ICMP messages

struct pseudo_header *ps = (struct pseudo_header *) pseudo;        //pseudo header
uint16_t* packet_id = (uint16_t*) packet_send;        //sequence/ID number of udp msg
struct sockaddr_in srcaddrs = { 0 };
socklen_t sa_len = sizeof(srcaddrs);

//union packet *packet_ary;
char * packets_e = NULL;        //empty packets
char *packets_f = NULL;        //filled packets

double time_val;

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
	send_len = data_size + sizeof(uint16_t);        // data size + size of packet id

	/* size of ICMP Echo message */
	icmp_data_len = 56;

	/*size of icmp packet*/
	icmp_len = sizeof(struct icmp) + icmp_data_len;

	/* size of ICMP reply + ip header */
	icmp_ip_len = sizeof(struct ip) + icmp_len;

	/* set up hints for getaddrinfo() */
	struct addrinfo hints = { 0 }; /* for get addrinfo */
	hints.ai_flags = AI_CANONNAME;
	if(tcp_bool == 1)
		hints.ai_protocol = IPPROTO_TCP;
	else
		hints.ai_protocol = IPPROTO_UDP;

	char str[8] = { 0 };
	snprintf(str, sizeof(str), "%d", dport);

	int err = getaddrinfo(dst_ip, str, &hints, &res);

	// get temp socket to obtain source IP -- its a hack
	{
		int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if(connect(s, res->ai_addr, res->ai_addrlen) == -1){
			perror("Connect failed");
			return -1;
		}
		if(getsockname(s, (struct sockaddr *) &srcaddrs, &sa_len)
				== -1){
			perror("getsockname() failed");
			return -1;
		}
		close(s);
	}

	printf("Local IP address is: %s\n", inet_ntoa(srcaddrs.sin_addr));

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

	err = setuid(0);/*get root privileges */
	if(err < 0){
		perror("Elevated privileges not acquired...");
		return EXIT_FAILURE;
	}

	/* Setup TCP Socket to bypass filters, else use UDP*/
	if(tcp_bool == 1)
		send_fd = socket(res->ai_family, SOCK_RAW, IPPROTO_TCP);
	else
		send_fd = socket(res->ai_family, SOCK_DGRAM, IPPROTO_UDP);

	if(send_fd == -1){
		perror("call to socket() failed for SEND");
		exit(EXIT_FAILURE);
	}

	int r = (res->ai_family == AF_INET) ? IPPROTO_IP : IPPROTO_IPV6;

	//set TTL
	setsockopt(send_fd, r, IP_TTL, &ttl, sizeof(ttl));

	socklen_t size = 1500 * num_packets;
	printf("Buffer size requested %u\n", size);

	setsockopt(send_fd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));

	/* acquire socket for icmp messages*/
	int l = res->ai_family == AF_INET ? IPPROTO_ICMP : IPPROTO_ICMPV6;
	icmp_fd = socket(res->ai_family, SOCK_RAW, l);

	if(icmp_fd == -1){
		perror("call to socket() failed for ICMP");
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
		perror("Elevated privileges not released");
		return EXIT_FAILURE;
	}

	//make ICMP packets
	if(res->ai_family == AF_INET){
		recv_data = recv4;

		if(tcp_bool == 1){
			/*
			 mktcphdr(packet_send, send_len, IPPROTO_TCP);
			 memcpy(syn_packet_1, packet_send,
			 send_len + sizeof(struct tcphdr));

			 mktcphdr(syn_packet_2, send_len, IPPROTO_TCP);
			 */

			setup_syn_packets();

			if((setup_tcp_packets()) == -1){
				errno = ENOMEM;
				perror("Packet setup failed...");
				return EXIT_FAILURE;
			}
		}else{
			mkipv4(icmp_send, icmp_len, res, IPPROTO_ICMP);
			mkicmpv4(icmp_send + sizeof(struct ip), icmp_data_len);
		}

	}else if(res->ai_family == AF_INET6){
		mkipv6(icmp_send, icmp_len, res, IPPROTO_ICMPV6);
		mkicmpv6(icmp_send + (sizeof(struct ip6_hdr)), icmp_data_len);
		recv_data = recv6;
	}else{
		errno = EPROTONOSUPPORT;
		perror("Protocol not supported");
		return EXIT_FAILURE;
	}

	int rc;
	register int i;
	pthread_t threads[2];
	void *status[2];
	if(lflag == 1){
		done = 0;        //boolean false
		detect('L');
	}

	sleep(5);        // sloppy replace with better metric

	{
		printf("\nClearing buffer...");
		//clear out rcvbuffer
		char buff[1500] = { 0 };
		if(tcp_bool == 1)
			while(recvfrom(send_fd, buff, 1500, MSG_DONTWAIT, NULL,
			NULL) != -1){
			}
		else
			while(recvfrom(recv_fd, buff, 1500, MSG_DONTWAIT, NULL,
			NULL) != -1){
			}
		printf("Done\n\n");
	}

	if(hflag == 1){

		done = 0;        //boolean false
		second_train = 1;

		if(!tcp_bool)
			fill_data(packet_send, data_size);
		detect('H');
	}
	if(res)
		freeaddrinfo(res);
	res = NULL;
	if(packets_e)
		free(packets_e);
	packets_e = NULL;

	if(packets_f)
		free(packets_f);
	packets_f = NULL;
	return EXIT_SUCCESS;
}

int detect(char c)
{
	int rc;
	register int i;
	pthread_t threads[2];
	void *status[2];
	int err = setuid(0);/*get root privileges */
	if(err < 0){
		perror("Elevated privileges not acquired...");
		return EXIT_FAILURE;
	}
	/* Acquire raw socket to listen for ICMP replies */
	if(tcp_bool == 1)
		recv_fd = socket(res->ai_family, SOCK_RAW, IPPROTO_TCP);
	else
		recv_fd = socket(res->ai_family, SOCK_RAW,
		IPPROTO_ICMP);
	if(recv_fd == -1){
		perror("call to socket() failed");
		return EXIT_FAILURE;
	}

	err = setuid(getuid());/*get root privileges */
	if(err < 0){
		perror("Elevated privileges not released...");
		return EXIT_FAILURE;
	}

	/*increase size of receive buffer*/
	int buffsize;
	int opts = 1500 * num_packets;
	socklen_t bufflen = sizeof(buffsize);
	setsockopt(recv_fd, SOL_SOCKET, SO_RCVBUF, &opts, sizeof(opts));
	getsockopt(send_fd, SOL_SOCKET, SO_SNDBUF, (void*) &buffsize, &bufflen);
	printf("Send Buffer size: %d\n", buffsize);
	getsockopt(recv_fd, SOL_SOCKET, SO_RCVBUF, (void*) &buffsize, &bufflen);
	printf("Receive Buffer size: %d\n", buffsize);
	rc = pthread_create(&threads[0], NULL, recv_data, &time_val);
	if(rc){
		printf("ERROR: return code from pthread_create()"
				" is %d\n", rc);
		exit(-1);
	}

	rc = pthread_create(&threads[1], NULL, send_train, status[1]);
	if(rc){
		printf("ERROR: return code from pthread_create() "
				"is %d\n", rc);
		exit(-1);
	}

	for(i = 0; i < 2; ++i){
		rc = pthread_join(threads[i], &status[i]);
		if(rc){
			printf("ERROR; return code from "
					"pthread_create() is %d\n", rc);
			exit(-1);
		}

		if(status[i] != NULL)
			return EXIT_FAILURE;

	}        //end for

	printf("%c %f sec\n", c, time_val);
	close(recv_fd);
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
	ip->ip_src.s_addr = srcaddrs.sin_addr.s_addr;
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
	//ip->ip6_src = ((struct sockaddr_in6*)srcaddrs)->sin6_addr;
	//inet_pton(AF_INET6, "192.168.1.101", &ip->ip6_src);
	ip->ip6_ctlun.ip6_un1.ip6_un1_flow = 0;
	ip->ip6_ctlun.ip6_un1.ip6_un1_hlim = ttl;
	ip->ip6_ctlun.ip6_un1.ip6_un1_nxt = htons(sizeof(struct ip6_hdr));
	ip->ip6_ctlun.ip6_un1.ip6_un1_plen = htons(size);

	return 0;
}

int mktcphdr(void* buff, size_t data_len, u_int8_t proto)
{
	int len = data_len + sizeof(struct tcphdr);
	struct tcphdr *tcp = (struct tcphdr *) buff;

	tcp->source = htons(sport);
	tcp->dest = htons(dport);
	tcp->seq = htonl(seq);
	tcp->ack = 0;
	tcp->doff = 5;
	tcp->window = (1 << 15) - 1;
	tcp->syn = 1;

	/* pseudo header for udp checksum */

	ps->source = srcaddrs.sin_addr.s_addr;
	ps->dest = ((struct sockaddr_in *) res->ai_addr)->sin_addr.s_addr;
	ps->zero = 0;
	ps->proto = proto;
	ps->len = htons(len);

	/*copy udp packet into pseudo header buffer to calculate checksum*/
	memcpy(ps + 1, tcp, len);

	/* set tcp checksum */
	tcp->check = ip_checksum(ps, len + sizeof(struct pseudo_header));
	return 0;
}

void setup_syn_packet(void* buff, uint16_t port)
{

	//setup tcpheader for syn packets...
	int len = sizeof(struct tcphdr);
	struct tcphdr *tcp = (struct tcphdr *) buff;

	tcp->source = htons(port);
	tcp->dest = htons(dport);
	tcp->seq = htonl(seq);
	tcp->ack = 0;
	tcp->doff = 5;
	tcp->window = (1 << 15) - 1;
	tcp->syn = 1;

	/* pseudo header for udp checksum */

	ps->source = srcaddrs.sin_addr.s_addr;
	ps->dest = ((struct sockaddr_in *) res->ai_addr)->sin_addr.s_addr;
	ps->zero = 0;
	ps->proto = IPPROTO_TCP;
	ps->len = htons(len);

	/*copy udp packet into pseudo header buffer to calculate checksum*/
	memcpy(ps + 1, tcp, len);

	/* set tcp checksum */
	tcp->check = ip_checksum(ps, len + sizeof(struct pseudo_header));
}

void setup_syn_packets()
{
	setup_syn_packet(syn_packet_1, sport);
	setup_syn_packet(syn_packet_2, syn_port);
}

int setup_tcp_packets()
{
	//packet_send is already set up;
	printf("sport: %d\ndport: %d\n", sport, dport);
	size_t len = send_len + sizeof(struct tcphdr);
	char buffer[1500]= {0};
	struct tcphdr *tcp = NULL;

	u_int32_t ack = 0;

	packets_e = (char *) calloc(num_packets, len);
	if(!packets_e)
		return -1;
	packets_f = (char *) calloc(num_packets, len);
	if(!packets_f)
		return -1;
	size_t pslen = len + sizeof(struct pseudo_header);

	/* pseudo header for udp checksum */

	ps->source = srcaddrs.sin_addr.s_addr;
	ps->dest = ((struct sockaddr_in *) res->ai_addr)->sin_addr.s_addr;
	ps->zero = 0;
	ps->proto = IPPROTO_TCP;
	ps->len = htons(len);

	char *ptr = packets_e;
	register int i = 0;

	//!!! we're moving this ptr wrong somehow we get 0 for ports and other errors
	for(i = 0; i < num_packets; ++i, ptr += len){

		tcp = (struct tcphdr *) ptr;

		tcp->source = htons(sport);
		tcp->dest = htons(dport);
		tcp->seq = htonl(seq += send_len);
		tcp->ack = 1;
		tcp->ack_seq = htonl(ack++);
		tcp->doff = 5;
		tcp->window = (1 << 15) - 1;
		tcp->syn = 0;

		/*copy udp packet into pseudo header buffer to calculate checksum*/
		memcpy((ps + 1), tcp, len);

		/* set tcp checksum */
		tcp->check = ip_checksum(ps, pslen);
	}

	seq = htonl(seq + send_len);
	ack = 0;
	ptr = packets_f;
	fill_data(buffer + sizeof(uint16_t), data_size);

	size_t offset = sizeof(struct tcphdr) + sizeof(uint16_t);

	for(i = 0; i < num_packets; ++i, ptr += len){

		tcp = (struct tcphdr *) ptr;

		tcp->source = htons(sport);
		tcp->dest = htons(dport);
		tcp->seq = htonl(seq += send_len);
		tcp->ack = 1;
		tcp->ack_seq = htonl(ack++);
		tcp->doff = 5;
		tcp->window = (1 << 15) - 1;
		tcp->syn = 0;

		memcpy(ptr + offset, buffer, data_size);

		/*copy udp packet into pseudo header buffer to calculate checksum*/
		memcpy(ps + 1, tcp, len);

		/* set tcp checksum */
		tcp->check = ip_checksum(ps, pslen);
	}

	return 0;
}		// end setup_tcp_packets()

int mkudphdr(void* buff, size_t udp_data_len, u_int8_t proto)
{
	struct ip* ip = (struct ip *) buff;
	ip--;
	int udp_len = udp_data_len + sizeof(struct udphdr);
	char pseudo[SIZE] = { 0 }; /* buffer for pseudo header */
	struct udphdr *udp = (struct udphdr *) buff;
	udp->source = htons(sport); /* set source port*/
	udp->dest = htons(dport); /* set destination port */
	udp->len = htons(udp_len); /* set udp length */
	udp->check = 0;/* zero out the udp checksum */

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
	udp->check = ip_checksum(ps, udp_len + sizeof(struct pseudo_header));

	return 0;
}

int mkicmpv4(void *buff, size_t datalen)
{
	/* set up icmp message header*/
	//struct ip *ip = (struct ip *) buff;
	//ip--;
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

void *send_udp(void* arg)
{
	int n;

	/*send Head ICMP Packet*/
	n = sendto(icmp_fd, icmp_send, icmp_ip_len, 0, res->ai_addr,
			res->ai_addrlen);
	if(n == -1){
		perror("Send error ICMP head");
		exit(EXIT_FAILURE);
	}

	*packet_id = 0;

	/*send data train*/
	int i = 0;

	for(i = 0; i < num_packets; ++i, (*packet_id)++){

		n = sendto(send_fd, packet_send, send_len, 0, res->ai_addr,
				res->ai_addrlen);
		if(n == -1){
			perror("Send error udp train");
			exit(EXIT_FAILURE);
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
			exit(EXIT_FAILURE);
		}
		usleep(tail_wait * 1000);
	}		//end for

	return NULL;
}

void *send_tcp(void* arg)
{
	int n;
	int len = send_len + sizeof(struct tcphdr);
	char buff[1500] = { 0 };
	//struct ip* ip = (struct ip*) packet_send;
	struct tcphdr* tcp = (struct tcphdr*) packet_send;
	struct tcphdr* ps_tcp = (struct tcphdr *) (ps + 1);

	//int length = send_len + sizeof(struct tcphdr) + sizeof(struct ip);
	printf("send_len : %d\n", (int) send_len);
	printf("Send syn packet\n");
	n = sendto(send_fd, syn_packet_1, sizeof(syn_packet_1), 0, res->ai_addr,
			res->ai_addrlen);
	if(n == -1){
		perror("Send error tcp syn");
		exit(EXIT_FAILURE);
	}
	// set up the buffer to receive the reply into
	struct ip *ip = (struct ip*) buff;
	struct tcphdr *tcp_reply = (struct tcphdr *) (ip + 1);
	struct sockaddr_in temp_res;
	socklen_t temp_size = sizeof(temp_res);
	//struct tcphdr *BAD = (struct tcphdr *) (ip);
	//printf("size of buff = %d\n", (int) sizeof(buff));
#if 1
	do{
		if((recvfrom(send_fd, buff, sizeof(buff), 0, 0,
				0 /*&temp_res, &temp_size*/)) == -1){
			perror("rcv error tcp SYN-ACK");
			exit(EXIT_FAILURE);
		}
		//printf("Port: %d\n", ntohs( tcp_reply->dest));
		//printf("BAD Port: %d\n", ntohs( BAD->dest));

	}while(((tcp_reply->dest != htons(sport)
			|| (((struct sockaddr_in*) res->ai_addr)->sin_addr.s_addr
					!= ip->ip_src.s_addr))));
#endif
	//sleep(1);

	printf("TCP SYN reply from IP: %s\n", inet_ntoa(ip->ip_src));
	printf("TCP SYN reply from port: %d to port: %d\n",
			ntohs(tcp_reply->source), ntohs(tcp_reply->dest));

	td = get_time();	//time stamp just before we begin sending

	/*send data train*/
	int i = 0;
	char *ptr = second_train ? packets_f : packets_e;

	for(i = 0; i < num_packets; ++i, ptr += len){
		n = sendto(send_fd, ptr, len, 0, res->ai_addr, res->ai_addrlen);
		if(n == -1){
			perror("Send error tcp train");
			exit(EXIT_FAILURE);
		}		// end if
	}		// end of

	rcv_bool = 1;
	for(i = 0; i < num_tail && done == 0; ++i){
		n = sendto(send_fd, syn_packet_2, sizeof(syn_packet_2), 0,
				res->ai_addr, res->ai_addrlen);
		if(n == -1){
			perror("Send error TCP Tail Syn");
			exit(EXIT_FAILURE);
		}
		usleep(tail_wait * 1000);
	}		// end for
	tcp->source = ps_tcp->source = htons(sport);
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
	//addr.sin_port = htons(syn_port);
	//inet_pton(AF_INET, dst_ip, &addr.sin_addr);

	/* length of address */
	socklen_t adrlen = sizeof(addr);

	if(tcp_bool == 1){
		while(rcv_bool == 0){
		}
		struct ip *ip = (struct ip*) packet_rcv;
		struct tcphdr *tcp = (struct tcphdr *) (ip + 1);
		do{
			if((n = recvfrom(send_fd, packet_rcv,
					sizeof(packet_rcv), 0,
					(struct sockaddr *) &addr, &adrlen))
					< 0){
				perror("recvfrom failed");
			}

		}while(tcp->dest != htons(syn_port)
				|| (ip->ip_src.s_addr
						!= ((struct sockaddr_in*) res->ai_addr)->sin_addr.s_addr));
		*time = get_time() - td;
		rcv_bool = 0;
		done = 1;

		printf("TCP reply from IP: %s\n", inet_ntoa(ip->ip_src));

		printf("TCP reply from port: %d to port: %d\n",
				ntohs(tcp->source), ntohs(tcp->dest));

	}else{

		/*Receive initial ICMP echo response && Time-stamp*/
		struct ip *ip = (struct ip *) packet_rcv;
		icmp = (struct icmp *) (ip + 1);
		struct udphdr* udp = (struct udphdr*) (&(icmp->icmp_data)
				+ sizeof(struct ip));

		uint32_t* bitset = make_bs_32(num_packets);
		uint16_t *id = (uint16_t *) (udp + 1);
		struct in_addr dest;
		inet_aton(dst_ip, &dest);

		for(;;){

			if((n = recvfrom(recv_fd, packet_rcv,
					sizeof(packet_rcv), 0,
					(struct sockaddr *) &addr, &adrlen))
					< 0){
				if(errno == EINTR)
					continue;
				perror("recvfrom failed");
				continue;
			}else if(ip->ip_src.s_addr != dest.s_addr){
				//printf("Echo sent to IP: %s\n", dst_ip);
				//printf("Echo reply from IP: %s\n", inet_ntoa(ip_rcv->ip_src));
				continue;
			}else if(icmp->icmp_type == 3 && icmp->icmp_code == 3){
				ack++;
				//printf("Received packet#: %d\n", *id);
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
						&& (get_bs_32(bitset, i + 1,
								num_packets)
								== 0))
					i++;
				int end = i;
				if(end - start == 0)
					printf("%d, ", start + 1);
				else
					printf("%d-%d, ", start + 1, end + 1);
			}
		}
		printf("\b\b \n");
		printf("Echo reply from IP: %s\n", inet_ntoa(ip->ip_src));
		if(bitset)
			free(bitset);
	}        // end if

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
		uint32_t word = 0;
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
	printf("%s IPAddress [-p PORT] [-T] [-H | -L] [-s DATA_SIZE] "
			"[-n NUM_PACKETS] [-t TTL] [-w TAIL_INTERVAL] "
			"[- NUM_TAIL][-f FILENAME_PAYLOAD]\n", program_name);
}

int check_args(int argc, char* argv[])
{
	if(argc < 2){
		errno = EINVAL;
		perror("Too few arguments, see use");
		print_use(argv[0]);
		return EXIT_FAILURE;
	}
	dst_ip = NULL;

	/* probably change default port from traceroute port */
	dport = 33434;        //80;
	sport = 13333;
	syn_port = 14444;
	entropy = 'B';        // default to 2 data trains
	data_size = 1024 - sizeof(uint16_t) - sizeof(struct tcphdr)
			- sizeof(struct ip);        //so we send 1 KB packets
	num_packets = 1000;        // send 1000 packets in udp data train
	ttl = 255;		// max ttl
	tail_wait = 10;        // wait 10 ms between ICMP tail messages
	num_tail = 20;		// send 20 ICMP tail messages
	file = "/dev/urandom";        // default to random data for compression detection

	send_train = send_udp;

	register int i;
	int check;
	int c = 0;
	int err = 0;        // error flag for options
	while((c = getopt(argc, argv, "HLTp:f:s:n:t:w:r:h")) != -1){
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
				dport = check;
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
		case 'f': {
			fflag = 1;
			file = optarg;
			int fd = open(file, O_RDONLY);
			if(fd < 0){
				fprintf(stderr, "Error opening file: "
						"\"%s\" : %s\n", file,
						strerror(errno));
				return EXIT_FAILURE;
			}
			close(fd);
			break;
		}
		case '?':
			err = 1;        // hmm we don't even use this ...
			printf("Arguments errors ...\n");
			return EXIT_FAILURE;
			break;
		case 'h':
			print_use(argv[0]);
			return EXIT_FAILURE;
			break;
		case 'T':
			tcp_bool = 1;
			send_train = send_tcp;
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
