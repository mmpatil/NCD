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

#include <arpa/inet.h>
#include <signal.h>
#include <sstream>

#include "co_op_data.hpp"
#include "co_op_udp_server.hpp"

using namespace detection;


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

class experiment
{
public:
    experiment(const detection::test_params& params_in, const sockaddr_in& cliaddr) : params(params_in), client(cliaddr)
    {
    }
    virtual ~experiment()
    {
        send_complete = false;
        abort_session = false;
    }

    bool run()
    {
#if 1
        measure();
#else
        timer(std::chrono::seconds(60));
        std::cout << "Timer has exited\n";
#endif
        std::lock_guard<std::mutex> abt_lk(abort_mutex);
        return !abort_session;
    }

    /**
     * @param timeout the time from now to stop execution
     * @param timeout_mutex the mutex protecting timeout_cv
     * @param timeout_cv condition variable to signal that timeout has occured
     */
    detection::test_results timer(std::chrono::seconds timeout)
    {
        std::unique_lock<std::mutex> timeout_lk(complete_mutex);

        // spawn child threads
        // std::thread t1(&experiment::measure, this);
        auto measure_future = std::async(std::launch::async, &experiment::measure, this);
// std::thread t2(&experiment::capture_traffic, this);
#if 0
        // wait until they signal or timer expires
        complete_cv.wait_until(timeout_lk, std::chrono::high_resolution_clock::now() + timeout);

        if(!send_complete)
        {
            // signal child threads to complete
            std::lock_guard<std::mutex> abort_guard(abort_mutex);
            abort_session = true;
            abort_cv.notify_all();
            tcpdump_cv.notify_all();
        }

        timeout_lk.release();
#endif
        // wait for child threads to complete
        // t1.join();
        measure_future.wait_for(timeout);

        // t2.join();
        std::cout << "Timer is Exiting..\n";
        return results;
    }

    void measure()
    {
        char buff[1500];

        // setup connection params
        sockaddr_in serv_addr = {};

        serv_addr.sin_family      = AF_INET;
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_addr.sin_port        = htons(params.port);

        int udp_fd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, IPPROTO_UDP);
        if(udp_fd < 0)
        {
            error_handler("failed to acquire socket for UDP data train");
        }
        std::cout << "UDP port number: " << params.port << std::endl;
        std::cout << "Packets to expect: " << params.num_packets << std::endl;
        std::cout << "Value of send_complete: " << (send_complete ? "true" : "false") << std::endl;
        int err = bind(udp_fd, (sockaddr*)&serv_addr, sizeof(serv_addr));

        if(err < 0)
        {
            error_handler("Failed to bind socket for data train");
        }

        socklen_t client_len = sizeof(client);

