#!/bin/sh
# File Containing Target IP Address
IP_FILE=$1
# path to local experiment direcory to rsync
EXPERIMENT_DIRECTORY=ncd

# Packages to install on remote machine
PACKAGES=traceroute ping vim tcpdump iperf python mysql zsh

# SSH into the planet lab node install required packages
pssh -H ${IP_FILE} -vl ucla_triton "sudo yum install $PACKAGES"

# Rsync the experimental directory on the Planet lab node
parallel-rsync -H ${IP_FILE} -l ${ucla_triton} ${EXPERIMENT_DIRECTORY} ~/${EXPERIMENT_DIRECTORY}


