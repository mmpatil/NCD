//
// Created by atlas on 3/29/16.
//

#ifndef SINGLE_THREADED_SERVER_HPP_
#define SINGLE_THREADED_SERVER_HPP_

#include <sstream>
#include "co_op_data.hpp"

class SingleThreadedServer
{
public:
    SingleThreadedServer();
    void run();
    void acceptor();

    void receive_train(detection::test_params params, sockaddr_in client);
    void terminate(std::string msg);
    void send_tcp_reply(int sock_fd, detection::test_results results);
    detection::test_results receive_tcp_parameters(int sock_fd);
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

};


#endif //SINGLE_THREADED_SERVER_HPP_
