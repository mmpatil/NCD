//
// Created by atlas on 3/5/16.
//



#include <gtest/gtest.h>

#include "../include/ip_checksum.h"

TEST(get_time_test, get_time_correct)
{
    struct timeval tv;
    double d, r;
    d = get_time();
    gettimeofday(&tv, NULL);
    r = tv.tv_sec;
    r += tv.tv_usec / 1000000;
    EXPECT_FLOAT_EQ(d, r);
}


int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}