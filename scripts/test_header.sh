#!/bin/sh

# script for adding Meta-data to test files

NOW=date
echo "DATE: $($NOW)"
echo "HOSTNAME: $HOSTNAME"
echo "USER: $USER"
echo "COMMAND: $@"
echo "TEST INFO:
Test run from $HOSTNAME


echo "ANALYSYS: \n"

echo "RAW DATA:\n"
