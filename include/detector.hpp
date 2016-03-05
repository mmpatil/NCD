
/*
  The MIT License

  Copyright (c) 2015-2016 Paul Kirth pk1574@gmail.com

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

/**
 * @author: Paul Kirth
 * @file: ncd.c
 */


#ifndef DETECTOR_HPP_
#define DETECTOR_HPP_


//#include <stdio.h>           /* for printf, fprintf, snprintf, perror, ... */
//#include <stdlib.h>          /* for EXIT_SUCCESS, EXIT_FAILURE, */
//#include <string.h>          /* for memcpy */
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
#include <thread>
#include <mutex>
#include <condition_variable>


#include "ip_checksum.h"
#include "bitset.h"
#include "packet.hpp"
#include "udp_packet.hpp"
#include "tcp_packet.hpp"
#include "icmp_packet.hpp"
#include "ip_tcp_packet.hpp"
#include "ip_udp_packet.hpp"
#include "ip_icmp_packet.hpp"

namespace detection {



    typedef std::vector<std::shared_ptr<packet>> packet_buffer_t;

    enum raw_level {
        none,
        transport_only,
        full
    };

/**
 * new class to manage all resources used during  discrimination detection
 * only responsible for a single train
 */
    class detector {
    public:
        detector(std::string src_ip, std::string dest_ip, uint8_t tos, uint16_t ip_length, uint16_t id,
                 uint16_t frag_off,
                 uint8_t ttl, uint8_t proto, uint16_t check_sum, uint32_t sport, uint32_t dport,
                 std::string filename = "/dev/urandom", uint16_t num_packets = 10, uint16_t data_length = 512,
                 uint16_t num_tail = 20, uint16_t tail_wait = 10, raw_level raw_status = none,
                 transport_type trans_proto = transport_type::udp)
                : src_ip(src_ip),
                  dest_ip(dest_ip),
                  trans(trans_proto),
                  ip_header{0, 0, tos, ip_length, id, frag_off, ttl, proto, check_sum, 0, 0},
                  sport(sport),
                  dport(dport),
                  res(nullptr),
                  payload_size(data_length),
                  num_packets(num_packets),
                  num_tail(num_tail),
                  tail_wait(tail_wait),
                  file(filename, std::ios::in | std::ios::binary),
                  raw(raw_status),
                  milliseconds(0),
                  elapsed{},
                  sockets_ready(false) {
            verbose = true;
            // get file stream to use in packet initialization;
            if (!file.is_open()) {
                std::string err_string = "Error: file " + filename + " could not be opened";
                std::ios_base::failure e(err_string);
                std::cerr << err_string << std::endl;
                throw e;
            }

            // setup the IP info ...
            setup_ip_info();
        }

        virtual ~detector() {
            if (res)
                freeaddrinfo(res);
            res = nullptr;
        }

