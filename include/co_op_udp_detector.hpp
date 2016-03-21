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
 * @file: udp_detector.hpp
 */

#ifndef CO_OP_UDP_DETECTOR_HPP
#define CO_OP_UDP_DETECTOR_HPP 1

#include "co_op_data.hpp"
#include "udp_detector.hpp"


namespace detection
{

    class co_op_udp_detector : public udp_detector
    {
    public:
        co_op_udp_detector(int test_id_in, std::string dest_ip, uint8_t tos, uint16_t id, uint16_t frag_off,
                           uint8_t ttl, uint8_t proto, uint16_t check_sum, uint32_t sport, uint32_t dport, bool last,
                           std::string filename = "/dev/urandom", uint16_t num_packets = 1000,
                           uint16_t data_length = 512, uint16_t num_tail = 20, uint16_t tail_wait = 10,
                           raw_level raw_status = none, transport_type trans_proto = transport_type::udp)
            : udp_detector(test_id_in, dest_ip, tos, id, frag_off, ttl, proto, check_sum, sport, dport, filename,
                           num_packets, data_length, num_tail, tail_wait, raw_status, trans_proto),
              last_train(last)
        {
            tcp_res = NULL;
        }

        virtual ~co_op_udp_detector()
        {
            if(tcp_res)
                freeaddrinfo(tcp_res);
        }

        inline virtual void detect()
        {
            std::lock_guard<std::mutex> lk(recv_ready_mutex);
            prepare();
            send_timestamp();
            send_train();
            send_tail();
            recv_ready = true;
            recv_ready_cv.notify_all();
        }        // end detect()

        virtual void setup_sockets()
        {
            send_fd = socket(res->ai_family, SOCK_DGRAM, IPPROTO_UDP);
            if(send_fd == -1)
            {
                perror("call to socket() failed for SEND");
                exit(EXIT_FAILURE);
            }        // end error check

            if(res->ai_family != AF_INET)
            {
                errno = EAFNOSUPPORT;
                perror("Detector only supports IPV4 at this time");
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

            // set up TCP socket
            /* set up hints for getaddrinfo() */
            addrinfo tcp_hints    = {}; /* for get addrinfo */
            tcp_hints.ai_flags    = AI_CANONNAME;
            tcp_hints.ai_protocol = IPPROTO_TCP;
            std::stringstream ss;
            ss << 15555;

            int err = getaddrinfo(dest_ip.c_str(), ss.str().c_str(), &tcp_hints, &tcp_res);
            if(err)
            {
                if(err == EAI_SYSTEM)
                    std::cerr << "Error looking up " << dest_ip << ":" << errno << std::endl;
                else
                    std::cerr << "Error looking up " << dest_ip << ":" << gai_strerror(err) << std::endl;
                exit(EXIT_FAILURE);
            }


            recv_fd = socket(tcp_res->ai_family, SOCK_STREAM, IPPROTO_TCP);

            if(recv_fd == -1)
            {
                perror("call to socket() failed for Recv");
                exit(EXIT_FAILURE);
            }        // end error check
        }

        virtual void send_timestamp()
        {
            // send message to server indicating how many packet to expect,
            // the packet size, and other parameters, then send the data train


            int err = connect(recv_fd, tcp_res->ai_addr, tcp_res->ai_addrlen);
            if(err == -1)
            {
                std::cerr << "Connect failed: " << errno << std::endl;
                perror("help:");
                exit(EXIT_FAILURE);
            }
            sockaddr_in srcaddrs = {};
            socklen_t sa_len = sizeof(srcaddrs);
            if(getsockname(recv_fd, (struct sockaddr*)&srcaddrs, &sa_len) == -1)
            {
                perror("getsockname() failed");
                exit(EXIT_FAILURE);
            }
            char str[32] = {};

            inet_ntop(AF_INET, &(srcaddrs.sin_addr), str, INET_ADDRSTRLEN);
            src_ip = str;

            // TODO: change underlying classes to write packet id to random location in payload -- chosen at program
            // start
            test_params p  = {};
            p.test_id      = test_id;
            p.last_train   = last_train;
            p.num_packets  = num_packets;
            p.payload_size = payload_size;
            p.port         = dport;
            p.offset       = 0;

            int n = send(recv_fd, &p, sizeof(p), 0);
            if(n == -1)
            {
                perror("Call to send() failed: error with TCP connection");
                exit(EXIT_FAILURE);
            }

        }        // end send_timstamp()

        virtual void send_tail()
        {
            bool done = true;
            int n     = send(recv_fd, &done, sizeof(done), 0);
            if(n == -1)
            {
                perror("Call to send() failed: error with TCP connection");
                exit(EXIT_FAILURE);
            }
        }

        virtual void prepare()
        {
            // take care of any setup
        }        // end prepare()

        virtual void receive()
        {
            // receive the tcp reply from the server containing the test results.
            test_results t = {};

            std::unique_lock<std::mutex> lk(recv_ready_mutex);
            recv_ready_cv.wait(lk, [this]() { return this->recv_ready; });
            lk.release();

            recv(recv_fd, &t, sizeof(t), 0);

            // bitset s = t.losses;
            this->packets_lost = t.lostpackets;
            milliseconds       = t.elapsed_time;
            pcap_id            = t.pcap_id;
            //t.success;
            close(recv_fd);
        }        // end receive()


    private:
        /* data */
        addrinfo* tcp_res;
        bool last_train;
    };
}


#endif /* ifndef CO_OP_UDP_DETECTOR_HPP */
