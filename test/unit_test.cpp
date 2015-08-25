/**
 * @author: Paul Kirth
 * @file: ncd.c
 */
#include <gtest/gtest.h>
#include "unit_test.hpp"

//extern "C" {
//#include "ncd.hpp"
//}


TEST(get_time_test, get_time_correct){
	struct timeval tv;
	double d, r;
	d = get_time();
	gettimeofday(&tv, NULL);
	r = tv.tv_sec;
	r += tv.tv_usec / 1000000;
	EXPECT_FLOAT_EQ(d, r);
}


TEST(init_detection_test, init_detection_works){


}

TEST(mkipv4_test, mkipv4_sets_values_correctly){


	/* for get addrinfo */
	struct addrinfo hints = {0};

	/* set up hints for getaddrinfo() */
	hints.ai_flags = AI_CANONNAME;
	hints.ai_protocol = IPPROTO_UDP;
	//struct addrinfo *res;

	int err = getaddrinfo("192.168.1.100", NULL, &hints, &res);
	EXPECT_EQ(0, err);

	int ttl = 50;
	struct ip ip = {0};
	struct ip* m = NULL;
	inet_pton(AF_INET, "192.168.1.100", &ip.ip_dst);
	ip.ip_hl = 5;
	ip.ip_id = htons(getpid());
	ip.ip_len = htons(1024);
	ip.ip_ttl = htons(ttl);
	ip.ip_v = 4;
	ip.ip_p = IPPROTO_UDP;
	destip = ip.ip_dst;

	char buff[1024] = {0};
	m = (struct ip*)buff;
	mkipv4(buff, 1024, IPPROTO_UDP);
	EXPECT_EQ(ip.ip_dst.s_addr, m->ip_dst.s_addr);
	EXPECT_EQ(ip.ip_id, m->ip_id);
	EXPECT_EQ(ip.ip_len, m->ip_len);
	EXPECT_EQ(ip.ip_ttl, m->ip_ttl);
	EXPECT_EQ(ip.ip_v, m->ip_v);
	EXPECT_EQ(ip.ip_p, m->ip_p);
	freeaddrinfo(res);
}

TEST(mkipv4_test, mkipv4_bad_inputs){
	char buff[1024] = {0};
	EXPECT_DEATH_IF_SUPPORTED(mkipv4(buff, 0, IPPROTO_UDP), "Invalid argument used in ICMP packet allocation");
	EXPECT_DEATH_IF_SUPPORTED(mkipv4(NULL, 1024, IPPROTO_UDP), "Invalid argument used in ICMP packet allocation");
	EXPECT_DEATH_IF_SUPPORTED(mkipv4(buff, 2000, IPPROTO_UDP), "Invalid argument used in ICMP packet allocation");
	EXPECT_DEATH_IF_SUPPORTED(mkipv4(buff, 1024, 0), "Invalid argument used in ICMP packet allocation");

}


TEST(mkicmpv4_test, mkicmpv4_sets_values_correctly){
	char buff[128] = {0};
	char str[128] = {0};
	int datalen = 56;
	struct icmp* icmp = (struct icmp*)buff;
	struct icmp* m = (struct icmp*)str;
	EXPECT_NE(nullptr, icmp);
	EXPECT_NE(nullptr, m);

	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	icmp->icmp_id = (u_int16_t)getpid();

	mkicmpv4(str, datalen);
	memcpy(icmp->icmp_data, m->icmp_data, datalen);
	icmp->icmp_seq = m->icmp_seq;        // so they can match (its random!)
	icmp->icmp_cksum = 0;
	icmp->icmp_cksum = ip_checksum(icmp, datalen + sizeof(struct icmp));

	EXPECT_EQ(icmp->icmp_cksum, m->icmp_cksum);
	EXPECT_EQ(icmp->icmp_code, m->icmp_code);
	EXPECT_EQ(icmp->icmp_type, m->icmp_type);
	EXPECT_EQ(icmp->icmp_id, m->icmp_id);
}


TEST(mkicmpv4_test, mkicmpv4_bad_inputs){
	char buff[1024] = {0};
	EXPECT_DEATH_IF_SUPPORTED(mkicmpv4(NULL, 1024), "Invalid argument used in ICMP packet header: NULL pointer used");
	EXPECT_DEATH_IF_SUPPORTED(mkicmpv4(buff, 2000), "Invalid argument used for ICMP packet header length");
}


TEST(fill_data_test, fill_data_bad_input){
	EXPECT_DEATH_IF_SUPPORTED(fill_data(NULL, 100), "Error opening file");
	char str[64] = "/dev/urandom";
	extern char* file;
	EXPECT_DEATH_IF_SUPPORTED(fill_data(NULL, 100), "Error opening file");
	file = str;
	char buff[1500] = {0};
	EXPECT_DEATH_IF_SUPPORTED(fill_data(NULL, 100), "Error reading file");
}

TEST_F(DetectionInitTest, test_init){
	init_detection();
	EXPECT_EQ(64, icmp_len);
	EXPECT_EQ(0, seq);
	EXPECT_NE(NULL, (uint64_t)res);
	EXPECT_STREQ("127.0.0.1", inet_ntoa(destip));
	EXPECT_LT(0, send_fd);
	EXPECT_LT(0, icmp_fd);
	EXPECT_EQ(64, ttl);
	EXPECT_EQ(5, tail_wait);
	EXPECT_EQ(20, num_tail);
	EXPECT_EQ(3000, num_packets);
	EXPECT_EQ(3, cooldown);
}


TEST_F(MeasureTest, measure_works)
{
	int err;
	err = init_detection();
	ASSERT_EQ(EXIT_SUCCESS, err);

	ASSERT_NO_FATAL_FAILURE(err = measure());
	ASSERT_EQ(EXIT_SUCCESS, err);

	if(tcp_bool)
			EXPECT_DOUBLE_EQ(time_val, td);
	else
			EXPECT_NE(time_val, td);
	EXPECT_NE(0, time_val);
	EXPECT_EQ(1, stop);
	EXPECT_EQ(0, recv_ready);
	EXPECT_EQ(num_packets, udp_ack);
}


TEST(check_args_test, Check_args_works){
	char const* args[6] = {"ncd_main", "127.0.0.1", "-n3000", "-t64", "-w5", "-c3"};
	size_t sz = sizeof(args) / sizeof(args[0]);
	//printf("Args length = %lu\n", sz);
	check_args(sz, (char**)args);

	EXPECT_EQ(64, ttl);
	EXPECT_EQ(5, tail_wait);
	EXPECT_EQ(20, num_tail);
	EXPECT_EQ(3000, num_packets);
	EXPECT_EQ(3, cooldown);

}

int main(int argc, char** argv){
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

