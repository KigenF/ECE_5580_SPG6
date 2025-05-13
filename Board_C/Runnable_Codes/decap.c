#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "generalFunctions.h"
#include "pack.h"
#include "shake.h"

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
#define q_bits 13
#define p_bits 10
#define t_bits 7
#define _b_bits 3
#define cipher_bits 5236
#define mu 43
#define sk_bits 16

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
#define CEIL_DIV(a,b) ((a+b-1)/b)
#define BITS_TO_BYTES(b) (CEIL_DIV(b,8))

// void pack_mu(uint16_t *in, uint16_t *result){
//     result[0] = ((in[2] % 4)*64) + (in[1] * 8) + in[0];
//     result[1] = ((in[5] % 6)*128) + (in[4] * 16) + (in[3] * 2) + (in[2] / 4);
//     result[2] = (in[7] * 32) + (in[6] * 4) +(in[5] / 6);
// }

// void pack_rear(uint16_t *out, uint16_t *in){
//     out[0] = ((in[2] % 4) * 64) + (in[1] * 8) + in[0];
//     out[1] = in[2] / 4;
// }

// void pack_m1(uint16_t *m1_out, uint16_t *m2_b_bits){
//     int i, j, outInd = 0;
//     uint16_t m2_front_temp[8], m2_front_out[3];
//     uint16_t m2_rear[(mu * _b_bits) - 40], m2_rear_out[2];
//     //Handles the front section of the pack
//     for(i = 0; i < 40; i+=8){
//         //first pack in this iteration of values to the front
//         for(j = 0; j<8; j++){
//             m2_front_temp[j] = m2_b_bits[i+j];
//         }
//         //then call pack_mu with that iteration
//         pack_mu(m2_front_temp, m2_front_out);
//         for(j=0;j<3;j++){
//             m1_out[outInd++] = m2_front_out[j];
//         }
//     }
//     //handling the rear section of the pack
//     //First populate the rear array
//     for(i=40; i < (mu * _b_bits);i++){
//         m2_rear[i-40] = m2_b_bits[i];
//     }
//     pack_rear(m2_rear_out, m2_rear);
//     m1_out[outInd++] = m2_rear_out[0];
//     m1_out[outInd] = m2_rear_out[1];
// }

// void diff_msg(uint16_t *out, uint16_t *v, uint16_t *xPrime){
//     int i;
//     // uint16_t v_row_val, xPrime_row_val;
//     for(i = 0; i < mu; i++){
//         //Realized that this was unnecessary, as these are not multidimensional in the normal sense
//         // for(j=0;j<p_bits;j++){
//         //     v_row[j] = v[(i*p_bits) + j];
//         //     xPrime_row[j] = xPrime[(i*p_bits) + j];
//         // }
//         out[i] = v[i] - xPrime[i];
//     }
// }

// void decompress_matrix(uint16_t *out, uint16_t *v){
//     //There's a descent chance I am going to have to rewrite a significant amount of my code, as I was treating the arrays as 1D integer arrays, as opposed to 2D bit arrays
//     int i;
//     for(i=0; i<mu; i++){
//         out[i] = v[i] * 8; //as the Cryptol shows it is v[i] # 0b000, or 2^n for each 0 post b
//     }

//     //or maybe not
// }

// void unpack_front_U_T(uint16_t *out, uint16_t *f){
//     out[0] = ((f[1] % 252) << 8) + f[0];
//     out[1] = ((f[2] % 240) << 6) + (f[1] >> 2);
//     out[2] = ((f[3] % 192) << 4) + (f[2] >> 4);
//     out[3] = f[4] + (f[3] >> 6);
// }

// void unpack_rear_U_T(uint16_t *out, uint16_t *r){
//     out[0] = ((r[1] % 252) << 8) + r[0];
//     out[1] = ((r[2] %240) << 6) + (r[1] >> 2);
// }

// void unpack_U_T(uint16_t *u_t, uint16_t *ct){
//     //len of ct is 5236
//     int i, j, u_t_ind = 0;
//     uint16_t ct_front[5], ct_rear[3];
//     uint16_t ct_front_temp[4], ct_rear_temp[2];
//     //grouped_front = groupBy`{5} (take`{5195} ct)
//     for(j=0;j<(5195/5);j++){
//         for(i=0;i<5;i++){   
//             ct_front[i] = ct[(j*5) + i];
//         }
//         unpack_front_U_T(ct_front_temp, ct_front); 
//         for(i=0;i<4;i++){
//             u_t[u_t_ind++] = ct_front_temp[i];
//         }
//     }
//     j=0;
//     for(i=5195;i<5198;i++){
//         ct_rear[j++] = ct[i];
//     }
//     unpack_rear_U_T(ct_rear_temp, ct_rear);    
//     u_t[u_t_ind++] = ct_rear_temp[0];
//     u_t[u_t_ind] = ct_rear_temp[1];
// }

