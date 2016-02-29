#!/bin/sh

EXPERIMENT=tcp_experiment.sh
USER=paul

ssh -t $USER@$1 "cd ncd/scripts; ./$EXPERIMENT"
