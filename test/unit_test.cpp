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

TEST(mkipv4_test, mkipv4_sets_values_correctly)
{
        /* for get addrinfo */
        struct addrinfo hints = { 0 };

        /* set up hints for getaddrinfo() */
        hints.ai_flags = AI_CANONNAME;
        hints.ai_protocol = IPPROTO_UDP;

        int err = getaddrinfo("192.168.1.100", NULL, &hints, &res);
        EXPECT_EQ(0,err);

        ttl = 50;
        struct ip ip = { 0 }, *m = NULL;
        inet_pton(AF_INET, "192.168.1.100", &ip.ip_dst);
        ip.ip_hl = 5;
        ip.ip_id = htons(getpid());
        ip.ip_len = htons(1024);
        ip.ip_ttl = ttl;
        ip.ip_v = 4;
        ip.ip_p = IPPROTO_UDP;

        char buff[1024] = { 0 };
        m = (struct ip *) buff;
        mkipv4(buff, 1024, IPPROTO_UDP);
        EXPECT_EQ(ip.ip_dst.s_addr, m->ip_dst.s_addr);
        EXPECT_EQ(ip.ip_id, m->ip_id);
        EXPECT_EQ(ip.ip_len, m->ip_len);
        EXPECT_EQ(ip.ip_ttl, m->ip_ttl);
        EXPECT_EQ(ip.ip_v, m->ip_v);
        EXPECT_EQ(ip.ip_p, m->ip_p);
        freeaddrinfo(res);
}

TEST(mkipv4_test, mkipv4_bad_inputs)
{
        char buff[1024] = { 0 };
        m = (struct ip *) buff;

        int err = mkipv4(buff, 0, IPPROTO_UDP);
        EXPECT_EQ(-1,err);

        int err = mkipv4(NULL, 1024, IPPROTO_UDP);
        EXPECT_EQ(-1,err);

        int err = mkipv4(buff, 1024, 0);
        EXPECT_EQ(-1,err);
}


int test_mkudphdr()
{
        return 0;
}

TEST(mkicmpv4_test, mkicmpv4_works)
{
        char buff[128] = { 0 };
        char str[128] = { 0 };
        int datalen = 56;
        struct icmp *icmp = (struct icmp *) buff;
        struct icmp *m = (struct icmp *) str;
        EXPECT_NE(NULL, icmp);
        EXPECT_NE(NULL, m);

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

TEST(fill_data_test, fill_data_works)
{

}


int main(int argc, char **argv)
{
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
}

