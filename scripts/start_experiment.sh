#!/bin/sh

EXPERIMENT=tcp_experiment.sh
USER=ucla_triton

ssh $USER@$1 "$EXPERIMENT"
