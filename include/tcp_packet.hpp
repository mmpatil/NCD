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
 * @file: tcp_packet.hpp
 */

#ifndef DETECTOR_TCP_PACKET_HPP
#define DETECTOR_TCP_PACKET_HPP

#include <netinet/ip.h>  /* for struct ip */
#include <netinet/tcp.h> /* for struct tcphdr */

#include "ip_checksum.h"
#include "packet.hpp"

namespace detection
{

    /**
     * Simplifies the setup of tcp packets
     *
     */
    class tcp_packet : public packet
    {
    public:
        /**
         *
         * creates a packet with a valid TCP header
         *
         * @param payload_length the lenght of the payload section
         * @param sport the source port
         * @param dport the destination port
         * @param trans_offset the offset from the begining of the packet that the transport header begins
         * @param sequence the packets sequence number -- if set later use zero <-- doing this delays when the checksum
         * can be done
         *
         * @param ack_sequence the sequence number of the ACK
         *
         * @param win the tcp windowsize
         * @param urg the urgent pointer
         * @param ack_flag the TCP ACK flag
         * @param syn_flag the TCP SYN flag
         * @param doff the offset that data begins after the header -- default is 5 the size of the TCP header in 32bit
         * words
         *
         */
        tcp_packet(size_t payload_length, uint16_t sport, uint16_t dport, uint32_t sequence, uint32_t ack_sequence,
                   uint16_t win, uint16_t urg, bool ack_flag, bool syn_flag, unsigned char doff = 5,
                   size_t trans_off = 0)
            : packet(payload_length + sizeof(tcphdr), trans_off + sizeof(tcphdr)), transport_offset(trans_off)
        {
            tcphdr* tcp_header  = (tcphdr*)&(data[transport_offset]);
            tcp_header->source  = htons(sport);
            tcp_header->dest    = htons(dport);
            tcp_header->seq     = htonl(sequence);
            tcp_header->ack_seq = htonl(ack_sequence);
            tcp_header->ack     = (ack_flag ? 1 : 0);
            tcp_header->syn     = (syn_flag ? 1 : 0);
            tcp_header->doff    = doff;
            tcp_header->window  = win;
            tcp_header->urg_ptr = urg;
        }

        virtual ~tcp_packet() {}


        /**
             * provides the TCP checksum from the provided pseudo header
             *
             *
             *@param ps the pseudo_header to used in creating the UDP checksum
             *
             */
        virtual void checksum(const pseudo_header& pseudo)
        {
            size_t offset = sizeof(pseudo);
            buffer_t buff(offset + data.size() - transport_offset);

            // copy the pseudo header into the buffer
            memcpy(&buff[0], &pseudo, offset);

            // copy the transport header and data into the buffer
            memcpy(&buff[offset], &data[transport_offset], data.size() - transport_offset);

            tcphdr* tcp_header = (tcphdr*)&(data[transport_offset]);
            tcp_header->check  = ip_checksum(buff.data(), buff.size());
        }

        size_t transport_offset;

    private:
    };

}        // end namespace detector

#endif        // DETECTOR_TCP_PACKET_HPP
