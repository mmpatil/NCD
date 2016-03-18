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
 * @file: tcp_detector.hpp
 */

#ifndef DETECTOR_TCP_DETECTOR_HPP
#define DETECTOR_TCP_DETECTOR_HPP

#include "detector.hpp"

namespace detection
{


    class tcp_detector : public detector
    {


    public:
        tcp_detector(std::string dest_ip, uint8_t tos, uint16_t id, uint16_t frag_off, uint8_t ttl, uint8_t proto,
                     uint16_t check_sum, uint32_t sport, uint32_t dport, std::string filename = "/dev/urandom",
                     uint16_t num_packets = 1000, uint16_t data_length = 512, uint16_t num_tail = 20,
                     uint16_t tail_wait = 10, raw_level raw_status = full,
                     transport_type trans_proto = transport_type::tcp, uint16_t syn_port_in = 22223)
            : detector(dest_ip, tos, data_length + sizeof(tcphdr) + sizeof(iphdr), id, frag_off, ttl, proto, check_sum,
                       sport, dport, filename, num_packets, data_length, num_tail, tail_wait, raw_status, trans_proto),
              syn_port(syn_port_in)
        {

            tcp_header.source = sport;
            tcp_header.dest   = dport;
            tcp_header.doff   = 5;
            // tcp_header.source = sport;
            setup_packet_train();
        }

        virtual void fix_data_train()
        {
            uint32_t num = 0;
            for(auto& item : data_train)
            {
                tcphdr* tcp = (tcphdr*)(item->data.data() + sizeof(iphdr));        //->data[trans_offset];
                num += item->data.size() - sizeof(iphdr);
                tcp->seq = htonl(num);
            }
        }

        virtual void populate_full()
        {
            data_train.reserve(num_packets);
            for(int i = 0; i < num_packets; ++i)
                data_train.push_back(std::make_shared<ip_tcp_packet>(ip_header, payload_size, sport, dport,
                                                                     tcp_header.seq, tcp_header.ack_seq, (1 << 15) - 1,
                                                                     tcp_header.urg_ptr, 1, 0, 5));

            fix_data_train();
        }

        virtual void populate_trans()
        {
            data_train.reserve(num_packets);
            for(int i = 0; i < num_packets; ++i)
                data_train.push_back(std::make_shared<tcp_packet>(
                  payload_size, sport, dport, tcp_header.seq, tcp_header.ack_seq, tcp_header.window, tcp_header.urg_ptr,
                  (uint16_t)tcp_header.ack, (uint16_t)tcp_header.syn, (uint16_t)tcp_header.doff));

            fix_data_train();
        }

        virtual void populate_none()
        {
            data_train.reserve(num_packets);
            for(int i = 0; i < num_packets; ++i)
                data_train.push_back(std::make_shared<packet>(payload_size));
        };

        virtual int transport_header_size() { return sizeof(tcphdr); }

