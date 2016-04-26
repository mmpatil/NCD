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

#include "base_udp_detector.hpp"
#include "co_op_data.hpp"


namespace detection
{


    /**
     * A co-operative UDP detector of delay discriminators
     *
     * Normal use requires 2 instances of the class to be used, with  very nearly identical parameters,
     * differing in one distinct way: IP header fields, UDP header fields, or by payload.
     * Both instances should craft a packet train that has the same number of packets, where each packet in the base
     * measurement is the same size as its counterpart in the discrimination measurement. The only difference between
     * the two trains should be in the protocol headers, or in the payload data. All other parameters should be
     * consistent between both packet trains.
     *
     * The measured delay between the base measurement and the discrimination measurement can be used to deduce the
     * precense of a delay discriminator along the transmission path.
     *
     * Once the discrimination has been identified, a traceroute like approach can be used to isolate the discriminating
     * node.
     *
     */
    class co_op_udp_detector : public base_udp_detector
    {
    public:
        /**
         * Constructor
         * @param test_id_in the UUID of the test
         * @param dest_ip string representation of the target IP address
         * @param tos the Type of Service field in the IP header
         * @param id the ID field in the IP header
         * @param tos the Type of Service field in the IP header
         * @param frag_off the fragmentation offset field in the IP header
         * @param ttl the Time to Live field in the IP header
         * @param proto the Protocol field in the IP header
         * @param check_sum the checksum field in the IP header
         * @param sport the source port field in the UDP header
         * @param dport the destination port field in the UDP header
         * @param filename the filename of the source file used to fill the data portion of the packets in the packet
         * train
         * @param num_packets the number of packets in the data train
         * @param data_length the size of the UDP payload
         * @param num_tail the number of tail ICMP messages to send for timestamping
         * @param tail_wait the time in milliseconds between tail ICMP messages
         * @param raw_status the level of raw sockets required -- requires different permissions and fills payloads
         * differently
         * @param trans_proto the transport protocol used
         */
        co_op_udp_detector(int test_id_in, std::string dest_ip, uint8_t tos, uint16_t id, uint16_t frag_off,
                           uint8_t ttl, uint8_t proto, uint16_t check_sum, uint32_t sport, uint32_t dport, bool last,
                           std::string filename = "/dev/urandom", uint16_t num_packets = 1000,
                           uint16_t data_length = 512, uint16_t num_tail = 20, uint16_t tail_wait = 10,
                           raw_level raw_status = none, transport_type trans_proto = transport_type::udp)
            : base_udp_detector(test_id_in, dest_ip, tos, id, frag_off, ttl, proto, check_sum, sport, dport, filename,
                                num_packets, data_length, num_tail, tail_wait, raw_status, trans_proto),
              last_train(last)
        {
            tcp_res = NULL;
        }

        /**
         * Destructor
         */
        virtual ~co_op_udp_detector()
        {
            if(tcp_res)
                freeaddrinfo(tcp_res);
        }

        /**
         * The detection phase of the program, sends the data train and timestamps the measurement
         */
        inline virtual void detect() override
        {
            //     std::lock_guard<std::mutex> lk(recv_ready_mutex);
            prepare();
            send_timestamp();
            sleep(2);
            send_train();
            send_tail();
            //            std::cout << "tail Sent ..\n";
            recv_ready = true;
            //       recv_ready_cv.notify_all();
        }        // end detect()

        /**
         * Runs the program by calling detect followed by receive() -- single threaded
         */
        virtual void run() override
        {
            detect();
            receive();
        }

