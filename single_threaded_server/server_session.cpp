//
// Created by atlas on 3/30/16.
//

#include "server_session.hpp"
#include <chrono>
#include <iostream>
#include <sstream>

namespace detection
{
    namespace server
    {
        uint32_t get_pcap_id(uint32_t expID)
        {
            std::cout << "Experimental ID = " << expID << std::endl;
            sleep(2);
            std::stringstream command;
            command << "~/experiment/pcap_script.py " << expID;
            // command <<"python pcap_script.py "  << expID;
            FILE* in = popen(command.str().c_str(), "r");
            uint32_t pcap_id;
            fscanf(in, "%u", &pcap_id);
            pclose(in);

            return pcap_id;
        }

        server_session::server_session(int tcp_sock, sockaddr_in client_in) : client(client_in)
        {
            tcp_fd   = tcp_sock;
            tcp_open = true;

            udp_fd   = 0;
            udp_open = false;

            client_len = sizeof(client);
        }


        server_session::~server_session()
        {
            closeTcp();
            closeUdp();
        }

        int server_session::setupUdpSocket(uint16_t port)
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
                terminate_session("failed to acquire socket for UDP data train");
            }

            int err = bind(udp_fd, (sockaddr*)&serv_addr, sizeof(serv_addr));
            if(err < 0)
            {
                terminate_session("Failed to bind socket for data train");
            }

            return udp_fd;
        }

        void server_session::processUdpPacket()
        {
            char recv_buff[1500] = {};
            uint16_t* id         = (uint16_t*)recv_buff;
            int n =
              recvfrom(udp_fd, recv_buff, sizeof(recv_buff), 0, reinterpret_cast<sockaddr*>(&client), &client_len);

            if(n < 0)
            {
                if(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
                    return;
                terminate_session("recvfrom() in data train failed");
            }
            else
            {
                packets_received++;
                bs.set(ntohs(*id));
            }
        }

        test_params server_session::getTcpParams()
        {
            results.success = (uint16_t) false;

            // zero initialize tcp message buffer;
            char buff[1500] = {};


            // get experiment parameters from client
            int err = (int)recv(tcp_fd, &buff, sizeof(buff), 0);
            if(err < 0)
            {
                terminate_session("Failure receiving experimental parameters from client");
            }

            if(err >= (int)sizeof(test_params))
            {
                params.deserialize(buff);
            }
            else
            {
                terminate_session("Failure receiving test parameters!!");
            }

            // std::cout << "Print prams a s a string:" <<std::endl;
            // std::cout << params <<std::endl << std::endl;

            return params;
        }

        bool server_session::getTcpComplete()
        {
            // do the tcp_recieve;
            bool stop = false;
            int err   = (int)recv(tcp_fd, &stop, sizeof(stop), 0);
            if(err < 0)
            {
                terminate_session("Failure receiving stop signal from client");
                return false;
            }
            return stop;
        }

        void server_session::sendResults()
        {
            char buff[sizeof(test_results)] = {};
            results.serialize(buff);
            int n = (int)send(tcp_fd, buff, sizeof(buff), 0);
            if(n < 0)
            {
                terminate_session("Error sending results to client");
            }
        }


        void server_session::terminate_session(std::string msg)
        {
            std::cerr << msg << std::endl;
            exit(-1);
        }

        void server_session::closeUdp()
        {
            if(udp_open)
            {
                close(udp_fd);
                udp_fd   = 0;
                udp_open = false;
            }
        }

        void server_session::closeTcp()
        {
            if(tcp_open)
            {
                close(tcp_fd);
                tcp_fd   = 0;
                tcp_open = false;
            }
        }

        void server_session::run()
        {
            fd_set master;
            fd_set read_fds;
            int fd_max;

            FD_ZERO(&master);
            FD_ZERO(&read_fds);


            // set up server_session
            getTcpParams();
            setupUdpSocket(params.port);
            bool tcp_complete = false;

            fd_max = std::max(tcp_fd, udp_fd);

            FD_SET(tcp_fd, &master);
            FD_SET(udp_fd, &master);

            int err;
            // main receive loop
            auto marker = std::chrono::high_resolution_clock::now();
            do
            {
                read_fds = master;        // copy it
                err      = select(fd_max + 1, &read_fds, NULL, NULL, NULL);
                if(err < 0)
                {
                    terminate_session("select() has errored...");
                }

                for(int i = 0; i <= fd_max; ++i)
                {
                    if(FD_ISSET(i, &read_fds))
                    {
                        if(i == tcp_fd)
                        {
                            tcp_complete = getTcpComplete();
                        }
                        else if(i == udp_fd)
                        {
                            processUdpPacket();
                        }        // end if
                    }
                }        // end for
            } while(!tcp_complete && (packets_received < params.num_packets));


            auto timestamp = std::chrono::high_resolution_clock::now() - marker;
            // prepare results
            results.success      = true;
            results.pcap_id      = get_pcap_id(params.test_id);
            results.elapsed_time = std::chrono::duration_cast<std::chrono::nanoseconds>(timestamp).count() / 1000000.0;
            results.lostpackets  = params.num_packets - packets_received;

            // send results
            sendResults();
        }
    }
}        // end namespace detection
