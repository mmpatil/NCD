using Pcap

function getPacketId(udphdr::UdpHdr)
    id = ntoh(reinterpret(UInt16, udphdr.data[1:2])[1])
end





cap = PcapOffline("test.pcap")
rec = pcap_get_record(cap)
layers = decode_pkt(rec.payload)

# go through all udp packets, and get the packet ID
# then plot all packet IDs for each stream

getPacketId(layers.protocol)
