//
// Created by atlas on 3/5/16.
//

#ifndef DETECTOR_PACKET_HPP
#define DETECTOR_PACKET_HPP


#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

namespace detection
{

    /* define a buffer type
     * we use a vector of char in place of a char array
     * to get a byte addressable region of raw memory that can have varying size
     *
     */
    typedef std::vector<char> buffer_t;
    /**
     * strongly typed enumeration
     *
     * lists transport protocols directly supported
     * extend the enum class to support more protocols
     *
     */
    enum class transport_type
    {
        udp,
        tcp
    };
    /**
     * struct for udp pseudo header
     */
    struct __attribute__((__packed__)) pseudo_header
    {
        uint32_t source;
        uint32_t dest;
        uint8_t zero;
        uint8_t proto;
        uint16_t len;
    };

    /**
     * provides a base class for managing packets
     * this class only provides a single raw buffer to use
     * and doesn't directly support transport or network headers
     */
    class packet
    {
    public:
        /**
         * constructor
         *
         * TODO: change the constructor to add data_offset to the initializer for data and
         * make derived classes only worry about setting the offset, ie where the next packet type down starts
         *
         * @param payload_length the length of the buffer -- should probably change inheriting classes to
         * only pass the offset w/o worrying about increasing the buffer size
         *
         * @param data_offset the offset from the start of the buffer that the data (read payload) begins
         * The payload is expected not to hold any header information above the application layer (ie http headers are
         * OK since they are application layer headers, but TCP headers generally should be set using a subclass
         */
        packet(size_t payload_length, size_t data_offset = 0)
            : data(payload_length + data_offset), filled(false), data_offset(data_offset)
        {
        }

        /* destructor */
        virtual ~packet() {}

        /**
         * Reads data from a file into the payload region of a packet
         *
         *@param file a file stream to read data from into the payload region
         *@param packet_id each packet is expected to hold a packet_id in the first 16 bits
         */
        virtual void fill(std::ifstream& file, uint16_t packet_id)
        {
            if(data_offset >= data.size())
                throw std::ios_base::failure("The Data offset in the packet is too large...");
            if(file.is_open())
            {
                memcpy(&data[data_offset], &packet_id, sizeof(packet_id));
                uint16_t* id = (uint16_t*)&data[data_offset];
                *id          = packet_id;
                file.read(&data[data_offset + sizeof(packet_id)], data.size() - data_offset);
            }
            else
            {
                std::string err_string = "Error filling packet from file: file was not open";
                std::cerr << err_string << std::endl;
                std::ios_base::failure e(err_string);
                throw e;
            }
            filled = true;
        }


        /**
         * checksum the packet -- data only payloads don't need this
         * Assumes that checksum requires a pseudo header per UDP/TCP
         *
         * @param ps a pseudo_header struct used during the checksum
         *
         */
        virtual void checksum(const pseudo_header& ps) {}

        /* data*/
        buffer_t data;        // a raw -- resizable -- buffer to use
        bool filled;
        // boolean denotes if the payload has been filled from file yet
        size_t data_offset;        // the offset form the beginning of the packet where the payload begins
    };


}        // end namespace detector
#endif        // DETECTOR_PACKET_HPP
