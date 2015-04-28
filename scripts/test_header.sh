#!/bin/sh

# script for adding Meta-data to test files
echo "DATE: $(date)"
echo "HOSTNAME: $(hostname)"
echo "USER: $USER"
echo "COMMAND: $@"
echo "TEST INFO:
Test run from $(hostname)\n"

echo "ANALYSYS: \n"

echo "RAW DATA:\n"
