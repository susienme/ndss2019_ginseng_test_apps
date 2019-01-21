#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <slib.h>

#define ENABLE_RESET_COUNTERS() 	\
	asm volatile (					\
		"mov x15, 0x80000000\n" 	\
		"msr PMCNTENSET_EL0, x15\n"	\
									\
		"mov x15, 5\n"				\
		"mrs x14, pmcr_el0\n"		\
		"orr x14, x14, x15\n" 		\
		"msr pmcr_el0, x14\n"		\
	::: "x15", "x14"				\
	)

#define READ_COUNTER(x)				\
	asm volatile (					\
		"mrs %0, pmccntr_el0\n"		\
		: "=r" (x)					\
		::"memory"					\
		)

// Autogen UUID: 0x83fedcca14e043118d62d3834d812cf8
#define TOKEN_KEYTOP_TOP 		0x83fedcca14e04311
#define TOKEN_KEYTOP_BOTTOM 	0x8d62d3834d812cf8

// Autogen UUID: 0x0dfde1e0e1bc4b50a9265ae952a1b3f2
#define TOKEN_KEYBOTTOM_TOP 	0x0dfde1e0e1bc4b50
#define TOKEN_KEYBOTTOM_BOTTOM 	0xa9265ae952a1b3f2

void sha1_block_armv8(uint32_t *, const uint8_t *, long, long, long);
#define SHA1_BLOCK_DATA_ORDER sha1_block_armv8

#define SHA_CBLOCK 64
typedef struct sha_state_st {
  uint32_t h[5];
  uint32_t Nl, Nh;
  uint8_t data[SHA_CBLOCK];
  unsigned num;
} SHA_CTX;


#define MY_MEMSET memset
#define MY_BZERO bzero
#define MY_MEMCPY memcpy

int SHA1_init(SHA_CTX *sha) {
  MY_MEMSET(sha, 0, sizeof(SHA_CTX));
  sha->h[0] = 0x67452301UL;
  sha->h[1] = 0xefcdab89UL;
  sha->h[2] = 0x98badcfeUL;
  sha->h[3] = 0x10325476UL;
  sha->h[4] = 0xc3d2e1f0UL;
  return 1;
}

#define HOST_l2c(l, c)                        \
  do {                                        \
    *((c)++) = (uint8_t)(((l) >> 24) & 0xff); \
    *((c)++) = (uint8_t)(((l) >> 16) & 0xff); \
    *((c)++) = (uint8_t)(((l) >> 8) & 0xff);  \
    *((c)++) = (uint8_t)(((l)) & 0xff);       \
  } while (0)


#define HASH_MAKE_STRING(c, s) \
  do {                         \
    uint32_t ll;               \
    ll = (c)->h[0];            \
    HOST_l2c(ll, (s));         \
    ll = (c)->h[1];            \
    HOST_l2c(ll, (s));         \
    ll = (c)->h[2];            \
    HOST_l2c(ll, (s));         \
    ll = (c)->h[3];            \
    HOST_l2c(ll, (s));         \
    ll = (c)->h[4];            \
    HOST_l2c(ll, (s));         \
  } while (0)

#define SHA_CBLOCK 64

int SHA1_updateOneQ(SHA_CTX *c, 
					const uint8_t *data1st, // must be 64: doesn't contain secret; only paddings
					slong tmp_key_top, 		// XORed secret
					slong tmp_key_bottom, 	// XORed secret
					const uint8_t *data2nd,	// time or SHA: not secret
					size_t len,				// len of time or SHA
					uint8_t *out
					) {			
	uint8_t data[SHA_CBLOCK*2];
	uint8_t *p;
	MY_BZERO(data, SHA_CBLOCK*2);
	MY_MEMCPY(data, data1st, SHA_CBLOCK);
	MY_MEMCPY(data+SHA_CBLOCK, data2nd, len);

	/*printf("We have key! tmp_key_top    = 0x%lX\n", tmp_key_top);
	printf("We have key! tmp_key_bottom = 0x%lX\n", tmp_key_bottom);*/

	data[SHA_CBLOCK+len] = 0x80;
	p = data + SHA_CBLOCK*2 - 8;

	HOST_l2c(0, p);
	HOST_l2c(512 + len*8, p);

	SHA1_BLOCK_DATA_ORDER(c->h, data, 2, tmp_key_top, tmp_key_bottom);
	HASH_MAKE_STRING(c, out); // change 5 ints -> 20 chars
	
	return 1;
}


