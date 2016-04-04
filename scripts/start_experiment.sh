#!/bin/sh

EXPERIMENT=police.sh
USER=ucla_triton

ssh -t ${USER}@$1 "cd remote_host; ./$EXPERIMENT"
