#!/bin/sh

EXPERIMENT=non_co_op.sh
USER=paul

ssh -t ${USER}@198.188.2.10 "cd ~/experiment; sudo ./$EXPERIMENT" $1
