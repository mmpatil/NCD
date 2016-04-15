//
// Created by atlas on 3/31/16.
//

#ifndef BASE_UPD_DETECTOR_HPP
#define BASE_UPD_DETECTOR_HPP

#include "detector.hpp"

namespace detection
{

    class base_udp_detector : public detector
    {

    public:
        base_udp_detector(uint16_t test_id_in, std::string dest_ip, uint8_t tos, uint16_t id, uint16_t frag_off,
                          uint8_t ttl, uint8_t proto, uint16_t check_sum, uint16_t sport, uint16_t dport,
                          std::string filename = "/dev/urandom", uint16_t num_packets = 1000,
                          uint16_t data_length = 512, uint16_t num_tail = 20, uint16_t tail_wait = 10,
                          raw_level raw_status = none, transport_type trans_proto = transport_type::udp,
                          bool verbose_option = false)
            : detector(test_id_in, dest_ip, tos, (uint16_t)(data_length + sizeof(udphdr) + sizeof(iphdr)), id, frag_off,
                       ttl, proto, check_sum, sport, dport, filename, num_packets, data_length, num_tail, tail_wait,
                       raw_status, trans_proto, verbose_option)
        {
            setup_packet_train();
        }

        virtual void populate_full() override
        {
            data_train.reserve(num_packets);
            for(int i = 0; i < num_packets; ++i)
                data_train.push_back(std::make_shared<ip_udp_packet>(ip_header, payload_size, sport, dport));
        }

        virtual void populate_trans() override
        {
            data_train.reserve(num_packets);
            for(int i = 0; i < num_packets; ++i)
                data_train.push_back(std::make_shared<udp_packet>(payload_size, sport, dport));
        }

        virtual void populate_none() override
        {
            data_train.reserve(num_packets);
            for(int i = 0; i < num_packets; ++i)
                data_train.push_back(std::make_shared<packet>(payload_size));
        }

        virtual int transport_header_size() override { return sizeof(udphdr); }

        virtual void setup_sockets() override
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
                perror("ncd only supports IPV4 at this time");
                exit(EXIT_FAILURE);
            }        // end error check

            // set TTL
            setsockopt(send_fd, IPPROTO_IP, IP_TTL, &ip_header.ttl, sizeof(ip_header.ttl));
            socklen_t size = 1500U * num_packets;
            setsockopt(send_fd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
        }

        virtual void send_train() override
        {
            int n;
            /*send data train*/
            for(const auto& item : data_train)
            {
                n = (int)sendto(send_fd, item->data.data(), item->data.size(), 0, res->ai_addr, res->ai_addrlen);
                if(n == -1)
                {
                    perror("call to sendto() failed: error sending UDP udp train");
                    exit(EXIT_FAILURE);
                }        // end if
            }            // end for
        }                // end send_train()
    };
}        // end namespace detection

#endif        // BASE_UPD_DETECTOR_HPP
