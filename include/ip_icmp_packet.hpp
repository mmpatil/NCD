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
 * @file: ip_icmp_packet.hpp
 */

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
