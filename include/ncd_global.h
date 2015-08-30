/**
 *@author Paul Kirth
 * @date 8/22/15.
 * @file ncd_global.h
 */

#ifndef NCD_NCD_GLOBAL_H
#define NCD_NCD_GLOBAL_H

#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>

/**
*  maximum ip packet size
*  1500 bytes Ethernet max size
*  40 IPv6 header max size
*  8 UDP header
*  2 16-bit packet ID
*/
#define SIZE (1500 - sizeof(struct ip))
#define UDP_DATA_SIZE (SIZE - sizeof(struct udphdr) - sizeof(uint16_t))
#define TCP_DATA_SIZE (SIZE - sizeof(struct tcphdr) - sizeof(uint16_t))

/**
 * struct for udp pseudo header
 */
struct __attribute__((__packed__)) pseudo_header
{
    uint32_t source;
    uint32_t dest;
    uint8_t zero;
    uint8_t proto;
    uint16_t len;
};

/*  Global Variables  */

/* file descriptors */
int icmp_fd;        // icmp socket file descriptor
int send_fd;        // udp socket file descriptor
int recv_fd;        // reply receiving socket file descriptor


/* command line arguments  */
uint16_t data_size;          // size of udp data payload
uint16_t num_packets;        // number of packets in udp data train
uint16_t num_tail;           // number of tail icmp messages sent tail_wait apart
uint16_t tail_wait;          // time between ICMP tail messages
uint16_t dport;              // destination port number
uint16_t sport;              // source port number
uint16_t syn_port;           // source port number
uint8_t ttl;                 // time to live


/* lengths of packets and data, etc. */
uint16_t send_len;             // length of data to be sent
uint32_t seq;                  // sequence number
uint16_t icmp_ip_len;          // length of IP icmp packet including payload
uint16_t icmp_len;             // length of ICMP packet
uint16_t icmp_data_len;        // length of ICMP data

/* Threading globals, mutexes and protected varibles */
int stop;              // boolean for if the send thread can stop (receive thread has received second response.
int recv_ready;        // bool for receiving SYN packets -- denotes if the program is ready to receive traffic
pthread_mutex_t stop_mutex;              // mutex for stop
pthread_mutex_t recv_ready_mutex;        // mutex for recv_ready
pthread_cond_t stop_cv;                  // condition variabl for stop -- denotes
pthread_cond_t recv_ready_cv;            // condition variable for recv_ready mutex


/* Time varibles */
double time_val;        // time as a double -- used to pass the value back to main
double td;              // time value as a double -- used in calcs
int udp_ack;

/* initialized globals */

extern char* packets_e;        // empty packets
extern char* packets_f;        // filled packets
extern char* dst_ip;           // destination ip address
extern char* file;             // name of file to read from /dev/urandom by default

/* flags */
extern uint8_t lflag;               // default option for low entropy -- set to on
extern uint8_t hflag;               // default option for high entropy -- set to on
extern int verbose;                  // flag for verbose output
extern const int num_threads;        // number of threads we'll use
extern int cooldown;                 // time in seconds to wait between data trains
extern uint8_t tcp_bool;            // bool for whether to use tcp or udp(1 == true, 0 == false)
extern int second_train;             // bool for if this is the second or first train

/* global buffers on the stack */
extern char pseudo[1500];             // buffer for pseudo header
extern char packet_rcv[1500];         // buffer for receiving replies
extern char packet_send[SIZE];        // buffer for sending data
extern char syn_packet_1[20];         // packet for head SYN
extern char syn_packet_2[20];         // packet for tail SYN
extern char icmp_send[128];           // buffer for ICMP messages

extern struct pseudo_header* ps;           // pseudo header
extern uint16_t* packet_id;               // sequence/ID number of udp msg
extern struct sockaddr_in srcaddrs;        // source IP address
extern struct in_addr destip;              // destination IP
extern socklen_t sa_len;                   // size of src address

extern struct addrinfo* res;              // addrinfo struct for getaddrinfo()
extern void* (*recv_data)(void*);         // function pointer so we can select properly for IPV4 or IPV6(no IPV6 yet
extern void* (*send_train)(void*);        // function pointer to send data: UDP or TCP

#endif        // NCD_NCD_GLOBAL_H
