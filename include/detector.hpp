//#include <stdio.h>           /* for printf, fprintf, snprintf, perror, ... */
//#include <stdlib.h>          /* for EXIT_SUCCESS, EXIT_FAILURE, */
//#include <string.h>          /* for memcpy */
#include <sys/time.h>        /* for gettimeofday() */
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
#include <pthread.h> /* for pthread */
//#include "ncd_global.h"

#include <string>
#include <sstream>
#include <fstream>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <memory>
#include <vector>

#include "ip_checksum.h"
#include "bitset.h"

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
    packet(size_t payload_length, size_t data_offset = 0)
        : data(payload_length + sizeof(uint16_t), 0), filled(false), data_offset(data_offset)
    {
    }
    virtual ~packet();
    virtual void fill(std::ifstream& file, uint16_t packet_id)
    {
        if(file.is_open())
        {
            memcpy(&data[0], &packet_id, sizeof(packet_id));
            file.read(&data[data_offset + sizeof(uint16_t)], data.size() - data_offset);
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
    virtual void checksum(const pseudo_header& ps) {}
    buffer_t data;
    bool filled;
    size_t data_offset;
};

class udp_packet : public packet
{
public:
    udp_packet(size_t payload_length, uint16_t sport, uint16_t dport, size_t trans_offset = 0)
        : packet(sizeof(udphdr) + payload_length, trans_offset + sizeof(udphdr)), transport_offset(trans_offset)
    {
        udphdr* udp_header = (udphdr*)&(data[trans_offset]);
        udp_header->source = htons(sport);
        udp_header->dest   = htons(dport);
        udp_header->len    = htons(payload_length + sizeof(udphdr));
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
    tcp_packet(size_t payload_length, uint16_t sport, uint16_t dport, uint32_t sequence, uint32_t ack_sequence,
               uint16_t win, uint16_t urg, bool ack_flag, bool syn_flag, char doff, size_t trans_off = 0)
        : packet(payload_length + sizeof(tcphdr), trans_off + sizeof(tcphdr)), transport_offset(trans_off)
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
    ip_tcp_packet(const iphdr& ip, size_t payload_length, uint16_t sport, uint16_t dport, uint32_t sequence,
                  uint32_t ack_sequence, uint16_t win, uint16_t urg, bool ack_flag, bool syn_flag, char doff)
        : tcp_packet(payload_length + sizeof(iphdr), sizeof(iphdr), sport, dport, sequence, ack_sequence, win, urg,
                     ack_flag, syn_flag, doff),
          ps{ip.saddr, ip.daddr, 0, IPPROTO_TCP, htons(payload_length + sizeof(pseudo_header) + sizeof(udphdr))}
    {
        std::memcpy(&data[0], &ip, sizeof(iphdr));
    }

    virtual ~ip_tcp_packet();

    virtual void fill(std::ifstream& file, uint16_t packet_id)
    {
        packet::fill(file, packet_id);

        tcp_packet::checksum(ps);

        iphdr* ip = (iphdr*)&data[0];
        ip->check = ip_checksum(&data[0], data.size());
    }
    pseudo_header ps;

private:
    /* data */
};

class ip_udp_packet : public udp_packet
{
public:
    ip_udp_packet(const iphdr& ip, size_t payload_length, uint16_t sport, uint16_t dport)
        : udp_packet(payload_length + sizeof(iphdr), sizeof(iphdr), sport, dport),
          ps{ip.saddr, ip.daddr, 0, IPPROTO_TCP, htons(payload_length + sizeof(pseudo_header) + sizeof(udphdr))}

    {
        std::memcpy(&data[0], &ip, sizeof(iphdr));
    }

    virtual ~ip_udp_packet();
    virtual void fill(std::ifstream& file, uint16_t packet_id)
    {
        packet::fill(file, packet_id);

        udp_packet::checksum(ps);

        iphdr* ip = (iphdr*)&data[0];
        ip->check = ip_checksum(&data[0], data.size());
    }

    pseudo_header ps;

private:
    /* data */
};


typedef std::vector<std::shared_ptr<packet>> packet_buffer_t;

enum raw_level
{
    none,
    transport_only,
    full
};

/**
 * new class to manage all resources used during  discrimination detection
 * only responsible for a single train
 */
class detector
{


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
    virtual void setup_sockets();        // pure virtual
    virtual void setup_packet_train()
    {
        switch(raw)
        {
        case full:
            populate_full();
            break;
        case transport_only:
            populate_trans();
            break;
        // none is also the current default, make appropriate changes if necessary
        case none:
        default:
            populate_none();
            break;
        }

        uint16_t packet_id = 0;
        for(auto& item : data_train)
        {
            item->fill(file, packet_id++);
            if(raw == transport_only)
            {
                pseudo_header ps = {ip_header.saddr, ip_header.daddr, 0, IPPROTO_TCP,
                                    htons(payload_size + transport_header_size() + sizeof(pseudo_header))};
                item->checksum(ps);
            }
        }
    }
    virtual void populate_full();                 // pure virtual
    virtual void populate_trans();                // pure virtual
    virtual void populate_none();                 // pure virtual
    virtual int transport_header_size();          // returns size of transport header -- pure virtual
    virtual void send_train(void* status);        // sends the packet train -- pure virtual;
    virtual void receive();                       // receives responses from the target IP -- pure virtual
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

protected:
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

    // threading items
    bool recv_ready;        // bool for receiving SYN packets -- denotes if the program is ready to receive traffic
    bool stop;              // boolean for if the send thread can stop (receive thread has received second response.
    pthread_mutex_t stop_mutex;              // mutex for stop
    pthread_mutex_t recv_ready_mutex;        // mutex for recv_ready
    pthread_cond_t stop_cv;                  // condition variabl for stop -- denotes
    pthread_cond_t recv_ready_cv;            // condition variable for recv_ready mutex


    // internal data
    std::ifstream file;        // file to read payload in from -- could also be file with entire train pre made
    raw_level raw;
    // time
    double miliseconds;
    timeval elapsed;
};


class udp_detector : public detector
{
public:
    udp_detector(std::string src_ip, std::string dest_ip, uint8_t tos, uint16_t id, uint16_t frag_off, uint8_t ttl,
                 uint8_t proto, uint16_t check_sum, uint32_t sport, uint32_t dport,
                 std::string filename = "/dev/urandom", uint16_t num_packets = 1000, uint16_t data_length = 512,
                 uint32_t num_tail = 20, uint16_t tail_wait = 10, raw_level raw_status = none,
                 transport_type trans_proto = transport_type::udp)
        : detector(src_ip, dest_ip, tos, data_length + sizeof(udphdr) + sizeof(iphdr), id, frag_off, ttl, proto,
                   check_sum, sport, dport, filename, num_packets, data_length, num_tail, tail_wait, raw_status,
                   trans_proto)
    {
    }
    virtual ~udp_detector();

    virtual void populate_full()
    {
        data_train.resize(num_packets, std::make_shared<ip_udp_packet>(ip_header, payload_size, sport, dport));
    }
    virtual void populate_trans()
    {
        data_train.resize(num_packets, std::make_shared<udp_packet>(ip_header, payload_size, sport, dport));
    }
    virtual void populate_none()
    {
        data_train.resize(num_packets, std::make_shared<packet>(ip_header, payload_size, sport, dport));
    };

    virtual int transport_header_size() { return sizeof(udphdr); }

    virtual void setup_sockets() {}
    virtual void send_train(void* status)
    {
        char icmp_send[128]  = {0};        // buffer for ICMP messages
        int icmp_packet_size = 64;         // 64 byte icmp packet size up to a mx of 76 bytes for replies

        /* size of ICMP Echo message */
        // uint16_t icmp_data_len = (uint16_t)(icmp_packet_size - sizeof(struct icmp));

        /*size of icmp packet*/
        uint16_t icmp_len = (uint16_t)(icmp_packet_size);

        /* size of ICMP reply + ip header */
        uint16_t icmp_ip_len = (uint16_t)(sizeof(struct ip) + icmp_len);

        int n;
        struct timespec tail_wait_tv;

        // tail wait is in milliseconds, so multiply by 10^6 to convert to nanoseconds
        tail_wait_tv.tv_nsec = tail_wait * 1000000;

        /*send Head ICMP Packet*/
        n = sendto(icmp_fd, icmp_send, icmp_ip_len, 0, res->ai_addr, res->ai_addrlen);
        if(n == -1)
        {
            perror("Call to sendto() failed: error sending ICMP head packet");
            exit(EXIT_FAILURE);
        }

        /*send data train*/
        for(auto item : data_train)
        {
            n = sendto(send_fd, item->data.data(), item->data.size(), 0, res->ai_addr, res->ai_addrlen);
            if(n == -1)
            {
                perror("call to sendto() failed: error sending UDP udp train");
                exit(EXIT_FAILURE);
            }
        }

        struct icmp* icmp = (struct icmp*)(icmp_send + sizeof(struct ip));

        /*send tail ICMP Packets with timer*/
        pthread_mutex_lock(&stop_mutex);        // acquire lock
        for(int i = 0; i < num_tail && stop == 0; ++i)
        {
            /*not sure if changing the sequence number will help*/
            icmp->icmp_cksum = 0;
            icmp->icmp_seq += 1;
            icmp->icmp_cksum = ip_checksum(icmp, icmp_len);

            n = sendto(icmp_fd, icmp_send, icmp_ip_len, 0, res->ai_addr, res->ai_addrlen);
            if(n == -1)
            {
                perror("Call to sendto() failed: icmp tail");
                exit(EXIT_FAILURE);
            }

            pthread_cond_timedwait(&stop_cv, &stop_mutex, &tail_wait_tv);

        }        // end for

        pthread_mutex_unlock(&stop_mutex);        // release lock
        status = NULL;
        pthread_exit(status);
    }

    virtual void receive()
    {

        /*number of bytes received*/
        int n;

        /* number of echo replies*/
        int count = 0;

        /*number of port unreachable replies processed and ignored*/
        int udp_ack = 0;

        /* ICMP header */
        struct icmp* icmp;

        /* to receive data with*/
        struct sockaddr_in addr;

        char packet_rcv[1500] = {0};        // buffer for receiving replies
        /* length of address */
        socklen_t adrlen = sizeof(addr);

        /*Receive initial ICMP echo response && Time-stamp*/
        struct ip* ip      = (struct ip*)packet_rcv;
        icmp               = (struct icmp*)(ip + 1);
        struct udphdr* udp = (struct udphdr*)(&(icmp->icmp_data) + sizeof(struct ip));

        uint32_t* bitset = make_bs_32(num_packets);
        uint16_t* id     = (uint16_t*)(udp + 1);

        for(;;)
        {

            n = recvfrom(recv_fd, packet_rcv, sizeof(packet_rcv), 0, (struct sockaddr*)&addr, &adrlen);

            if(n < 0)
            {
                if(errno == EINTR)
                    continue;
                perror("recvfrom() failed");
                continue;
            }
            else if(ip->ip_src.s_addr != ip_header.daddr)
            {
                continue;
            }
            else if(icmp->icmp_type == 3 && icmp->icmp_code == 3)
            {
                udp_ack++;
                set_bs_32(bitset, *id, num_packets);
                continue;
            }
            else if(icmp->icmp_type == 0)
            {
                if(count == 0)
                {
                    miliseconds = get_time();
                    count = 1;
                }
                else
                {
                    miliseconds = get_time() - miliseconds;
                    pthread_mutex_lock(&stop_mutex);        // acquire lock
                    stop = 1;
                    pthread_cond_signal(&stop_cv);
                    pthread_mutex_unlock(&stop_mutex);        // release lock
                    break;
                }        // end if
            }
            else if(icmp->icmp_type == 11)
            {
                errno = ENETUNREACH;
                perror("TTL Exceeded");
                exit(EXIT_FAILURE);
            }        // end if

        }        // end for
        char* c = (second_train && !tcp_bool) ? "High" : "Low";

        if(!output_bool)
            printf("UDP %s Packets received: %d/%d\n", c, udp_ack, num_packets);

        // report the losses
        int* losses = (second_train && !tcp_bool) ? &high_losses : &low_losses;
        *losses     = num_packets - udp_ack;

        if(verbose)
        {
            printf("Missing Packets:  ");
            int i = 0;
            for(i = 0; i < num_packets; ++i)
            {
                if(get_bs_32(bitset, i, num_packets) == 0)
                {
                    int start = i;
                    while(i < num_packets && (get_bs_32(bitset, i + 1, num_packets) == 0))
                        i++;
                    int end = i;
                    if(end - start == 0)
                        printf("%d, ", start + 1);
                    else
                        printf("%d-%d, ", start + 1, end + 1);
                }
            }
            printf("\b\b \n");
        }

        if(verbose)
            printf("Echo reply from IP: %s\n", inet_ntoa(ip->ip_src));

        if(bitset)
            free(bitset);


    }        // end receive()

private:
    /* data */
    int send_fd;
    int recv_fd;
    int icmp_fd;
};
