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
        /**
         * Represents a single detection session: an interactive connection where the goal is to determin if delay
         * discimination exissts on the transmission path
         */
        class server_session
        {
        public:
            /**
             * Constructor
             *
             * @param tcp_sock the TCP socket file descriptor from accept()
             * @param client_in the information about the client connection populated from accept()
             */
            server_session(int tcp_sock, sockaddr_in client_in);

            /**
             *  Destructor
             */
            virtual ~server_session();

            /**
             * Sets up the UDP socket
             *
             * @method setupUdpSocket
             *
             * @param  port           the UDP port number that communication will take place on-- given from test_params
             *
             * @return returns the socket file descriptor
             */
            int setupUdpSocket(uint16_t port);

            /**
             * when a UDP packe is availible this function processes it. It is called from the main event loop whe
             * select() signals that a UDP packet is availible
             *
             * @method processUdpPacket
             */
            void processUdpPacket();

            /**
             * Recieves the test parameters from the initial connection, and handles deserialization
             *
             * @method getTcpParams
             *
             * @return Deseralized test_params
             */
            test_params getTcpParams();

            /**
             * Processes the Connection Complete signal from the TCP Connection
             *
             * @method getTcpComplete
             *
             * @return Returns the value of the sent boolean
             */
            bool getTcpComplete();

            /**
             * Sends the test_results back to the client.
             *
             * @method sendResults
             */
            void sendResults();

            /**
             * Terminates the session due to an un recoverable error state
             *
             * currently simply terminates the entire server, but ideally should terminate only this single session
             *
             * @method terminate_session
             *
             * @param  msg               A string message printed to stderr prior to program termination
             */
            void terminate_session(std::string msg);

            /**
             * Closes the UDP connection file descriptor
             *
             * @method closeUdp
             */
            void closeUdp();

            /**
             * Closes the TCP connection file descriptor
             *
             * @method closeTcp
             */
            void closeTcp();

            /**
             * Runs the main event loop for the session
             *
             * @method run
             */
            void run();

            /**
             * Initaites tcp dump as a child process to support packet captures
             *
             * @method capture_traffic
             */
            void capture_traffic();

        private:
            test_params params;                /// the test parameters-- recieved from client
            test_results results;              /// the test results -- sent to client.
            int udp_fd;                        /// the udp file descriptor for receiving the data train
            int tcp_fd;                        /// the TCP file descriptor for receiving the test parameters
            bool udp_open;                     /// boolean for if the udp socket is still open
            bool tcp_open;                     /// boolean for if the tcp socket is still open
            uint16_t packets_received;         /// the number of UDP packet received so far
            boost::dynamic_bitset<> bs;        /// a bitset representing the packets that were received
            sockaddr_in client;                /// the client information used for the udp connection
            socklen_t client_len;              /// the length of the client struct
        };
    }        // end namespace server
}        // end namespace detection
#endif        // DETECTOR_UNIT_TEST_SESSION_HPP
