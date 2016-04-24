#! /bin/env python

import os.system

if __name__ == "__main__":
    large_num_packets = range(400, 8000, 200)
    large_packet_sizes = [512, 1024, 1400]
    small _num_packets = range(4000, 50000, 1000)
    small_packet_sizes = [32, 64, 128, 256]

    args = []
    for size in large_num_packets:
        pack_arg = "--num_packets="+ str(size)
        for length in large_packet_sizes:
            len_arg = "--data_length=" + str(length)
            full_args = "--dport_disc=22222 " + len_arg + " " + pack_arg
            args.append(full_args)

    for size in small_num_packets:
        pack_arg = "--num_packets="+ str(size)
        for length in small_packet_sizes:
            len_arg = "--data_length=" + str(length)
            full_args = "--dport_disc=22222 " + len_arg + " " + pack_arg
            args.append(full_args)

    for paramteters in args:
        subprocess.call("./ExperimentSQL.py" + parameters)


