//
// Created by atlas on 3/5/16.
//

#ifndef DETECTOR_TCP_PACKET_HPP
#define DETECTOR_TCP_PACKET_HPP

#include <netinet/ip.h>      /* for struct ip */
#include <netinet/tcp.h>     /* for struct tcphdr */

#include "ip_checksum.h"
#include "packet.hpp"

namespace detector {

/**
 * Simplifies the setup of tcp packets
 *
 */
    class tcp_packet : public detector::packet {
    public:
        /**
         *
         * creates a packet with a valid TCP header
         *
         * @param payload_length the lenght of the payload section
         * @param sport the source port
         * @param dport the destination port
         * @param trans_offset the offset from the begining of the packet that the transport header begins
         * @param sequence the packets sequence number -- if set later use zero <-- doing this delays when the checksum can be done
         *
         * @param ack_sequence the sequence number of the ACK
         *
         * @param win the tcp windowsize
         * @param urg the urgent pointer
         * @param ack_flag the TCP ACK flag
         * @param syn_flag the TCP SYN flag
         * @param doff the offset that data begins after the header -- default is 5 the size of the TCP header in 32bit words
         *
         */
        tcp_packet(size_t payload_length, uint16_t sport, uint16_t dport, uint32_t sequence, uint32_t ack_sequence,
                   uint16_t win, uint16_t urg, bool ack_flag, bool syn_flag, unsigned char doff = 5,
                   size_t trans_off = 0)
                : packet(payload_length + sizeof(tcphdr), trans_off + sizeof(tcphdr)), transport_offset(trans_off) {
            tcphdr *tcp_header = (tcphdr *) &(data[transport_offset]);
            tcp_header->source = htons(sport);
            tcp_header->dest = htons(dport);
            tcp_header->seq = htonl(sequence);
            tcp_header->ack_seq = htonl(ack_sequence);
            tcp_header->ack = (ack_flag ? 1 : 0);
            tcp_header->syn = (syn_flag ? 1 : 0);
            tcp_header->doff = doff;
            tcp_header->window = win;
            tcp_header->urg_ptr = urg;
        }

        virtual ~tcp_packet() { }


/**
     * provides the TCP checksum from the provided pseudo header
     *
     *
     *@param ps the pseudo_header to used in creating the UDP checksum
     *
     */
        virtual void checksum(const detector::pseudo_header &pseudo) {
            size_t offset = sizeof(pseudo);
            detector::buffer_t buff(offset + data.size() - transport_offset);

            // copy the pseudo header into the buffer
            memcpy(&buff[0], &pseudo, offset);

            // copy the transport header and data into the buffer
            memcpy(&buff[offset], &data[transport_offset], data.size() - transport_offset);

            tcphdr *tcp_header = (tcphdr *) &(data[transport_offset]);
            tcp_header->check = ip_checksum(buff.data(), buff.size());
        }

        size_t transport_offset;

    private:
    };

}// end namespace detector

#endif //DETECTOR_TCP_PACKET_HPP
