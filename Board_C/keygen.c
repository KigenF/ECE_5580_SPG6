#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
// #include "keccak_generic32/libXKCP.a.headers/KeccakHash.h"
// #include "keccak_generic32/libXKCP.a.headers/SP800-185.h"
#include "generalFunctions.h"
#include "shake.h"
#include "oryx/xof/cshake.h"

#define p 1024

#define len_tau_2 2048
#define d 594
#define n 1
#define k 594 // k = d/n.
#define kappa_bits 16
#define q 8192
#define n_bar 7
#define m_bar 7
#define h 238
#define p_bits 10
#define t_bits 7
#define _b_bits 3
#define cipher_bits 5236
#define mu 43

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

// typedef cSHAKE_Instance cshake_ctx;
typedef CshakeContext cshake_ctx;
// Context struct for the XKCP
// typedef struct {
//     cshake_ctx generator_ctx; 
//     uint8_t output[SHAKE128_RATE > SHAKE256_RATE ? SHAKE128_RATE : SHAKE256_RATE]; 
//     size_t index;
//     void (* generate)(uint8_t *output, const size_t output_len);
// } drbg_ctx;

// Context for Oryx 
typedef struct {
    cshake_ctx generator_ctx; 
    uint8_t output[SHAKE128_RATE > SHAKE256_RATE ? SHAKE128_RATE : SHAKE256_RATE]; 
    size_t index;
    void (* generate)(uint8_t *output, const size_t output_len);
} drbg_ctx;

static drbg_ctx ctx;
static const uint8_t permutation_customization[2] = {0, 1};

static int round_element(uint16_t *x, const uint16_t a_bits, const uint16_t b_bits, const uint16_t b_mask, const uint16_t rounding_constant) {
    const uint16_t shift = (uint16_t) (a_bits - b_bits);
    *x = (uint16_t) (*x + rounding_constant);
    *x = (uint16_t) (*x >> shift);
    *x &= b_mask;
    return 0;
}

int round_matrix(uint16_t *matrix, const size_t len, const size_t els, const uint16_t a, const uint16_t b, const uint16_t rounding_constant) {
    size_t i;

    uint16_t b_mask = (uint16_t) (((uint16_t) 1 << b) - 1);
    for (i = 0; i < len * els; ++i) {
        round_element(matrix + i, a, b, b_mask, rounding_constant);
    }

    return 0;
}



static void generate_cshake128(uint8_t *output, const size_t output_len) {
    size_t i, j;

    i = ctx.index;
    for (j = 0; j < output_len; j++) {
        if (i >= SHAKE128_RATE) {
            // if(cSHAKE128_Squeeze(&ctx.generator_ctx, ctx.output, SHAKE128_RATE * 8) !=0){
            //     abort();
            // }
            cshakeSqueeze(&ctx.generator_ctx, ctx.output, SHAKE128_RATE * 8);
            i = 0;
        }
        output[j] = ctx.output[i++];
    }
    ctx.index = i;
}

