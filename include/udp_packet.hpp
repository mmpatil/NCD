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
 * @file: udp_packet.hpp
 */

#ifndef DETECTOR_UDP_PACKET_HPP
#define DETECTOR_UDP_PACKET_HPP


#include <netinet/ip.h>  /* for struct ip */
#include <netinet/udp.h> /* for struct udphdr */

#include "ip_checksum.h"
#include "packet.hpp"


namespace detection
{

    /**
     * The udp_packet class is designed to simplify the initialization of udp packets
     *
     * This subclass of packet adds a checksum feature, and adds transport layer information to
     * the raw packet buffer.
     *
     */
    class udp_packet : public packet
    {
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
            : packet(payload_length, trans_offset + sizeof(udphdr)), transport_offset(trans_offset)
        {
            udphdr* udp_header = (udphdr*)&(data[trans_offset]);
            udp_header->source = htons(sport);
            udp_header->dest   = htons(dport);
            udp_header->len    = htons((uint16_t)(payload_length + sizeof(udphdr)));
        }

        virtual ~udp_packet() {}


        /**
         * provides the UDP checksum from the provided pseudo header
         *
         *
         *@param ps the pseudo_header to used in creating the UDP checksum
         *
         */
        virtual void checksum(const pseudo_header& ps)
        {
            size_t offset = sizeof(ps);
            auto len = data.size();
            buffer_t buff(offset + len - transport_offset);

            // copy the pseudo header into the buffer
            memcpy(&buff[0], &ps, offset);

            // copy the transport header and data into the buffer
            memcpy(&buff[offset], &data[transport_offset], data.size() - transport_offset);

            udphdr* udp_header = (udphdr*)&(data[transport_offset]);
            udp_header->check  = ip_checksum(buff.data(), buff.size());
        }

        size_t transport_offset;        // the offset in bytes from the beginning of the packet to the start of the
                                        // transport header

    private:
        /* data */
    };
}
#endif        // DETECTOR_UDP_PACKET_HPP
