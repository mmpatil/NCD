#!/usr/bin/env python

import pcapy
import dpkt
import socket
import struct
import pandas

from ggplot import *

def get_packet_id(filename):
    with open(filename) as f:
        pcap = dpkt.pcap.Reader(f)
        ids = {}
        vals = []
        for ts, buf in pcap:
            eth = dpkt.sll.SLL(buf)
            #eth = dpkt.ethernet.Ethernet(buf)
            ip = eth.data
            udp = ip.data
            packet_id = socket.ntohs(struct.unpack_from('H',udp.data[:2])[0])
            ids[packet_id] = udp.dport
            vals.append(1)

    df = {'Packet ID' : ids.keys(), 'Dest Port': ids.values(), 'Value': vals }
    return df


df = get_packet_id('test.pcap')

df2 = get_packet_id('test2.pcap')

points1 = pandas.DataFrame(df)
points2 = pandas.DataFrame(df2)

points = pandas.concat([points1, points2])


plot = ggplot(aes(x='Packet ID', color='Dest Port'), data = points)

plot + geom_point()

print plot + geom_histogram(binwidth=1)
