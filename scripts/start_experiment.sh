#!/bin/sh

EXPERIMENT=tcp_experiment.sh
USER=paul

ssh $USER@$1 "cd ncd/scripts; ./$EXPERIMENT"