        // recv data
        uint32_t packets_received = 0;
        boost::dynamic_bitset<> bitset(params.num_packets);
        uint16_t* id = reinterpret_cast<uint16_t*>(buff + params.offset);
        int n;
        while(packets_received < params.num_packets)
        {

            std::lock_guard<std::mutex> complete_guard(complete_mutex);
            if(send_complete)
                break;


            {
                std::lock_guard<std::mutex> guard(abort_mutex);        // acquire lock
                if(abort_session)
                {
                    close(udp_fd);
                    break;
                }
            }

            n = recvfrom(udp_fd, buff, sizeof(buff), 0, reinterpret_cast<sockaddr*>(&client), &client_len);

            if(n < 0)
            {
                if(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
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
        close(udp_fd);

        {
            std::lock_guard<std::mutex> lk(complete_mutex);
            send_complete = true;
            complete_cv.notify_all();
            tcpdump_cv.notify_all();
        }

        // log all missing packets
        std::string packet_state;
        to_string(bitset, packet_state);

        results.lostpackets = params.num_packets - packets_received;
        std::cout << "Packets recived: " << packets_received << std::endl;
        std::cout << "Exiting Meausre()" << std::endl;

        // results.lost_string =packet_state;
        // results.bitset = bitset;

        // exit
    }

    void capture_traffic()
    {
        // spawn child process -- tcpdump
        pid_t tcpdump_id = fork();

        std::ostringstream convert;
        convert << params.port;
        std::string port = convert.str();
        // std::stringstream str;
        // str << " -i eth0 "<< " src ip " << inet_ntoa(client.sin_addr) <<" and (udp dest port " << port << " and src
        // port " << client.sin_port << ")";
        if(tcpdump_id == 0)
        {
            execl("/usr/sbin/tcpdump", "/usr/sbin/tcpdump", "-i", "lo", "udp and port ", port.data(), "-w", "temp.pcap",
                  (char*)0);
            return;
        }

        // std::cout << "Waiting to kill tcpdump..." << std::endl;
        // wait to be signaled
        std::unique_lock<std::mutex> lk(tcpdump_mutex);
        tcpdump_cv.wait(lk, [this]() { return this->abort_session || this->send_complete; });

        lk.release();
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // then kill child process -- tcpdump
        kill(tcpdump_id, SIGINT);
        // std::cout << "killed tcpdump" << std::endl;

        // exit
    }

    void complete()
    {
        std::lock_guard<std::mutex> lk(complete_mutex);
        if(!send_complete)
        {
            send_complete = true;
            complete_cv.notify_all();
            tcpdump_cv.notify_all();
        }
    }


    void error_handler(std::string msg)
    {
        // handle errors and report messages
        std::lock_guard<std::mutex> guard(abort_mutex);
        abort_session = true;
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
    bool abort_session;        // used when signaling child threads to abort execution
    std::mutex tcpdump_mutex;
    std::condition_variable tcpdump_cv;

    sockaddr_in client;
};

// global constants
const int port         = 15555;
const int time_to_quit = 60;
const std::chrono::seconds max_time(time_to_quit);

co_op_udp_server::co_op_udp_server()
{
    listen_fd     = 0;
    open          = false;
    abort_session = false;
}


co_op_udp_server::~co_op_udp_server()
{
    if(open)
        close(listen_fd);
}

void co_op_udp_server::listener()
{
    // start listening on a specific , non-reserved port
    listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(listen_fd < 0)
    {
        std::ios_base::failure e("Failed to acquire socket for listen()");
        throw e;
    }
    open = true;

    int val;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    // setup struct for server address
    sockaddr_in serv_addr     = {};
    sockaddr_in client_addr   = {};
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port        = htons(port);
    int err                   = bind(listen_fd, (sockaddr*)&serv_addr, sizeof(serv_addr));

    if(err < 0)
    {
        std::ios_base::failure e("Failed to bind socket for listen()");
        throw e;
    }

    listen(listen_fd, 4);
    socklen_t client_len = 0;

    while(true)
    {
        if(this->abort_session)
            break;
        int temp_fd = accept(listen_fd, (sockaddr*)&client_addr, &client_len);
        std::thread th(&co_op_udp_server::process_udp, this, temp_fd, client_addr);
        th.detach();
    }
    close(listen_fd);
    open = false;
}

void co_op_udp_server::process_udp(int sock_fd, sockaddr_in client)
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

    experiment exp(*params, client);


    bool send_complete = false;

    auto marker = std::chrono::high_resolution_clock::now();
    // spawn worker threads
    auto value = std::async(std::launch::async, &experiment::run, &exp);

    double val = 0;
    // listen for when send is complete and signal that it is
    while(!send_complete)
    {
        err = recv(sock_fd, &send_complete, sizeof(send_complete), 0);
        if(err < 0)
        {
            error_handler("Failure receiving experimental parameters from client");
        }
    }

    exp.complete();

    // timestamp ASAP -- even before reporting errors
    auto timestamp = std::chrono::high_resolution_clock::now() - marker;
    val            = std::chrono::duration_cast<std::chrono::nanoseconds>(timestamp).count() / 1000000.0;


    if(!send_complete)
    {
        error_handler("A serious error transferring data has occurred");
    }

    std::cout << "send is complete... waiting for future results\n";
    // report back the results
    detection::test_results ret = exp.get_results();
    ret.success                 = value.get();        // synchronization point with call to async
    ret.elapsed_time            = val;

    if(params->test_id == 0)
    {
        std::cerr << "TEST ID has no value!!!!" << std::endl;
        throw;
    }

    ret.pcap_id = get_pcap_id(params->test_id);

    send(sock_fd, &ret, sizeof(ret), 0);
    close(sock_fd);
    std::cout << "process udp is exiting, sock_fd is closed..." << std::endl;
    // process_data();
}


void co_op_udp_server::error_handler(std::string msg)
{        // handle errors and report messages

    std::cerr << msg << std::endl;
    abort_session = true;
}


/**
 * maybe not needed.... we might not need to process the file outside of executing an external script
 */
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
