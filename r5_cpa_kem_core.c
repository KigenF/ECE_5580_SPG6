#include "r5_cpa_kem_keygen.h"
#include <stdlib.h>
#include <stdio.h>

#include <libkeccak.a.headers/SP800-185.h>

#define len_tau_2 2048
#define d 594
#define n 1
#define k 594 // k = d/n.
#define kappa_bytes 16
#define q 8192
#define n_bar 7
#define h 238
#define nr_bits 10

#define SHAKE128_RATE 168
#define SHAKE256_RATE 136

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define LITTLE_ENDIAN16(x) (x)
#else
#define LITTLE_ENDIAN16(x) ( \
    (((x) & 0xFF00U) >> 8) |  \
    (((x) & 0x00FFU) << 8)    \
)
#endif

#define ROUND(x) ((int)(x + 0.5))
#define CEIL_DIV(a,b) ((a+b-1)/b)
#define BITS_TO_BYTES(b) (CEIL_DIV(b,8))

typedef cSHAKE_Instance cshake_ctx;

typedef struct {
    cshake_ctx generator_ctx; 
    uint8_t output[SHAKE128_RATE > SHAKE256_RATE ? SHAKE128_RATE : SHAKE256_RATE]; 
    size_t index;
    void (* generate)(uint8_t *output, const size_t output_len);
} drbg_ctx;

static drbg_ctx ctx;
static const uint8_t permutation_customization[2] = {0, 1};

void print_hex(const char *var, const unsigned char *data, const size_t nr_elements, const size_t element_size) {
    size_t i, ii;
    if (var != NULL) {
        printf("%s[%zu]=", var, nr_elements);
    }
    for (i = 0; i < nr_elements; ++i) {
        if (i > 0) {
            printf(" ");
        }
        for (ii = element_size; ii > 0; --ii) {
            printf("%02hhX", data[i * element_size + ii - 1]);
        }
    }
    if (var != NULL) {
        printf("\n");
    }
}

void *checked_malloc(size_t size) {
	void *tmp = malloc(size);
	if(tmp == NULL) abort();
	return tmp;
}

void *checked_calloc(size_t count, size_t size) {
	void *temp = calloc(count, size);
	if (temp == NULL) {
		abort();
	}
	return temp;
}

void cshake128(unsigned char *output, size_t output_len, const unsigned char *input, const size_t input_len, const unsigned char *customization, const size_t customization_len) {
	if (cSHAKE128(input, input_len * 8, output, output_len * 8, NULL, 0, customization, customization_len) != 0) {
		abort();
	}
}

int drbg_sampler16_2_once(uint16_t *x, const size_t xlen, const void *seed, const size_t seed_size, const uint32_t range) {
	cshake128((uint8_t *) x, xlen * sizeof (uint16_t), (const uint8_t *) seed, seed_size, NULL, 0); // For the case where seed is 16 bytes.

    /* Bring in range and flip byte order if necessary */
    for (size_t i = 0; i < xlen; ++i) {
        x[i] = (uint16_t) ((LITTLE_ENDIAN16(x[i])) & (range - 1));
    }

    return 0;
}

void create_A_master(uint8_t * in, uint16_t * out) {
	drbg_sampler16_2_once(out, len_tau_2, in, kappa_bytes, q);
}

void cshake128_init(cshake_ctx *ctx, const unsigned char *customization, const size_t customization_len) {
	if (cSHAKE128_Initialize(ctx, 0, NULL, 0, customization, customization_len * 8) != 0) {
		abort();
	}
}

void cshake128_absorb(cshake_ctx *ctx, const unsigned char *input, const size_t input_len) {
	if (cSHAKE128_Update(ctx, input, input_len * 8) != 0) {
		abort();
	}
	if (cSHAKE128_Final(ctx, NULL) != 0) {
		abort();
	}
}

void cshake128_squeezeblocks(cshake_ctx *ctx, unsigned char *output, const size_t nr_blocks) {
	if (cSHAKE128_Squeeze(ctx, output, nr_blocks * SHAKE128_RATE * 8) != 0) {
		abort();
	}
}

static void generate_cshake128(uint8_t *output, const size_t output_len) {
    size_t i, j;

    i = ctx.index;
    for (j = 0; j < output_len; j++) {
        if (i >= SHAKE128_RATE) {
            cshake128_squeezeblocks(&ctx.generator_ctx, ctx.output, 1);
            i = 0;
        }
        output[j] = ctx.output[i++];
    }
    ctx.index = i;
}

void drbg_init_customization(const void *seed, const size_t seed_size, const uint8_t *customization, const size_t customization_len) {
	cshake128_init(&ctx.generator_ctx, customization, customization_len);
	cshake128_absorb(&ctx.generator_ctx, seed, seed_size);
	ctx.index = SHAKE128_RATE;
	ctx.generate = &generate_cshake128;
}

int drbg(void *x, const size_t xlen) {
    ctx.generate(x, xlen);
    return 0;
}

uint16_t drbg_sampler16_2(const uint32_t range) {
    uint16_t rnd;
    drbg(&rnd, sizeof (rnd));
    rnd = LITTLE_ENDIAN16(rnd);
    rnd = (uint16_t) (rnd & (range - 1));
    return rnd;
}

