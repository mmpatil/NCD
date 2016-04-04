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
 * @file: udp_detector.hpp
 */


#ifndef DETECTOR_UNIT_TEST_CO_OP_DATA_HPP
#define DETECTOR_UNIT_TEST_CO_OP_DATA_HPP

#include <arpa/inet.h>
#include <cstdint>
#include <istream>
#include <ostream>

namespace detection
{
    /**
     * Used to transfer Test data back from a remote host
     */
    struct test_results
    {

        uint16_t success;
        double elapsed_time;
        uint16_t lostpackets;
        uint16_t pcap_id;

        friend std::ostream& operator<<(std::ostream& os, const detection::test_results& res)
        {
            std::string s = res.success ? "true" : "false";

            os << res.lostpackets << " " << res.elapsed_time << " " << res.pcap_id << s;
            return os;
        }        // char losses[512];

        friend std::istream& operator>>(std::istream& is, detection::test_results& res)
        {
            std::string s;        // = res.success ? "true" : "false";

            is >> res.lostpackets >> res.elapsed_time >> res.pcap_id >> s;
            res.success = (s == "true");
            return is;
        }

        /**
         * Serializes the test_results data structure for network transmission
         *
         * @param dest a pointer to the destination buffer to serialize this datastructure into -- genreally cha[]
         */
        void serialize(void* dest)
        {
            uint16_t* d    = reinterpret_cast<uint16_t*>(dest);
            uint16_t* time = reinterpret_cast<uint16_t*>(&elapsed_time);

            *d = htons(success);
            d++;
            // serialize the double
            *d = htons(time[0]);
            d++;
            *d = htons(time[1]);
            d++;
            *d = htons(time[2]);
            d++;
            *d = htons(time[3]);
            d++;

            *d = htons(lostpackets);
            d++;
            *d = htons(pcap_id);
        }


        /**
         * Deserializes the data from souce buffer into this struct. Read from network byte order to host byte order.
         *
         * @param source pointer to the soucrce buffer -- gernaally a char[]
         *
         * @return [description]
         */
        test_results deserialize(void* source)
        {
            uint16_t* d = reinterpret_cast<uint16_t*>(source);

            uint16_t* time = reinterpret_cast<uint16_t*>(&elapsed_time);

            success = ntohs(*d);
            d++;

            // deserialize the double;
            time[0] = ntohs(*d);
            d++;
            time[1] = ntohs(*d);
            d++;
            time[2] = ntohs(*d);
            d++;
            time[3] = ntohs(*d);
            d++;

            lostpackets = ntohs(*d);
            d++;
            pcap_id = ntohs(*d);
            return *this;
        }
    };


    /**
     * Used to tranfer test parameters to a remote host:w
     *
     */
    struct test_params
    {
        uint16_t num_packets;
        uint16_t payload_size;
        uint16_t port;
        uint16_t offset;
        uint16_t test_id;
        uint16_t last_train;

        friend std::ostream& operator<<(std::ostream& os, const detection::test_params& par)
        {
            std::string s = par.last_train ? "last" : "not last";

            os << par.num_packets << " " << par.payload_size << " " << par.port << " " << par.offset << " "
               << par.test_id << " " << s;
            return os;
        }

        friend std::istream& operator>>(std::istream& is, detection::test_params& par)
        {
            std::string s;        // =

            is >> par.num_packets >> par.payload_size >> par.port >> par.offset >> par.test_id >> s;

            par.last_train = (s == "last");
            return is;
        }


        /**
         * Serializes the test_results data structure for network transmission
         *
         * @param dest a pointer to the destination buffer to serialize this datastructure into -- genreally cha[]
         */
        void serialize(void* dest)
        {
            uint16_t* d = reinterpret_cast<uint16_t*>(dest);
            *d          = htons(num_packets);
            d++;
            *d = htons(payload_size);
            d++;
            *d = htons(port);
            d++;
            *d = htons(offset);
            d++;
            *d = htons(test_id);
            d++;
            *d = htons(last_train);
        }


        /**
         * Deserializes the data from souce buffer into this struct. Read from network byte order to host byte order.
         *
         * @param source pointer to the soucrce buffer -- gernaally a char[]
         *
         * @return [description]
         */
        test_params deserialize(void* source)
        {
            uint16_t* d = reinterpret_cast<uint16_t*>(source);
            num_packets = ntohs(*d);
            d++;
            payload_size = ntohs(*d);
            d++;
            port = ntohs(*d);
            d++;
            offset = ntohs(*d);
            d++;
            test_id = ntohs(*d);
            d++;
            last_train = ntohs(*d);
            return *this;
        }
    };
}
#endif        // DETECTOR_UNIT_TEST_CO_OP_DATA_HPP
