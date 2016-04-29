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

#ifndef DETECTOR_UDP_DETECTOR_HPP
#define DETECTOR_UDP_DETECTOR_HPP

#include "base_udp_detector.hpp"

namespace detection
{

    /**
     *
     * A non-cooperative delay discrimination detector. It uses a UDP packet train, with ICMP messages to detect delay
     * discrimination in single sided environments.
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
    class udp_detector : public base_udp_detector
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
        udp_detector(uint16_t test_id_in, std::string dest_ip, uint8_t tos, uint16_t id, uint16_t frag_off, uint8_t ttl,
                     uint8_t proto, uint16_t check_sum, uint16_t sport, uint16_t dport,
                     std::string filename = "/dev/urandom", uint16_t num_packets = 1000, uint16_t data_length = 512,
                     uint16_t num_tail = 20, uint16_t tail_wait = 10, raw_level raw_status = none,
                     transport_type trans_proto = transport_type::udp)
            : base_udp_detector(test_id_in, dest_ip, tos, id, frag_off, ttl, proto, check_sum, sport, dport, filename,
                                num_packets, data_length, num_tail, tail_wait, raw_status, trans_proto, false),
              icmp_send(ip_header, 64 - sizeof(udphdr), ICMP_ECHO, 0, (uint16_t)getpid(), (uint16_t)rand()),
              stop(false),
              recv_ready(false)
        {
            // setup_sockets();
        }

        inline virtual void detect() override
        {
            prepare();

            std::unique_lock<std::mutex> lk(recv_ready_mutex);
            recv_ready_cv.wait(lk, [this]() { return this->recv_ready; });
            lk.release();

            send_timestamp();
            send_train();
            send_tail();
        }        // end detect()
                 /**
                  * sets up the sockets used by the UDP detector. should be invoked prior to execution
                  *
                  * @method setup_sockets
                  */
        virtual void setup_sockets()
        {
            sockaddr_in srcaddrs;
            socklen_t sa_len;
            // get temp socket to obtain source IP -- its a hack
            {
                int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
                if(connect(s, res->ai_addr, res->ai_addrlen) == -1)
                {
                    perror("Connect failed");
                    exit(-1);
                }
                if(getsockname(s, (struct sockaddr*)&srcaddrs, &sa_len) == -1)
                {
                    perror("getsockname() failed");
                    exit(-1);
                }
                char str[32] = {};


                inet_ntop(AF_INET, &(srcaddrs.sin_addr), str, INET_ADDRSTRLEN);
                src_ip = str;

                close(s);
            }        // end temp socket

            /*get root privileges */
            int err = setuid(0);
            if(err < 0)
            {
                perror("Elevated privileges not acquired...");
                exit(EXIT_FAILURE);
            }

            /* acquire socket for icmp messages*/
            icmp_fd = socket(res->ai_family, SOCK_RAW, IPPROTO_ICMP);

            if(icmp_fd == -1)
            {
                perror("call to socket() failed for ICMP");
                exit(EXIT_FAILURE);
            }

            /* set up our own IP header*/
            int icmp_hdrincl = 1;
            if(setsockopt(icmp_fd, IPPROTO_IP, IP_HDRINCL, &icmp_hdrincl, sizeof(icmp_hdrincl)) == -1)
            {
                perror("setsockopt() failed icmp");
            }


            recv_fd = socket(res->ai_family, SOCK_RAW, IPPROTO_ICMP);
            if(recv_fd == -1)
            {
                perror("call to socket() failed");
                exit(EXIT_FAILURE);
            }

            /*increase size of receive buffer*/
            int opts = 1500 * num_packets;
            setsockopt(recv_fd, SOL_SOCKET, SO_RCVBUF, &opts, sizeof(opts));

#if DEBUG
            if(verbose)
                printf("Buffer size requested %u\n", opts);
#endif

            base_udp_detector::setup_sockets();

            /*give up privileges */
            err = setuid(getuid());
            if(err < 0)
            {
                perror("Elevated privileges not released");
                exit(EXIT_FAILURE);
            }

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

        /**
         * Sets up and the header and payload of the ICMP packets used as timestamps
         *
         * @method setup_icmp_packet
         */
        virtual void setup_icmp_packet()
        {
            /* size of ICMP Echo message */
            uint16_t icmp_data_len = (uint16_t)(icmp_packet_size - sizeof(struct icmp) - sizeof(uint16_t));

            iphdr icmp_ip_header(ip_header);
            icmp_ip_header.protocol = IPPROTO_ICMP;
            icmp_send =
              ip_icmp_packet(icmp_ip_header, icmp_data_len, ICMP_ECHO, 0, (uint16_t)getpid(), (uint16_t)rand());
        }

        /**
         * deploys receive() and detect() in their own respective threads and joins them;
         *
         * @method run
         */
        virtual void run()
        {
            std::vector<std::thread> threads;
            threads.emplace_back(&udp_detector::receive, this);
            threads.emplace_back(&udp_detector::detect, this);

            for(auto& t : threads)
            {
                t.join();
            }        // end for

        }        // end measure()

        /**
         * Sends the initial timestamp -- precedes the entire datatrain
         *
         * @method send_timestamp
         */
        inline virtual void send_timestamp()
        {
            /*send Head ICMP Packet*/
            int n = sendto(icmp_fd, icmp_send.data.data(), icmp_send.data.size(), 0, res->ai_addr, res->ai_addrlen);
            if(n == -1)
            {
                perror("Call to sendto() failed: error sending ICMP packet");
                exit(EXIT_FAILURE);
            }
#if DEBUG
            std::cout << "Timestamp Sent\n";
#endif
        }


        /**
         * Takes care of special preparation required before send_timestamp() is called
         *
         * @method prepare
         */
        virtual void prepare() { setup_icmp_packet(); }


        /**
         * Sends the ICMP tail packets to timestamp the end of the measurement
         *
         * @method send_tail
         */
        virtual void send_tail()
        {
            struct icmp* icmp = (struct icmp*)(icmp_send.data.data() + sizeof(struct ip));

            /*send tail ICMP Packets with timer*/

            std::unique_lock<std::mutex> stop_lock(stop_mutex);        // acquire lock
#if DEBUG
            std::cout << "Sending Tail ..." << std::endl;
#endif
            for(int i = 0; i < num_tail && !stop; ++i)
            {
                // get timestamp
                auto now = std::chrono::system_clock::now() + std::chrono::milliseconds(tail_wait);

                // do some work
                /*not sure if changing the sequence number will help*/
                icmp->icmp_cksum = 0;
                icmp->icmp_seq += 1;
                icmp->icmp_cksum = ip_checksum(icmp, icmp_packet_size);
                send_timestamp();

                // wait until we should send the next tail_message
                stop_cv.wait_for(stop_lock, std::chrono::milliseconds(tail_wait));
            }        // end for

            stop_lock.unlock();        // release lock

#if DEBUG
            std::cout << "Tail Sent" << std::endl;
#endif
        }

        /**
         * Recives ICMP replies from the target IP -- relies on the reliableness of ICMP echo responses
         *
         * @method receive
         */
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

            buff.fill(0);

            /* length of address */
            socklen_t adrlen = sizeof(addr);

            /*Receive initial ICMP echo response && Time-stamp*/
            struct ip* ip      = (struct ip*)buff.data();
            icmp               = (struct icmp*)(ip + 1);
            struct udphdr* udp = (struct udphdr*)(&(icmp->icmp_data) + sizeof(struct ip));
            uint32_t* bitset   = make_bs_32(num_packets);
            uint16_t* id       = (uint16_t*)(udp + 1);

            {
                std::lock_guard<std::mutex> lk(recv_ready_mutex);
                recv_ready = true;
                recv_ready_cv.notify_all();
            }

            for(;;)
            {

                n = recvfrom(recv_fd, buff.data(), buff.size(), 0, (struct sockaddr*)&addr, &adrlen);

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
                    set_bs_32(bitset, ntohs(*id), num_packets);
                    continue;
                }
                else if(icmp->icmp_type == 0)
                {
                    if(count == 0)
                    {
                        milliseconds = get_time();
                        count        = 1;
                    }
                    else
                    {
                        milliseconds = get_time() - milliseconds;
                        std::lock_guard<std::mutex> guard(stop_mutex);        // acquire lock
                        stop = true;
                        stop_cv.notify_all();
                        break;        // release lock
                    }                 // end if
                }
                else if(icmp->icmp_type == 11)
                {
                    errno = ENETUNREACH;
                    perror("TTL Exceeded");
                    exit(EXIT_FAILURE);
                }        // end if

            }        // end for

            if(!sql_output)
                printf("UDP Packets received: %d/%d\n", udp_ack, num_packets);

            // report the losses
            packets_lost = num_packets - udp_ack;

            if(verbose)
            {
                printf("Missing Packets:  ");
                size_t i = 0;
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
        int icmp_fd;                                 /// the socket file descriptor for ICMP replies
        ip_icmp_packet icmp_send;                    /// the ICMP packet to send for timestamps
        const uint16_t icmp_packet_size = 64;        /// 64 byte icmp packet size up to a mx of 76 bytes for replies

        /* Synchronization Variables */
        bool stop;                                    // boolean to stop reciveing data and timestamp
        std::mutex stop_mutex;                        /// mutex for stop
        std::mutex recv_ready_mutex;                  /// mutex for recv_ready
        bool recv_ready;                              // boolean that the receive thread has completed preparations
        std::condition_variable stop_cv;              /// condition variable for stop -- denotes
        std::condition_variable recv_ready_cv;        /// condition variable for recv_ready mutex
    };

}        // end namespace detection

#endif        // DETECTOR_UDP_DETECTOR_HPP
