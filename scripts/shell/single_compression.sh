#!/bin/sh

NAME="SingleSided Compression Discrimination"
CMD="--dport_base=44444 --dport_disc=44444"

/home/paul/experiment/remote_host/SingleExperiment.py "$NAME" $CMD $@

