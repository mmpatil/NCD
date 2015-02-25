#!/bin/sh

# script for adding Meta-data to test files

NOW=date
echo "DATE: $($NOW)"
echo "HOSTNAME: $HOSTNAME"
echo "USER: $USER"
echo "COMMAND: $@"
echo "TEST INFO:
Test run from home machine, behind Time Warner NAT
Some more relevant data
The end ....\n"

echo "ANALYSYS: \n"

echo "RAW DATA:\n"
