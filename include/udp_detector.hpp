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

#include "detector.hpp"

namespace detection
{

    class udp_detector : public detector
    {

    public:
        udp_detector(std::string src_ip, std::string dest_ip, uint8_t tos, uint16_t id, uint16_t frag_off, uint8_t ttl,
                     uint8_t proto, uint16_t check_sum, uint32_t sport, uint32_t dport,
                     std::string filename = "/dev/urandom", uint16_t num_packets = 1000, uint16_t data_length = 512,
                     uint16_t num_tail = 20, uint16_t tail_wait = 10, raw_level raw_status = none,
                     transport_type trans_proto = transport_type::udp)
            : detector(src_ip, dest_ip, tos, (uint16_t)(data_length + sizeof(udphdr) + sizeof(iphdr)), id, frag_off,
                       ttl, proto, check_sum, sport, dport, filename, num_packets, data_length, num_tail, tail_wait,
                       raw_status, trans_proto),
              icmp_send(ip_header, 64 - sizeof(udphdr), ICMP_ECHO, 0, (uint16_t)getpid(), (uint16_t)rand())
        {
            setup_packet_train();
            // setup_sockets();
        }

        virtual void populate_full()
        {
            data_train.reserve(num_packets);
            for(int i = 0; i < num_packets; ++i)
                data_train.push_back(std::make_shared<ip_udp_packet>(ip_header, payload_size, sport, dport));
        }

        virtual void populate_trans()
        {
            data_train.reserve(num_packets);
            for(int i = 0; i < num_packets; ++i)
                data_train.push_back(std::make_shared<udp_packet>(payload_size, sport, dport));
        }

        virtual void populate_none()
        {
            data_train.reserve(num_packets);
            for(int i = 0; i < num_packets; ++i)
                data_train.push_back(std::make_shared<packet>(payload_size));
        }

        virtual int transport_header_size() { return sizeof(udphdr); }

        virtual void setup_sockets()
        {

            /*get root privileges */
            int err = setuid(0);
            if(err < 0)
            {
                perror("Elevated privileges not acquired...");
                exit(EXIT_FAILURE);
            }

            send_fd = socket(res->ai_family, SOCK_DGRAM, IPPROTO_UDP);

            if(send_fd == -1)
            {
                perror("call to socket() failed for SEND");
                exit(EXIT_FAILURE);
            }        // end error check

            if(res->ai_family != AF_INET)
            {
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

            /*give up privileges */
            err = setuid(getuid());
            if(err < 0)
            {
                perror("Elevated privileges not released");
                exit(EXIT_FAILURE);
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


        virtual void setup_icmp_packet()
        {
            /* size of ICMP Echo message */
            uint16_t icmp_data_len = (uint16_t)(icmp_packet_size - sizeof(struct icmp) - sizeof(uint16_t));

            iphdr icmp_ip_header(ip_header);
            icmp_ip_header.protocol = IPPROTO_ICMP;
            icmp_send =
              ip_icmp_packet(icmp_ip_header, icmp_data_len, ICMP_ECHO, 0, (uint16_t)getpid(), (uint16_t)rand());
        }


        inline virtual void send_timestamp()
        {
            /*send Head ICMP Packet*/
            int n = sendto(icmp_fd, icmp_send.data.data(), icmp_send.data.size(), 0, res->ai_addr, res->ai_addrlen);
            if(n == -1)
            {
                perror("Call to sendto() failed: error sending ICMP packet");
                exit(EXIT_FAILURE);
            }
        }


        virtual void send_train()
        {
            int n;
            /*send data train*/
            for(const auto& item : data_train)
            {
                n = sendto(send_fd, item->data.data(), item->data.size(), 0, res->ai_addr, res->ai_addrlen);
                if(n == -1)
                {
                    perror("call to sendto() failed: error sending UDP udp train");
                    exit(EXIT_FAILURE);
                }        // end if
            }            // end for
        }                // end send_train()


        virtual void prepare() { setup_icmp_packet(); }

        virtual void send_tail()
        {
            struct icmp* icmp = (struct icmp*)(icmp_send.data.data() + sizeof(struct ip));

            /*send tail ICMP Packets with timer*/
            std::unique_lock<std::mutex> stop_lock(stop_mutex);        // acquire lock
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
                stop_cv.wait_until(stop_lock, now);
            }        // end for

            stop_lock.unlock();        // release lock
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

            buff.fill(0);

            /* length of address */
            socklen_t adrlen = sizeof(addr);

            /*Receive initial ICMP echo response && Time-stamp*/
            struct ip* ip      = (struct ip*)buff.data();
            icmp               = (struct icmp*)(ip + 1);
            struct udphdr* udp = (struct udphdr*)(&(icmp->icmp_data) + sizeof(struct ip));

            uint32_t* bitset = make_bs_32(num_packets);
            uint16_t* id = (uint16_t*)(udp + 1);
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
                    set_bs_32(bitset, *id, num_packets);
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
        int icmp_fd;
        ip_icmp_packet icmp_send;
        const uint16_t icmp_packet_size = 64;        // 64 byte icmp packet size up to a mx of 76 bytes for replies
    };

}        // end namespace detection

#endif        // DETECTOR_UDP_DETECTOR_HPP
