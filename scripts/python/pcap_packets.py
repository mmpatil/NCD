#!/usr/bin/env python

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

    print "Total packets recieved: %d" % (len(ids))

    df = {'Packet ID' : ids.keys(), 'Dest Port': ids.values(), 'Value': vals }
    return df


def plot_losses(pcap1, pcap2):
    df = get_packet_id(pcap1)
    df2 = get_packet_id(pcap2)
    points1 = pandas.DataFrame(df)
    points2 = pandas.DataFrame(df2)
    #points = pandas.concat([points1, points2])
    plot = ggplot(aes(x='Packet ID', color='Dest Port'), data = points2)
    #plot + geom_point()
    print plot + geom_histogram(binwidth=1)


def main():
    """
    baseline = ['/home/atlas/TestResults/Baseline/2824.pcap', '/home/atlas/TestResults/Baseline/2825.pcap']
    plot_losses(baseline[0], baseline[1])
    """
    policing = ['/home/atlas/TestResults/Police/day-2-policing/1707.pcap', '/home/atlas/TestResults/Police/day-2-policing/1708.pcap']
    plot_losses(policing[0], policing[1])

    shaping = ['/home/atlas/TestResults/misc/6985.pcap', '/home/atlas/TestResults/misc/6986.pcap']
    plot_losses(shaping[0], shaping[1])

    compression = ['/home/atlas/TestResults/Compression/9101.pcap', '/home/atlas/TestResults/Compression/9102.pcap']
    plot_losses(compression[0], compression[1])



if __name__=="__main__":
    main()
