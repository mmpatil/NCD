/**
 * @author: Paul Kirth
 * @file: ncd.c
 */

#include "ncd.h"
#include "bitset.h"

char* packets_e = NULL;                 //empty packets
char* packets_f = NULL;                 //filled packets
char* dst_ip = NULL;        // destination ip address
char* file = NULL;          //name of file to read from /dev/urandom by default

/* flags */
u_int8_t lflag = 1;         // default option for low entropy -- set to on
u_int8_t hflag = 1;         // default option for high entropy -- set to on
int verbose = 0;                        // flag for verbose output
const int num_threads = 2;
int cooldown = 5;                       // time in seconds to wait between data trains
u_int8_t tcp_bool = 0;        //bool for whether to use tcp or udp(1 == true, 0 == false)

int second_train = 0;

char pseudo[1500] = {0};                // buffer for pseudo header
char packet_rcv[1500] = {0};            // buffer for receiving replies
char packet_send[SIZE] = {0};           // buffer for sending data
char syn_packet_1[20] = {0};            // packet for head SYN
char syn_packet_2[20] = {0};            // packet for tail SYN
char icmp_send[128] = {0};              // buffer for ICMP messages

struct pseudo_header* ps = (struct pseudo_header*)pseudo;       //pseudo header
u_int16_t* packet_id = (u_int16_t*)packet_send;                 //sequence/ID number of udp msg
struct sockaddr_in srcaddrs = {0};      // source IP address
struct in_addr destip = {0};     // destination IP
socklen_t sa_len = sizeof(srcaddrs);    // size of src address

struct addrinfo* res = NULL;            // addrinfo struct for getaddrinfo()
void* (* recv_data)(void*) = NULL;      // function pointer so we can select properly for IPV4 or IPV6(no IPV6 yet
void* (* send_train)(void*) = NULL;     // function pointer to send data: UDP or TCP

/*  Just returns current time as double, with most possible precision...  */
double get_time(void)
{
	struct timeval tv;
	double d;
	gettimeofday(&tv, NULL);
	d = ((double)tv.tv_usec) / 1000000. + (unsigned long)tv.tv_sec;
	return d;
}

