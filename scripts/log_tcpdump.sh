#!/bin/sh
FILENAME=`./make_log.sh tcpdump`
#nc 9876 -l&
#sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST -j DROP

./test_runner.sh "sudo tcpdump -i any -v" > $FILENAME
#./test_runner.sh "sudo tcpdump -i any -v -w $FILENAME" > ${FILENAME}_meta
#sudo iptables -D OUTPUT -p tcp --tcp-flags RST RST -j DROP
#kill %1
