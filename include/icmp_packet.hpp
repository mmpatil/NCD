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
 * @file: icmp_packet.hpp
 */

#ifndef DETECTOR_ICMP_PACKET_HPP
#define DETECTOR_ICMP_PACKET_HPP

#include <netinet/ip.h>      /* for struct ip */
#include <netinet/ip_icmp.h> /* for struct icmp */

#include "ip_checksum.h"
#include "packet.hpp"


namespace detection
{

    /**
    * Simplifies the setup of ICMP Packets
    */
    class icmp_packet : public detection::packet
    {

    public:
        /**
         *
         */
        icmp_packet(size_t length, uint8_t type, uint8_t code, uint16_t id, uint16_t seq, size_t offset = 0)
            : packet(length + sizeof(icmp), sizeof(icmp))
        {
            /* set up icmp message header*/
            struct icmp* icmp = (struct icmp*)&data[offset];
            icmp->icmp_type   = type;
            icmp->icmp_code   = code;
            icmp->icmp_id     = id;
            icmp->icmp_seq    = seq;

            // memset(icmp->icmp_data, 0xa5, length);
            gettimeofday((struct timeval*)icmp->icmp_data, nullptr);

            icmp->icmp_cksum = 0;
            icmp->icmp_cksum = ip_checksum(icmp, data.size() - offset);
        }

        virtual ~icmp_packet() {}
    };

}        // end namespace detection
#endif        // DETECTOR_ICMP_PACKET_HPP
