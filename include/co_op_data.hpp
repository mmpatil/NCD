//
// Created by atlas on 3/17/16.
//

#ifndef DETECTOR_UNIT_TEST_CO_OP_DATA_HPP
#define DETECTOR_UNIT_TEST_CO_OP_DATA_HPP

#include <cstdint>
#include <ostream>

namespace detection
{
    struct test_results
    {
        bool success;
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
        uint64_t test_id;
        bool last_train;
    };
}


std::ostream& operator<<(std::ostream& os, const detection::test_results& res)
{
    std::string s = res.success ? "true" : "false";

    os << res.lostpackets << " " << res.elapsed_time << s;
    return os;
}

std::ostream& operator<<(std::ostream& os, const detection::test_params& par)
{
    std::string s = par.last_train ? "Last" : "not last";

    os << par.num_packets << " " << par.payload_size << " " << par.port << " " << par.offset << " " << s;
    return os;
}
#endif        // DETECTOR_UNIT_TEST_CO_OP_DATA_HPP
