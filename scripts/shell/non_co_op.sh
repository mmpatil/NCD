#!/bin/sh

EXPERIMENT_FOLDER=/home/paul/experiment/remote_host/
EXPERIMENT=single_compression.sh
USER=ucla_triton
ARGS="--dest_ip=$@"
echo $ARGS

cd $EXPERIMENT_FOLDER; ./$EXPERIMENT --dest_ip=$@
