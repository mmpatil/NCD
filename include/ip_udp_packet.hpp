//
// Created by atlas on 3/5/16.
//

#ifndef DETECTOR_IP_UDP_PACKET_HPP
#define DETECTOR_IP_UDP_PACKET_HPP


#include <netinet/ip.h>

#include "udp_packet.hpp"

namespace detection{


    class ip_udp_packet : public udp_packet {
    public:
        ip_udp_packet(const iphdr &ip, size_t payload_length, uint16_t sport, uint16_t dport)
                : udp_packet(payload_length + sizeof(iphdr), sizeof(iphdr), sport, dport),
                  ps{ip.saddr, ip.daddr, 0, IPPROTO_TCP,
                     htons(payload_length + sizeof(pseudo_header) + sizeof(udphdr))} {
            std::memcpy(&data[0], &ip, sizeof(iphdr));
        }

        virtual ~ip_udp_packet() { }

        virtual void fill(std::ifstream &file, uint16_t packet_id) {
            packet::fill(file, packet_id);

            udp_packet::checksum(ps);

            iphdr *ip = (iphdr *) &data[0];
            ip->check = ip_checksum(&data[0], data.size());
        }

        pseudo_header ps;

    private:
        /* data */
    };


}// end namespace detection


#endif //DETECTOR_IP_UDP_PACKET_HPP
