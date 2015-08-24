/*
 * unit_test.h
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
		virtual void SetUp(){
			char const *args[8] = {"ncd_main", "127.0.0.1", "-p9876", "-n3000", "-t64", "-w5", "-c3", "-v"};
			size_t sz = sizeof(*args)/sizeof(args);
			printf("%lu\n", sz);
			check_args(8, (char**)args);
		}
};


#endif /* UNIT_TEST_H_ */
