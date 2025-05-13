#include <stdlib.h>
#include <stdint.h>
#include "shake.h"

#define p 1024

#define len_tau_2 100
// #define d 594
#define d 100
#define n 1
// #define k 594 // k = d/n.
#define k 100 // k = d/n.
#define kappa_bytes 16
#define q 8192
#define n_bar 7
#define m_bar 7
#define h 98
#define p_bits 10
#define t_bits 7
#define _b_bits 3
#define cipher_bits 5236
#define mu 43
#define sk_bits 16

// #define SHAKE128_RATE 168
// #define SHAKE256_RATE 136

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define LITTLE_ENDIAN16(x) (x)
#else
#define LITTLE_ENDIAN16(x) ( \
    (((x) & 0xFF00U) >> 8) |  \
    (((x) & 0x00FFU) << 8)    \
)
#endif

// typedef CshakeContext cshake_ctx;
// // Context for Oryx 
// typedef struct {
//     cshake_ctx generator_ctx; 
//     uint8_t output[SHAKE128_RATE > SHAKE256_RATE ? SHAKE128_RATE : SHAKE256_RATE]; 
//     size_t index;
//     void (* generate)(uint8_t *output, const size_t output_len);
// } drbg_ctx;

// static drbg_ctx ctx;



// void generate_cshake128(uint8_t *output, const size_t output_len) {
//     size_t i, j;

//     i = ctx.index;
//     for (j = 0; j < output_len; j++) {
//         if (i >= SHAKE128_RATE) {
//             // if(cSHAKE128_Squeeze(&ctx.generator_ctx, ctx.output, SHAKE128_RATE * 8) !=0){
//             //     abort();
//             // }
//             cshakeSqueeze(&ctx.generator_ctx, ctx.output, SHAKE128_RATE * 8);
//             i = 0;
//         }
//         output[j] = ctx.output[i++];
//     }
//     ctx.index = i;
// }

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

void transpose(uint16_t *input, uint16_t *output, uint16_t rows, uint16_t cols) {
    int i, j;
    for (i = 0; i < rows; ++i) {
        for (j = 0; j < cols; ++j) {
            output[j * rows + i] = input[i * cols + j];
        }
    }
}

int dotMatMat(uint16_t *matrix_out, uint16_t *matrix1, const uint16_t m1_rows, const uint16_t m1_cols, uint16_t *matrix2, const uint16_t m2_rows, const uint16_t m2_cols)
{
    uint16_t m1_ct, m1_rt, m2_ct, row_sum;

    for(m1_rt = 0; m1_rt < m1_rows; m1_rt++)
    {
        for(m2_ct = 0; m2_ct < m2_cols; m2_ct++)
        {
            row_sum = 0;
            for(m1_ct = 0; m1_ct < m1_cols; m1_ct++){
                row_sum += matrix1[(m1_rt * m1_cols) + m1_ct] * matrix2[(m1_ct * m2_cols) + m2_ct];
            }
            matrix_out[(m1_rt * m2_cols) + m2_ct] = row_sum;
        }
    }
    return 0;
}

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

// static int create_secret_vector(uint16_t *vector, const uint16_t len) {
//     size_t j;
//     uint16_t idx;
//     const uint32_t range_divisor = (uint32_t) (0x10000 / len); \
//     const uint32_t range_limit = len * range_divisor;
//     uint16_t rnd;
//     uint8_t srnd; 

//     memset(vector, 0, sizeof(vector) * len);

//     for (j = 0; j < h; ++j) {
//         do {
//             // idx = drbg_sampler16(len);
//             do {
//                 // drbg(&rnd, sizeof (rnd));
//                 ctx.generate(&srnd, sizeof(rnd));
//                 rnd = (uint16_t)(srnd);
//                 rnd = (uint16_t) LITTLE_ENDIAN16(rnd);
//             } while (rnd >= range_limit);
//             rnd = (uint16_t) (rnd / range_divisor);
//             idx = rnd;
//         } while (vector[idx] != 0);
//         vector[idx] = (j & 1) ? -1 : 1;
//     }

//     return 0;
// }