static int permutation_tau_2(uint32_t *row_disp, const unsigned char *seed){
    uint32_t i;
    uint16_t rnd;
    uint8_t *v = calloc(len_tau_2, 1), tmp_rnd;

    // drbg_init_customization(seed, kappa_bits, permutation_customization, sizeof (permutation_customization));
    
    // if(cSHAKE128_Initialize(&ctx.generator_ctx, 0, NULL, 0, permutation_customization, sizeof(permutation_customization)) != 0){


    if(cshakeInit(&ctx.generator_ctx, 0, NULL, 0, (char_t *)permutation_customization, sizeof(permutation_customization))){
        abort();
    }

    // if (cSHAKE128_Update(&ctx.generator_ctx, seed, kappa_bits * 8) != 0) {
	// 	abort();
	// }

    cshakeAbsorb(&ctx.generator_ctx, seed, kappa_bits * 8);

	// if (cSHAKE128_Final(&ctx.generator_ctx, NULL) != 0) {
	// 	abort();
	// }

    cshakeFinal(&ctx.generator_ctx);

    ctx.index = SHAKE128_RATE;
    ctx.generate = &generate_cshake128;
    
    for (i = 0; i < k; ++i) {
        do {
            // drbg(&tmp_rnd, sizeof (tmp_rnd));
            ctx.generate(&tmp_rnd, sizeof(tmp_rnd));
            tmp_rnd = LITTLE_ENDIAN16(tmp_rnd);
            tmp_rnd = (uint16_t) (tmp_rnd & (len_tau_2 - 1));
            rnd = tmp_rnd;
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

    // len_a = (size_t) (k * k * n);

    /* Create A/A_Master*/
    //formerly a checked malloc, but may need to replace because of heap limitations
	A_master = malloc((size_t) (len_tau_2 + d) * sizeof (*A_master));

	// create_A_master(sigma, A_master);, actually drbg_sampler16_2_once
    // Attempt to replace craete_A_master(), which simply calls drbg_sampler16_2_once
    shake128((uint8_t *) A_master, len_tau_2 * sizeof(uint16_t), (const uint8_t *) sigma, kappa_bits);
    for (size_t i = 0; i < len_tau_2; ++i) {
        A_master[i] = (uint16_t) ((LITTLE_ENDIAN16(A_master[i])) & (q - 1));
    }
    
	memcpy(A_master + len_tau_2, A_master, d * sizeof (*A_master));

    /* Compute and apply the permutation to get A */
	A_permutation = malloc(k * sizeof (*A_permutation));

	/* Compute and apply permutation */
	permutation_tau_2(A_permutation, sigma);

	for (i = 0; i < k; ++i) {
		memcpy(A + (i * els_row), A_master + A_permutation[i], els_row * sizeof (*A));
	}

	/* Free allocated memory */
	free(A_master);
	free(A_permutation);
}

static int create_secret_vector(uint16_t *vector, const uint16_t len) {
    size_t j;
    uint16_t idx;
    const uint32_t range_divisor = (uint32_t) (0x10000 / len); \
    const uint32_t range_limit = len * range_divisor;
    uint16_t rnd;
    uint8_t srnd; 

    memset(vector, 0, sizeof(vector) * len);

    for (j = 0; j < h; ++j) {
        do {
            // idx = drbg_sampler16(len);
            do {
                // drbg(&rnd, sizeof (rnd));
                ctx.generate(&srnd, sizeof(rnd));
                rnd = (uint16_t)(srnd);
                rnd = (uint16_t) LITTLE_ENDIAN16(rnd);
            } while (rnd >= range_limit);
            rnd = (uint16_t) (rnd / range_divisor);
            idx = rnd;
        } while (vector[idx] != 0);
        vector[idx] = (j & 1) ? -1 : 1;
    }

    return 0;
}

void create_S_T(uint8_t *in, uint16_t *out){
    size_t i;
    const uint16_t len = (uint16_t) (k * n);
	uint8_t *sk = in;
	uint16_t *S_T = out;

    /* Initialize drbg */
    // drbg_init(sk, kappa_bits);
    // if(cSHAKE128_Initialize(&ctx.generator_ctx, 0, NULL, 0, NULL, 0) != 0){
    if(cshakeInit(&ctx.generator_ctx, 0, NULL, 0, NULL, 0)){
        abort();
    }
    // if (cSHAKE128_Update(&ctx.generator_ctx, sk, kappa_bits * 8) != 0) {
	// 	abort();
	// }
    cshakeAbsorb(&ctx.generator_ctx, sk, kappa_bits * 8);
	// if (cSHAKE128_Final(&ctx.generator_ctx, NULL) != 0) {
	// 	abort();
	// }
    cshakeFinal(&ctx.generator_ctx);
    ctx.index = SHAKE128_RATE;
    ctx.generate = &generate_cshake128;

    /* Create rows of sparse vectors */
    for (i = 0; i < n_bar; ++i) {
        create_secret_vector(&S_T[i * len], len);
    }
}

size_t pack(uint8_t *packed, const uint16_t *m, const size_t els, const uint8_t nr_bits){
    size_t packed_len = (size_t)(((els * nr_bits)+7)/ 8);
    uint16_t mask = (uint16_t)((1 << nr_bits) -1);
    size_t bits_done = 0;
    size_t i, index, bit_index; 
    uint16_t val;

    memset(packed, 0, packed_len);
    if(nr_bits == 8){
        for(i = 0; i < els; i++){
            packed[i] = (uint8_t) m[i];
        }
    }
    else{
        for(i = 0; i < els; i++){
            index = bits_done >> 3;
            bit_index = bits_done & 7;
            val = m[i] & mask;
            packed[index] = (uint8_t) (packed[index] | (val << bit_index));
            if (bit_index + nr_bits > 8) {
                /* Spill over to next packed byte */
                packed[index + 1] = (uint8_t) (packed[index + 1] | (val >> (8 - bit_index)));
                if (bit_index + nr_bits > 16) {
                    /* Spill over to next packed byte */
                    packed[index + 2] = (uint8_t) (packed[index + 2] | (val >> (16 - bit_index)));
                }
            }
            bits_done += nr_bits;
        }
    }
    return  packed_len;
}

void pack_pk(uint8_t *sigma, uint16_t *b, uint8_t *out){
    size_t packing_index = 0;
    uint8_t *packed_pk = out;

    memcpy(packed_pk, sigma, kappa_bits);
    packing_index += kappa_bits;

    packing_index += pack((packed_pk + packing_index), b, d*n_bar, p_bits);
}

int pkeKeygen(uint8_t pk_byte[5214], uint8_t sk_byte[16], uint8_t sigma_byte[16]){
    uint16_t A[d*d];// = checked_malloc(d * d * sizeof(int16_t)); 
    uint16_t S_T[n_bar*d];// = checked_malloc(n_bar * d * sizeof(int16_t)); 
    uint16_t S[d*n_bar];// = checked_malloc(d * n_bar * sizeof(int16_t));
    uint16_t B[d*n_bar];// = checked_malloc(d * n_bar * sizeof(int16_t));
    //create A
    create_A(sigma_byte, A);
    //Create S_T
    create_S_T(sk_byte, S_T);
    //Transpose S_T to S
    transpose(S_T, S, n_bar, d);
    free(S_T);
    //dotMatMat A and S
    dotMatMat(B, A, d, d, S, n_bar, d);
    //round B 
    round_matrix(B, (size_t)d * n_bar, n, q, p_bits, 4);
    //pack_pk with sigma and B_b
    pack_pk(sigma_byte, B, pk_byte);
    return 0;
}

int kemKeygen(uint8_t sigma[16], uint8_t sk[16], uint8_t pk[5214]){

    // if(pkeKeygen(pk, sk, sigma) == 0){
    //     return 0;
    // }
    // return -1;
    pkeKeygen(pk,sk,sigma);
    return 0;
    
}

int add(int a, int b){
    return (a + b);
}
