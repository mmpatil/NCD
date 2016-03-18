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

#include <bits/ios_base.h>
#include <condition_variable>
#include <future>
#include <netinet/in.h>
#include <iostream>


#include "co_op_udp_server.hpp"

using namespace detection;

class experiment{
public:
    experiment(const detection::test_params& params_in): params(params_in){}
    virtual ~experiment(){}
    bool run(){
        std::lock_guard<std::mutex> abt_lk(abort_mutex);
        return !abort;
    }


private:
    detection::test_params params;
    detection::test_results results;
    std::mutex abort_mutex;
    std::condition_variable abort_cv;
    bool abort; // used when signaling child threads to abort execution
};

// global constants
const int port         = 15555;
const int time_to_quit = 60;
const std::chrono::seconds max_time(time_to_quit);

co_op_udp_server::co_op_udp_server()
{
}

void co_op_udp_server::listener()
{
    // start listening on a specific , non-reserved port
    int listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // setup struct for server address
    sockaddr_in serv_addr   = {};
    sockaddr_in client_addr = {};

    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port        = htons(port);
    int err                   = bind(listen_fd, (sockaddr*)&serv_addr, sizeof(serv_addr));
    if(err < 0)
    {
        std::ios_base::failure e("Failed to bind socket for listen()");
        throw e;
    }

    listen(listen_fd, 2);
    socklen_t client_len = 0;
    while(true)
    {
        int temp_fd = accept(listen_fd, (sockaddr*)&client_addr, &client_len);
        std::thread th(&co_op_udp_server::process_udp, this, temp_fd);
        th.detach();
    }
}

void co_op_udp_server::process_udp(int sock_fd)
{

    // zero initialize tcp message buffer;
    char buff[1500] = {};

    test_params* params = (test_params*)buff;


    // get experiment parameters from client
    int err = recv(sock_fd, &buff, sizeof(buff), 0);
    if(err < 0)
    {
        error_handler("Failure receiving experimental parameters from client");
        return;
    }

    std::mutex complete_mutex;
    std::condition_variable complete_cv;
    bool send_complete = false;


    auto a = params->num_packets;
    auto b = params->offset;
    auto c = params->payload_size;
    auto d = params->port;

    int q = a + b + c + d;
    q--;
    q++;
    if(!q)
    {
    }

    auto marker = std::chrono::high_resolution_clock::now();
    // spawn worker threads
    auto value = std::async(std::launch::async, &co_op_udp_server::timer, this, max_time);

    double val = 0;
    // listen for when send is complete and signal that it isa
    {
        std::lock_guard<std::mutex> complete_guard(complete_mutex);
        err = recv(sock_fd, &send_complete, sizeof(send_complete), 0);

        // timestamp ASAP -- even before reporting errors
        auto timestamp = std::chrono::high_resolution_clock::now() - marker;
        val            = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp).count();

        if(err < 0)
        {
            error_handler("Failure receiving experimental parameters from client");
        }

        if(!send_complete)
        {
            error_handler("A serious error transfering data has occured");
        }

        complete_cv.notify_all();
    }


    // report back the results
    detection::test_results ret = value.get();
    ret.elapsed_time            = val;

    send(sock_fd, &ret, sizeof(ret), 0);

    process_data();
}

/**
 * @param timeout the time from now to stop execution
 * @param timeout_mutex the mutex protecting timeout_cv
 * @param timeout_cv condition variable to signal that timeout has occured
 */
detection::test_results co_op_udp_server::timer(std::chrono::seconds timeout)
{

    detection::test_results ret;
    std::mutex timeout_mutex;
    std::condition_variable timeout_cv;
    std::unique_lock<std::mutex> timeout_lk(timeout_mutex);
    bool complete = false;
    // spawn child threads
    std::thread t1(&co_op_udp_server::measure, this);

    std::thread t2(&co_op_udp_server::capture_traffic, this);

    // wait until they signal or timer expires
    timeout_cv.wait_until(timeout_lk, std::chrono::high_resolution_clock::now() + timeout);

    if(!complete)
    {
        // signal child threads to complete
    }

    // wait for child threads to complete


    t1.join();
    t2.join();

    return ret;
}

void co_op_udp_server::measure()
{
    // setup connection params

    // open socket

    // recv data

    // log all missing packets

    //exit
}

void co_op_udp_server::capture_traffic()
{
    // spawn child process -- tcpdump

    // wait to be signaled then kill child process -- tcpdump

    //exit
}

void co_op_udp_server::process_data()
{
    //write everything to a file

    // sync with database -- consider firing off child process that executes a script

    //exit
}


void co_op_udp_server::error_handler(std::string msg)
{
    // handle errors and report messages

    std::cerr << msg << std::endl;
}
