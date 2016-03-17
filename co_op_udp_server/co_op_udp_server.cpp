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

#include <sys/socket.h>
#include <netinet/in.h>
#include <bits/ios_base.h>
#include <thread>
#include <condition_variable>
#include "co_op_udp_server.hpp"

// global constants
const int port = 15555;
const int timeout = 60;
const std::chrono::seconds s(60);

co_op_udp_server::co_op_udp_server()
{ }

void co_op_udp_server::listener()
{
    // start listening on a specific , non-reserved port
    int listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // setup struct for server address
    sockaddr_in serv_addr = {};
    sockaddr_in client_addr = {};

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    int err = bind(listen_fd, (sockaddr *) &serv_addr, sizeof(serv_addr));
    if(err < 0) {
        std::ios_base::failure e("Failed to bind socket for listen()");
        throw e;
    }

    listen(listen_fd, 2);
    socklen_t client_len = 0;
    while(true) {
        int temp_fd = accept(listen_fd, (sockaddr *) &client_addr, &client_len);
        std::thread th(&co_op_udp_server::process_udp, this, temp_fd);
        th.detach();
    }

}

void co_op_udp_server::process_udp(int sock_fd)
{
    // zero initialize tcp message buffer;
    char buff[1500] = {};

    // get experiment parameters from client
    int err = recv(sock_fd, &buff, sizeof(buff), 0);
    if(err < 0) {
        error_handler("Failure receiving experimental parameters from client");
        return;
    }




}


void co_op_udp_server::timer(std::chrono::seconds timeout)
{
    std::condition_variable timeout_cv;


}

void co_op_udp_server::measure()
{ }

void co_op_udp_server::capture_traffic()
{ }

void co_op_udp_server::process_data()
{ }

void co_op_udp_server::sync_ids()
{ }


void co_op_udp_server::error_handler(std::string msg)
{

}

