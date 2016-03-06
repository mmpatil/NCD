/*
 * ncd_unit_test.hpp
 *
 *  Created on: Dec 23, 2014
 *      Author: atlas
 */

#ifndef UNIT_TEST_H_
#define UNIT_TEST_H_

extern "C" {
#include "ncd.h"
}

class DetectionInitTest : public ::testing::Test
{
  protected:
    virtual void SetUp()
    {
        char const* args[6] = {"ncd_main", "127.0.0.1", "-n3000", "-t64", "-w5", "-c3"};
        size_t sz = sizeof(args) / sizeof(args[0]);
        check_args(sz, (char**)args);
    }
};

class MeasureUdpTest : public ::testing::Test
{
  protected:
    virtual void SetUp()
    {
        char const* args[6] = {"fake program!!!", "127.0.0.1", "-n4786", "-t33", "-w9", "-c7"};
        size_t sz = sizeof(args) / sizeof(args[0]);
        check_args(sz, (char**)args);
    }
};


class DetectUdpTest : public ::testing::Test
{
  protected:
    virtual void SetUp()
    {
        char const* args[6] = {"fake program!!!", "127.0.0.1", "-n2200", "-t64", "-w4", "-c2"};
        size_t sz = sizeof(args) / sizeof(args[0]);
        check_args(sz, (char**)args);
    }
};


#endif /* UNIT_TEST_H_ */