        virtual void setup_sockets()
        {

            /*get root privileges */
            int err = setuid(0);
            if(err < 0)
            {
                perror("Elevated privileges not acquired...");
                exit(EXIT_FAILURE);
            }

            send_fd = socket(res->ai_family, SOCK_RAW, IPPROTO_TCP);

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

            if(raw == full)
            {
                /* set up our own IP header*/
                int tcp_hdrincl = 1;
                if(setsockopt(send_fd, IPPROTO_IP, IP_HDRINCL, &tcp_hdrincl, sizeof(tcp_hdrincl)) == -1)
                {
                    perror("setsockopt() failed icmp");
                }
            }

            /*give up privileges */
            err = setuid(getuid());
            if(err < 0)
            {
                perror("Elevated privileges not released");
                exit(EXIT_FAILURE);
            }

            recv_fd = socket(res->ai_family, SOCK_RAW, IPPROTO_TCP);

            if(recv_fd == -1)
            {
                perror("call to socket() failed");
                exit(EXIT_FAILURE);
            }

            /* give up root privileges */
            err = setuid(getuid());
            if(err < 0)
            {
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

        virtual void setup_syn_packets()
        {
            pseudo_header syn_ps = {};
            syn_ps.source        = ip_header.saddr;
            syn_ps.dest          = ip_header.daddr;
            syn_ps.zero          = 0;
            syn_ps.len           = htons(sizeof(tcphdr));
            syn_ps.proto         = IPPROTO_TCP;

            syn_packet_1.reset(new ip_tcp_packet(ip_header, 0, sport, dport, 0, 0, (1 << 15) - 1, 0, false, true, 5));
            syn_packet_1->checksum(syn_ps);

            syn_packet_2.reset(
              new ip_tcp_packet(ip_header, 0, sport + 1, syn_port, 0, 0, (1 << 15) - 1, 0, false, true, 5));
            syn_packet_2->checksum(syn_ps);
        }

        virtual void prepare() { setup_syn_packets(); }


        virtual void send_timestamp()
        {
            int n;

            // set up the buffer to receive the reply into
            struct ip* ip            = (struct ip*)buff.data();
            struct tcphdr* tcp_reply = (struct tcphdr*)(ip + 1);

            n = sendto(send_fd, syn_packet_1->data.data(), syn_packet_1->data.size(), 0, res->ai_addr, res->ai_addrlen);
            if(n == -1)
            {
                perror("Call to sendto() failed: tcp syn");
                exit(EXIT_FAILURE);
            }

            do
            {
                if((recvfrom(send_fd, buff.data(), buff.size(), 0, 0, 0)) == -1)
                {
                    perror("call to recvfrom() failed: tcp SYN-ACK");
                    exit(EXIT_FAILURE);
                }

            } while((tcp_reply->dest != htons(sport)) || (ip_header.saddr != ip->ip_src.s_addr));


            if(verbose)
            {
                printf("TCP SYN reply from IP: %s\n", inet_ntoa(ip->ip_src));
                printf("TCP SYN reply from port: %d to port: %d\n", ntohs(tcp_reply->source), ntohs(tcp_reply->dest));
            }

            milliseconds = get_time();        // time stamp just before we begin sending
        }


        virtual void send_train()
        {
            int n;
            // uint32_t ack_seq = ntohl(tcp_reply->seq);
            // tcphdr* tcp;
            //        uint32_t uniform_data_len = sizeof(tcphdr) + payload_size;
            /*send data train*/
            for(auto item : data_train)
            {
                //         tcp      = (tcphdr*)&item->data[((ip_tcp_packet*)item.get())->transport_offset];
                //          tcp->ack_seq = htonl(ack_seq);// += uniform_data_len);
                //           item->checksum(ps);

                n = sendto(send_fd, item->data.data(), item->data.size(), 0, res->ai_addr, res->ai_addrlen);
                if(n == -1)
                {
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

        virtual void send_tail()
        {
            int n;
            std::unique_lock<std::mutex> stop_lock(stop_mutex);        // acquire lock
            for(int i = 0; i < num_tail && !stop; ++i)
            {
                n = sendto(send_fd, syn_packet_2->data.data(), syn_packet_2->data.size(), 0, res->ai_addr,
                           res->ai_addrlen);
                if(n == -1)
                {
                    perror("Call to sendto() failed: TCP Tail Syn");
                    exit(EXIT_FAILURE);
                }

                stop_cv.wait_for(stop_lock, std::chrono::milliseconds(tail_wait));
            }        // end for

            stop_lock.unlock();        // release lock
        }

        virtual void receive()
        {

            /*number of bytes received*/
            int n;

            /* to receive data with*/
            struct sockaddr_in addr;

            socklen_t adrlen = sizeof(addr);

            buff.fill(0);

            std::unique_lock<std::mutex> recv_ready_lock(recv_ready_mutex);
            // while(!recv_ready)
            //{
            recv_ready_cv.wait(recv_ready_lock, [this]() { return this->recv_ready; });
            //}

            struct ip* ip      = (struct ip*)buff.data();
            struct tcphdr* tcp = (struct tcphdr*)(ip + 1);
            do
            {
                n = recvfrom(send_fd, buff.data(), buff.size(), 0, (struct sockaddr*)&addr, &adrlen);
                if(n < 0)
                {
                    perror("recvfrom() failed");
                    exit(EXIT_FAILURE);
                }

            } while((tcp->dest != htons(syn_port)) || (ip->ip_src.s_addr != ip_header.saddr));
            milliseconds = get_time() - milliseconds;

            {
                std::lock_guard<std::mutex> stop_guard(stop_mutex);
                stop = true;
            }        // release lock guard

            stop_cv.notify_all();

            if(verbose)
            {
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

}        // end namespace detection


#endif        // DETECTOR_TCP_DETECTOR_HPP
