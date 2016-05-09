#!/bin/sh

NAME="SPQ Disc V2"
CMD="--dport_base=22222 --dport_disc=44444 --junk_interval=3 --saturation_length=1000 --saturation_port=22222 "

./ExperimentSQL.py "$NAME" $CMD

