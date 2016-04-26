/*
  The MIT License

  Copyright (c) 2015-2016 Paul Kirth pk1574@gmail.com

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

/**
 * @author: Paul Kirth
 * @file: udp_detector.hpp
 */

#include "spq_co_op_detector.hpp"

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
    int cooldown;
    string dest_ip;
    string cfg_file;

    bool verbose    = false;
    bool sql_output = true;

    // common options -- do not vary between base and discrimination streams
    uint16_t num_packets;        // = 1000;
    uint16_t data_length;        // = 512;
    uint16_t num_tail;           // = 20;
    uint16_t tail_wait;          // = 10;
    raw_level raw_status;        // = full;
    transport_type trans_proto = transport_type::udp;


    // Base settings
    uint32_t sport_base;
    uint32_t dport_base;
    string filename_base;        // = "/dev/urandom";

    // Base TCP options
    uint8_t tos_base;              // = 0;
    uint16_t id_base;              // = 0;
    uint16_t frag_off_base;        // = 0;
    uint8_t ttl_base;              // = 255;
    uint8_t proto_base;            // = IPPROTO_UDP
    // uint16_t check_sum;          // =0;
    uint16_t syn_port_in_base = 22223;

    // Discrimination settings
    uint32_t sport_disc;
    uint32_t dport_disc;
    string filename_disc;        // = "/dev/urandom";

    // Discrimination TCP options
    uint8_t tos_disc;
    uint16_t id_disc;
    uint16_t frag_off_disc;
    uint8_t ttl_disc;
    uint8_t proto_disc;
    // uint16_t check_sum;
    uint16_t syn_port_in_disc = 22223;
    uint16_t saturation_port;
    uint16_t saturation_length;

    uint16_t junk_interval = 50;

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
        ("saturation_port", po::value<uint16_t>(&saturation_port)->default_value(33335), "The port number of the saturation train")
        ("saturation_length", po::value<uint16_t>(&saturation_length)->default_value(600), "The number of Saturation Packets")
        ("junk_interval", po::value<uint16_t>(&junk_interval)->default_value(50), "The number of junk packets between data packets")
        ("test_id_in", po::value<int>(&test_id_in)->default_value(77777), "Experimental ID number")
        ("cooldown", po::value<int>(&cooldown)->default_value(2), "The time in seconds to wait between tests")
        ("dest_ip", po::value<string>(&dest_ip), "Destination IP Address")
        ("sport_base", po::value<uint32_t>(&sport_base)->default_value(9876), "Source Port # for Base stream")
        ("sport_disc", po::value<uint32_t>(&sport_disc)->default_value(9876), "Source Port # for Discrimination stream")
        ("dport_base", po::value<uint32_t>(&dport_base)->default_value(33434), "Destination Port # for Base stream")
        ("dport_disc", po::value<uint32_t>(&dport_disc)->default_value(33434), "Destination Port # for Discrimination stream")
        ("filename_base", po::value<string>(&filename_base)->default_value("/dev/urandom"), "Filename for packet data for Base stream")
        ("filename_disc", po::value<string>(&filename_disc)->default_value("/dev/urandom"), "Filename for packet data for Discrimination stream")
        ("num_packets,n", po::value<uint16_t>(&num_packets)->default_value(20), "Number of packets in the data train")
        ("data_length,l", po::value<uint16_t>(&data_length)->default_value(20), "Size of payload")
        ("num_tail,t", po::value<uint16_t>(&num_tail)->default_value(20), "Number of packets in the tail train")
        ("tail_wait,w", po::value<uint16_t>(&tail_wait)->default_value(10), "Time to wait between tail packets")
        ("raw_status,r", po::value<raw_level>(&raw_status)->default_value(full), "The level to which raw sockets are used [none | trans | full]")
        ("trans_proto", po::value<transport_type>(&trans_proto)->zero_tokens()->implicit_value(transport_type::udp), "The transport protocol to use [udp | tcp]")
        ("verbose,v", po::value<bool>(&verbose), "Print verbose output")
        ("sql_output", po::value<bool>(&sql_output), "Print output suitable for SQL parser")
        ;
    // clang-format on

    // udp options

    // tcp options

    // clang-format off
    po::options_description tcp_opts("TCP Options");
    tcp_opts.add_options()
        ("tos_base", po::value<uint8_t>(&tos_base)->default_value(0), "Type of Service for Base stream")
        ("id_base", po::value<uint16_t>(&id_base)->default_value(0), "TCP ID for Base stream")
        ("frag_off_base", po::value<uint16_t>(&frag_off_base)->default_value(0), "Time to wait between tail packets for Base stream")
        ("ttl_base", po::value<uint8_t>(&ttl_base)->default_value(10), "Time to Live for Base stream")
        ("proto_base", po::value<uint8_t>(&proto_base)->default_value(IPPROTO_TCP), "TCP Protcol for Base stream")
        ("syn_port_in_base", po::value<uint16_t>(&syn_port_in_base)->default_value(22223), "TCP SYN Port # for Base stream")
        ("tos_disc", po::value<uint8_t>(&tos_disc)->default_value(0), "Type of Service for Discrimination stream")
        ("id_disc", po::value<uint16_t>(&id_disc)->default_value(0), "TCP ID for Discrimination stream")
        ("frag_off_disc", po::value<uint16_t>(&frag_off_disc)->default_value(0), "Time to wait between tail packets for Discrimination stream")
        ("ttl_disc", po::value<uint8_t>(&ttl_disc)->default_value(10), "Time to Live for Discrimination stream")
        ("proto_disc", po::value<uint8_t>(&proto_disc)->default_value(IPPROTO_TCP), "TCP Protcol for Discrimination stream")
        ("syn_port_in_disc", po::value<uint16_t>(&syn_port_in_disc)->default_value(22223), "TCP SYN Port # for Discrimination stream")
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

    spq_co_op_detector base(spq_co_op_detector::priority::high, saturation_port, saturation_length, junk_interval, test_id_in, dest_ip, tos_base, id_base, frag_off_base, ttl_base, IPPROTO_UDP, 0, sport_base,
                            dport_base, false, filename_base, num_packets, data_length, num_tail, tail_wait, raw_level::transport_only,
                            trans_proto);
    spq_co_op_detector disc(spq_co_op_detector::priority::low, saturation_port, saturation_length, junk_interval, test_id_in, dest_ip, tos_disc, id_disc, frag_off_base, ttl_disc, IPPROTO_UDP, 0, sport_disc,
                            dport_disc, true, filename_disc, num_packets, data_length, num_tail, tail_wait, raw_level::transport_only,
                            trans_proto);


    auto saturate_list = base.data_train;

    for(size_t i = 0; i < base.data_train.size(); ++i)
    {
        if( i%junk_interval == 0)
        {
            swap(base.data_train[i], disc.data_train[i]);
        }
    }

    // make the length congruent
    swap(base.data_train, disc.data_train);

    base.verbose = disc.verbose = verbose;
    base.sql_output = disc.sql_output = sql_output;
    base.measure();

    std::this_thread::sleep_for(std::chrono::seconds(cooldown));

    disc.measure();


    return 0;
}
