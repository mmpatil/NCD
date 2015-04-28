#!/bin/sh
FILENAME=`./make_log.sh tcpdump`

#sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST -j DROP

./test_runner.sh "sudo tcpdump -i any -v  -w $FILENAME"

#sudo iptables -D OUTPUT -p tcp --tcp-flags RST RST -j DROP
