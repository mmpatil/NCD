//#include <stdio.h>           /* for printf, fprintf, snprintf, perror, ... */
//#include <stdlib.h>          /* for EXIT_SUCCESS, EXIT_FAILURE, */
//#include <string.h>          /* for memcpy */
//#include <sys/time.h>        /* for gettimeofday() */
//#include <errno.h>           /* for errno*/
#include <unistd.h>          /* for close() */
#include <sys/socket.h>      /* for socket(), setsockopt(), etc...*/
#include <netinet/ip.h>      /* for struct ip */
#include <netinet/ip_icmp.h> /* for struct icmp */
#include <netinet/tcp.h>     /* for struct tcphdr */
#include <netinet/udp.h>     /* for struct udphdr */
#include <netdb.h>           /* for getaddrinfo() */
#include <arpa/inet.h>       /* for inet_pton() */
//#include <signal.h>          /* for kill() */
//#include <fcntl.h>           /* for O_RDONLY */
//#include <ctype.h>           /* for inet_pton() */
//#include <pthread.h>         /* for pthread */
//#include "ncd_global.h"

#include <string>
#include <sstream>
#include <fstream>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <cstring>


#include <vector>

#include "ip_checksum.h"

typedef std::vector<char> buffer_t;

enum class transport_type
{
    udp,
    tcp
};


/**
 * struct for udp pseudo header
 */
struct __attribute__((__packed__)) pseudo_header
{
    uint32_t source;
    uint32_t dest;
    uint8_t zero;
    uint8_t proto;
    uint16_t len;
};


class packet
{
public:
    packet(size_t length, size_t data_offset = 0) : data(length, 0), filled(false), data_offset(data_offset) {}
    virtual ~packet();
    virtual void fill(std::ifstream& file)
    {
        if(file.is_open())
        {
            file.read(&data[data_offset], data.size() - data_offset);
        }
        else
        {
            std::string err_string = "Error filling packet from file: file was not open";
            std::cerr << err_string << std::endl;
            std::ios_base::failure e(err_string);
            throw e;
        }
        filled = true;
    }

    buffer_t data;
    bool filled;
    size_t data_offset;
};

class udp_packet : public packet
{
public:
    udp_packet(size_t length, uint16_t sport, uint16_t dport, size_t trans_offset = 0)
        : packet(sizeof(udphdr)+length, trans_offset + sizeof(udphdr)), transport_offset(trans_offset)
    {
        udphdr* udp_header = (udphdr*)&(data[trans_offset]);
        udp_header->source = htons(sport);
        udp_header->dest   = htons(dport);
    }

    virtual ~udp_packet();

    virtual void checksum(const pseudo_header& ps)
    {
        size_t offset = sizeof(ps);
        buffer_t buff(offset + data.size());
        // copy the pseudo header into the buffer
        memcpy(&buff[0], &ps, offset);
        // copy the transport header and data into the buffer
        memcpy(&buff[offset], &data[0], data.size());

        udphdr* udp_header = (udphdr*)&(data[0]);
        udp_header->check  = ip_checksum(&buff[0], buff.size());
    }

private:
    /* data */
    size_t transport_offset;
};


class tcp_packet : public packet
{
public:
    tcp_packet(size_t length, uint16_t sport, uint16_t dport, uint32_t sequence, uint32_t ack_sequence, uint16_t win,
               uint16_t urg, bool ack_flag, bool syn_flag, char doff, size_t trans_off =0)
        : packet(length + sizeof(tcphdr), trans_off+ sizeof(tcphdr)), transport_offset(trans_off)
    {
        tcphdr* tcp_header  = (tcphdr*)&(data[transport_offset]);
        tcp_header->source  = htons(sport);
        tcp_header->dest    = htons(dport);
        tcp_header->seq     = htonl(sequence);
        tcp_header->ack_seq = htonl(ack_sequence);
        tcp_header->ack     = ack_flag;
        tcp_header->syn     = syn_flag;
        tcp_header->doff    = doff;
        tcp_header->window  = win;
        tcp_header->urg_ptr = urg;
    }

    virtual ~tcp_packet();


    virtual void checksum(const pseudo_header& ps)
    {
        size_t offset = sizeof(ps);
        buffer_t buff(offset + data.size());
        // copy the pseudo header into the buffer
        memcpy(&buff[0], &ps, offset);
        // copy the transport header and data into the buffer
        memcpy(&buff[offset], &data[0], data.size());

        tcphdr* tcp_header = (tcphdr*)&(data[0]);
        tcp_header->check  = ip_checksum(&buff[0], buff.size());
    }

private:
    size_t transport_offset;
};


