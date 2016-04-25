#!/bin/sh

EXPERIMENT=no-police.sh
USER=ucla_triton

ssh -T ${USER}@$1 "cd remote_host; ./$EXPERIMENT"
