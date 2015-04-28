#!/bin/sh
DATE="date +%F"
DIRNAME=../test_data/$($hostname)/$($DATE)
if [ -d "$DIRNAME" ]
then
	COUNT=`ls -afq | wc -l`
else
	COUNT=1
	mkdir $DIRNAME
fi
echo $COUNT

#sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST -j DROP

./test_runner.sh "sudo tcpdump -i any -v" > $DIRNAME/"test_$COUNT"

#sudo iptables -D OUTPUT -p tcp --tcp-flags RST RST -j DROP
