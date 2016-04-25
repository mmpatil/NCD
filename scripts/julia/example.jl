using Pcap

function display_eth_hdr(ethhdr::EthHdr)
    println("Ethernet Header")
    println("  |- Src Mac  : $(ethhdr.src_mac)")
    println("  |- Dest Mac : $(ethhdr.dest_mac)")
    println("  |- Type     : $(ethhdr.ptype)")
end # function display_ip_hdr

function display_ip_hdr(iphdr::IpHdr)
    println("IP Header")
    println("  |- Version         : $(iphdr.version)")
    println("  |- Length          : $(iphdr.length)")
    println("  |- Type of Service : $(iphdr.services)")
    println("  |- Total Length    : $(iphdr.totlen)")
    println("  |- ID              : $(iphdr.id)")
    println("  |- TTL             : $(iphdr.ttl)")
    println("  |- Protocol        : $(iphdr.protocol)")
    println("  |- Src Ip          : $(iphdr.src_ip)")
    println("  |- Dest Ip         : $(iphdr.dest_ip)")
    println("  |- Checksum        : 0x$(hex(iphdr.checksum,4))")
end # function display_ip_hdr

function display_udp_hdr(udphdr::UdpHdr)
    println("UDP Header")
    println("  |- Src Port  : $(udphdr.src_port)")
    println("  |- Dest Port : $(udphdr.dest_port)")
    println("  |- Length    : $(udphdr.length)")
    println("  |- Checksum  : 0x$(hex(udphdr.checksum, 4))")
    print("  |- Data : ")

    n = 0
    for byte = udphdr.data
        if n % 16 == 0 && n != 0
            print("\n            ")
        end
        print("$(hex(byte, 2)) ")
        n = n + 1
    end
end # function display_udp_hdr

cap     = PcapOffline("test.pcap")
rec     = pcap_get_record(cap)
layers  = decode_pkt(rec.payload)



function print_udp_packet()
    println("---------- UDP Packet ----------\n")
    display_eth_hdr(layers.datalink)
    display_ip_hdr(layers.network)
    if (layers.network.protocol == 17)
        display_udp_hdr(layers.protocol)
    end
    println("\n\n--------------------------------\n")
end
