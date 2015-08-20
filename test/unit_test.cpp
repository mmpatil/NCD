/**
 * @author: Paul Kirth
 * @file: ncd.c
 */

#include "unit_test.h"
#include "ncd.c"

TEST(get_time_test, get_time_correct)
{
        struct timeval tv;
        double d, r;
        d = get_time();
        gettimeofday(&tv, NULL);
        r = tv.tv_sec;
        r += tv.tv_usec / 1000000;
        EXPECT_FLOAT_EQ(d, r);
}

int test_comp_det()
{
        return 0;
}

TEST(mkipv4_test, mkipv4_works)
{

        struct addrinfo hints = { 0 }; /* for get addrinfo */

        /* set up hints for getaddrinfo() */
        hints.ai_flags = AI_CANONNAME;
        hints.ai_protocol = IPPROTO_UDP;

        int err = getaddrinfo("192.168.1.100", NULL, &hints, &res);

        ttl = 50;
        struct ip ip = { 0 }, *m = NULL;
        inet_pton(AF_INET, "192.168.1.100", &ip.ip_dst);
        ip.ip_hl = 5;
        ip.ip_id = htons(getpid());
        ip.ip_len = htons(1024);
        ip.ip_ttl = ttl;
        ip.ip_v = 4;
        ip.ip_p = IPPROTO_UDP;

        char* buff[1024] = { 0 };
        m = (struct ip *) buff;
        mkipv4(buff, 1024, res, IPPROTO_UDP);
        EXPECT_EQ(ip.ip_dst.s_addr, m->ip_dst.s_addr);
        EXPECT_EQ(ip.ip_id, m->ip_id);
        EXPECT_EQ(ip.ip_len, m->ip_len);
        EXPECT_EQ(ip.ip_ttl, m->ip_ttl);
        EXPECT_EQ(ip.ip_v, m->ip_v);
        EXPECT_EQ(ip.ip_p, m->ip_p);
        freeaddrinfo(res);

}

TEST(mkipv6_test, mkipv6_works)
{

        struct addrinfo hints = { 0 }; /* for get addrinfo */

        /* set up hints for getaddrinfo() */
        hints.ai_flags = AI_CANONNAME;
        hints.ai_protocol = IPPROTO_UDP;

        int err = getaddrinfo("192.168.1.101", NULL, &hints, &res);
        ttl = 50;
        struct ip6_hdr ip = { 0 }, *m = NULL;        // = (struct ip6_hdr *) buff;
        ip.ip6_dst = ((struct sockaddr_in6*) res->ai_addr)->sin6_addr;
        inet_pton(AF_INET6, "192.168.1.101", &ip.ip6_src);
        ip.ip6_ctlun.ip6_un1.ip6_un1_flow = 0;
        ip.ip6_ctlun.ip6_un1.ip6_un1_hlim = ttl;
        ip.ip6_ctlun.ip6_un1.ip6_un1_nxt = htons(sizeof(struct ip6_hdr));
        ip.ip6_ctlun.ip6_un1.ip6_un1_plen = htons(1024);

        char* buff[1024] = { 0 };
        m = (struct ip6_hdr *) buff;
        mkipv6(buff, 1024, res, IPPROTO_UDP);
        EXPECT_EQ(ip.ip6_ctlun.ip6_un1.ip6_un1_flow,
                        m->ip6_ctlun.ip6_un1.ip6_un1_flow);
        EXPECT_EQ(ip.ip6_ctlun.ip6_un1.ip6_un1_hlim,
                        m->ip6_ctlun.ip6_un1.ip6_un1_hlim);
        EXPECT_EQ(ip.ip6_ctlun.ip6_un1.ip6_un1_plen,
                        m->ip6_ctlun.ip6_un1.ip6_un1_plen);
        EXPECT_EQ(ip.ip6_ctlun.ip6_un1.ip6_un1_nxt,
                        m->ip6_ctlun.ip6_un1.ip6_un1_nxt);

        for(int i = 0; i < 4; ++i){
                EXPECT_EQ(ip.ip6_dst.__in6_u.__u6_addr32[i],
                                m->ip6_dst.__in6_u.__u6_addr32[i]);
                EXPECT_EQ(ip.ip6_src.__in6_u.__u6_addr32[i],
                                m->ip6_src.__in6_u.__u6_addr32[i]);
        }
}

int test_mkudphdr()
{
        return 0;
}

TEST(mkicmpv4_test, mkicmpv4_works)
{
        char buff[128] = { 0 }, str[128] = { 0 };
        int datalen = 56;
        struct icmp *icmp = (struct icmp *) buff, *m = (struct icmp *) str;
        icmp->icmp_type = ICMP_ECHO;
        icmp->icmp_code = 0;
        icmp->icmp_id = (u_int16_t) getpid();

        int ret = mkicmpv4(str, datalen);
        memcpy(icmp->icmp_data, m->icmp_data, datalen);
        icmp->icmp_seq = m->icmp_seq;        // so they can match (its random!)
        icmp->icmp_cksum = 0;
        icmp->icmp_cksum = ip_checksum(icmp, datalen + sizeof(struct icmp));

        EXPECT_EQ(icmp->icmp_cksum, m->icmp_cksum);
        EXPECT_EQ(icmp->icmp_code, m->icmp_code);
        EXPECT_EQ(icmp->icmp_type, m->icmp_type);
        EXPECT_EQ(icmp->icmp_id, m->icmp_id);
        EXPECT_EQ(0, ret);
}

TEST(mkicmpv6_test, mkicmpv6_works)
{
        char buff[128] = { 0 }, str[128] = { 0 };
        int datalen = 56;
        struct icmp6_hdr *icmp = (struct icmp6_hdr *) buff, *m =
                        (struct icmp6_hdr *) str;
        icmp->icmp6_type = ICMP6_ECHO_REQUEST;
        icmp->icmp6_code = 0;
        icmp->icmp6_id= htons((u_int16_t) getpid());

        int ret = mkicmpv6(str, datalen);
        memcpy(&icmp->icmp6_dataun, &m->icmp6_dataun, datalen);
        icmp->icmp6_seq= m->icmp6_seq;        // so they can match (its random!)
        icmp->icmp6_cksum = 0;
        icmp->icmp6_cksum = ip_checksum(icmp,
                        datalen + sizeof(struct icmp6_hdr));

        EXPECT_EQ(icmp->icmp6_cksum, m->icmp6_cksum);
        EXPECT_EQ(icmp->icmp6_code, m->icmp6_code);
        EXPECT_EQ(icmp->icmp6_type, m->icmp6_type);
        EXPECT_EQ(icmp->icmp6_id, m->icmp6_id);
        EXPECT_EQ(0, ret);
}

TEST(fill_data_test, fill_data_works)
{

}

int test_send_train()
{
        return 0;
}

int test_recv4()
{
        return 0;
}

int test_recv6()
{
        return 0;
}

int test_ip_checksum()
{
        return 0;
}

int main(int argc, char **argv)
{
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
}

