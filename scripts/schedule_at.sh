#!/bin/sh
# This script takes lines from a tab separated file,
# and uses them to compose an 'at' command
# the first field in the file is expected to be a time,
# in a format compatible with 'at' (see docs)
# the Second field is an IP address used to  determine target hosts
# eventually it should become part of the command,
# the Third field is the command to be scheduled.

#TODO change the schedule scripts to make an actual call to ncd
# then when schedule.txt is created the IP should already be in
# command portion. Also determine a good way to manage flags

# Timeout: maximum running time for an experiment in seconds
TIMEOUT=55

IFS=$'\t'
while read line
do
    arr=(${line})
    at ${arr[0]} << ENDMARKER
    ${arr[2]} ${arr[1]}
ENDMARKER
done < $1
