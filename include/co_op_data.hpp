//
// Created by atlas on 3/17/16.
//

#ifndef DETECTOR_UNIT_TEST_CO_OP_DATA_HPP
#define DETECTOR_UNIT_TEST_CO_OP_DATA_HPP

#include <cstdint>


namespace detection
{
    struct test_results
    {
        double elapsed_time;
        uint32_t lostpackets;
        // char losses[512];
    };

    struct test_params
    {
        uint32_t num_packets;
        uint32_t payload_size;
        uint16_t port;
        uint16_t offset;
    };
}
#endif        // DETECTOR_UNIT_TEST_CO_OP_DATA_HPP