        virtual void setup_sockets() = 0;        // pure virtual
        virtual void setup_packet_train() {
            uint8_t proto = trans == transport_type::udp ? IPPROTO_UDP : IPPROTO_TCP;
            // populate the empty data_train based on raw and transport type
            switch (raw) {
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
            ps = {ip_header.saddr, ip_header.daddr, 0, proto,
                  htons((uint16_t) (payload_size + transport_header_size() + sizeof(pseudo_header)))};

            uint16_t packet_id = 0;
            for (auto &item : data_train) {
                item->fill(file, packet_id++);
                if (raw == transport_only) {
                    item->checksum(ps);
                }
            }
        }

        virtual void populate_full() = 0;         // pure virtual
        virtual void populate_trans() = 0;        // pure virtual
        virtual void populate_none() = 0;         // pure virtual
        virtual void send_train() = 0;            // sends the packet train -- pure virtual;
        virtual void receive() = 0;               // receives responses from the target IP -- pure virtual
        virtual void send_timestamp() = 0;        // sends time stamping packets must send inital packets, can be reused
        virtual void send_tail() = 0;             // sends the tail set of time stamping packets
        virtual int transport_header_size() = 0;        // returns size of transport header -- pure virtual

        inline virtual void detect() {
            prepare();
            send_timestamp();
            send_train();
            send_tail();
        }        // end detect()

        virtual void prepare() { };

        virtual void measure() {
            if (!sockets_ready) {
                setup_sockets();
                sockets_ready = true;
            }
            // initialize synchronization variables
            stop = false;        // boolean false
            recv_ready = false;        // boolean false

            std::vector<std::thread> threads;
            threads.emplace_back(&detector::receive, this);
            threads.emplace_back(&detector::detect, this);

            for (auto &t : threads) {
                t.join();
            }        // end for

            if (!sql_output)
                printf("%f sec\n", milliseconds);        // are these unit correct now???
            close(recv_fd);

        }        // end measure()

        // stay the same
        virtual void output_results() {

            std::stringstream out;
            switch (trans) {
                case transport_type::udp:
                    out << "UDP";
                    break;
                case transport_type::tcp:
                    out << "TCP";
                default:
                    break;
            };

            out << src_ip << " " << dest_ip << " " << sport << " " << dport << " " << num_packets << " " << num_tail <<
            " "
            << payload_size << " " << tail_wait << " " << packets_lost << " " << milliseconds << std::endl;

            std::cout << out.str();
        }

        virtual void setup_ip_info() {
            // set leading bits for version and IP header length -- can change later;
            ip_header.version = 4;
            ip_header.ihl = 5;

            /* set up hints for getaddrinfo() */
            addrinfo hints = {}; /* for get addrinfo */
            hints.ai_flags = AI_CANONNAME;


            // choose the correct protocol
            switch (trans) {
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
            if (err) {
                if (err == EAI_SYSTEM)
                    std::cerr << "Error looking up " << dest_ip << ":" << errno << std::endl;
                else
                    std::cerr << "Error looking up " << dest_ip << ":" << gai_strerror(err) << std::endl;
                exit(EXIT_FAILURE);
            }

            // set destination address
            ip_header.daddr = ((struct sockaddr_in *) res->ai_addr)->sin_addr.s_addr;

            // get temp socket to obtain source IP -- its a hack
            {
                struct sockaddr_in srcaddrs = {};                      // source IP address
                socklen_t sa_len = sizeof(srcaddrs);        // size of src address

                int temp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
                if (connect(temp_sock, res->ai_addr, res->ai_addrlen) == -1) {
                    std::cerr << "Connect failed: " << errno;
                    exit(EXIT_FAILURE);
                }
                if (getsockname(temp_sock, (struct sockaddr *) &srcaddrs, &sa_len) == -1) {
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
        //tcphdr tcp_header;                   // transport layer header --- should probably get rid of this
        //udphdr udp_header;                   // transport layer header --- should probably get rid of this
        uint16_t sport;                      // transport layer source port -- should this be here?
        uint16_t dport;                      // transport layer source port -- should this be here?
        bool verbose;                        // verbose output
        bool sql_output;                     // output minimal information for our SQL data structures
        int packets_lost;                    // number of packets lost -- for loss based approach
        addrinfo *res;
        packet_buffer_t data_train;

        // file descriptors
        int send_fd;
        int recv_fd;
        // int other _fd; // not sure we should use this????

        // cli arg
        uint16_t payload_size;
        uint16_t num_packets;
        uint16_t num_tail;
        u_int16_t tail_wait;

        // threading items
        bool recv_ready;        // bool for receiving SYN packets -- denotes if the program is ready to receive traffic
        bool stop;              // boolean for if the send thread can stop (receive thread has received second response.
        std::mutex stop_mutex;                        // mutex for stop
        std::mutex recv_ready_mutex;                  // mutex for recv_ready
        std::condition_variable stop_cv;              // condition variable for stop -- denotes
        std::condition_variable recv_ready_cv;        // condition variable for recv_ready mutex


        // internal data
        std::ifstream file;        // file to read payload in from -- could also be file with entire train pre made
        raw_level raw;
        // time
        double milliseconds;
        timeval elapsed;
        bool sockets_ready;
        pseudo_header ps;
        std::array<char, 1500> buff;
    };



    class tcp_detector : public detector {

    public:
        tcp_detector(std::string src_ip, std::string dest_ip, uint8_t tos, uint16_t id, uint16_t frag_off, uint8_t ttl,
                     uint8_t proto, uint16_t check_sum, uint32_t sport, uint32_t dport,
                     std::string filename = "/dev/urandom", uint16_t num_packets = 1000, uint16_t data_length = 512,
                     uint16_t num_tail = 20, uint16_t tail_wait = 10, raw_level raw_status = full,
                     transport_type trans_proto = transport_type::tcp, uint16_t syn_port_in = 22223)
                : detector(src_ip, dest_ip, tos, data_length + sizeof(tcphdr) + sizeof(iphdr), id, frag_off, ttl, proto,
                           check_sum, sport, dport, filename, num_packets, data_length, num_tail, tail_wait, raw_status,
                           trans_proto),
                  syn_port(syn_port_in) {

            tcp_header.source = sport;
            tcp_header.dest = dport;
            tcp_header.doff = 5;
            //tcp_header.source = sport;
            setup_packet_train();
        }

        virtual void fix_data_train() {
            uint32_t num = 0;
            for (auto &item : data_train) {
                tcphdr *tcp = (tcphdr *) (item->data.data() + sizeof(iphdr));        //->data[trans_offset];
                num += item->data.size() - sizeof(iphdr);
                tcp->seq = htonl(num);
            }
        }

        virtual void populate_full() {
            data_train.reserve(num_packets);
            for (int i = 0; i < num_packets; ++i)
                data_train.push_back(
                        std::make_shared<ip_tcp_packet>(ip_header, payload_size, sport, dport, tcp_header.seq,
                                                        tcp_header.ack_seq, (1 << 15) - 1, tcp_header.urg_ptr,
                                                        1, 0, 5));

            fix_data_train();
        }

        virtual void populate_trans() {
            data_train.reserve(num_packets);
            for (int i = 0; i < num_packets; ++i)
                data_train.push_back(std::make_shared<tcp_packet>(
                        payload_size, sport, dport, tcp_header.seq, tcp_header.ack_seq, tcp_header.window,
                        tcp_header.urg_ptr,
                        (uint16_t) tcp_header.ack, (uint16_t) tcp_header.syn, (uint16_t) tcp_header.doff));

            fix_data_train();
        }

        virtual void populate_none() {
            data_train.reserve(num_packets);
            for (int i = 0; i < num_packets; ++i)
                data_train.push_back(std::make_shared<packet>(payload_size));
        };

        virtual int transport_header_size() { return sizeof(tcphdr); }

        virtual void setup_sockets() {

            /*get root privileges */
            int err = setuid(0);
            if (err < 0) {
                perror("Elevated privileges not acquired...");
                exit(EXIT_FAILURE);
            }

            send_fd = socket(res->ai_family, SOCK_RAW, IPPROTO_TCP);

            if (send_fd == -1) {
                perror("call to socket() failed for SEND");
                exit(EXIT_FAILURE);
            }        // end error check

            if (res->ai_family != AF_INET) {
                errno = EAFNOSUPPORT;
                perror("ncd only supports IPV4 at this time");
                exit(EXIT_FAILURE);
            }        // end error check

            // set TTL
            setsockopt(send_fd, IPPROTO_IP, IP_TTL, &ip_header.ttl, sizeof(ip_header.ttl));

            socklen_t size = 1500U * num_packets;

#if DEBUG
            if(verbose)
                printf("Buffer size requested %u\n", size);
#endif

            setsockopt(send_fd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));

            if (raw == full) {
                /* set up our own IP header*/
                int tcp_hdrincl = 1;
                if (setsockopt(send_fd, IPPROTO_IP, IP_HDRINCL, &tcp_hdrincl, sizeof(tcp_hdrincl)) == -1) {
                    perror("setsockopt() failed icmp");
                }
            }

            /*give up privileges */
            err = setuid(getuid());
            if (err < 0) {
                perror("Elevated privileges not released");
                exit(EXIT_FAILURE);
            }

            recv_fd = socket(res->ai_family, SOCK_RAW, IPPROTO_TCP);

            if (recv_fd == -1) {
                perror("call to socket() failed");
                exit(EXIT_FAILURE);
            }

            /* give up root privileges */
            err = setuid(getuid());
            if (err < 0) {
                perror("Elevated privileges not released...");
                exit(EXIT_FAILURE);
            }

            /*increase size of receive buffer*/

            int opts = 1500 * num_packets;


            setsockopt(recv_fd, SOL_SOCKET, SO_RCVBUF, &opts, sizeof(opts));

#if DEBUG
            if(verbose)
            {
                int buffsize;
                socklen_t bufflen = sizeof(buffsize);

                getsockopt(send_fd, SOL_SOCKET, SO_SNDBUF, (void*)&buffsize, &bufflen);

                printf("Send Buffer size: %d\n", buffsize);
                getsockopt(recv_fd, SOL_SOCKET, SO_RCVBUF, (void*)&buffsize, &bufflen);
                printf("Receive Buffer size: %d\n", buffsize);
            }
#endif
        }

        virtual void setup_syn_packets() {
            pseudo_header syn_ps = {};
            syn_ps.source = ip_header.saddr;
            syn_ps.dest = ip_header.daddr;
            syn_ps.zero = 0;
            syn_ps.len = htons(sizeof(tcphdr));
            syn_ps.proto = IPPROTO_TCP;

            syn_packet_1.reset(new ip_tcp_packet(ip_header, 0, sport, dport, 0, 0, (1 << 15) - 1, 0, false, true, 5));
            syn_packet_1->checksum(syn_ps);

            syn_packet_2.reset(
                    new ip_tcp_packet(ip_header, 0, sport + 1, syn_port, 0, 0, (1 << 15) - 1, 0, false, true, 5));
            syn_packet_2->checksum(syn_ps);
        }

        virtual void prepare() { setup_syn_packets(); }


        virtual void send_timestamp() {
            int n;

            // set up the buffer to receive the reply into
            struct ip *ip = (struct ip *) buff.data();
            struct tcphdr *tcp_reply = (struct tcphdr *) (ip + 1);

            n = sendto(send_fd, syn_packet_1->data.data(), syn_packet_1->data.size(), 0, res->ai_addr, res->ai_addrlen);
            if (n == -1) {
                perror("Call to sendto() failed: tcp syn");
                exit(EXIT_FAILURE);
            }

            do {
                if ((recvfrom(send_fd, buff.data(), buff.size(), 0, 0, 0)) == -1) {
                    perror("call to recvfrom() failed: tcp SYN-ACK");
                    exit(EXIT_FAILURE);
                }

            } while ((tcp_reply->dest != htons(sport)) || (ip_header.saddr != ip->ip_src.s_addr));


            if (verbose) {
                printf("TCP SYN reply from IP: %s\n", inet_ntoa(ip->ip_src));
                printf("TCP SYN reply from port: %d to port: %d\n", ntohs(tcp_reply->source), ntohs(tcp_reply->dest));
            }

            milliseconds = get_time();        // time stamp just before we begin sending
        }


        virtual void send_train() {
            int n;
            // uint32_t ack_seq = ntohl(tcp_reply->seq);
            // tcphdr* tcp;
            //        uint32_t uniform_data_len = sizeof(tcphdr) + payload_size;
            /*send data train*/
            for (auto item : data_train) {
                //         tcp      = (tcphdr*)&item->data[((ip_tcp_packet*)item.get())->transport_offset];
                //          tcp->ack_seq = htonl(ack_seq);// += uniform_data_len);
                //           item->checksum(ps);

                n = sendto(send_fd, item->data.data(), item->data.size(), 0, res->ai_addr, res->ai_addrlen);
                if (n == -1) {
                    perror("call to sendto() failed: error sending TCP train");
                    exit(EXIT_FAILURE);
                }
            }

            {
                std::lock_guard<std::mutex> recv_guard(recv_ready_mutex);        // acquire lock
                recv_ready = true;
            }

            recv_ready_cv.notify_all();
        }

        virtual void send_tail() {
            int n;
            std::unique_lock<std::mutex> stop_lock(stop_mutex);        // acquire lock
            for (int i = 0; i < num_tail && !stop; ++i) {
                n = sendto(send_fd, syn_packet_2->data.data(), syn_packet_2->data.size(), 0, res->ai_addr,
                           res->ai_addrlen);
                if (n == -1) {
                    perror("Call to sendto() failed: TCP Tail Syn");
                    exit(EXIT_FAILURE);
                }

                stop_cv.wait_for(stop_lock, std::chrono::milliseconds(tail_wait));
            }        // end for

            stop_lock.unlock();        // release lock
        }

        virtual void receive() {

            /*number of bytes received*/
            int n;

            /* to receive data with*/
            struct sockaddr_in addr;

            socklen_t adrlen = sizeof(addr);

            buff.fill(0);

            std::unique_lock<std::mutex> recv_ready_lock(recv_ready_mutex);
            // while(!recv_ready)
            //{
            recv_ready_cv.wait(recv_ready_lock, [this]() {
                return this->recv_ready;
            });
            //}

            struct ip *ip = (struct ip *) buff.data();
            struct tcphdr *tcp = (struct tcphdr *) (ip + 1);
            do {
                n = recvfrom(send_fd, buff.data(), buff.size(), 0, (struct sockaddr *) &addr, &adrlen);
                if (n < 0) {
                    perror("recvfrom() failed");
                    exit(EXIT_FAILURE);
                }

            } while ((tcp->dest != htons(syn_port)) || (ip->ip_src.s_addr != ip_header.saddr));
            milliseconds = get_time() - milliseconds;

            {
                std::lock_guard<std::mutex> stop_guard(stop_mutex);
                stop = true;
            }        // release lock guard

            stop_cv.notify_all();

            if (verbose) {
                printf("TCP reply from IP: %s\n", inet_ntoa(ip->ip_src));

                printf("TCP reply from port: %d to port: %d\n", ntohs(tcp->source), ntohs(tcp->dest));
            }

        }        // end receive()


    private:
        /* data */
        int icmp_fd;
        uint16_t syn_port;
        std::unique_ptr<ip_tcp_packet> syn_packet_1;
        std::unique_ptr<ip_tcp_packet> syn_packet_2;
        tcphdr tcp_header;
    };
}// end namespace detector

#endif // end detector.hpp
