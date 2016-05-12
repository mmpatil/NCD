#!/bin/sh

EXPERIMENT=spq.sh
USER=ucla_triton

ssh -T ${USER}@$1 "cd remote_host; sudo ./$EXPERIMENT"
