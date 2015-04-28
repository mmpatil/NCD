#!/bin/sh
DIRNAME= ../$HOSTNAME/Test_Data/$DATE 
DATE=date
if[ -d $DIRNAME ]; then
	COUNT=ls -afq | wc -l
else
	COUNT=1
	mkdir $DIRNAME
fi

sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST -j DROP

./test_runner.sh "sudo tcpdump -i any -v" > $DIRNAME/$count

sudo iptables -D OUTPUT -p tcp --tcp-flags RST RST -j DROP
