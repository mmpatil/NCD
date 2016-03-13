
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
 * @file: detector.hpp
 */


#ifndef DETECTOR_HPP_
#define DETECTOR_HPP_

/* Linux header files */
#include <arpa/inet.h>  /* for inet_pton() */
#include <netdb.h>      /* for getaddrinfo() */
#include <sys/socket.h> /* for socket(), setsockopt(), etc...*/
#include <unistd.h>     /* for close() */

/* STL Header files */
#include <condition_variable> /* for condition variables */
#include <cstdint>            /* for fixed size integers */
#include <cstring>            /* for memset */
#include <fstream>            /* for fstream -- writing output */
#include <iostream>           /* for standard io */
#include <memory>             /* for std::shared_ptr */
#include <mutex>              /* for std::mutex */
#include <sstream>            /* for std::stringstream */
#include <stdexcept>          /* for std::exception */
#include <string>             /* for std::string */
#include <thread>             /* for std::thread */
#include <vector>             /* for std:vector*/
#include <algorithm>          /* for std::transform */

/* project header files */
#include "ip_checksum.h"
#include "ip_icmp_packet.hpp"
#include "ip_tcp_packet.hpp"
#include "ip_udp_packet.hpp"
#include "simple_bitset.h"

namespace detection
{
    typedef std::vector<std::shared_ptr<packet>> packet_buffer_t;

    enum raw_level
    {
        none,
        transport_only,
        full
    };

    std::istream& operator>>(std::istream& in, raw_level& val)
    {
        std::string token;

        in >> token;
        std::transform(token.begin(), token.end(), token.begin(), ::tolower);

        if(token == "full")
        {
            val = full;
        }
        else if(token == "transport_only")
        {
            val = transport_only;
        }
        else
        {
            val = none;
        }
        return in;
    }

    std::ostream& operator<<(std::ostream& out, const raw_level& val)
    {
        if(val == full)
        {
           return out << "full";
        }
        else if(val == transport_only)
        {
            return out << "transport_only";
        }
        else
        {
            return out << "none";
        }
    }


    /**
     * new class to manage all resources used during  discrimination detection
     * only responsible for a single train
     */
    class detector
    {
    public:
        detector(std::string dest_ip, uint8_t tos, uint16_t ip_length, uint16_t id,
                 uint16_t frag_off, uint8_t ttl, uint8_t proto, uint16_t check_sum, uint32_t sport, uint32_t dport,
                 std::string filename = "/dev/urandom", uint16_t num_packets = 10, uint16_t data_length = 512,
                 uint16_t num_tail = 20, uint16_t tail_wait = 10, raw_level raw_status = none,
                 transport_type trans_proto = transport_type::udp)
            : dest_ip(dest_ip),
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
              sockets_ready(false)
        {
            verbose = true;
            // get file stream to use in packet initialization;
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
            res = nullptr;
        }

        virtual void setup_sockets() = 0;        // pure virtual
        virtual void setup_packet_train()
        {
            uint8_t proto = trans == transport_type::udp ? IPPROTO_UDP : IPPROTO_TCP;
            // populate the empty data_train based on raw and transport type
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
            ps = {ip_header.saddr, ip_header.daddr, 0, proto,
                  htons((uint16_t)(payload_size + transport_header_size() + sizeof(pseudo_header)))};

            uint16_t packet_id = 0;
            for(auto& item : data_train)
            {
                item->fill(file, packet_id++);
                if(raw == transport_only)
                {
                    item->checksum(ps);
                }
            }
        }

        virtual void populate_full()  = 0;        // pure virtual
        virtual void populate_trans() = 0;        // pure virtual
        virtual void populate_none()  = 0;        // pure virtual
        virtual void send_train()     = 0;        // sends the packet train -- pure virtual;
        virtual void receive()        = 0;        // receives responses from the target IP -- pure virtual
        virtual void send_timestamp() = 0;        // sends time stamping packets must send inital packets, can be reused
        virtual void send_tail()      = 0;        // sends the tail set of time stamping packets
        virtual int transport_header_size() = 0;        // returns size of transport header -- pure virtual

        inline virtual void detect()
        {
            prepare();
            send_timestamp();
            send_train();
            send_tail();
        }        // end detect()

        virtual void prepare(){};

        virtual void measure()
        {
            if(!sockets_ready)
            {
                setup_sockets();
                sockets_ready = true;
            }
            // initialize synchronization variables
            stop       = false;        // boolean false
            recv_ready = false;        // boolean false

            std::vector<std::thread> threads;
            threads.emplace_back(&detector::receive, this);
            threads.emplace_back(&detector::detect, this);

            for(auto& t : threads)
            {
                t.join();
            }        // end for

            if(!sql_output)
                printf("%f sec\n", milliseconds);        // are these unit correct now???
            close(recv_fd);

        }        // end measure()

        // stay the same
        virtual void output_results()
        {

            std::stringstream out;
            switch(trans)
            {
            case transport_type::udp:
                out << "UDP";
                break;
            case transport_type::tcp:
                out << "TCP";
            default:
                break;
            };

            out << src_ip << " " << dest_ip << " " << sport << " " << dport << " " << num_packets << " " << num_tail
                << " " << payload_size << " " << tail_wait << " " << packets_lost << " " << milliseconds << std::endl;

            std::cout << out.str();
        }

        virtual void setup_ip_info()
        {
            // set leading bits for version and IP header length -- can change later;
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
        std::string src_ip;          // string with IP address
        std::string dest_ip;         // string with IP address
        transport_type trans;        // enum containing the transport type
        iphdr ip_header;             // IP header struct -- holds all the values
        // tcphdr tcp_header;                   // transport layer header --- should probably get rid of this
        // udphdr udp_header;                   // transport layer header --- should probably get rid of this
        uint16_t sport;          // transport layer source port -- should this be here?
        uint16_t dport;          // transport layer source port -- should this be here?
        bool verbose;            // verbose output
        bool sql_output;         // output minimal information for our SQL data structures
        int packets_lost;        // number of packets lost -- for loss based approach
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

}        // end namespace detector

#endif        // end detector.hpp
