//
// Created by atlas on 3/5/16.
//

#ifndef DETECTOR_IP_TCP_PACKET_HPP
#define DETECTOR_IP_TCP_PACKET_HPP

#include <netinet/ip.h> /* for struct ip */

#include "tcp_packet.hpp"

namespace detection
{

    /**
     * creates IP-TCP-Packets
     */
    class ip_tcp_packet : public detection::tcp_packet
    {
    public:
        ip_tcp_packet(const iphdr& ip, size_t payload_length, uint16_t sport, uint16_t dport, uint32_t sequence,
                      uint32_t ack_sequence, uint16_t win, uint16_t urg, bool ack_flag, bool syn_flag,
                      unsigned char doff = 5)

            : tcp_packet(payload_length + sizeof(iphdr), sport, dport, sequence, ack_sequence, win, urg, ack_flag,
                         syn_flag, doff, sizeof(iphdr)),
              ps{ip.saddr, ip.daddr, 0, IPPROTO_TCP, htons(payload_length + sizeof(pseudo_header) + sizeof(tcphdr))}
        {
            std::memcpy(&data[0], &ip, sizeof(iphdr));
        }

        virtual ~ip_tcp_packet() {}

        virtual void fill(std::ifstream& file, uint16_t packet_id)
        {
            packet::fill(file, packet_id);

            checksum(ps);

            iphdr* ip = (iphdr*)&data[0];
            ip->check = ip_checksum(&data[0], data.size());
        }

        pseudo_header ps;

    private:
        /* data */
    };

}        // end namespace detection

#endif        // DETECTOR_IP_TCP_PACKET_HPP
