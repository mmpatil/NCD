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

#ifndef DETECTOR_UNIT_TEST_CO_OP_UDP_SERVER_HPP
#define DETECTOR_UNIT_TEST_CO_OP_UDP_SERVER_HPP

#include <string>
#include <chrono>
#include "co_op_data.hpp"

class co_op_udp_server {

public:
    co_op_udp_server();
    virtual ~co_op_udp_server(){}

    void listener();
    void process_udp(int sock_fd);
    void timer(std::chrono::seconds timeout);
    void measure();
    void capture_traffic();
    void process_data();
    void sync_ids();
    void error_handler(std::string msg);

private:



};


#endif //DETECTOR_UNIT_TEST_CO_OP_UDP_SERVER_HPP
