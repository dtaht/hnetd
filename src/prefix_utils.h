/*
 * Author: Pierre Pfister <pierre pfister@darou.fr>
 *
 * Copyright (c) 2014 Cisco Systems, Inc.
 *
 * Prefixes manipulation utilities.
 *
 */

#ifndef UTILS_H
#define UTILS_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>

/* Prefix structure.
 * All bits following the plen first are ignored. */
struct prefix {
	struct in6_addr prefix;
	uint8_t plen;
};

extern struct prefix ipv4_in_ipv6_prefix;
extern struct prefix ipv6_ula_prefix;
extern struct prefix ipv6_ll_prefix;
extern struct prefix ipv6_global_prefix;

ssize_t unhexlify(uint8_t *dst, size_t len, const char *src);
char *hexlify(char *dst, const uint8_t *src, size_t len);

/* Copy bits of memory from src to dst.
 * Starts from bit #frombit and copies nbits.
 */
void bmemcpy(void *dst, const void *src, size_t frombit, size_t nbits);

/* Tests whether p1 contains p2 */
bool prefix_contains(const struct prefix *p1,
					const struct prefix *p2);

#define prefix_is_ipv4(prefix) \
	prefix_contains(&ipv4_in_ipv6_prefix, prefix)

#define prefix_is_ipv6_ula(prefix) \
	prefix_contains(&ipv6_ula_prefix, prefix)

#define prefix_is_ll(prefix) \
	prefix_contains(&ipv6_ll_prefix, prefix)

#define prefix_is_global(prefix) \
	prefix_contains(&ipv6_global_prefix, prefix)

/* Compare two prefixes according to a complete order definition.
 * Will be used in trees.
 * Returns zero if equals, positive if p1 > p2,
 * negative value otherwise.
 * A prefix with longer prefix length is always smaller.
 * When prefix length is the same, bigger prefix value is bigger. */
int prefix_cmp(const struct prefix *p1,
		const struct prefix *p2);

void prefix_cpy(struct prefix *dst, const struct prefix *src);

/* Returns the IPv6 prefix length if p is IPv6, and the
 * IPv4 prefix length if p is IPv4. */
uint8_t prefix_af_length(const struct prefix *p);


/* Choose a random prefix of length plen, inside p, and returns 0.
 * Or returns -1 if there is not enaugh space in p for a prefix of
 * length plen. */
int prefix_random(const struct prefix *p, struct prefix *dst,
		uint8_t plen);

/* Generates a pseudo-random sub-prefix of p, of length plen.
 * The seed is used to generate the random part ctr may be used to
 * generate other random values with the same seed. */
int prefix_prandom(const char *seed, size_t seedlen, uint32_t ctr,
		const struct prefix *p, struct prefix *dst,
		uint8_t plen);

/* Increments the prefix by one. The protected_len first bits are never modified.
 * Instead, the increment loops back to the first prefix (all other bits are zero).
 * (p->plen - protected_len) must be <= 32 and  p->plen < protected_len.
 * Returns -1 if this requirement is not respected.
 * Returns 1 if the returned address is the smallest address (All zeroes except the protected bits).
 * Returns 0 otherwise.
 * Can be used with dst == p */
int prefix_increment(struct prefix *dst, const struct prefix *p, uint8_t protected_len);

/* Gets the highest prefix that can be returned by increment.
 * Returns -1 if p->plen < protected_len. Returns 0 otherwise.
 * Can be used with dst == p. */
int prefix_last(struct prefix *dst, const struct prefix *p, uint8_t protected_len);

/* Sets prefix's last bits to zero.
 * May be useful when printing the prefix.
 * src and dst can be the same pointer. */
void prefix_canonical(struct prefix *dst, const struct prefix *src);

void prefix_number(struct prefix *dst, const struct prefix *src, uint32_t id, uint8_t id_len);

/* Maximum needed space to print a prefix */
#define PREFIX_MAXBUFFLEN (INET6_ADDRSTRLEN + 4)

/* Writes the prefix into the specified buffer of length dst_len.
 * Returns dst upon success and NULL if buffer size is too small.
 * Canonical means the last bits of the prefix are considered as
 * zeros. */
char *prefix_ntop(char *dst, size_t dst_len,
		const struct prefix *prefix,
		bool canonical);

/* Parses IPv4 and IPv6 prefixes and write it into IPv6 prefix structure.
 * If no prefix-length is provided, 128 is used.
 * The complete address is written in p, even bits that are after the
 * prefix length. */
int prefix_pton(const char *addr, struct prefix *p);

/* This is provided as an option, to have prefix_ntop with
 * heap-allocated destination buffer (if we run out of heap, we have
 * lot bigger problems too so little need to worry here). */
#define PREFIX_NTOP(p, canonical)  \
	prefix_ntop(alloca(PREFIX_MAXBUFFLEN), PREFIX_MAXBUFFLEN, p, canonical)

/* Convenience variant of PREFIX_NTOP, which just chooses to
 * canonicalize the prefix. */
#define PREFIX_REPR(p) PREFIX_NTOP(p, true)

#define ADDR_REPR(addr) \
		(IN6_IS_ADDR_V4MAPPED(addr))?\
				inet_ntop(AF_INET, &(addr)->s6_addr[12], alloca(INET_ADDRSTRLEN), INET_ADDRSTRLEN):\
				inet_ntop(AF_INET6, addr, alloca(INET6_ADDRSTRLEN), INET6_ADDRSTRLEN)

#endif
