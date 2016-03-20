#include "tcp_detector.hpp"
#include "udp_detector.hpp"
#include "co_op_udp_detector.hpp"

#include <boost/program_options.hpp>


using namespace std;
using namespace boost;
using namespace detection;

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
    // get program options
    // common options
    int test_id_in;
    string dest_ip;
    string cfg_file;

    bool verbose    = false;
    bool sql_output = true;


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
    // uint16_t check_sum;
    uint16_t syn_port_in = 22223;

    po::options_description cli("CLI Only Options");

    // clang-format off
    cli.add_options()
        ("help,h", "produce help message")
        ("config,c", po::value<string>(&cfg_file)->default_value("detector.cfg"), "Name of a configuration file")
        ("version", "print version string")
        ;
    // clang-format on

    po::options_description general("General Options");

    // clang-format off
    general.add_options()
        ("test_id_in", po::value<int>(&test_id_in), "Experimental ID number")
        ("dest_ip", po::value<string>(&dest_ip), "Destination IP Address")
        ("sport,s", po::value<uint32_t>(&sport)->default_value(9876), "Source Port #")
        ("dport,p", po::value<uint32_t>(&dport)->default_value(33434), "Destination Port #")
        ("filename,f", po::value<string>(&filename)->default_value("/dev/urandom"), "Filename for packet data")
        ("num_packets,n", po::value<uint16_t>(&num_packets)->default_value(20), "Number of packets in the data train")
        ("data_length", po::value<uint16_t>(&data_length)->default_value(20), "Size of payload")
        ("num_tail,t", po::value<uint16_t>(&num_tail)->default_value(20), "Number of packets in the tail train")
        ("tail_wait", po::value<uint16_t>(&tail_wait)->default_value(10), "Time to wait between tail packets")
        ("raw_status", po::value<raw_level>(&raw_status)->default_value(full), "Time to wait between tail packets")
        ("trans_proto", po::value<transport_type>(&trans_proto)->zero_tokens()->implicit_value(transport_type::tcp), "Time to wait between tail packets")
        ("verbose,v", po::value<bool>(&verbose), "Print verbose output")
        ("sql_output,q", po::value<bool>(&sql_output), "Print output suitable for SQL parser")
        ;
    // clang-format on

    // udp options

    // tcp options

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


    po::options_description cmd_line;
    cmd_line.add(cli).add(general).add(tcp_opts);

    po::options_description cfg_desc;
    cfg_desc.add(general).add(tcp_opts);

    po::positional_options_description p;
    p.add("dest_ip", -1);

    po::variables_map vm;

    store(po::command_line_parser(argc, argv).options(cmd_line).positional(p).run(), vm);
    notify(vm);


    ifstream ifs(cfg_file.c_str());
    if(!ifs)
    {
        cout << "can not open config file: " << cfg_file << "\n";
        return 0;
    }
    else
    {
        store(parse_config_file(ifs, cfg_desc), vm);
        notify(vm);
    }

    if(vm.count("help"))
    {
        cout << cmd_line << "\n";
        return 0;
    }

    if(vm.count("version"))
    {
        cout << "Detector, version 1.0\n";
        return 0;
    }

    std::shared_ptr<detector> test(nullptr);
    switch(trans_proto)
    {
    case transport_type::udp:
        test = std::make_shared<udp_detector>(test_id_in, dest_ip, tos, 0, 0, 255, IPPROTO_UDP, 0, sport, dport);
        break;
    case transport_type::tcp:
        test = std::make_shared<tcp_detector>(test_id_in, dest_ip, tos, 0, 0, 255, IPPROTO_TCP, 0, sport, dport);
    default:
        break;
    }
    if(!test)
        return -1;

    test->verbose    = verbose;
    test->sql_output = sql_output;
    test->measure();


    return 0;
}
