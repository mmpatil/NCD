#! /bin/env python

import subprocess

from vary_parameters import ExecuteCommands #, ExecuteCommandsDebug


def main():
    large_num_packets = range(500, 8000, 500)
    large_packet_sizes = [512, 1024, 1400]

    command_map = {"--num_packets": large_num_packets,
                   "--packet_sizes": large_packet_sizes}
    command = "./ExperimentSQL.py"
    name = 'Policing Parameters Test V2'

    ExecuteCommands(command, name, command_map)

    small_num_packets = range(4000, 20000, 500)
    small_packet_sizes = [32, 64, 128, 256]

    command_map = {"--num_packets": small_num_packets,
                   "--packet_sizes": small_packet_sizes}

    ExecuteCommands(command, name, command_map)



if __name__ == "__main__":
    main()


