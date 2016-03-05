//
// Created by atlas on 3/5/16.
//

#ifndef DETECTOR_IP_ICMP_PACKET_HPP
#define DETECTOR_IP_ICMP_PACKET_HPP


#include "icmp_packet.hpp"
#include <netinet/ip.h>

namespace detection
{

    class ip_icmp_packet : public icmp_packet
    {

    public:
        ip_icmp_packet(const iphdr& ip, size_t length, uint8_t type, uint8_t code, uint16_t id, uint16_t seq)
            : icmp_packet(length + sizeof(iphdr), type, code, id, seq, sizeof(iphdr))
        {
            std::memcpy(&data[0], &ip, sizeof(iphdr));
        }

        virtual ~ip_icmp_packet() {}
    };

}        // end namespace detection

#endif        // DETECTOR_IP_ICMP_PACKET_HPP