class ip_tcp_packet : public tcp_packet
{
public:
    ip_tcp_packet(const iphdr& ip, size_t length, uint16_t sport, uint16_t dport, uint32_t sequence,
                  uint32_t ack_sequence, uint16_t win, uint16_t urg, bool ack_flag, bool syn_flag, char doff)
        : tcp_packet(length + sizeof(tcphdr) + sizeof(iphdr), sizeof(iphdr), sport, dport, sequence, ack_sequence, win, urg, ack_flag,
                     syn_flag, doff)
    {
        std::memcpy(&data[0], &ip, sizeof(iphdr));
    }

    virtual ~ip_tcp_packet();
private:
    /* data */
};

typedef std::vector<std::reference_wrapper<packet>> packet_buffer_t;


/**
 * new class to manage all resources used during  discrimination detection
 * only responsible for a single train
 */
class detector
{

    enum raw_level
    {
        none,
        transport_only,
        full
    };

public:
    detector(std::string src_ip, std::string dest_ip, uint8_t tos, uint16_t ip_length, uint16_t id, uint16_t frag_off,
             uint8_t ttl, uint8_t proto, uint16_t check_sum, uint32_t sport, uint32_t dport,
             std::string filename = "/dev/urandom", uint16_t num_packets = 1000, uint16_t data_length = 512,
             uint32_t num_tail = 20, uint16_t tail_wait = 10, raw_level raw_status = none,
             transport_type trans_proto = transport_type::udp)
        : src_ip(src_ip),
          dest_ip(dest_ip),
          trans(trans_proto),
          ip_header{0, 0, tos, ip_length, id, frag_off, ttl, proto, check_sum, 0, 0},
          sport(sport),
          dport(dport),
          res(nullptr),
          num_packets(num_packets),
          num_tail(num_tail),
          data_length(data_length),
          tail_wait(tail_wait),
          raw(raw_status),
          miliseconds(0),
          elapsed{}
    {
        // get file stream to use in packet initialization;
        file.open(filename);
        if(!file.is_open())
        {
            std::string err_string = "Error: file " + filename + " could not be opened";
            std::ios_base::failure e(err_string);
            std::cerr << err_string << std::endl;
            throw e;
        }

        // setup the IP info ...
        setup_ip_info();
    }

    virtual ~detector()
    {
        if(res)
            freeaddrinfo(res);
    }

    virtual void setup_packet_train()
    {
        // allocate buffer for the entire data train

        switch(raw)
        {
        case full:

            //data_train.resize(packet_length);
        case transport_only:
            break;
        default:
            break;
        }

        //data_train.resize(packet_length * num_packets);
    }

    virtual void setup_transport_header();        // sets up the transport header -- pure virutal
    virtual int transport_header_size();          // returns size of transport header -- pure virtual
    virtual void send_train();                    // sends the packet train -- pure virtual;
    virtual void receive();                       // receives responses from the target IP -- pure virtual
    virtual void create_packet();                 // creates a single packet -- pure virtual
    virtual void setup_ip_info()
    {
        // set leading bits for version and ip header length -- can change later;
        ip_header.version = 4;
        ip_header.ihl     = 5;

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
    }            // end setup_ip_info()

private:
    /*ip data internal */
    std::string src_ip;                  // string with IP address
    std::string dest_ip;                 // string with IP address
    transport_type trans;                // enum containing the transport type
    iphdr ip_header;                     // IP header struct -- holds all the values
    tcphdr tcp_header;                   // transport layer header --- should probably get rid of this
    udphdr udp_header;                   // transport layer header --- should probably get rid of this
    uint32_t sport;                      // transport layer source port -- should this be here?
    uint32_t dport;                      // transport layer source port -- should this be here?
    bool verbose;                        // verbose output
    bool sql_output;                     // output minimal information for our SQL data structures
    int packets_lost;                    // number of packets lost -- for loss based approach
    const size_t num_threads = 2;        // number of threads used by pthread
    addrinfo* res;
    packet_buffer_t data_train;

    // file descriptors
    int send_fd;
    int recv_fd;
    // int other _fd; // not sure we should use this????

    // cli arg
    uint16_t payload_size;
    uint16_t num_packets;
    uint16_t num_tail;
    uint16_t data_length;
    u_int16_t tail_wait;
    // pointers
    // packets

    // packet size, payload size, transport size, ip size


    // threading items

    // internal data
    std::ifstream file;        // file to read payload in from -- could also be file with entire train pre made
    raw_level raw;
    // time
    double miliseconds;
    timeval elapsed;
};

