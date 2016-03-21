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

#include <cstdint>
#include <ostream>

namespace detection
{
    struct test_results
    {
        bool success;
        double elapsed_time;
        uint32_t lostpackets;
        uint32_t pcap_id;
        // char losses[512];
    };


    struct test_params
    {
        uint32_t num_packets;
        uint32_t payload_size;
        uint16_t port;
        uint16_t offset;
        uint32_t test_id;
        bool last_train;
    };
}


std::ostream& operator<<(std::ostream& os, const detection::test_results& res)
{
    std::string s = res.success ? "true" : "false";

    os << res.lostpackets << " " << res.elapsed_time << " " << res.pcap_id << s;
    return os;
}

std::ostream& operator<<(std::ostream& os, const detection::test_params& par)
{
    std::string s = par.last_train ? "Last" : "not last";

    os << par.num_packets << " " << par.payload_size << " " << par.port << " " << par.offset << " " << par.test_id
       << " " << s;
    return os;
}
#endif        // DETECTOR_UNIT_TEST_CO_OP_DATA_HPP
