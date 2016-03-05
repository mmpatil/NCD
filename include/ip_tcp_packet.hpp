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
 * @file: ip_tcp_packet.hpp
 */

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
