#include "ncd.h"


int comp_det(char* address, char * port, size_t num_packets, size_t time_wait){


	printf("Address %s\nport: %s\nnum_packets: %d\ntime_wait: %d\n\n", address, port, num_packets, time_wait);

	//data
	int ret = 0; //integer code for the return 1 == compression, 0 == no compression, -1 == error
	struct addrinfo *res, hints;

	char low_data[1100] = { 0 }; /* low entropy data*/
	char high_data[1100]; /* high entropy data*/

	/*get random data for high entropy datagrams*/
	int random = open("/dev/urandom", O_RDONLY);
	read(random, high_data, sizeof(high_data));
	close(random);



	//set up ICMP Timestamp packets
	struct icmphdr req;
	bzero(&hints, sizeof hints);
	hints.ai_flags = AI_CANONNAME;
	hints.ai_family = 0;
	hints.ai_socktype =0;


	int fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if(fd ==-1){
		perror("call to socket() failed");
		exit(EXIT_FAILURE);// consider doing something better here
	}

	int hdrincl = 1;
	if (setsockopt(fd,IPPROTO_IP,IP_HDRINCL,&hdrincl,sizeof(hdrincl))==-1) {
		perror("setsockopt() failed");
		exit( EXIT_FAILURE);
	}

	req.type = ICMP_TIMESTAMP;
	req.code = 0;
	req.checksum = 0;
	req.un.echo.id = htons(rand());		//maybe change
	req.un.echo.sequence = htons(1);	// ditto ....
	req.checksum = ip_checksum(&req, sizeof(req));

	int err = getaddrinfo(address, NULL, &hints, &res);

	if (err != 0){
		if (err == EAI_SYSTEM)
		        fprintf(stderr, "looking up www.example.com: %s\n", strerror(errno));
		else
		        fprintf(stderr, "looking up www.example.com: %s\n", gai_strerror(err));
		exit(EXIT_FAILURE);
	}



	printf("\ncannon name: %s \nsa_data %s\nai_protocol: %d\n\n",res->ai_canonname, res->ai_addr->sa_data, res->ai_protocol);
	printf("\nAI_FLAGS:%d \nSocket_type: %d\n\n",res->ai_flags, res->ai_socktype);

	if(sendto(fd, &req, 8, res->ai_flags, res->ai_addr, res->ai_addrlen) ==  -1){
		perror("icmp sendto() failed");
		exit(EXIT_FAILURE);
	}







	// set up low and high entropy data

	// set up raw socket

	// set up any structs and data needed by send/recieve

	freeaddrinfo(res);



	return ret;
}



int send_data(){
	int ret = 0;

	return ret;
}



int recv_data(){
	int ret = 0;

	return ret;
}


/*
struct icmphdr* make_icmp(char *address, unsigned char code, void * data, ssize_t data_len){

}
*/


uint16_t ip_checksum(void* vdata,size_t length) {
    // Cast the data pointer to one that can be indexed.
    char* data=(char*)vdata;

    // Initialise the accumulator.
    uint64_t acc=0xffff;

    // Handle any partial block at the start of the data.
    unsigned int offset=((uintptr_t)data)&3;
    if (offset) {
        size_t count=4-offset;
        if (count>length) count=length;
        uint32_t word=0;
        memcpy(offset+(char*)&word,data,count);
        acc+=ntohl(word);
        data+=count;
        length-=count;
    }

    // Handle any complete 32-bit blocks.
    char* data_end=data+(length&~3);
    while (data!=data_end) {
        uint32_t word;
        memcpy(&word,data,4);
        acc+=ntohl(word);
        data+=4;
    }
    length&=3;

    // Handle any partial block at the end of the data.
    if (length) {
        uint32_t word=0;
        memcpy(&word,data,length);
        acc+=ntohl(word);
    }

    // Handle deferred carries.
    acc=(acc&0xffffffff)+(acc>>32);
    while (acc>>16) {
        acc=(acc&0xffff)+(acc>>16);
    }

    // If the data began at an odd byte address
    // then reverse the byte order to compensate.
    if (offset&1) {
        acc=((acc&0xff00)>>8)|((acc&0x00ff)<<8);
    }

    // Return the checksum in network byte order.
    return htons(~acc);
}
