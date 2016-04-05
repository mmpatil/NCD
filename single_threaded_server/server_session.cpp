//
// Created by atlas on 3/30/16.
//

#include "server_session.hpp"
#include <chrono>
#include <iostream>
#include <signal.h>
#include <sstream>

#define PCAP_ON 1

namespace detection
{
    namespace server
    {
        /**
         * Calls a python script for getting a UUID for the pcap file to associate the measurement with
         *
         * @method get_pcap_id
         *
         * @param  expID       The UUID of the experiment.
         *
         * @return returns the UUID for the pcap file
         */
        uint16_t get_pcap_id(uint32_t expID)
        {
#if DEBUG
            std::cout << "Experimental ID = " << expID << std::endl;
            sleep(2);
#endif
            // create the command string to pass to popen()
            std::stringstream command;
            command << "~/experiment/pcap_script.py " << expID;

            // make the call to the external program to consult the MYSQL database
            FILE* in = popen(command.str().c_str(), "r");
            uint16_t pcap_id;

            // read results back
            fscanf(in, "%hu", &pcap_id);

            // close pipe
            pclose(in);

            return pcap_id;
        }

        server_session::server_session(int tcp_sock, sockaddr_in client_in) : client(client_in)
        {
            // assign the socket values and set their booleans
            tcp_fd   = tcp_sock;
            tcp_open = true;

            udp_fd   = 0;
            udp_open = false;

            client_len     = sizeof(client);
            must_terminate = false;
            packets_received = 0;
        }


        server_session::~server_session()
        {
            // close any open sockets on destruction
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
            udp_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if(udp_fd < 0)        // error state
            {
                terminate_session("failed to acquire socket for UDP data train");
                return -1;
            }

            udp_open = true;
            int val= 1;
            setsockopt(udp_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
            int err = bind(udp_fd, (sockaddr*)&serv_addr, sizeof(serv_addr));
            if(err < 0)        // error state
            {
                terminate_session("Failed to bind socket for data train");
                return -1;
            }

            // return the new socket file descriptor
            return udp_fd;
        }

        void server_session::processUdpPacket()
        {
            char recv_buff[1500] = {};
            uint16_t* id         = (uint16_t*)recv_buff;
            int n =
              recvfrom(udp_fd, recv_buff, sizeof(recv_buff), 0, reinterpret_cast<sockaddr*>(&client), &client_len);

            if(n < 0)        // error state
            {
                // any of these erro values are acceptable, and we can move onto the next packet
                if(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
                    return;

                // any other value represents an undefined behavior, and we must terminate the session
                terminate_session("recvfrom() in data train failed");
                return;
            }
            else
            {
                // when we receive a UDP packet, we must mark them as received
                packets_received++;
               // bs.set(ntohs(*id));
            }
        }

        test_params server_session::getTcpParams()
        {
            // initialize our results to a failure
            results.success = (uint16_t) false;

            // zero initialize tcp message buffer;
            char buff[1500] = {};


            // get experiment parameters from client
            int err = (int)recv(tcp_fd, &buff, sizeof(buff), 0);
            if(err < 0)        // error state
            {
                terminate_session("Failure receiving experimental parameters from client");
                return params;
            }

            // if we have enough bytes to read the test parameters we can proceed
            if(err >= (int)sizeof(test_params))
            {
                params.deserialize(buff);
            }
            else
            {
                // we do not handle partial data yet, and must terminate
                terminate_session("Failure receiving test parameters!!");
                return params;
            }
#if DEBUG
            std::cout << "Print prams a s a string:" << std::endl;
            std::cout << params << std::endl << std::endl;
#endif
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
            // zero initialize a fixed size buffer
            char buff[sizeof(test_results)] = {};

            // serialize the results into the send buffer
            results.serialize(buff);
            /*
            std::cout << "Results = " << results << std::endl;
            test_results* my_res = (test_results*)buff;
            std::cout << "sent Results = " << *my_res <<std::endl;
            */
            // transmit the results to the client
            int n = (int)send(tcp_fd, buff, sizeof(buff), 0);
            if(n < 0)
            {
                terminate_session("Error sending results to client");
                return;
            }
        }


        void server_session::terminate_session(std::string msg)
        {
            // print the error message to standard error
            std::cerr << msg << std::endl;

            // terminate the entire program -- consider only exiting the session
            must_terminate = true;
            // exit(-1);
        }

        void server_session::closeUdp()
        {
            // if the udp socket is open, close it
            if(udp_open)
            {
                close(udp_fd);
                udp_fd   = 0;
                udp_open = false;
            }
        }

        void server_session::closeTcp()
        {
            // if the tcp socket is stil open, close it
            if(tcp_open)
            {
                close(tcp_fd);
                tcp_fd   = 0;
                tcp_open = false;
            }
        }

        void server_session::run()
        {
            // structs for select()
            fd_set master;
            fd_set read_fds;
            int fd_max;

            // initialize the structs from select()
            FD_ZERO(&master);
            FD_ZERO(&read_fds);


            // set up server_session
            getTcpParams();
            if(must_terminate)
                return;

            setupUdpSocket(params.port);
            if(must_terminate)
                return;

            bool tcp_complete = false;

            fd_max = std::max(tcp_fd, udp_fd);

            FD_SET(tcp_fd, &master);
            FD_SET(udp_fd, &master);

#ifdef PCAP_ON
            // get packet traces

            pid_t child_id = fork();
            if(child_id == 0)
            {
                capture_traffic();
                _exit(-1);        // if wer're here its an error
            }
#endif
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
                    break;
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
            } while(!tcp_complete && (packets_received < params.num_packets) && !must_terminate);

            //CloseUdp();

            auto timestamp = std::chrono::high_resolution_clock::now() - marker;

            // give tcpdump time to handle things;
            sleep(1);
#ifdef PCAP_ON
            kill(child_id, SIGINT);
#endif
            // if we aren't in an error state, send good results, otherwise, send failure
            if(!must_terminate)
            {
                // prepare results
                results.success = (uint16_t )true;
                results.pcap_id = get_pcap_id(params.test_id);
                results.elapsed_time =
                  std::chrono::duration_cast<std::chrono::nanoseconds>(timestamp).count() / 1000000.0;
                results.lostpackets = params.num_packets - packets_received;
#if DEBUG
                std::cout << "Didn't have to terminate" << std::endl;
                std::cout << "Pcap ID = " << results.pcap_id << std::endl;
#endif
#ifdef PCAP_ON
                std::stringstream ss;
                ss << results.pcap_id << ".pcap";
                rename("temp.pcap", ss.str().data());
#endif
            }

            // send results
            sendResults();
        }

        void server_session::capture_traffic()
        {
            std::ostringstream convert;
            convert << params.port;
            std::string port = convert.str();

            execl("/usr/sbin/tcpdump", "/usr/sbin/tcpdump", "-i", "any", "udp and port ", port.data(), "-w",
                  "temp.pcap", (char*)0);

            _exit(0);
        }
    }
}        // end namespace detection