        /**
         * sets up Sockets needed to send and recive UDP messages
         */
        virtual void setup_sockets() override
        {
            int proto;
            if(raw == raw_level::none)
                proto = SOCK_DGRAM;
            else
                proto = SOCK_RAW;

            send_fd = socket(res->ai_family, proto, IPPROTO_UDP);
            if(send_fd == -1)
            {
                perror("call to socket() failed for SEND");
                exit(EXIT_FAILURE);
            }        // end error check

            sockaddr_in sin = {};
            sin.sin_family = AF_INET;
            sin.sin_port = htons(sport);

           if ( bind(send_fd, (sockaddr*)&sin, sizeof(sin)) < 0)
           {
               perror("Could not bind socket");
               exit(EXIT_FAILURE);
           }


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


        /**
         * Sends the test parameters to the remote host, and initiates timestamping
         */
        virtual void send_timestamp() override
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
            socklen_t sa_len     = sizeof(srcaddrs);
            if(getsockname(recv_fd, (struct sockaddr*)&srcaddrs, &sa_len) == -1)
            {
                perror("getsockname() failed");
                exit(EXIT_FAILURE);
            }
            char str[32] = {};

            auto p = setup_test_params();

            inet_ntop(AF_INET, &(srcaddrs.sin_addr), str, INET_ADDRSTRLEN);
            src_ip = str;
            char param_buffer[12] = {};
            p.serialize(param_buffer);
            //            std::cout << "The actual params: " << p <<std::endl;

            int n = send(recv_fd, param_buffer, sizeof(p), 0);
            /*int n = send(recv_fd, ss.str().data(), ss.str().size(), 0);*/
            if(n == -1)
            {
                perror("Call to send() failed: error with TCP connection");
                exit(EXIT_FAILURE);
            }

            //           std::cout << "Serialized params: " << *((test_params*)param_buffer) <<std::endl;

        }        // end send_timstamp()

        virtual test_params setup_test_params()
        {
            // TODO: change underlying classes to write packet id to random location in payload -- chosen at program
            test_params p  = {};
            p.test_id      = test_id;
            p.last_train   = last_train;
            p.num_packets  = num_packets;
            p.payload_size = payload_size;
            p.port         = dport;
            p.offset       = 0;
            return p;
        }


        /**
         * Sends a TCP message indicating that the last packet has been sent. contains 1 nonzero octet
         */
        virtual void send_tail() override
        {
            uint16_t done = (uint16_t) true;
            int n         = send(recv_fd, &done, sizeof(done), 0);
            if(n == -1)
            {
                perror("Call to send() failed: error with TCP connection");
                exit(EXIT_FAILURE);
            }
        }


        virtual void prepare() override
        {
            // take care of any setup
        }        // end prepare()

        /**
         * receives the test results back from the remote host
         */
        virtual void receive() override
        {
            // receive the tcp reply from the server containing the test results.
            test_results t = {};

            //            std::unique_lock<std::mutex> lk(recv_ready_mutex);
            //          recv_ready_cv.wait(lk, [this]() { return this->recv_ready; });
            //        lk.release();

            char results_buff[sizeof(t)];
            recv(recv_fd, results_buff, sizeof(t), 0);

            t.deserialize(results_buff);
            // bitset s = t.losses;
            this->packets_lost = t.lostpackets;
            milliseconds       = t.elapsed_time;
            pcap_id            = t.pcap_id;
            // t.success;
            close(recv_fd);
        }        // end receive()

        /**
         * Tests the serialization of the test_params datastructure
         *
         * @param p The params to serialize
         *
         * @return true = Success or false = failure
         */
        bool testSerialization(test_params p)
        {
            char buff[12] = {};
            p.serialize(buff);
            test_params q = {};
            q.deserialize(buff);
            if(p.num_packets != q.num_packets)
                return false;
            if(p.offset != q.offset)
                return false;
            if(p.test_id != q.test_id)
                return false;
            if(p.port != q.port)
                return false;
            if(p.payload_size != q.payload_size)
                return false;
            if(p.last_train != q.last_train)
                return false;
            return true;
        }

    private:
        /* data */
        addrinfo* tcp_res;
        bool last_train;
    };
}


#endif /* ifndef CO_OP_UDP_DETECTOR_HPP */
