#!/bin/sh
sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST -j DROP
sudo ./ncd_main www.google.com -p 80 -n 500
sudo iptables -D OUTPUT -p tcp --tcp-flags RST RST -j DROP
