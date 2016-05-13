#! /bin/env python

import subprocess

class CommandMaker:

    def __init__(self, command, exp_name, params=None):
        self.name = command
        self.experiment_name = exp_name

        self.params = params

        self.full_command = self.name + " " + self.experiment_name

        if params != None:
            self.full_command += " " + self.params
        self.map={}

    def add_parameters(self, map):
        self.map.update(map)

    def get_commands(self):
        return self.make_commands(self.full_command, self.map.items())


    def get_command_list(self, command, option, values):
        command_list = []
        for item in values:
            new_command = command + " " + option + str(item)
            command_list.append(new_command)
        return command_list


    def make_commands(self, command, map_list):
        new_commands = []
        if not map_list:
            return []
        command_list = self.make_commands(command, map_list[1:])
        option, values = map_list[0]
        if not command_list:
            command_list.append(command)

        for cmd in command_list:
            for item in self.get_command_list(cmd,option, values):
                new_commands.append(item)

        return new_commands





def ExecuteCommands(command_name, fixed_params, param_map):
    # type: (basestring, basestring, dictionary) -> void
    """
    Executes the provided command using subprocess with all combinations of parameters from
    :param command_name: the string name of the command (may require './' prefix)
    :param fixed_params: string with static parameters
    :param param_map: A dictionary mapping parameter flags with a list of their possible values
    """
    maker = CommandMaker(command_name, fixed_params)
    maker.add_parameters(param_map)
    commands = maker.get_commands()
    for cmd in commands:
        subprocess.call(cmd, shell=True)


def ExecuteCommandsDebug(command, fixed_param, command_map):
    # type: (basestring, basestring, dictionary) -> void
    """
    Debug prints the generated command using subprocess
    :param command: the string name of the command (may require './' prefix)
    :param fixed_param: string with static parameters
    :param command_map: A dictionary mapping parameter flags with a list of their values
    """
    maker = CommandMaker(command, fixed_param)
    maker.add_parameters(command_map)
    commands = maker.get_commands()
    for cmd in commands:
        print cmd




if __name__ == "__main__":
    """
    large_num_packets = range(400, 8000, 200)
    large_packet_sizes = [512, 1024, 1400]
    small_num_packets = range(4000, 50000, 1000)
    small_packet_sizes = [32, 64, 128, 256]

    args = []
    for size in large_num_packets:
        pack_arg = "--num_packets="+ str(size)
        for length in large_packet_sizes:
            len_arg = "--data_length=" + str(length)
            full_args = "--dport_disc=33333 " + len_arg + " " + pack_arg
            args.append(full_args)

    for size in small_num_packets:
        pack_arg = "--num_packets="+ str(size)
        for length in small_packet_sizes:
            len_arg = "--data_length=" + str(length)
            full_args = "--dport_disc=33333 " + len_arg + " " + pack_arg
            args.append(full_args)

    for parameters in args:
        subprocess.call("./ExperimentSQL.py 'Compression Parameters' " + parameters, shell=True)
        """
    cmd = CommandMaker('./ExperimentSQL.py', 'Compression Parameters')
    mappings= {"--num_packets=" : range(400,1000, 200),
               "--packet_size=" : [512, 1024, 1400]}

    cmd.add_parameters(mappings)

    for item in  cmd.get_commands():
        print item