// void unpack_front_v(uint16_t *out, uint16_t *f){
//      out[0] = f[0] % 128;
//      out[1] = ((f[1] % 192) << 1) + (f[0] >> 7);
//      out[2] = ((f[2] % 224) << 2) + (f[1] >> 6);
//      out[3] = ((f[3] % 240) << 3) + (f[2] >> 5);
//      out[4] = ((f[4] % 248) << 4) + (f[3] >> 4);
//      out[5] = ((f[5] % 252) << 5) + (f[4] >> 3);
//      out[6] = ((f[6] % 254) << 6) + (f[5] >> 2);
//      out[7] = (f[6] >> 1);
// } 

// void unpack_rear_v(uint16_t *out, uint16_t *r){
//     out[0] = r[0] % 128;
//     out[1] = ((r[1] % 192) << 6) + (r[0] >> 7);
//     out[2] = ((r[2] % 224) << 5) + (r[1] >> 6);
// }

// void unpack_v(uint16_t *v, uint16_t *ct){
//     int i, j, vind = 0;
//     uint16_t front[7], rear[cipher_bits - 5233];
//     uint16_t front_temp[8], rear_temp[3];
//     for(i = 5198; i < 35 + 5198; i+=7){
//         for(j=0; j<7;j++){
//             front[j] = ct[i + j];
//         }
//         unpack_front_v(front_temp, front);  
//         for(j=0;j<8;j++){
//             v[vind++] = front_temp[j];
//         }
//     }
//     for(i = 5233; i < cipher_bits; i++){
//         rear[i-5233] = ct[i];
//     }
//     unpack_rear_v(rear_temp, rear);     
//     v[vind++] = rear_temp[0];
//     v[vind++] = rear_temp[1];
//     v[vind] = rear_temp[2];
// }

// void unpack_ct(uint16_t *u_t_out, uint16_t *v_out, uint16_t *cipherText){
//     //This is gonna be a long one
//     unpack_U_T(u_t_out, cipherText); 
//     unpack_v(v_out, cipherText);    
// }

// void r5_cpa_pke_decrypt(uint16_t *message, uint16_t *ciphertext, uint16_t *sharedkey){
//     uint8_t S_T8[sk_bits];
//     uint16_t S_T[sk_bits];
//     uint16_t U_T[d * m_bar];
//     uint16_t v[mu], U[m_bar * d];
//     uint16_t v_p_bits[mu];
//     uint16_t X_Prime[n_bar * m_bar];
//     uint16_t m2[mu];
//     uint16_t m1[17];
//     int i;

//     create_S_T(S_T8, sharedkey);
//     for(i=0;i<sk_bits;i++){
//         S_T[i] = (uint16_t)(S_T8[i]);
//     }
//     unpack_ct(U_T, v, ciphertext);
//     transpose(U_T, U, m_bar, d);
//     decompress_matrix(v_p_bits, v);
//     dotMatMat(X_Prime, S_T, n_bar, d, U, d, m_bar);
//     diff_msg(m2, v_p_bits, X_Prime);
//     //using h for rounding constant, may need to go back and verify this later
//     round_matrix(m2, mu, 1, p_bits, _b_bits, h);
//     pack_m1(m1,m2);
//     //almost certainly could have used memcpy or something similar, but don't intend to take the time to re-figure them out right now
//     for(i=0;i<16;i++){
//         message[i] = m1[i];
//     }
// }

static int decompress_element(uint16_t *x, const uint16_t a_bits, const uint16_t b_bits, const uint16_t b_mask) {
    const uint16_t shift = (uint16_t) (b_bits - a_bits);
    *x = (uint16_t) (*x << shift);
    *x &= b_mask;
    return 0;
}

