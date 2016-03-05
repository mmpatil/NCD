//
// Created by atlas on 3/5/16.
//

#ifndef DETECTOR_UDP_PACKET_HPP
#define DETECTOR_UDP_PACKET_HPP


#include <netinet/ip.h>      /* for struct ip */
#include <netinet/udp.h>     /* for struct udphdr */

#include "ip_checksum.h"
#include "packet.hpp"



namespace detection {

/**
 * The udp_packet class is designed to simplify the initialization of udp packets
 *
 * This subclass of packet adds a checksum feature, and adds transport layer information to
 * the raw packet buffer.
 *
 */
    class udp_packet : public packet {
    public:
        /**
         * Constructor -- sets up a udp packet
         *
         * Note that the checksum cannot be completed until a pseudo header is provided
         *
         * @param payload_length the length of the payload section
         * @param sport the source port
         * @param dport the destination port
         * @param trans_offset the offset from the beginningng of the packet that the transport header begins
         * @
         */
        udp_packet(size_t payload_length, uint16_t sport, uint16_t dport, size_t trans_offset = 0)
                : packet(payload_length, trans_offset + sizeof(udphdr)), transport_offset(trans_offset) {
            udphdr *udp_header = (udphdr * ) & (data[trans_offset]);
            udp_header->source = htons(sport);
            udp_header->dest = htons(dport);
            udp_header->len = htons((uint16_t) (payload_length + sizeof(udphdr)));
        }

        virtual ~udp_packet() { }


        /**
         * provides the UDP checksum from the provided pseudo header
         *
         *
         *@param ps the pseudo_header to used in creating the UDP checksum
         *
         */
        virtual void checksum(const pseudo_header &ps) {
            size_t offset = sizeof(ps);
            buffer_t buff(offset + data.size() - transport_offset);

            // copy the pseudo header into the buffer
            memcpy(&buff[0], &ps, offset);

            // copy the transport header and data into the buffer
            memcpy(&buff[offset], &data[transport_offset], data.size() - transport_offset);

            udphdr *udp_header = (udphdr * ) & (data[transport_offset]);
            udp_header->check = ip_checksum(buff.data(), buff.size());
        }

        size_t transport_offset;// the offset in bytes from the beginning of the packet to the start of the transport header

    private:
        /* data */
    };

}
#endif //DETECTOR_UDP_PACKET_HPP
