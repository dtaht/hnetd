/*
 * Author: Pierre Pfister
 *
 * prefix_utils.c unit testing
 *
 */

#include "prefix_utils.h"
#include "sput.h"

#include <stdio.h>
#include <stdbool.h>

struct prefix p_allones_128 = {
		.prefix = { .s6_addr = {
				0xff,0xff, 0xff,0xff,  0xff,0xff, 0xff,0xff,
				0xff,0xff, 0xff,0xff,  0xff,0xff, 0xff,0xff}},
		.plen = 128 };
static const char *p_allones_128_s = "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff/128";

struct prefix p_allones_67 = {
		.prefix = { .s6_addr = {
				0xff,0xff, 0xff,0xff,  0xff,0xff, 0xff,0xff,
				0xff,0xff, 0xff,0xff,  0xff,0xff, 0xff,0xff}},
		.plen = 67 };
static const char *p_allones_67_s = "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff/67";

struct prefix p_allones_67_can = {
		.prefix = { .s6_addr = {
				0xff,0xff, 0xff,0xff,  0xff,0xff, 0xff,0xff,
				0xe0}},
		.plen = 67 };
static const char *p_allones_67_can_s = "ffff:ffff:ffff:ffff:e000::/67";

static struct prefix p1  = { { .s6_addr = {0x00,0x10}}, 12 };
static const char *p1_s = "10::/12";
static struct prefix p10 = { { .s6_addr = {0x00,0x10}}, 16 };
static struct prefix p11 = { { .s6_addr = {0x00,0x11}}, 16 };
static struct prefix p1f = { { .s6_addr = {0x00,0x1f}}, 16 };
static struct prefix p2  = { { .s6_addr = {0x00,0x20}}, 12 };

static struct prefix px  = { { .s6_addr = {0x00,0x20, 0x01}}, 12 };
static const char *px_s = "20:100::/12";

void bmemcpy(void *dst, const void *src,
		size_t frombit, size_t nbits);
void bmemcpy_shift(void *dst, size_t dst_start,
		const void *src, size_t src_start,
		size_t nbits);

void prefix_pton_t(void)
{
	struct prefix p;
	struct prefix p4_1 = {
			.prefix = { .s6_addr = {
					0x00,0x00, 0x00,0x00,  0x00,0x00, 0x00,0x00,
					0x00,0x00, 0xff,0xff,  0x0a }},
					.plen = 104 };

	struct prefix p4_2 = {
			.prefix = { .s6_addr = {
					0x00,0x00, 0x00,0x00,  0x00,0x00, 0x00,0x00,
					0x00,0x00, 0xff,0xff,  0xc0,0xa8, 0x00,0x01 }},
					.plen = 128 };

	sput_fail_unless(prefix_pton(p_allones_67_s, &p) == 1, "Parse 1");
	sput_fail_unless(!prefix_cmp(&p, &p_allones_67), "Parse value 1");

	sput_fail_unless(prefix_pton(p1_s, &p) == 1, "Parse 2");
	sput_fail_unless(!prefix_cmp(&p, &p1), "Parse value 2");

	sput_fail_unless(prefix_pton(px_s, &p) == 1, "Parse 3");
	sput_fail_unless(!prefix_cmp(&p, &px), "Parse value 3");

	sput_fail_unless(prefix_pton("10::/rrx", &p) == 0, "Parse error 1");
	sput_fail_unless(prefix_pton("10::/129", &p) == 0, "Parse error 2");
	sput_fail_unless(prefix_pton("10::/-1", &p) == 0, "Parse error 3");
	sput_fail_unless(prefix_pton("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255./10", &px) == 0, "Parse error 4");
	sput_fail_unless(prefix_pton("10.0.0.0/64", &px) == 0, "Parse error 5");
	sput_fail_unless(prefix_pton("blah-chombier/64", &px) == 0, "Parse error 6");

	/* Parsing IPv4 */
	sput_fail_unless(prefix_pton("10.0.0.0/8", &p) == 1, "Parse v4 1");
	sput_fail_unless(!prefix_cmp(&p, &p4_1), "Parse v4 value 1");

	sput_fail_unless(prefix_pton("192.168.0.1/32", &p) == 1, "Parse v4 2");
	sput_fail_unless(!prefix_cmp(&p, &p4_2), "Parse v4 value 2");

	/* Parsing without slash */
	sput_fail_unless(prefix_pton("192.168.0.1", &p) == 1, "Parse v4 without /");
	sput_fail_unless(!prefix_cmp(&p, &p4_2), "Parse v4 without /");

	sput_fail_unless(prefix_pton("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff", &p) == 1, "Parse v6 without /");
	sput_fail_unless(!prefix_cmp(&p, &p_allones_128), "Parse v6 without /");
}