int init_detection()
{
	int icmp_packet_size = 64; // 64 byte icmp packet size up to a mx of 76 bytes for replies, we just use the min

	/* Init global size variables with runtime data  */
	send_len = data_size + (u_int16_t)sizeof(u_int16_t);        // data size + size of packet id

	/* size of ICMP Echo message */
	icmp_data_len = (uint16_t)(icmp_packet_size - sizeof(struct icmp));

	/*size of icmp packet*/
	icmp_len = (u_int16_t)(icmp_packet_size);

	/* size of ICMP reply + ip header */
	icmp_ip_len = (u_int16_t)(sizeof(struct ip) + icmp_len);

	seq=0;

	/* set up hints for getaddrinfo() */
	struct addrinfo hints = {0}; /* for get addrinfo */
	hints.ai_flags = AI_CANONNAME;
	if(tcp_bool == 1)
		hints.ai_protocol = IPPROTO_TCP;
	else
		hints.ai_protocol = IPPROTO_UDP;

	/* pass a string of the destination point to getaddrinfo */
	char str[8] = {0};
	snprintf(str, sizeof(str), "%d", dport);

	int err = getaddrinfo(dst_ip, str, &hints, &res);

	/*taken from http://stackoverflow.com/questions/17914550/getaddrinfo-error-success*/
	if(err != 0){
		if(err == EAI_SYSTEM)
			fprintf(stderr, "looking up %s: %s\n", dst_ip, strerror(errno));
		else
			fprintf(stderr, "looking up %s: %s\n", dst_ip, gai_strerror(err));
		exit(EXIT_FAILURE);
	}

	// get temp socket to obtain source IP -- its a hack
	{
		int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if(connect(s, res->ai_addr, res->ai_addrlen) == -1){
			perror("Connect failed");
			return -1;
		}
		if(getsockname(s, (struct sockaddr*)&srcaddrs, &sa_len)
		   == -1){
			perror("getsockname() failed");
			return -1;
		}
		close(s);
	}// end temp socket

	/* Store the destination IP */
	destip = ((struct sockaddr_in*)res->ai_addr)->sin_addr;

	/* print basic header for NCD */
	char *str_temp = tcp_bool ? "TCP": "UDP";
	printf( "%s NCD\n", str_temp);


	/* Verbose output: metadata */
	if(verbose){
		printf("Local IP: %s\n", inet_ntoa(srcaddrs.sin_addr));
		printf("Target IP: %s\n", inet_ntoa(destip));
		printf("Source port: %d\n", sport);
		printf("Destination port: %d\n", dport);
		if(tcp_bool)
			printf("SYN Port: %d\n", syn_port);
		printf("Data Train length: %d\n", num_packets);
		printf("Tail length: %d\n", num_tail);
		printf("Packet Size: %d\n", data_size);
		printf("Tail wait: %d\n", tail_wait);
		printf("TTL: %d\n", ttl);
		printf("\n");
	}// end verbose

	/* get root privileges */
	err = setuid(0);
	if(err < 0){
		perror("Elevated privileges not acquired...");
		return EXIT_FAILURE;
	}// end error check

	/* Setup TCP Socket to bypass filters, else use UDP*/
	if(tcp_bool == 1)
		send_fd = socket(res->ai_family, SOCK_RAW, IPPROTO_TCP);
	else
		send_fd = socket(res->ai_family, SOCK_DGRAM, IPPROTO_UDP);

	if(send_fd == -1){
		perror("call to socket() failed for SEND");
		exit(EXIT_FAILURE);
	} // end error check

	if(res->ai_family != AF_INET){
		errno = EAFNOSUPPORT;
		perror("ncd only supports IPV4 at this time");
		exit(EXIT_FAILURE);
	}// end error check

	//set TTL
	setsockopt(send_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

	socklen_t size = 1500U * num_packets;

#if DEBUG
	if(verbose)
			printf("Buffer size requested %u\n", size);
#endif

	setsockopt(send_fd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));

	/* acquire socket for icmp messages*/
	icmp_fd = socket(res->ai_family, SOCK_RAW, IPPROTO_ICMP);

	if(icmp_fd == -1){
		perror("call to socket() failed for ICMP");
		exit(EXIT_FAILURE);
	}

	/* set up our own IP header*/
	int icmp_hdrincl = 1;
	if(setsockopt(icmp_fd, IPPROTO_IP, IP_HDRINCL, &icmp_hdrincl, sizeof(icmp_hdrincl)) == -1){
		perror("setsockopt() failed icmp");
		exit(EXIT_FAILURE);
	}

	/*give up privileges */
	err = setuid(getuid());
	if(err < 0){
		perror("Elevated privileges not released");
		return EXIT_FAILURE;
	}

	recv_data = recv4;

	if(tcp_bool == 1){
		setup_syn_packets();
		if((setup_tcp_packets()) == -1){
			errno = ENOMEM;
			perror("Packet setup failed...");
			return EXIT_FAILURE;
		}
	} else{
		/* make ICMP packets */
		mkipv4(icmp_send, icmp_len, IPPROTO_ICMP);
		mkicmpv4(icmp_send + sizeof(struct ip), icmp_data_len);
	}//end if
	return EXIT_SUCCESS;
}

