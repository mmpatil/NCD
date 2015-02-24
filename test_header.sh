#!/bin/sh
# script for adding Meta-data to test files

#Holds mostly filler text for now

NOW=$(date)
echo "DATE: $NOW" > $1
echo "COMMAND: '$2'" >> $1
echo "TEST INFO: 
Test run from home machine, behind Time Warner NAT 
Here's some more relevant data
The end.....\n" >> $1

