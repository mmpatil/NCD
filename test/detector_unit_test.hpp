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

#ifndef DETECTOR_UNIT_TEST_DETECTOR_UNIT_TEST_HPP
#define DETECTOR_UNIT_TEST_DETECTOR_UNIT_TEST_HPP


#include <gtest/gtest.h>

#include "../include/detector.hpp"

using namespace detection;


class MockDetector : public detector
{

public:
    MockDetector(const std::string& dest_ip, uint8_t tos, uint16_t ip_length, uint16_t id, uint16_t frag_off,
                 uint8_t ttl, uint8_t proto, uint16_t check_sum, uint32_t sport, uint32_t dport,
                 const std::string& filename, uint16_t num_packets, uint16_t data_length, uint16_t num_tail,
                 uint16_t tail_wait, const raw_level& raw_status, const transport_type& trans_proto)
        : detector(dest_ip, tos, ip_length, id, frag_off, ttl, proto, check_sum, sport, dport, filename, num_packets,
                   data_length, num_tail, tail_wait, raw_status, trans_proto)
    {
    }


    virtual void setup_sockets() override {}


    virtual void populate_full() override {}


    virtual void populate_trans() override {}


    virtual void populate_none() override {}


    virtual void send_train() override {}


    virtual void receive() override {}


    virtual void send_timestamp() override {}


    virtual void send_tail() override {}


    virtual int transport_header_size() override { return 0; }


    virtual void prepare() override { detector::prepare(); }


    virtual void output_results() override { detector::output_results(); }
};


class detector_unit_test : public ::testing::Test
{

protected:
    virtual void SetUp() {}
};


#endif        // DETECTOR_UNIT_TEST_DETECTOR_UNIT_TEST_HPP
