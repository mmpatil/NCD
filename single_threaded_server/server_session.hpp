//
// Created by atlas on 3/30/16.
//

#ifndef SESSION_HPP_
#define SESSION_HPP_

#include "co_op_data.hpp"
#include <boost/dynamic_bitset.hpp>

namespace detection
{
    namespace server
    {
        class server_session
        {
        public:
            server_session(int tcp_sock, sockaddr_in client_in);

            virtual ~server_session();

            int setupUdpSocket(uint16_t port);

            void processUdpPacket();

            test_params getTcpParams();

            bool getTcpComplete();

            void sendResults();

            void terminate_session(std::string msg);

            void closeUdp();

            void closeTcp();

            void run();

            void capture_traffic();

        private:
            test_params params;
            test_results results;
            int udp_fd;
            int tcp_fd;
            bool udp_open;
            bool tcp_open;

            uint16_t packets_received;
            boost::dynamic_bitset<> bs;
            sockaddr_in client;
            sockaddr_in server;        // maybe we can remove this...
            socklen_t client_len;
        };
    }        // end namespace server
}        // end namespace detection
#endif        // DETECTOR_UNIT_TEST_SESSION_HPP