int decompress_matrix(uint16_t *matrix, const size_t len, const size_t els, const uint16_t a_bits, const uint16_t b_bits) {
    size_t i;

    const uint16_t b_mask = (uint16_t) (((uint16_t) 1 << b_bits) - 1);
    for (i = 0; i < len * els; ++i) {
        decompress_element(matrix + i, a_bits, b_bits, b_mask);
    }

    return 0;
}

static int diff_msg(uint16_t *result, const size_t len, const uint16_t *matrix_a, const uint16_t *matrix_b, const uint16_t mod) {
    size_t i;

    for (i = 0; i < len; ++i) {
        result[i] = (uint16_t) ((matrix_a[i] - matrix_b[i]) & (mod - 1));
    }

    return 0;
}


int r5_cpa_pke_decrypt(uint8_t *m, const uint8_t *sk, const uint8_t *ct) {
    /* Length of matrices, vectors, bit strings */
    size_t len_s = (size_t) (k * n_bar * n);
    size_t len_u = (size_t) (k * m_bar * n);
    size_t len_x_prime = (size_t) (n_bar * m_bar * n);
    size_t len_m1 = (size_t) BITS_TO_BYTES(mu * _b_bits);

    uint16_t S_T[k * n_bar * n];
    uint16_t U[k * m_bar * n];
    uint16_t U_T[k * m_bar * n];
    uint16_t v[mu];
    uint16_t X_prime[n_bar * m_bar * n];
    uint16_t m2[mu];

    /* Message plus error correction */
    // uint8_t m1 = checked_calloc(len_m1, 1);
    uint8_t m1[BITS_TO_BYTES(mu * _b_bits)];

    //rounding constant
    uint16_t z_bits = (uint16_t) (q_bits - p_bits + t_bits);
    uint16_t h3 = (uint16_t) ((uint16_t) (1 << (p_bits - t_bits - 1)) + (uint16_t) (1 << (p_bits - _b_bits - 1)) - (uint16_t) (1 << (q_bits - z_bits - 1)));


    /* Generate S_T from sk */
    create_S_T(sk, S_T);

    /* Unpack cipher text */
    unpack_ct(U_T, v, ct, len_u, p_bits, mu, t_bits);

    /* Transpose U^T */
    transpose(U, U_T, m_bar, k);

    /* Decompress v t -> p */
    decompress_matrix(v, mu, 1, t_bits, p_bits);

    /* X' = S^T * U */
    dotMatMat(X_prime, S_T, n_bar, k, (uint16_t *) U, k, m_bar);


    /* v - X' (mod p) */
    diff_msg(m2, mu, v, X_prime, 1024);

    /* Compress msg p -> B */
    // round_matrix(m2, params->mu, 1, params->p_bits, params->b_bits, params->h3);
    round_matrix(m2, mu, 1, p_bits, _b_bits, h3);

    /* Convert the message to bit string format */
    pack(m1, m2, mu, _b_bits);

    memcpy(m, m1, kappa_bytes);

// #ifdef NIST_KAT_GENERATION
//     print_hex("r5_cpa_pke_decrypt: m", m, params->kappa_bytes, 1);
// #endif

    return 0;
}


int r5_cpa_kem_decap(uint8_t *key, const uint8_t *ct, const uint8_t *sk){
    uint8_t hash_input[kappa_bytes + cipher_bits];
    uint8_t m[kappa_bytes];

    /* Decrypt m */
    r5_cpa_pke_decrypt(m, sk, ct);

    /* k = H(m, ct) */
    // memcpy(hash_input, m, kappa_bytes);
    // memcpy(hash_input + kappa_bytes, ct, ct_size);
    // hash(k, params->kappa_bytes, hash_input, (size_t) (params->kappa_bytes + params->ct_size), params->kappa_bytes);

    return 0;
}

// void r5_cpa_kem_decap(uint8_t *keyStream, uint16_t *ciphertext, uint16_t *sharedkey){
//     uint16_t message[17];
//     uint16_t msgCt[17+cipher_bits];
//     int i, cntr = 0;
//     r5_cpa_pke_decrypt(message, ciphertext, sharedkey);
//     //need to figure out how the hash works
//     //need to combine message and ciphertext
//     for(i=0; i < 17; i++){
//         msgCt[cntr++] = message[i];
//     }
//     for(i=0; i < cipher_bits; i++){
//         msgCt[cntr++] = ciphertext[i];
//     }
//     cshakeCompute(128, msgCt, (17+cipher_bits), NULL, 0, NULL, 0, keyStream, kappa_bits);
// }