#define SHA1_DIGEST_LENGTH 20
void hmac_sha1(slong key_top, slong key_bottom,
               const uint8_t *data,
               uint8_t *result) {
	SHA_CTX ctx;
	uint8_t hashed_key[SHA1_DIGEST_LENGTH];
	uint8_t tmp_key[64];
	uint8_t sha[SHA1_DIGEST_LENGTH];
	slong tmp_key_top;
	slong tmp_key_bottom;

	// The key for the inner digest is derived from our key, by padding the key
	// the full length of 64 bytes, and then XOR'ing each byte with 0x36.
	tmp_key_top = key_top ^ 0x3636363636363636UL;
	tmp_key_bottom = key_bottom ^ 0x3636363636363636UL;

	MY_MEMSET(tmp_key, 0x36, 64);
	
	SHA1_init(&ctx);
	SHA1_updateOneQ(&ctx, tmp_key, tmp_key_top, tmp_key_bottom, data, 8, sha);

	// The key for the outer digest is derived from our key, by padding the key
	// the full length of 64 bytes, and then XOR'ing each byte with 0x5C.
	tmp_key_top = key_top ^ 0x5C5C5C5C5C5C5C5CUL;
	tmp_key_bottom = key_bottom ^ 0x5C5C5C5C5C5C5C5CUL;
	MY_MEMSET(tmp_key, 0x5C, 64);

	// Compute outer digest
	SHA1_init(&ctx);
	SHA1_updateOneQ(&ctx, tmp_key, tmp_key_top, tmp_key_bottom, sha, SHA1_DIGEST_LENGTH, result);
}


int genCode(slong key_top, slong key_bottom) {
	/*
	 * SDATA: key_top(arg), key_bottom(arg)
	 */
	uint8_t challenge[8];
	uint64_t tm = time(NULL)/30;
	uint8_t resultFull[SHA1_DIGEST_LENGTH];
  	unsigned int truncatedHash = 0;
  	int offset = 0;

	for (int i = 8; i--; tm >>= 8) 
		challenge[i] = tm;

  	hmac_sha1(	key_top, key_bottom, 		// key
  				challenge, 					// time to be hashed
  				resultFull);	// result
  	offset = resultFull[SHA1_DIGEST_LENGTH - 1] & 0xF;

	for (int i = 0; i < 4; ++i) {
		truncatedHash <<= 8;
		truncatedHash  |= resultFull[offset + i];
	}
	truncatedHash &= 0x7FFFFFFF;
	truncatedHash %= 1000000;

	printf("OTP: %06d\n", truncatedHash);
	return truncatedHash;
}

void run() {
	/*
	 * SDATA: key_top, key_bottom
	 */

	slong key_top;
	slong key_bottom;
	
	ss_read(TOKEN_KEYTOP_TOP, TOKEN_KEYTOP_BOTTOM, key_top);
	ss_read(TOKEN_KEYBOTTOM_TOP, TOKEN_KEYBOTTOM_BOTTOM, key_bottom);
	
	genCode(key_top, key_bottom);
}

#define SYSTIME(t, o) 						\
	t.tv_sec = t.tv_nsec = 0;				\
	clock_gettime(CLOCK_MONOTONIC, &t);		\
	o = t.tv_sec*1000000000UL + t.tv_nsec;

int main (int argc, char *argv[]) {
	unsigned long t_start, t_end;

	struct timespec t;
	unsigned long t_start_ns, t_end_ns;

	ss_init();

	run();

	return 0;
}