int detect()
{

	int err;
	err = init_detection();
	if (err != 0)
		return err;

	if(lflag == 1){
		measure();
	}

	if(lflag && hflag){
		if(verbose)
			printf("Waiting between data trains ...");

		sleep(cooldown);        // sloppy replace with better metric

		if(verbose)
			printf("Done.\n");

		if(verbose)
			printf("\nClearing buffer...");

		//clear out rcvbuffer
		char buff[1500] = {0};
		if(tcp_bool == 1){
			while(recvfrom(send_fd, buff, 1500, MSG_DONTWAIT, NULL, NULL) != -1){
			}
		}

		while(recvfrom(recv_fd, buff, 1500, MSG_DONTWAIT, NULL, NULL) != -1){
		}

		if(verbose)
			printf("Done\n\n");
	}

	if(hflag == 1){
		second_train = 1;

		if(!tcp_bool)
			fill_data(packet_send, data_size);
		measure();
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

int measure()
{
	// initialize synchronization variables
	stop = 0;        //boolean false
	recv_ready = 0;        //boolean false

	pthread_mutex_init(&stop_mutex, NULL);
	pthread_cond_init(&stop_cv, NULL);

	pthread_mutex_init(&recv_ready_mutex, NULL);
	pthread_cond_init(&recv_ready_cv, NULL);

	pthread_t threads[num_threads];
	pthread_attr_t attr;

	int rc;
	register int i;

	void* status[2];

	/*get root privileges */
	int err = setuid(0);
	if(err < 0){
		perror("Elevated privileges not acquired...");
		return EXIT_FAILURE;
	}
	/* Acquire raw socket to listen for ICMP replies */
	if(tcp_bool == 1)
		recv_fd = socket(res->ai_family, SOCK_RAW, IPPROTO_TCP);
	else
		recv_fd = socket(res->ai_family, SOCK_RAW, IPPROTO_ICMP);
	if(recv_fd == -1){
		perror("call to socket() failed");
		return EXIT_FAILURE;
	}

	/* give up root privileges */
	err = setuid(getuid());
	if(err < 0){
		perror("Elevated privileges not released...");
		return EXIT_FAILURE;
	}

	/*increase size of receive buffer*/

	int opts = 1500 * num_packets;

	setsockopt(recv_fd, SOL_SOCKET, SO_RCVBUF, &opts, sizeof(opts));

#if DEBUG
	if(verbose){
			int buffsize;
			socklen_t bufflen = sizeof(buffsize);

			getsockopt(send_fd, SOL_SOCKET, SO_SNDBUF, (void*) &buffsize,
							&bufflen);

			printf("Send Buffer size: %d\n", buffsize);
			getsockopt(recv_fd, SOL_SOCKET, SO_RCVBUF, (void*) &buffsize,
							&bufflen);
			printf("Receive Buffer size: %d\n", buffsize);
	}
#endif

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	rc = pthread_create(&threads[0], &attr, recv_data, &time_val);
	if(rc){
		printf("ERROR: return code from pthread_create()"
				       " is %d\n", rc);
		exit(-1);
	}

	rc = pthread_create(&threads[1], &attr, send_train, status[1]);
	if(rc){
		printf("ERROR: return code from pthread_create() "
				       "is %d\n", rc);
		exit(-1);
	}

	for(i = 0; i < num_threads; ++i){
		rc = pthread_join(threads[i], &status[i]);
		if(rc){
			printf("ERROR; return code from "
					       "pthread_create() is %d\n", rc);
			exit(-1);
		}

		if(status[i] != NULL)
			return EXIT_FAILURE;

	}        //end for
	char c = second_train ? 'H' : 'L';
	printf("%c %f sec\n", c, time_val);
	close(recv_fd);

	//cleanup synchronization variables
	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&stop_mutex);
	pthread_cond_destroy(&stop_cv);
	pthread_mutex_destroy(&recv_ready_mutex);
	pthread_cond_destroy(&recv_ready_cv);

	return EXIT_SUCCESS;
}

void mkipv4(void* buff, u_int16_t size, u_int8_t proto)
{
	if(!buff || size < sizeof(struct ip) || size > SIZE || !proto)
	{
		errno = EINVAL;
		perror("Invalid argument used in ICMP packet allocation");
		exit(EXIT_FAILURE);
	}

	/* create IP header*/
	struct ip* ip = (struct ip*)buff;
	ip->ip_v = 4;
	ip->ip_hl = 5;
	ip->ip_len = htons(size);
	ip->ip_id = htons((u_int16_t)getpid());
	ip->ip_src.s_addr = srcaddrs.sin_addr.s_addr;
	ip->ip_dst = destip;
	ip->ip_off |= ntohs(IP_DF);
	ip->ip_ttl = ttl;
	ip->ip_p = proto;

}

void setup_syn_packet(void* buff, u_int16_t port)
{
	//setup tcpheader for syn packets...
	u_int16_t len = (u_int16_t)sizeof(struct tcphdr);
	struct tcphdr* tcp = (struct tcphdr*)buff;

	tcp->source = htons(port);
	tcp->dest = htons(dport);
	tcp->seq = htonl(seq);
	tcp->ack = 0;
	tcp->doff = 5;
	tcp->window = (1 << 15) - 1;
	tcp->syn = 1;

	/* pseudo header for udp checksum */

	ps->source = srcaddrs.sin_addr.s_addr;
	ps->dest = destip.s_addr;
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

int setup_tcp_train( char** buff, int fill)
{
	//packet_send is already set up;
	u_int16_t len = send_len + (u_int16_t)sizeof(struct tcphdr);
	char buffer[1500] = {0};
	struct tcphdr* tcp = NULL;

	u_int32_t ack = 0;

	(*buff) = (char*)calloc(num_packets, len);
	if(!(*buff)){
		perror("Failure allocating memory durring TCP packet creation,"
				       " consider sending less data...\n");
		exit(-1);
	}

	size_t pslen = len + sizeof(struct pseudo_header);

	/* pseudo header for checksum */
	ps->source = srcaddrs.sin_addr.s_addr;
	ps->dest = destip.s_addr;
	ps->zero = 0;
	ps->proto = IPPROTO_TCP;
	ps->len = htons(len);

	if(fill)
		fill_data(buffer + sizeof(u_int16_t), data_size);

	size_t offset = sizeof(struct tcphdr) + sizeof(u_int16_t);

	char* ptr = *buff;
	u_int16_t* pk_num;
	register int i = 0;

	for(i = 0; i < num_packets; ++i, ptr += len){

		tcp = (struct tcphdr*)ptr;
		pk_num = (u_int16_t*)(tcp + 1);

		tcp->source = htons(sport);
		tcp->dest = htons(dport);
		tcp->seq = htonl(seq += send_len);
		tcp->ack = 1;
		tcp->ack_seq = htonl(ack++);
		tcp->doff = 5;
		tcp->window = (1 << 15) - 1;
		tcp->syn = 0;

		*pk_num = (u_int16_t)i;

		if(fill)
			memcpy(ptr + offset, buffer, data_size);

		/*copy udp packet into pseudo header buffer to calculate checksum*/
		memcpy(ps + 1, tcp, len);

		/* set tcp checksum */
		tcp->check = ip_checksum(ps, pslen);
	}

	return 0;
}

int setup_tcp_packets()
{

	int err;
	err = setup_tcp_train(&packets_e, 0);
	if(err != 0)
		return err;

	err = setup_tcp_train(&packets_f, 1);
	if(err != 0)
		return err;

	return 0;
}        // end setup_tcp_packets()

void mkicmpv4(void* buff, size_t datalen)
{
	if(!buff )
	{
		errno = EINVAL;
		perror("Invalid argument used in ICMP packet header: NULL pointer used");
		exit(EXIT_FAILURE);
	}

	if(datalen == 0 || datalen > SIZE)
	{
		errno = EINVAL;
		perror("Invalid argument used for ICMP packet header length");
		exit(EXIT_FAILURE);
	}


	/* set up icmp message header*/
	struct icmp* icmp = (struct icmp*)buff;
	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	icmp->icmp_id = (u_int16_t)getpid();
	icmp->icmp_seq = (u_int16_t)rand();
	memset(icmp->icmp_data, 0xa5, datalen);
	gettimeofday((struct timeval*)icmp->icmp_data, NULL);
	icmp->icmp_cksum = 0;
	icmp->icmp_cksum = ip_checksum(icmp, datalen + sizeof(struct icmp));

}

void* send_udp(void *status )
{
	int n;
	struct timespec tail_wait_tv;
	//tail wait is in milliseconds, so multiply by 10^6 to convert to nanoseconds
	tail_wait_tv.tv_nsec = tail_wait * 1000000;

	/*send Head ICMP Packet*/
	n = sendto(icmp_fd, icmp_send, icmp_ip_len, 0, res->ai_addr, res->ai_addrlen);
	if(n == -1){
		perror("Call to sendto() failed: error sending ICMP head packet");
		exit(EXIT_FAILURE);
	}

	*packet_id = 0;

	/*send data train*/
	register int i;
	for(i = 0; i < num_packets; ++i, (*packet_id)++){

		n = sendto(send_fd, packet_send, send_len, 0, res->ai_addr, res->ai_addrlen);
		if(n == -1){
			perror("call to() failed: error sending UDP udp train");
			exit(EXIT_FAILURE);
		}
	}

	struct icmp* icmp = (struct icmp*)(icmp_send + sizeof(struct ip));

	/*send tail ICMP Packets w/ timer*/
	pthread_mutex_lock(&stop_mutex);        //acquire lock
	for(i = 0; i < num_tail && stop == 0; ++i){
		/*not sure if changing the sequence number will help*/
		icmp->icmp_cksum = 0;
		icmp->icmp_seq += 1;
		icmp->icmp_cksum = ip_checksum(icmp, icmp_len);

		n = sendto(icmp_fd, icmp_send, icmp_ip_len, 0, res->ai_addr, res->ai_addrlen);
		if(n == -1){
			perror("Call to sendto() failed: icmp tail");
			exit(EXIT_FAILURE);
		}

		pthread_cond_timedwait(&stop_cv, &stop_mutex, &tail_wait_tv);

	}        //end for

	pthread_mutex_unlock(&stop_mutex);        // release lock
	status = NULL;
	pthread_exit(status);
}

void* send_tcp(void* status)
{
	int n;
	struct timespec tail_wait_tv;

	//tail wait is in milliseconds, so multiply by 10^6 to convert to nanoseconds
	tail_wait_tv.tv_nsec = tail_wait * 1000000;

	int len = send_len + sizeof(struct tcphdr);
	char buff[1500] = {0};
	struct tcphdr* tcp = (struct tcphdr*)packet_send;
	struct tcphdr* ps_tcp = (struct tcphdr*)(ps + 1);

	n = sendto(send_fd, syn_packet_1, sizeof(syn_packet_1), 0, res->ai_addr, res->ai_addrlen);
	if(n == -1){
		perror("Call to sendto() failed: tcp syn");
		exit(EXIT_FAILURE);
	}
	// set up the buffer to receive the reply into
	struct ip* ip = (struct ip*)buff;
	struct tcphdr* tcp_reply = (struct tcphdr*)(ip + 1);

	do{
		if((recvfrom(send_fd, buff, sizeof(buff), 0, 0, 0)) == -1){
			perror("call to recvfrom() failed: tcp SYN-ACK");
			exit(EXIT_FAILURE);
		}

	} while((tcp_reply->dest != htons(sport))
	        || (destip.s_addr != ip->ip_src.s_addr));

	if(verbose){
		printf("TCP SYN reply from IP: %s\n", inet_ntoa(ip->ip_src));
		printf("TCP SYN reply from port: %d to port: %d\n",
		       ntohs(tcp_reply->source),
		       ntohs(tcp_reply->dest));
	}
	td = get_time();    //time stamp just before we begin sending

	/*send data train*/
	register int i = 0;
	char* ptr = second_train ? packets_f : packets_e;

	for(i = 0; i < num_packets; ++i, ptr += len){
		n = sendto(send_fd, ptr, len, 0, res->ai_addr, res->ai_addrlen);
		if(n == -1){
			perror("Call to sendto() failed: tcp train");
			exit(EXIT_FAILURE);
		}        // end if
	}        // end of

	pthread_mutex_lock(&recv_ready_mutex);        // acquire lock
	recv_ready = 1;
	pthread_cond_signal(&recv_ready_cv);
	pthread_mutex_unlock(&recv_ready_mutex);        // release lock

	pthread_mutex_lock(&stop_mutex);        //acquire lock
	for(i = 0; i < num_tail && stop == 0; ++i){
		n = sendto(send_fd, syn_packet_2, sizeof(syn_packet_2), 0, res->ai_addr, res->ai_addrlen);
		if(n == -1){
			perror("Call to sendto() failed: TCP Tail Syn");
			exit(EXIT_FAILURE);
		}
		pthread_cond_timedwait(&stop_cv, &stop_mutex, &tail_wait_tv);
	}        // end for
	pthread_mutex_unlock(&stop_mutex);        // release lock

	tcp->source = ps_tcp->source = htons(sport);        //reset tcp sport for next train
	status = NULL;
	pthread_exit(status);
}

void fill_data(void* buff, size_t size)
{
	/* fill with random data from file location */
	int fd = open(file, O_RDONLY);
	if(fd < 0){
		perror("Error opening file");
		exit(-1);
	}
	/* fill buffer with size bytes of data from file*/
	int err = read(fd, buff, size);
	if(err < 0){
		perror("Error reading file");
		exit(-1);
	}
	close(fd);
}

void* recv4(void* t)
{
	double* time = (double*)t;

	/*number of bytes received*/
	int n;

	/* number of echo replies*/
	int count = 0;

	/*number of port unreachable replies processed and ignored*/
	int ack = 0;

	/* ICMP header */
	struct icmp* icmp;

	/* to receive data with*/
	struct sockaddr_in addr;

	/* length of address */
	socklen_t adrlen = sizeof(addr);

	if(tcp_bool == 1){
		pthread_mutex_lock(&recv_ready_mutex);        // release lock
		while(recv_ready == 0){
			pthread_cond_wait(&recv_ready_cv, &recv_ready_mutex);
		}
		pthread_mutex_unlock(&recv_ready_mutex);        // release lock

		struct ip* ip = (struct ip*)packet_rcv;
		struct tcphdr* tcp = (struct tcphdr*)(ip + 1);
		do{
			n = recvfrom(send_fd, packet_rcv, sizeof(packet_rcv), 0, (struct sockaddr*)&addr, &adrlen);
			if(n < 0){
				perror("recvfrom failed");
				exit(EXIT_FAILURE);
			}

		} while((tcp->dest != htons(syn_port))
		        || (ip->ip_src.s_addr != destip.s_addr));
		*time = get_time() - td;

		pthread_mutex_lock(&recv_ready_mutex);        // release lock
		recv_ready = 0;
		pthread_mutex_unlock(&recv_ready_mutex);        // release lock

		pthread_mutex_lock(&stop_mutex);        // release lock
		stop = 1;
		pthread_cond_signal(&stop_cv);
		pthread_mutex_unlock(&stop_mutex);        // release lock

		if(verbose){
			printf("TCP reply from IP: %s\n",
			       inet_ntoa(ip->ip_src));

			printf("TCP reply from port: %d to port: %d\n",
			       ntohs(tcp->source), ntohs(tcp->dest));
		}

	} else{

		/*Receive initial ICMP echo response && Time-stamp*/
		struct ip* ip = (struct ip*)packet_rcv;
		icmp = (struct icmp*)(ip + 1);
		struct udphdr* udp = (struct udphdr*)(&(icmp->icmp_data)
		                                      + sizeof(struct ip));

		u_int32_t* bitset = make_bs_32(num_packets);
		u_int16_t* id = (u_int16_t*)(udp + 1);
		struct in_addr dest;
		inet_aton(dst_ip, &dest);

		for(;;){

			n = recvfrom(recv_fd, packet_rcv, sizeof(packet_rcv), 0, (struct sockaddr*)&addr, &adrlen);

			if(n < 0){
				if(errno == EINTR)
					continue;
				perror("recvfrom failed");
				continue;
			} else if(ip->ip_src.s_addr != dest.s_addr){
				continue;
			} else if(icmp->icmp_type == 3 && icmp->icmp_code == 3){
				ack++;
				set_bs_32(bitset, *id, num_packets);
				continue;
			} else if(icmp->icmp_type == 0){
				if(count == 0){
					*time = get_time();
					count = 1;
				} else{
					*time = get_time() - *time;
					pthread_mutex_lock(&stop_mutex);        // acquire lock
					stop = 1;
					pthread_cond_signal(&stop_cv);
					pthread_mutex_unlock(&stop_mutex);        // release lock
					break;
				}        //end if
			} else if(icmp->icmp_type == 11){
				errno = ENETUNREACH;
				perror("TTL Exceeded");
				exit(EXIT_FAILURE);
			}        // end if

		}        // end for
		printf("UDP Packets received: %d/%d\n", ack, num_packets);

		if(verbose){
			printf("Missing Packets:  ");
			register int i = 0;
			for(i = 0; i < num_packets; ++i){
				if(get_bs_32(bitset, i, num_packets) == 0){
					int start = i;
					while(i < num_packets
					      && (get_bs_32(bitset,
					                    i + 1,
					                    num_packets)
					          == 0))
						i++;
					int end = i;
					if(end - start == 0)
						printf("%d, ", start + 1);
					else
						printf("%d-%d, ", start + 1,
						       end + 1);
				}
			}
			printf("\b\b \n");
		}
		if(verbose)
			printf("Echo reply from IP: %s\n",
			       inet_ntoa(ip->ip_src));
		if(bitset)
			free(bitset);
	}        // end if

	pthread_exit(NULL);
}

u_int16_t ip_checksum(void* vdata, size_t length)
{
	/* Cast the data pointer to one that can be indexed. */
	char* data = (char*)vdata;

	/* Initialize the accumulator. */
	u_int64_t acc = 0xffff;

	/* Handle any partial block at the start of the data. */
	unsigned int offset = ((uintptr_t)data) & 3;
	if(offset){
		size_t count = 4 - offset;
		if(count > length)
			count = length;
		u_int32_t word = 0;
		memcpy(offset + (char*)&word, data, count);
		acc += ntohl(word);
		data += count;
		length -= count;
	}

	/* Handle any complete 32-bit blocks. */
	char* data_end = data + (length & ~3);
	while(data != data_end){
		u_int32_t word = 0;
		memcpy(&word, data, 4);
		acc += ntohl(word);
		data += 4;
	}
	length &= 3;

	/* Handle any partial block at the end of the data. */
	if(length){
		u_int32_t word = 0;
		memcpy(&word, data, length);
		acc += ntohl(word);
	}

	/* Handle deferred carries. */
	acc = (acc & 0xffffffff) + (acc >> 32);
	while(acc >> 16){
		acc = (acc & 0xffff) + (acc >> 16);
	}

	/* If the data began at an odd byte address */
	/* then reverse the byte order to compensate. */
	if(offset & 1){
		acc = ((acc & 0xff00) >> 8) | ((acc & 0x00ff) << 8);
	}

	/* Return the checksum in network byte order. */
	return htons(~acc);
}

void print_use(char* program_name)
{
	printf("Usage: %s [-H | -L] [-T] [-v] [-p PORT] [-c COOLDOWN ]\n"
			       "\t\t  [-f FILENAME_PAYLOAD] [-s DATA_SIZE] [-n NUM_PACKETS]\n"
			       "\t\t  [-t TTL] [-w TAIL_INTERVAL] [-r NUM_TAIL] IPAddress\n",
	               program_name);
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

	/* Default to 1 kilobyte packets */
	data_size = 1024 - sizeof(u_int16_t) - sizeof(struct tcphdr) - sizeof(struct ip);
	num_packets = 1000;             // send 1000 packets in udp data train
	ttl = 255;                      // max ttl
	tail_wait = 10;                 // wait 10 ms between ICMP tail messages
	num_tail = 20;                  // send 20 ICMP tail messages
	file = "/dev/urandom";          // default to random data for compression detection

	send_train = send_udp;

	int check;
	int c = 0;
	while((c = getopt(argc, argv, "HLTvp:c:f:s:n:t:w:r:h")) != -1){
		switch(c){
			case 'H':
				lflag = 0;
		        break;
			case 'L':
				hflag = 0;
		        break;
			case 'p':
				check = atoi(optarg);
		        if(check < (1 << 16) && check > 0){
			        dport = (u_int16_t)check;
		        } else{
			        errno = ERANGE;
			        perror("Port range: 1 - 65535");
			        return EXIT_FAILURE;
		        }
		        break;
			case 's':
				check = atoi(optarg);
		        if(check < 1 || check > (int)TCP_DATA_SIZE){
			        errno = ERANGE;
			        char str[256] = {0};
			        snprintf(str, 256, "Valid UDP data size: 1-%lu",
			                 TCP_DATA_SIZE);
			        perror(str);
			        return EXIT_FAILURE;
		        }
		        data_size = (u_int16_t)check;
		        break;
			case 'n':
				check = atoi(optarg);
		        if(check < 1 || check > 10000){
			        errno = ERANGE;
			        perror("# of packets: 0 - 10000");
			        return EXIT_FAILURE;
		        }
		        num_packets = (u_int16_t)check;
		        break;
			case 't':
				check = atoi(optarg);
		        if(check < 0 || check > 255){
			        errno = ERANGE;
			        perror("TTL range: 0 - 255");
			        return EXIT_FAILURE;
		        } else
			        ttl = (u_int8_t)check;
		        break;
			case 'w':
				check = atoi(optarg);
		        if(check < 0){
			        errno = ERANGE;
			        perror("Time wait must be positive");
			        return EXIT_FAILURE;
		        }
		        tail_wait = (u_int16_t)check;
		        break;
			case 'r':
				check = atoi(optarg);
		        if(check < 1 || check > 1000){
			        errno = ERANGE;
			        perror("# Tail Packets: 1 - 1,000");
			        return EXIT_FAILURE;
		        }
		        num_tail = (u_int16_t)check;
		        break;
			case 'f':{
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
			case 'h':
				print_use(argv[0]);
		        return EXIT_FAILURE;
		        break;
			case 'T':
				tcp_bool = 1;
		        send_train = send_tcp;
		        break;
			case 'v':
				verbose = 1;
		        break;
			case 'c':
				cooldown = atoi(optarg);
		        if(cooldown < 0 || cooldown > 1000){
			        errno = ERANGE;
			        perror("Cooldown Range: 1 - 1,000 Seconds");
			        return EXIT_FAILURE;
		        }
		        break;

			case ':':
				errno = ERANGE;
		        fprintf(stderr, "%s: Invalid option '-%s', check use\n", argv[0], optarg);
		        print_use(argv[0]);
		        return EXIT_FAILURE;
				break;
			default:
				errno = ERANGE;
		        fprintf(stderr, "%s: error -- check use\n", argv[0]);
				print_use(argv[0]);
		        return EXIT_FAILURE;
		}        // end switch
	}        //end while
	/* these are the arguments after the command-line options */
	for(; optind < argc; optind++){
		dst_ip = argv[optind];
		//printf("argument: \"%s\"\n", dst_ip);
	}
	if(!dst_ip){
		fprintf(stderr, "Error: Target IP missing\n");
		print_use(argv[0]);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
