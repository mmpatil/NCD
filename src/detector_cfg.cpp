#include "udp_detector.hpp"
#include "tcp_detector.hpp"

#include <boost/program_options.hpp>


using namespace std;
using namespace boost;
using namespace detection;

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
    // get program options
    // common options
    string dest_ip;

    uint32_t sport;
    uint32_t dport;

    string filename;             // = "/dev/urandom";
    uint16_t num_packets;        // = 1000;
    uint16_t data_length;        // = 512;
    uint16_t num_tail;           // = 20;
    uint16_t tail_wait;          // = 10;
    raw_level raw_status;        // = full;
    transport_type trans_proto = transport_type::udp;

    // TCP options

    uint8_t tos;
    uint16_t id;
    uint16_t frag_off;
    uint8_t ttl;
    uint8_t proto;
    uint16_t check_sum;
    uint16_t syn_port_in = 22223;

    po::options_description cli("CLI Only Options");

    // clang-format off
    cli.add_options()
        ("help,h", "produce help message")
        ("config,c", po::value<string>(&cfg_file)->default_value("detector.cfg"), "Name of a configuration file")
        ("version,v", "print version string")
        ;
    // clang-format on

    po::options_description general("General Options");

    // clang-format off
    general.add_options()
        ("dest_ip", po::value<string>(&dest_ip), "Destination IP Address")
        ("sport,s", po::value<unsigned int>(&sport)->default_value(9876), "Source Port #")
        ("dport,p", po::value<uint32_t>(&dport)->default_value(33434), "Destination Port #")
        ("filename,f", po::value<string>(&filename)->default_value("/dev/urandom"), "Filename for packet data")
        ("num_packets,n", po::value<uint16_t>(&num_packets)->default_value(20), "Number of packets in the data train")
        ("data_length", po::value<uint16_t>(&data_length)->default_value(20), "Size of payload")
        ("num_tail,t", po::value<uint16_t>(&num_tail)->default_value(20), "Number of packets in the tail train")
        ("tail_wait", po::value<uint16_t>(&tail_wait)->default_value(10), "Time to wait between tail packets")
        ("raw_status", po::value<raw_level>(&raw_status)->default_value(full), "Time to wait between tail packets")
        ("trans_proto", po::value<transport_type>(&trans_proto)->zero_tokens()->implicit_value(transport_type::tcp), "Time to wait between tail packets")
        ;
    // clang-format on

    // udp options

    //tcp options

    // clang-format off
    po::options_description tcp_opts("TCP Options");
    tcp_opts.add_options()
        ("tos", po::value<uint8_t>(&tos)->default_value(0), "Type of Service")
        ("id", po::value<uint16_t>(&id)->default_value(0), "TCP ID")
        ("frag_off", po::value<uint16_t>(&frag_off)->default_value(0), "Time to wait between tail packets")
        ("ttl", po::value<uint8_t>(&ttl)->default_value(10), "Time to Live")
        ("proto", po::value<uint8_t>(&proto)->default_value(IPPROTO_TCP), "TCP Protcol")
        ("syn_port_in", po::value<uint16_t>(&syn_port_in)->default_value(22223), "TCP SYN Port #")
        ;
    // clang-format on


    po::positional_options_description p;
    p.add("dest_ip", -1);
    po::variables_map vm;

    store(po::command_line_parser(argc,argv)).options(cli).positional(p).run(),vm);
    notify(vm);





    return 0;
}
