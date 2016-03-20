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
        co_op_udp_detector(int test_id_in, std::string dest_ip, uint8_t tos, uint16_t id, uint16_t frag_off, uint8_t ttl, uint8_t proto,
                           uint16_t check_sum, uint32_t sport, uint32_t dport, std::string filename = "/dev/urandom",
                           uint16_t num_packets = 1000, uint16_t data_length = 512, uint16_t num_tail = 20,
                           uint16_t tail_wait = 10, raw_level raw_status = none,
                           transport_type trans_proto = transport_type::udp)
            : udp_detector(test_id_in, dest_ip, tos, id, frag_off, ttl, proto, check_sum, sport, dport, filename, num_packets,
                           data_length, num_tail, tail_wait, raw_status, trans_proto)
        {
        }

        virtual ~co_op_udp_detector();

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
            recv_fd = socket(res->ai_family, SOCK_STREAM, IPPROTO_TCP);

            if(send_fd == -1)
            {
                perror("call to socket() failed for SEND");
                exit(EXIT_FAILURE);
            }        // end error check
        }

        virtual void send_timestamp()
        {
            // send message to server indicating how many packet to expect,
            // the packet size, and other parameters, then send the data train
            int err = connect(recv_fd, res->ai_addr, res->ai_addrlen);
            if(err == -1)
            {
                std::cerr << "Connect failed: " << errno;
                exit(EXIT_FAILURE);
            }

            //TODO: Rework these default parameters and get them from elsewhere.
            test_params p  = {};
            p.test_id = test_id;
            p.last_train = true;//TODO: this needs to be redesigned
            p.test_id = 12345; //TODO: get the id from MYSQL
            p.num_packets  = num_packets;
            p.payload_size = payload_size;
            p.port         = dport;
            p.offset = 0;        // TODO: change underlying classes to write packet id to random location in payload --
                                 // chosen at program start

            int n = sendto(recv_fd, &p, sizeof(p), 0, res->ai_addr, res->ai_addrlen);
            if(n == -1)
            {
                perror("Call to sendto() failed: error sending ICMP packet");
                exit(EXIT_FAILURE);
            }

        }        // end send_timstamp()

        virtual void send_train()
        {
            // send the train as normal

        }        // end send_train()

        virtual void send_tail()
        {
            bool done = true;
            int n     = sendto(recv_fd, &done, sizeof(done), 0, res->ai_addr, res->ai_addrlen);
            if(n == -1)
            {
                perror("Call to sendto() failed: error sending ICMP packet");
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

            recv(recv_fd, &t, sizeof(t), 0);

            // bitset s = t.losses;
            this->packets_lost = t.lostpackets;
            milliseconds       = t.elapsed_time;
            close(recv_fd);
        }        // end receive()


    private:
        /* data */
    };
}


#endif /* ifndef CO_OP_UDP_DETECTOR_HPP */
