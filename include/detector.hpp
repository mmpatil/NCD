//#include <stdio.h>           /* for printf, fprintf, snprintf, perror, ... */
//#include <stdlib.h>          /* for EXIT_SUCCESS, EXIT_FAILURE, */
//#include <string.h>          /* for memcpy */
//#include <sys/time.h>        /* for gettimeofday() */
//#include <errno.h>           /* for errno*/
#include <sys/socket.h>      /* for socket(), setsockopt(), etc...*/
#include <netinet/ip.h>      /* for struct ip */
#include <netinet/ip_icmp.h> /* for struct icmp */
#include <netinet/tcp.h>     /* for struct tcphdr */
#include <netinet/udp.h>     /* for struct udphdr */
#include <netdb.h>           /* for getaddrinfo() */
#include <arpa/inet.h>       /* for inet_pton() */
//#include <signal.h>          /* for kill() */
//#include <fcntl.h>           /* for O_RDONLY */
//#include <unistd.h>          /* for _________ */
//#include <ctype.h>           /* for inet_pton() */
//#include <pthread.h>         /* for pthread */
//#include "ncd_global.h"

#include <string>
#include <sstream>
#include <fstream>
#include <cstdint>
#include <iostream>

#include <unistd.h>

enum class transport_type
{
    udp,
    tcp
};


/**
 * new class to manage all resources used during  discrimination detection
 * only responsible for a single train
 */
class detector
{
public:
    detector(std::string src_ip, std::string dest_ip, uint8_t tos, uint16_t ip_length, uint16_t id, uint16_t frag_off,
             uint8_t ttl, uint8_t proto, uint16_t check_sum, uint32_t sport, uint32_t dport, std::string filename = "")
        : src_ip(src_ip), dest_ip(dest_ip), tos(tos), ip_length(ip_length), ip_id(id), frag_off(frag_off), ttl(ttl),
          proto(proto), check_sum(check_sum), sport(sport), dport(dport), res(nullptr)
    {
        file.open(filename);
        setup_ip_info();

        // source and destination addresses handled in initialize()
        ip_header.tos      = tos;
        ip_header.tot_len  = ip_length;
        ip_header.id       = id;
        ip_header.frag_off = frag_off;
        ip_header.ttl      = ttl;
        ip_header.protocol=proto;
        ip_header.check = check_sum;
    }

    virtual ~detector()
    {
        if(res)
            freeaddrinfo(res);
    }

    virtual void setup_packet_train();
    virtual void setup_transport_header();
    virtual void send_train();
    virtual void receive();
    virtual void create_packet();
    void setup_ip_info()
    {
        /* set up hints for getaddrinfo() */
        addrinfo hints = {}; /* for get addrinfo */
        hints.ai_flags = AI_CANONNAME;


        // choose the correct protocol
        switch(trans)
        {
        case transport_type::udp:
            hints.ai_protocol = IPPROTO_UDP;
            break;
        case transport_type::tcp:
            hints.ai_protocol = IPPROTO_TCP;
            break;
        default:
            std::cerr << "Error: the transport_type selected is not supported" << std::endl;
            break;
        }

        /* pass a string of the destination point to getaddrinfo */
        std::stringstream ss;
        ss << dport;

        // get destination IP just in case it was a URL
        int err = getaddrinfo(dest_ip.c_str(), ss.str().c_str(), &hints, &res);
        if(err)
        {
            if(err == EAI_SYSTEM)
                std::cerr << "Error looking up " << dest_ip << ":" << errno << std::endl;
            else
                std::cerr << "Error looking up " << dest_ip << ":" << gai_strerror(err) << std::endl;
            exit(EXIT_FAILURE);
        }

        // set destination address
        ip_header.daddr = ((struct sockaddr_in*)res->ai_addr)->sin_addr.s_addr;

        // get temp socket to obtain source IP -- its a hack
        {
            struct sockaddr_in srcaddrs = {};                      // source IP address
            socklen_t sa_len            = sizeof(srcaddrs);        // size of src address

            int temp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if(connect(temp_sock, res->ai_addr, res->ai_addrlen) == -1)
            {
                std::cerr << "Connect failed: " << errno;
                exit(EXIT_FAILURE);
            }
            if(getsockname(temp_sock, (struct sockaddr*)&srcaddrs, &sa_len) == -1)
            {
                std::cerr << "getsockname() failed: " << errno;
                exit(EXIT_FAILURE);
            }

            ip_header.saddr = srcaddrs.sin_addr.s_addr;
            close(temp_sock);
        }        // end temp socket
    }// end setup_ip_info()

private:
    /* data */
    std::string src_ip;          // string with IP address
    std::string dest_ip;         // string with IP address
    std::ifstream file;          // file to read payload in from -- could also be file with entire train pre made
    transport_type trans;        // enum containing the transport type
    iphdr ip_header;             // IP header struct -- holds all the values
    tcphdr tcp_header;           // transport layer header --- should probably get rid of this
    udphdr udp_header;           // transport layer header --- should probably get rid of this
    uint8_t tos;                 // type of service field --  Planetlab has limited support for changing TOS
    uint16_t ip_length;          // total length of IP packet
    uint16_t ip_id;              // identification field
    uint16_t frag_off;           // fragmentation offset
    uint8_t ttl;                 // time to live
    uint8_t proto;               // protocol
    uint16_t check_sum;          // IP checksum value
    uint32_t saddr;              // numerical value for source address
    uint32_t daddr;              // numerical value for destination address
    uint32_t sport;              // transport layer source port -- should this be here?
    uint32_t dport;              // transport layer source port -- should this be here?
    bool verbose;                // verbose output
    bool sql_output;             // output minimal information for our SQL data structures
    int packets_lost;            // number of packets lost -- for loss based approach
    const size_t num_threads = 2;        // number of threads used by pthread
    addrinfo* res;
};

