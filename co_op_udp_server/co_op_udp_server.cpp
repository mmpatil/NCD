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
#include <boost/dynamic_bitset.hpp>
#include <condition_variable>
#include <fstream>
#include <future>
#include <iostream>
#include <netinet/in.h>
#include <signal.h>
#include <sstream>
#include <unistd.h>

#include "co_op_data.hpp"
#include "co_op_udp_server.hpp"

using namespace detection;

class experiment
{
public:
    experiment(const detection::test_params& params_in) : params(params_in) {}
    virtual ~experiment() {}
    bool run()
    {
        std::lock_guard<std::mutex> abt_lk(abort_mutex);
        return !abort;
    }

    /**
     * @param timeout the time from now to stop execution
     * @param timeout_mutex the mutex protecting timeout_cv
     * @param timeout_cv condition variable to signal that timeout has occured
     */
    detection::test_results timer(std::chrono::seconds timeout)
    {

        std::mutex timeout_mutex;
        std::condition_variable timeout_cv;
        std::unique_lock<std::mutex> timeout_lk(timeout_mutex);
        bool complete = false;
        // spawn child threads
        std::thread t1(&experiment::measure, this);

        std::thread t2(&experiment::capture_traffic, this);

        // wait until they signal or timer expires
        timeout_cv.wait_until(timeout_lk, std::chrono::high_resolution_clock::now() + timeout);

        if(!complete)
        {
            // signal child threads to complete
            std::lock_guard<std::mutex> abort_guard(abort_mutex);
            abort = true;
            abort_cv.notify_all();
        }

        // wait for child threads to complete


        t1.join();
        t2.join();

        return results;
    }

    void measure()
    {
        char buff[1500];

        // setup connection params
        sockaddr_in serv_addr   = {0};
        sockaddr_in client_addr = {};

        serv_addr.sin_family      = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port        = htons(params.port);

        int udp_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if(udp_fd < 0)
        {
            error_handler("failed to aquire socket for UDP data train");
        }

        int err = bind(udp_fd, (sockaddr*)&serv_addr, sizeof(serv_addr));
        if(err < 0)
        {
            error_handler("Failed to bind socket for listen()");
        }

        socklen_t client_len = sizeof(client_addr);

        // recv data
        uint32_t packets_received = 0;
        boost::dynamic_bitset<> bitset(params.num_packets);
        uint16_t* id = reinterpret_cast<uint16_t*>(buff + params.offset);
        int n;
        while(!send_complete && (packets_received < params.num_packets))
        {

            {
                std::lock_guard<std::mutex> guard(abort_mutex);        // acquire lock
                if(abort)
                    return;
            }

            n = recvfrom(udp_fd, buff, sizeof(buff), 0, reinterpret_cast<sockaddr*>(&client_addr), &client_len);

            if(n < 0)
            {
                if(errno == EINTR)
                    continue;
                std::cerr << "recvfrom() in data train failed" << std::endl;
                continue;
            }
            else
            {
                packets_received++;
                bitset.set(*id);
            }
        }

        {
            std::lock_guard<std::mutex> lk(complete_mutex);
            send_complete = true;
        }

        complete_cv.notify_all();

        // log all missing packets
        std::string packet_state;
        to_string(bitset, packet_state);

        results.lostpackets = params.num_packets - packets_received;
        // results.lost_string =packet_state;
        // results.bitset = bitset;

        // exit
    }

    void capture_traffic()
    {
        // spawn child process -- tcpdump
        pid_t id = fork();

        std::ostringstream convert;
        convert << params.port;
        std::string port = convert.str();
        if(id == 0)
        {
            execl("/usr/sbin/tcpdump", "/usr/sbin/tcpdump", "-i", "eth0", "udp", "port ", port.data(), "-w",
                  "temp.pcap", (char*)0);
        }

        // wait to be signaled
        std::unique_lock<std::mutex> lk(complete_mutex);
        complete_cv.wait(lk, [this]() {
            std::lock_guard<std::mutex> mylk(this->abort_mutex);
            return this->send_complete && !this->abort;
        });

        lk.release();

        // then kill child process -- tcpdump
        kill(id, SIGINT);

        // exit
    }

    void complete()
    {
        if(!send_complete)
        {
            std::lock_guard<std::mutex> lk(complete_mutex);
            send_complete = true;
            complete_cv.notify_all();
        }
    }


    void error_handler(std::string msg)
    {
        // handle errors and report messages
        std::lock_guard<std::mutex> guard(abort_mutex);
        abort = true;
        std::cerr << msg << std::endl;

        abort_cv.notify_all();
    }

    detection::test_results get_results() { return results; }

private:
    std::mutex complete_mutex;
    std::condition_variable complete_cv;
    bool send_complete = false;
    detection::test_params params;
    detection::test_results results;
    std::mutex abort_mutex;
    std::condition_variable abort_cv;
    bool abort;        // used when signaling child threads to abort execution
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

    test_params* params = reinterpret_cast<test_params*>(buff);


    // get experiment parameters from client
    long err = recv(sock_fd, &buff, sizeof(buff), 0);
    if(err < 0)
    {
        error_handler("Failure receiving experimental parameters from client");
        return;
    }

    experiment exp(*params);

    bool send_complete = false;

    auto marker = std::chrono::high_resolution_clock::now();
    // spawn worker threads
    auto value = std::async(&experiment::run, &exp);

    double val = 0;
    // listen for when send is complete and signal that it is
    err = recv(sock_fd, &send_complete, sizeof(send_complete), 0);

    exp.complete();

    // timestamp ASAP -- even before reporting errors
    auto timestamp = std::chrono::high_resolution_clock::now() - marker;
    val            = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp).count();

    if(err < 0)
    {
        error_handler("Failure receiving experimental parameters from client");
    }

    if(!send_complete)
    {
        error_handler("A serious error transferring data has occurred");
    }


    // report back the results
    detection::test_results ret = exp.get_results();
    ret.success                 = value.get();
    ret.elapsed_time            = val;

    send(sock_fd, &ret, sizeof(ret), 0);

    process_data();
}


void co_op_udp_server::error_handler(std::string msg)
{        // handle errors and report messages

    std::cerr << msg << std::endl;
}

void co_op_udp_server::process_data()
{
    // write everything to a file
    std::ofstream out("results.txt", std::ofstream::out | std::ofstream::app);
    test_params p;
    test_results res;
    out << p << " " << res;


    out.close();        // close file

    std::cout << "Done processing data ....\n";

    // sync with database -- consider firing off child process that executes a script

    // exit
    if(p.last_train)
    {
        // we don't need the file anymore, remove it.
        // std::ofstream o("results.txt")
    }
}
