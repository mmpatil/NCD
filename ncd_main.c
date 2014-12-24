/*
 * ncd_main.c
 *
 *  Created on: Dec 23, 2014
 *      Author: atlas
 */

#include "ncd.h"

/**
 * Main function
 * only calls comp_detection()
 *
 * @argv[1] Destination IP address
 * @argv[2] Port Number
 * @argv[3] High or low entropy data 'H' or 'L'
 * @argv[4] Size of udp data
 * @argv[5] Number of packets in UDP Data Train
 * @argv[6] Time to Live
 * @argv[7] Wait time in milliseconds
 * @argv[8] Number of tail ICMP messages to send
 *
 */
int main(int argc, char *argv[])
{
	/* Check ARGs */
	if(check_args(argc, argv) != EXIT_SUCCESS)
		return EXIT_FAILURE;
	return comp_det();
}
