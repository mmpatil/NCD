//
// Created by atlas on 3/29/16.
//

#include "SingleThreadedServer.hpp"
#include "server_session.hpp"
#include <arpa/inet.h>
#include <boost/dynamic_bitset.hpp>
#include <chrono>
#include <fcntl.h>
#include <iostream>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>
using namespace detection;

SingleThreadedServer::SingleThreadedServer()
{
}

void SingleThreadedServer::run()
{
    acceptor();
}

void SingleThreadedServer::acceptor()
{
    // pid_t fork_id;
    uint16_t port = 15555;
    // start listening on a specific , non-reserved port
    listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(listen_fd < 0)
    {
        std::ios_base::failure e("Failed to acquire socket for listen()");
        throw e;
    }

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
        close(listen_fd);
        terminate("Failed to bind socket for listen()");
    }

    listen(listen_fd, 4);
    socklen_t client_len = 0;

    //    while(true)
    {
        int n = 0;
        while(n < 2)
        {
            int temp_fd = accept(listen_fd, (sockaddr*)&client_addr, &client_len);
            if(temp_fd < 0)
            {
                terminate("Error: the call to accept() has failed...");
                exit(-1);
            }
            server::server_session s(temp_fd, client_addr);
            s.run();
            n++;
        }
        //        close(temp_fd);
    }
    close(listen_fd);
}


void SingleThreadedServer::receive_train(test_params params, sockaddr_in client)
{
    char buff[1500];
    test_results results = {};
    results.success      = false;


    // setup connection params
    sockaddr_in serv_addr = {};

    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port        = htons(params.port);

    int udp_fd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, IPPROTO_UDP);
    if(udp_fd < 0)
    {
        terminate("failed to acquire socket for UDP data train");
    }
    std::cout << "UDP port number: " << params.port << std::endl;
    std::cout << "Packets to expect: " << params.num_packets << std::endl;
    int err = bind(udp_fd, (sockaddr*)&serv_addr, sizeof(serv_addr));

    if(err < 0)
    {
        terminate("Failed to bind socket for data train");
    }

    socklen_t client_len = sizeof(client);

    // recv data
    uint32_t packets_received = 0;
    boost::dynamic_bitset<> bitset(params.num_packets);
    uint16_t* id = reinterpret_cast<uint16_t*>(buff + params.offset);
    int n;
    bool send_complete = false;

    auto marker = std::chrono::high_resolution_clock::now();
    while(!send_complete && packets_received < params.num_packets)
    {
#if 0
        n = read(send_complete_fd[1], &send_complete, sizeof(send_complete));
        if(n < 0)
        {
            if(errno != EAGAIN || errno != EWOULDBLOCK)
            {
                terminate("Error reading send complete from pipe!");
                write(send_complete_fd[1], &results, sizeof(results));
                return;
            }
        }
#endif

        if(!send_complete)
        {
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
                bitset.set(ntohs(*id));
            }
        }
    }

    auto timestamp       = std::chrono::high_resolution_clock::now() - marker;
    results.elapsed_time = std::chrono::duration_cast<std::chrono::nanoseconds>(timestamp).count() / 1000000.0;

    close(udp_fd);

    // log all missing packets
    std::string packet_state;
    to_string(bitset, packet_state);

    results.lostpackets = params.num_packets - packets_received;
    std::cout << "Packets recived: " << packets_received << std::endl;
    std::cout << "Exiting Meausre()" << std::endl;

    write(send_complete_fd[1], &results, sizeof(results));
    _exit(0);
}

test_results SingleThreadedServer::receive_tcp_parameters(int sock_fd, sockaddr_in client)
{
    test_results t = {};
    t.success      = false;
    pipe2(send_complete_fd, O_NONBLOCK);
    // zero initialize tcp message buffer;
    char buff[1500] = {};

    // test_params* params = reinterpret_cast<test_params*>(buff);
    test_params params;

    // get experiment parameters from client
    int err = recv(sock_fd, &buff, sizeof(buff), 0);
    if(err < 0)
    {
        terminate("Failure receiving experimental parameters from client");
        return t;
    }

    if(err >= (int)sizeof(test_params))
    {
        params.deserialize(buff);
    }
    else
    {
        terminate("Failure receiving test parameters!!");
        return t;
    }

    setup_udp_socket(params.port);

#if 0
    bool send_complete;
    do{
        // read in from all availible sockets

        // select socket

        // if its tcp socket, read in send complete

        // if its the udp socket, read in
        //increment the counters, etc.


       if(send_complete)
           break;
    }while(!send_complete);
#endif


    pid_t capture_id = fork();
    if(capture_id == 0)
    {
        // child process
        capture_packets(params);
        _exit(0);
    }


    pid_t fork_id = fork();
    if(fork_id == 0)
    {
        // child logic;
        // do the udp train.
        receive_train(params, client);
        _exit(0);
        // maybe read from pipe to synchronize....
        // return t;
    }
    else
    {
        // parent logic;
        // do the tcp_recieve;
        bool stop;
        err = recv(sock_fd, &stop, sizeof(stop), 0);
        if(err < 0)
        {
            terminate("Failure receiving stop signal from client");
            return t;
        }
        write(send_complete_fd[0], &stop, sizeof(stop));
        int n;
        bool send_tcp = false;
        do
        {
            n = read(send_complete_fd[0], &t, sizeof(t));
            if(n < 0)
            {
                if(errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    continue;
                }
                terminate("Error reading test results from pipe");
                return t;
            }
            else
            {
                send_tcp = true;
            }
        } while(!send_tcp);

        t.pcap_id = get_pcap_id(params.test_id);
        std::stringstream ss;
        ss << t.pcap_id << ".pcap";
        rename("temp.pcap", ss.str().c_str());

        return t;
    }

    // experiment exp(*params, client);


    // bool send_complete = false;
}

void SingleThreadedServer::terminate(std::string msg)
{
    std::cerr << msg << std::endl;
    exit(-1);
}

void SingleThreadedServer::send_tcp_reply(int sock_fd, test_results results)
{
    char buff[sizeof(test_results)] = {};
    results.serialize(buff);
    int n = send(sock_fd, buff, sizeof(buff), 0);
    if(n < 0)
    {
        terminate("Error sending results to client");
    }
}

void SingleThreadedServer::capture_packets(test_params params)
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
        _exit(0);
    }

    // std::cout << "Waiting to kill tcpdump..." << std::endl;
    // wait to be signaled
    bool stop = false;
    int n;
    do
    {
        n = read(capture_fd[1], &stop, sizeof(stop));
        if(n < 0)
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK)
                continue;
            terminate("error encountered reading terminate signal for packet capture from pipe");
        }
    } while(!stop);
    sleep(1);
    // then kill child process -- tcpdump
    kill(tcpdump_id, SIGINT);
    _exit(0);
    // std::cout << "killed tcpdump" << std::endl;
}

int SingleThreadedServer::setup_udp_socket(uint16_t port)
{
    // setup connection params
    sockaddr_in serv_addr = {};

    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port        = htons(port);

    // int udp_fd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, IPPROTO_UDP);
    int udp_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(udp_fd < 0)
    {
        terminate("failed to acquire socket for UDP data train");
    }

    int err = bind(udp_fd, (sockaddr*)&serv_addr, sizeof(serv_addr));
    if(err < 0)
    {
        terminate("Failed to bind socket for data train");
    }

    return udp_fd;
}