void prefix_last_t(void)
{
	struct prefix px;
	sput_fail_unless(prefix_last(&px, &p10, 18) == -1, "Forbidden arg");

	sput_fail_unless(prefix_last(&px, &p10, 16) == 0, "Ret is 0");
	sput_fail_if(prefix_cmp(&px, &p10), "Unmodified");

	sput_fail_unless(prefix_last(&px, &p10, 12) == 0, "Ret is 0");
	sput_fail_if(prefix_cmp(&px, &p1f), "px == p1f");

	sput_fail_unless(prefix_last(&px, &p11, 12) == 0, "Ret is 0");
	sput_fail_if(prefix_cmp(&px, &p1f), "px == p1f");
}

void prefix_increment_t(void)
{
	struct prefix px;

	sput_fail_unless(prefix_increment(&px, &p1, 8) == 0, "Ret is 0");
	sput_fail_if(prefix_cmp(&px, &p2), "px == p2");

	sput_fail_unless(prefix_increment(&px, &p10, 8) == 0, "Ret is 0");
	sput_fail_if(prefix_cmp(&px, &p11), "px == p11");

	sput_fail_unless(prefix_increment(&px, &p10, 16) == 1, "Single possibility");
	sput_fail_if(prefix_cmp(&px, &p10), "px == p10");

	sput_fail_unless(prefix_increment(&px, &p1f, 12) == 1, "Looping");
	sput_fail_if(prefix_cmp(&px, &p10), "px == p10");

	sput_fail_unless(prefix_increment(&px, &p1f, 120) == -1, "Forbidden");

	px.plen = 120;
	sput_fail_unless(prefix_increment(&px, &px, 4) == -1, "Forbidden");
	px.plen = 42;
	sput_fail_unless(prefix_increment(&px, &px, 10) == 0, "32 bits authorized");
	px.plen = 43;
	sput_fail_unless(prefix_increment(&px, &px, 10) == -1, "33 bits forbidden");
}

void bmemcpy_t(void)
{
	uint8_t u1[] = {0xff, 0xff, 0xff, 0xff};
	uint8_t u2[] = {0x00, 0x00, 0x00, 0x00};
	uint8_t u3[] = {0xff, 0xff, 0x00, 0x00};
	uint8_t u4[] = {0x07, 0xff, 0xff, 0x00};
	uint8_t u5[] = {0x01, 0xff, 0xff, 0xc0};
	uint8_t u6[] = {0x00, 0xff, 0xff, 0xf0};
	uint8_t dst[4];

	bmemcpy(&dst, &u2, 0, 32);
	sput_fail_if(memcmp(dst, u2, 4), "32bit copy");

	memset(dst, 0, 4);
	bmemcpy(&dst, &u1, 0, 16);
	sput_fail_if(memcmp(dst, u3, 4), "16bit copy");

	memset(dst, 0, 4);
	bmemcpy(&dst, &u1, 5, 19);
	sput_fail_if(memcmp(dst, u4, 4), "5 to 24 bits copy");

	memset(dst, 0, 4);
	bmemcpy(&dst, &u1, 7, 19);
	sput_fail_if(memcmp(dst, u5, 4), "7 to 26 bits copy");

	memset(dst, 0, 4);
	bmemcpy(&dst, &u1, 8, 20);
	sput_fail_if(memcmp(dst, u6, 4), "8 to 28 bits copy");

}

