#!/bin/sh
FILENAME=`./make_log.sh ncd`
sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST -j DROP
./test_runner.sh "sudo ./ncd_main 131.179.192.201 -p 9876 -n 50" > $FILENAME
sudo iptables -D OUTPUT -p tcp --tcp-flags RST RST -j DROP
