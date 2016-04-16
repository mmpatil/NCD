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
 * @file: ip_checksum.h
 */

#ifndef SINGLE_THREADED_SERVER_HPP_
#define SINGLE_THREADED_SERVER_HPP_

#include "co_op_data.hpp"
#include <sstream>

class SingleThreadedServer
{
public:
    SingleThreadedServer();
    void run();
    void acceptor();

    void receive_train(detection::test_params params, sockaddr_in client);
    void terminate(std::string msg);
    void send_tcp_reply(int sock_fd, detection::test_results results);
    detection::test_results receive_tcp_parameters(int sock_fd, sockaddr_in client);
    void capture_packets(detection::test_params params);

private:
    int listen_fd;
    bool open;
    bool abort_session;

    int send_complete_fd[2];

    int capture_fd[2];

    uint32_t get_pcap_id(uint32_t expID)
    {
        std::stringstream command;
        command << "~/experiment/pcap_script.py " << expID;
        // command <<"python pcap_script.py "  << expID;
        FILE* in = popen(command.str().c_str(), "r");
        uint32_t pcap_id;
        fscanf(in, "%u", &pcap_id);
        pclose(in);

        return pcap_id;
    }

    int setup_udp_socket(uint16_t port);
};


#endif        // SINGLE_THREADED_SERVER_HPP_