void bmemcpy_shift_t(void)
{
	uint8_t u1[] = {0xff, 0xff, 0xff, 0xff};
	uint8_t u2[] = {0x00, 0x00, 0x00, 0x00};
	uint8_t u3[] = {0x03, 0xff, 0xfc, 0x00};
	uint8_t u4[] = {0x00, 0x18, 0x00, 0x00};
	uint8_t u5[] = {0x00, 0x00, 0xff, 0xe0};
	uint8_t u6[] = {0x01, 0x80, 0x00, 0x00};

	bmemcpy_shift(u2, 6, u1, 6, 16);
	sput_fail_if(memcmp(u2, u3, 4), "Copy 16 bits without shift");
	memset(u2, 0, 4);

	bmemcpy_shift(u2, 6, u1, 5, 16);
	sput_fail_if(memcmp(u2, u3, 4), "Copy 16 bits with shift");
	memset(u2, 0, 4);

	bmemcpy_shift(u2, 11, u1, 7, 2);
	sput_fail_if(memcmp(u2, u4, 4), "Copy 2 bits");
	memset(u2, 0, 4);

	bmemcpy_shift(u2, 16, u1, 5, 11);
	sput_fail_if(memcmp(u2, u5, 4), "Copy 11 bits");
	memset(u2, 0, 4);

	bmemcpy_shift(u2, 7, u1, 11, 2);
	sput_fail_if(memcmp(u2, u6, 4), "Copy 2 bits");
	memset(u2, 0, 4);
}

void prefix_print_nocan_t(void)
{
	char buff[PREFIX_MAXBUFFLEN];
	const char *ret;

	ret = prefix_ntop(buff, 5, &p_allones_128, false);
	sput_fail_if(ret, "Buffer too short (1)");

	ret = prefix_ntop(buff, 43,
			&p_allones_128, false);
	sput_fail_if(ret, "Buffer too short (2)");
}

void prefix_equal_t(void)
{
	sput_fail_if(prefix_cmp(&p_allones_67, &p_allones_67),
			"Same prefixes should be equal (1)");
	sput_fail_if(prefix_cmp(&p_allones_67_can, &p_allones_67_can),
			"Same prefixes should be equal (2)");
	sput_fail_if(prefix_cmp(&p1, &p1),
			"Same prefixes should be equal (3)");
	sput_fail_if(prefix_cmp(&p_allones_67, &p_allones_67_can),
			"Canonical prefix should equal non-canonical one");
	sput_fail_unless(prefix_cmp(&p1, &p10),
			"Different prefix length should imply not equal");
	sput_fail_unless(prefix_cmp(&p1, &p_allones_67),
			"Different prefixes should not be equal");
}

void prefix_canonical_t(void)
{
	struct prefix p;
	prefix_canonical(&p, &p_allones_67);

	sput_fail_if(memcmp(&p, &p_allones_67_can, sizeof(struct prefix)),
				"Canonical transform");
}

void prefix_print_can_t(void)
{
	char buff[PREFIX_MAXBUFFLEN];
	const char *ret;

	ret = prefix_ntop(buff, PREFIX_MAXBUFFLEN, &p_allones_67, true);
	sput_fail_if(strcmp(ret, p_allones_67_can_s), "Canonical prefix print");

	ret = prefix_ntop(buff, PREFIX_MAXBUFFLEN, &p_allones_67, false);
	sput_fail_if(strcmp(ret, p_allones_67_s), "Non canonical prefix print");


	ret = prefix_ntop(buff, PREFIX_MAXBUFFLEN,
				&p_allones_128, false);
	sput_fail_if(strcmp(ret, p_allones_128_s), "Print all_ones/128");

	ret = prefix_ntop(buff, PREFIX_MAXBUFFLEN,
			&p1, false);
	sput_fail_if(strcmp(ret, p1_s), "Print p1");

	ret = prefix_ntop(buff, PREFIX_MAXBUFFLEN,
			&px, false);
	sput_fail_if(strcmp(ret, px_s), "Print px");
}