static int create_secret_vector_hardcode(uint16_t *vector, const uint16_t len) {
    size_t j;

    // (void)len;

    // static const uint16_t FIXED_POS[h]    = { 3, 7, 1, 5, 0, 2 };
    // static const int16_t  FIXED_VALUE[h]  = { +1, -1, +1, -1, +1, -1 };

    // initialize
    memset(vector, 0, sizeof(uint16_t) * len);

    // // copy the matrix
    // for (j = 0; j < h; ++j) {
    //     // check length
    //     if (FIXED_POS[j] >= len) return -1;

    //     vector[FIXED_POS[j]] = FIXED_VALUE[j];
    // }

    for (j = 0; j < h; ++j) {
        vector[j] = (j & 1) ? (uint16_t)-1 : (uint16_t)1;
    }
    return 0;
}

void create_S_T(uint8_t *in, uint16_t *out){
    size_t i;
    const uint16_t len = (uint16_t) (k * n);
	uint8_t *sk = in;
	uint16_t *S_T = out;

    /* Initialize drbg */
    // if(cshakeInit(&ctx.generator_ctx, 128, NULL, 0, NULL, 0)){
    //     abort();
    // }
    // cshakeAbsorb(&ctx.generator_ctx, sk, kappa_bits * 8);
    // cshakeFinal(&ctx.generator_ctx);
    // ctx.index = SHAKE128_RATE;
    // ctx.generate = &generate_cshake128;

    /* Create rows of sparse vectors */
    for (i = 0; i < n_bar; ++i) {
        create_secret_vector_hardcode(&S_T[i * len], len);
    }
}

int create_R_T(uint16_t *R_T, uint8_t *rho) {
    size_t i;
    const uint16_t len = (uint16_t) (k * n);

    for (i = 0; i < m_bar; ++i) {
        create_secret_vector_hardcode(&R_T[i * len], len);
    }

    return 0;
}

static int permutation_tau_2(uint32_t *row_disp, const unsigned char *seed){
    uint32_t i;
    // uint16_t rnd;
    // uint8_t v[len_tau_2] = {0};
    // uint8_t tmp_rnd;

    // drbg_init_customization(seed, kappa_bits, permutation_customization, sizeof (permutation_customization));


    // if(cshakeInit(&ctx.generator_ctx, 128, NULL, 0, (char_t *)permutation_customization, sizeof(permutation_customization))){
    //     abort();
    // }
    // cshakeAbsorb(&ctx.generator_ctx, seed, kappa_bits * 8);
    // cshakeFinal(&ctx.generator_ctx);
    // ctx.index = SHAKE128_RATE;
    // ctx.generate = &generate_cshake128;
    
    //simple hard-code
    for (i = 0; i < k; ++i) {
        row_disp[i] = i;
    }
    return 0;
}

void create_A(uint8_t *sigma, uint16_t *A) {
    uint32_t i;
    uint16_t A_master[len_tau_2 + d] = {0};
    uint32_t A_permutation[k] = {0};
    const uint16_t els_row = (uint16_t) (k * n);

    /* Create A/A_Master*/
    //formerly a checked malloc, but may need to replace because of heap limitations
	// A_master = malloc((size_t) (len_tau_2 + d) * sizeof (*A_master)); //Changed to an array

	// create_A_master(sigma, A_master);, actually drbg_sampler16_2_once
    // Attempt to replace craete_A_master(), which simply calls drbg_sampler16_2_once
    shake128((uint8_t *) A_master, len_tau_2 * sizeof(uint16_t), (const uint8_t *) sigma, kappa_bytes);
    for (i = 0; i < len_tau_2; ++i) {
        A_master[i] = (uint16_t) ((LITTLE_ENDIAN16(A_master[i])) & (q - 1));
    }
    
	memcpy(A_master + len_tau_2, A_master, d * sizeof (*A_master));

    /* Compute and apply the permutation to get A */
	// A_permutation = malloc(k * sizeof (*A_permutation));

	/* Compute and apply permutation */
	permutation_tau_2(A_permutation, sigma);

	for (i = 0; i < k; ++i) {
		memcpy(A + (i * els_row), A_master + A_permutation[i], els_row * sizeof (*A));
	}
}