//
// Created by atlas on 3/5/16.
//

#ifndef DETECTOR_ICMP_PACKET_HPP
#define DETECTOR_ICMP_PACKET_HPP

#include <netinet/ip.h>      /* for struct ip */
#include <netinet/ip_icmp.h> /* for struct icmp */

#include "ip_checksum.h"
#include "packet.hpp"


namespace detection {

    /**
    * Simplifies the setup of ICMP Packets
    */
    class icmp_packet : public detection::packet {

    public:


        /**
         *
         */
        icmp_packet(size_t length, uint8_t type, uint8_t code, uint16_t id, uint16_t seq, size_t offset = 0)
                : packet(length + sizeof(icmp), sizeof(icmp)) {
            /* set up icmp message header*/
            struct icmp *icmp = (struct icmp *) &data[offset];
            icmp->icmp_type = type;
            icmp->icmp_code = code;
            icmp->icmp_id = id;
            icmp->icmp_seq = seq;

            // memset(icmp->icmp_data, 0xa5, length);
            gettimeofday((struct timeval *) icmp->icmp_data, nullptr);

            icmp->icmp_cksum = 0;
            icmp->icmp_cksum = ip_checksum(icmp, data.size() - offset);
        }

        virtual ~icmp_packet() { }
    };

}// end namespace detection
#endif //DETECTOR_ICMP_PACKET_HPP
