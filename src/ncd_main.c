/*
 * ncd_main.c
 *
 *  Created on: Dec 23, 2014
 *      Author: Paul Kirth
 */

#include "ncd.h"

/**
 * Main function
 * checks commandline args with check_args, then calls detect()
 */
int main(int argc, char *argv[])
{
        /* Check ARGs */
        if(check_args(argc, argv) != EXIT_SUCCESS)
                return EXIT_FAILURE;
    return detect();
}
