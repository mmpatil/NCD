#!/bin/sh

# a script for running a test with Meta-data included at the 
# beginning of output

# use- test_runner.sh command filename

./test_header.sh $2 $1

$1 >> $2

