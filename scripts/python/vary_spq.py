#! /bin/env python

import subprocess


def all_args():
    large_num_packets = range(400, 8000, 200)
    large_packet_sizes = [512, 1024, 1400]
    small_num_packets = range(4000, 50000, 1000)
    small_packet_sizes = [32, 64, 128, 256]

    args = []
    for junk in range(1, 10):
        junk_args = "--junk_interval=" + str(junk)
        for size in large_num_packets:
            pack_arg = junk_args + " --num_packets=" + str(size)
            for length in large_packet_sizes:
                len_arg = "--data_length=" + str(length)
                full_args = "--dport_disc=33333 " + len_arg + " " + pack_arg
                args.append(full_args)

        for size in small_num_packets:
            pack_arg = junk_args + " --num_packets=" + str(size)
            for length in small_packet_sizes:
                len_arg = "--data_length=" + str(length)
                full_args = "--dport_disc=33333 " + len_arg + " " + pack_arg
                args.append(full_args)


def junk_only():
    args = []
    for junk in range(1, 10):
        junk_args = "--junk_interval=" + str(junk)
        full_args = "--dport_disc=22223 " + junk_args
        args.append(full_args)
    return args


def main():
    args = junk_only()
    for parameters in args:
        subprocess.call("./ExperimentSQL.py 'Compression Parameters' " + parameters, shell=True)


if __name__ == "__main__":
    main()