static int permutation_tau_2(uint32_t *row_disp, const unsigned char *seed) {
    uint32_t i;
    uint16_t rnd;
    uint8_t *v = checked_calloc(len_tau_2, 1);

    drbg_init_customization(seed, kappa_bytes, permutation_customization, sizeof (permutation_customization));

    for (i = 0; i < k; ++i) {
        do {
            rnd = drbg_sampler16_2(len_tau_2);
        } while (v[rnd]);
        v[rnd] = 1;
        row_disp[i] = rnd;
    }

    free(v);

    return 0;
}

void create_A(uint8_t *sigma, uint16_t *A) {
    uint32_t i;
    uint16_t *A_master = NULL;
    uint32_t *A_permutation = NULL;
    const uint16_t els_row = (uint16_t) (k * n);
    size_t len_a;

    len_a = (size_t) (k * k * n);

    /* Create A/A_Master*/
    //A = checked_malloc(len_a * sizeof (*A));
	A_master = checked_malloc((size_t) (len_tau_2 + d) * sizeof (*A_master));
	create_A_master(sigma, A_master);
	memcpy(A_master + len_tau_2, A_master, d * sizeof (*A_master));

    /* Compute and apply the permutation to get A */
	A_permutation = checked_malloc(k * sizeof (*A_permutation));

	/* Compute and apply permutation */
	permutation_tau_2(A_permutation, sigma);
	for (i = 0; i < k; ++i) {
		memcpy(A + (i * els_row), A_master + A_permutation[i], els_row * sizeof (*A));
	}

	/* Free allocated memory */
	free(A_master);
	free(A_permutation);
}

void drbg_init(const void *seed, const size_t seed_size) {
    drbg_init_customization(seed, seed_size, NULL, 0);
}

uint16_t drbg_sampler16(const uint32_t range) {
    const uint32_t range_divisor = (uint32_t) (0x10000 / range); \
    const uint32_t range_limit = range * range_divisor;
    uint16_t rnd;
    do {
        drbg(&rnd, sizeof (rnd));
        rnd = (uint16_t) LITTLE_ENDIAN16(rnd);
    } while (rnd >= range_limit);
    rnd = (uint16_t) (rnd / range_divisor);
    return rnd;
}

static int create_secret_vector(int16_t *vector, const uint16_t len) {
    size_t i;
    uint16_t idx;
    memset(vector, 0, sizeof (*vector) * len);

    for (i = 0; i < h; ++i) {
        do {
            idx = drbg_sampler16(len);
        } while (vector[idx] != 0);
        vector[idx] = (i & 1) ? -1 : 1;
    }

    return 0;
}

void create_S_T(uint8_t * in, uint16_t * out) {
    size_t i;
    const uint16_t len = (uint16_t) (k * n);
	uint8_t *sk = in;
	int16_t *S_T = out;

    /* Initialize drbg */
    drbg_init(sk, kappa_bytes);

    /* Create rows of sparse vectors */
    for (i = 0; i < n_bar; ++i) {
        create_secret_vector(&S_T[i * len], len);
    }
}

size_t pack(unsigned char *packed, const uint16_t *m, const size_t els) {
    const size_t packed_len = (size_t) (BITS_TO_BYTES(els * nr_bits));
    const uint16_t mask = (uint16_t) ((1 << nr_bits) - 1);
    size_t i;
    uint16_t val;
    size_t bits_done = 0;
    size_t idx;
    size_t bit_idx;

    memset(packed, 0, packed_len);
    if (nr_bits == 8) {
        for (i = 0; i < els; ++i) {
            packed[i] = (uint8_t) m[i];
        }
    } else {
        for (i = 0; i < els; ++i) {
            idx = bits_done >> 3;
            bit_idx = bits_done & 7;
            val = m[i] & mask;
            packed[idx] = (uint8_t) (packed[idx] | (val << bit_idx));
            if (bit_idx + nr_bits > 8) {
                /* Spill over to next packed byte */
                packed[idx + 1] = (uint8_t) (packed[idx + 1] | (val >> (8 - bit_idx)));
                if (bit_idx + nr_bits > 16) {
                    /* Spill over to next packed byte */
                    packed[idx + 2] = (uint8_t) (packed[idx + 2] | (val >> (16 - bit_idx)));
                }
            }
            bits_done += nr_bits;
        }
    }

    return packed_len;
}

void pack_pk(uint8_t * in0, uint16_t * in1, uint8_t * out) {
    size_t packed_idx = 0;
	unsigned char * packed_pk = out;
	unsigned char * sigma = in0;
	uint16_t *B = in1;

	/*printf("B = \n[");
	for(size_t i = 0; i < d * n_bar; i ++)
	{
		if(i % 8 == 0 && i > 0) printf("\n ");
		printf("0x%04x, ", *(B + i));
	}
	printf("]\n");*/

    /* Pack sigma */
    memcpy(packed_pk + packed_idx, sigma, kappa_bytes);
    packed_idx += kappa_bytes;
    /* Pack B */
    packed_idx += pack((packed_pk + packed_idx), B, d * n_bar);
	/*printf("packed_pk: \n[");
	for(int i = 0; i < 5214; i ++)
	{
		if(i > 0 && i % 7 == 0) printf("\n ");
		printf("0x%04x, ", *(packed_pk + i));
	}
	printf("]\n");*/
}
