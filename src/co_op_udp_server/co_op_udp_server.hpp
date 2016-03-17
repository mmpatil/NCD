//
// Created by atlas on 3/16/16.
//

#ifndef DETECTOR_UNIT_TEST_CO_OP_UDP_SERVER_HPP
#define DETECTOR_UNIT_TEST_CO_OP_UDP_SERVER_HPP


class co_op_udp_server {

public:
    co_op_udp_server();
    virtual ~co_op_udp_server(){}

    void listener();
    void process_udp();
    void timer();
    void measure();
    void capture_traffic();
    void process_data();
    void sync_ids();

private:



};


#endif //DETECTOR_UNIT_TEST_CO_OP_UDP_SERVER_HPP
