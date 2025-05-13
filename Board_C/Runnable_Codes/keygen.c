#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "generalFunctions.h"
#include "shake.h"
#include "pack.h"

#define p 1024

#define len_tau_2 100
#define d 100
#define n 1
#define k 100 // k = d/n.
#define kappa_bytes 16
#define q 8192
#define n_bar 7
#define m_bar 7
// #define h 238
#define h 98
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
// typedef CshakeContext cshake_ctx;
// Context struct for the XKCP
// typedef struct {
//     cshake_ctx generator_ctx; 
//     uint8_t output[SHAKE128_RATE > SHAKE256_RATE ? SHAKE128_RATE : SHAKE256_RATE]; 
//     size_t index;
//     void (* generate)(uint8_t *output, const size_t output_len);
// } drbg_ctx;

// Context for Oryx 
// typedef struct {
//     cshake_ctx generator_ctx; 
//     uint8_t output[SHAKE128_RATE > SHAKE256_RATE ? SHAKE128_RATE : SHAKE256_RATE]; 
//     size_t index;
//     void (* generate)(uint8_t *output, const size_t output_len);
// } drbg_ctx;

// static drbg_ctx ctx;
// static const uint8_t permutation_customization[2] = {0, 1};

// static uint16_t shake_sampler16(KeccakContext *xof, uint16_t range) {
//     uint16_t v;
//     uint32_t limit = (uint32_t)UINT16_MAX + 1;
//     uint32_t threshold = limit - (limit % range);

//     do {
//         uint8_t buf[2];
//         keccakSqueeze(xof, buf, 2);
//         v = (uint16_t)(buf[0] | (buf[1] << 8));
//     } while ((uint32_t)v >= threshold);

//     return v % range;
// }



int pkeKeygen(uint8_t pk_byte[5214], uint8_t sk_byte[16], uint8_t sigma_byte[16]){
    uint16_t A[d*d];// = checked_malloc(d * d * sizeof(int16_t)); 
    uint16_t S_T[n_bar*d];// = checked_malloc(n_bar * d * sizeof(int16_t)); 
    uint16_t S[d*n_bar];// = checked_malloc(d * n_bar * sizeof(int16_t));
    uint16_t B[d*n_bar];// = checked_malloc(d * n_bar * sizeof(int16_t));
    // //create A
    create_A(sigma_byte, A);
    // //Create S_T
    create_S_T(sk_byte, S_T);
    //Transpose S_T to S
    transpose(S_T, S, n_bar, d);
    // //dotMatMat A and S
    dotMatMat(B, A, d, d, S, d, n_bar);
    // //round B, I am going back to this while working on decap, and I am not entirely sure why the rounding constant was set as 4, but don't have enough info to verify/change it 
    round_matrix(B, (size_t)d * n_bar, n, q, p_bits, 4);
    // //pack_pk with sigma and B_b
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