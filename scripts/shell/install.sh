#!/bin/sh
# File Containing Target IP Address
IP_FILE=$1
# path to local experiment direcory to rsync
EXPERIMENT_DIRECTORY=remote_host
SLICE_HOME=/home/ucla_triton

#echo ${SLICE_HOME}

# Packages to install on remote machine
PACKAGES="MySQL-python vim tcpdump iperf python mysql zsh python-json python-simplejson"

# SSH into the planet lab node install required packages
#parallel-ssh -t 120 -h ${IP_FILE} -vl ucla_triton "sudo yum --nogpgcheck install -y ${PACKAGES}"

#parallel-ssh -h ${IP_FILE} -vl ucla_triton "rm -rf remote_host"

# Rsync the experimental directory on the Planet lab node
parallel-rsync -h ${IP_FILE} -X "--copy-links" -rl ucla_triton ${EXPERIMENT_DIRECTORY} ${SLICE_HOME}/

