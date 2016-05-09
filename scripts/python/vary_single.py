#! /usr/bin/env python

import subprocess

if __name__ == "__main__":
    num_packets = range(400, 8000, 200)
    packet_sizes = [1024, 1400]
    num_tail = range(20, 5000, 10)
    tail_wait = range(2,20,2)
    args = []
    for size in num_packets:
        pack_arg = "--num_packets="+ str(size)
        for length in packet_sizes:
            len_arg = "--data_length=" + str(length)
            for tail in num_tail:
                tail_arg = "--num_tail=" +str(tail)
                for wait in tail_wait:
                    wait_arg = "--tail_wait=" +str(wait)
                    full_args = "--dport_disc=33333 " + len_arg + " " + pack_arg + " " + tail_arg + " " + wait_arg
                    args.append(full_args)

    for parameters in args:
        subprocess.call("./ExperimentSQL.py 'Non-cooperative Compression Parameters' " + parameters, shell=True)
