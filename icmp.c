#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>



void make_icmp(char *address, unsigned char code, void * data, ssize_t data_len){
	struct icmphdr req;

	int fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if(fd ==-1)
		exit(FAILURE);// cosider doing something better here

	int hdrinc = 1;

	if (setsockopt(fd,IPPROTO_IP,IP_HDRINCL,&hdrincl,sizeof(hdrincl))==-1) {
    		die("%s",strerror(errno));
	}

	req.type = ICMP_TSTAMP;
	req.code = 0;
	req.checksum = 0;
	req.icmp_seq=htons(1);
	req.icmp_id = getpid();
}

void make_icmp(char *address, unsigned char code)
	make_icmp(address, code, NULL, 0);
}	
 