void prefix_contains_t(void)
{
	sput_fail_if(prefix_contains(&p1, &p2),
			"p1 and p2 are disjoint");
	sput_fail_if(prefix_contains(&p2, &p1),
			"p1 and p2 are disjoint");
	sput_fail_unless(prefix_contains(&p1, &p11),
			"p1 contains p11");
	sput_fail_unless(prefix_contains(&p1, &p1f),
			"p1 contains p1f");
	sput_fail_if(prefix_contains(&p2, &p11),
			"p2 do not contain p11");
}

void prefix_cmp_t(void)
{
	sput_fail_unless(prefix_cmp(&p1, &p11) > 0,
			"Prefix compare diff. plen (1)");
	sput_fail_unless(prefix_cmp(&p11, &p1) < 0,
			"Prefix compare diff. plen (2)");
	sput_fail_unless(prefix_cmp(&p_allones_67, &p_allones_128) > 0,
			"Prefix compare diff. plen (3)");

	sput_fail_unless(prefix_cmp(&p2, &p1) > 0,
			"Prefix compare value (1)");
	sput_fail_unless(prefix_cmp(&p10, &p11) < 0,
			"Prefix compare value (2)");
}

void prefix_random_t(void)
{
	int i;
	struct prefix p;

	sput_fail_unless(prefix_random(&p_allones_67, &p, 60),
			"Too short plen for random prefix");

	prefix_random(&p_allones_67_can, &p, 67);
	sput_fail_if(prefix_cmp(&p_allones_67_can, &p),
			"Only one possible random prefix");

	bool success = true;
	for(i = 0; i < 20; i++) {
		prefix_random(&p_allones_67, &p, 70);
		if(!prefix_contains(&p_allones_67, &p)) {
			success = false;
			break;
		}
	}
	sput_fail_unless(success, "Random prefix is in src prefix");
}

void prefix_number_t()
{
	struct prefix p;
	prefix_number(&p, &p10, 0, 1);
	sput_fail_if(prefix_cmp(&p, &p10), "Correct prefix");
	prefix_number(&p, &p10, 1, 1);
	sput_fail_if(prefix_cmp(&p, &p11), "Correct prefix");
	prefix_number(&p, &p10, 1, 0);
	sput_fail_if(prefix_cmp(&p, &p10), "Correct prefix");
	prefix_number(&p, &p10, 0xf, 1);
	sput_fail_if(prefix_cmp(&p, &p11), "Correct prefix");
	prefix_number(&p, &p10, 0xf, 4);
	sput_fail_if(prefix_cmp(&p, &p1f), "Correct prefix");
}

int main(__attribute__((unused)) int argc, __attribute__((unused))char **argv)
{
  sput_start_testing();
  sput_enter_suite("prefix_utils"); /* optional */
  sput_run_test(bmemcpy_t);
  sput_run_test(bmemcpy_shift_t);
  sput_run_test(prefix_print_nocan_t);
  sput_run_test(prefix_equal_t);
  sput_run_test(prefix_canonical_t);
  sput_run_test(prefix_print_can_t);
  sput_run_test(prefix_contains_t);
  sput_run_test(prefix_cmp_t);
  sput_run_test(prefix_increment_t);
  sput_run_test(prefix_last_t);
  sput_run_test(prefix_pton_t);
  sput_run_test(prefix_number_t);
  sput_leave_suite(); /* optional */
  sput_finish_testing();
  return sput_get_return_value();
}
