#include <iostream>
#include <memory>

#include "../include/tcp_detector.hpp"
#include "../include/udp_detector.hpp"


using namespace detection;

#define SIZE (1500 - sizeof(struct ip))
#define UDP_DATA_SIZE (SIZE - sizeof(struct udphdr) - sizeof(uint16_t))
#define TCP_DATA_SIZE (SIZE - sizeof(struct tcphdr) - sizeof(uint16_t))


// globals used to proccess command line args
std::string dest_ip;

/* probably change default port from traceroute port */
uint16_t dport    = 33434;        // 80;
uint16_t sport    = 13333;
uint16_t syn_port = 14444;

/* Default to 1 kilobyte packets */
uint16_t data_size   = 1024 - sizeof(uint16_t) - sizeof(struct tcphdr) - sizeof(struct ip);
uint16_t num_packets = 10;                    // send 1000 packets in udp data train
uint16_t ttl         = 255;                   // max ttl
uint16_t tail_wait   = 10;                    // wait 10 ms between ICMP tail messages
uint16_t num_tail    = 20;                    // send 20 ICMP tail messages
std::string file     = "/dev/urandom";        // default to random data for compression detection
bool verbose         = false;
bool sql_output      = true;
int cooldown         = 5;
transport_type trans = transport_type::udp;
uint8_t tos          = 0;

// stay the same
void print_use(char* program_name)
{
    printf("Usage: %s [-T] [-o] [-v] [-p PORT] [-c COOLDOWN]\n"
           "\t\t  [-f FILENAME_PAYLOAD] [-s DATA_SIZE] [-n NUM_PACKETS]\n"
           "\t\t  [-t TTL] [-w TAIL_INTERVAL] [-r NUM_TAIL] IPAddress\n",
           program_name);
}

// stay the same... with small modifications
int check_args(int argc, char* argv[])
{
    if(argc < 2)
    {
        errno = EINVAL;
        perror("Too few arguments, see use");
        print_use(argv[0]);
        return EXIT_FAILURE;
    }
    int check;
    int c  = 0;
    optind = 1;
    while((c = getopt(argc, argv, "Tovp:c:f:s:n:t:w:r:h")) != -1)
    {
        switch(c)
        {
        // case 'H':
        // lflag = 0;
        // break;
        // case 'L':
        // hflag = 0;
        // break;
        case 'p':
            check = atoi(optarg);
            if(check < (1 << 16) && check > 0)
            {
                dport = (uint16_t)check;
            }
            else
            {
                errno = ERANGE;
                perror("Port range: 1 - 65535");
                return EXIT_FAILURE;
            }
            break;
        case 's':
            check = atoi(optarg);
            if(check < 1 || check > (int)TCP_DATA_SIZE)
            {
                errno         = ERANGE;
                char str[256] = {0};
                snprintf(str, 256, "Valid UDP data size: 1-%lu", TCP_DATA_SIZE);
                perror(str);
                return EXIT_FAILURE;
            }
            data_size = (uint16_t)check;
            break;
        case 'n':
            check = atoi(optarg);
            if(check < 1 || check > 10000)
            {
                errno = ERANGE;
                perror("# of packets: 0 - 10000");
                return EXIT_FAILURE;
            }
            num_packets = (uint16_t)check;
            break;
        case 't':
            check = atoi(optarg);
            if(check < 0 || check > 255)
            {
                errno = ERANGE;
                perror("TTL range: 0 - 255");
                return EXIT_FAILURE;
            }
            else
                ttl = (uint8_t)check;
            break;
        case 'w':
            check = atoi(optarg);
            if(check < 0)
            {
                errno = ERANGE;
                perror("Time wait must be positive");
                return EXIT_FAILURE;
            }
            tail_wait = (uint16_t)check;
            break;
        case 'r':
            check = atoi(optarg);
            if(check < 1 || check > 1000)
            {
                errno = ERANGE;
                perror("# Tail Packets: 1 - 1,000");
                return EXIT_FAILURE;
            }
            num_tail = (uint16_t)check;
            break;
        case 'f':
        {
            file = optarg;
            break;
        }
        case 'h':
            print_use(argv[0]);
            return EXIT_FAILURE;
            break;
        case 'T':
            trans = transport_type::tcp;
            break;
        case 'v':
            verbose = true;
            break;
        case 'c':
            cooldown = atoi(optarg);
            if(cooldown < 0 || cooldown > 1000)
            {
                errno = ERANGE;
                perror("Cooldown Range: 1 - 1,000 Seconds");
                return EXIT_FAILURE;
            }
            break;
        case 'o':
            sql_output = true;
            break;
        case ':':
            errno = ERANGE;
            fprintf(stderr, "%s: Invalid option '-%s', check use\n", argv[0], optarg);
            print_use(argv[0]);
            return EXIT_FAILURE;
            break;
        default:
            errno = ERANGE;
            fprintf(stderr, "%s: error --check use, unkown option '%s'\n", argv[0], optarg);
            print_use(argv[0]);
            return EXIT_FAILURE;
        }        // end switch
    }            // end while
    /* these are the arguments after the command-line options */
    for(; optind < argc; optind++)
    {
        dest_ip = argv[optind];
        // printf("argument: \"%s\"\n", dst_ip);
    }
    if(dest_ip == "")
    {
        fprintf(stderr, "Error: Target IP missing\n");
        print_use(argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


int main(int argc, char* argv[])
{
    check_args(argc, argv);
    std::shared_ptr<detector> test(nullptr);
    switch(trans)
    {
    case transport_type::udp:
        test = std::make_shared<udp_detector>("", dest_ip, tos, 0, 0, 255, IPPROTO_UDP, 0, sport, dport);
        break;
    case transport_type::tcp:
        test = std::make_shared<tcp_detector>("", dest_ip, tos, 0, 0, 255, IPPROTO_TCP, 0, sport, dport);
    default:
        break;
    }
    if(!test)
        return -1;

    test->measure();
    return 0;
}
