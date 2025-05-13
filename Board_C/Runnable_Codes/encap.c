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
#define q_bits 13
#define p_bits 10
#define t_bits 7
#define b_bits 3
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
#define CEIL_DIV(a,b) ((a+b-1)/b)
#define BITS_TO_BYTES(b) (CEIL_DIV(b,8))

static int add_msg(uint16_t *result, const size_t len, const uint16_t *matrix, const uint8_t *m, const uint8_t bits_coeff, const uint8_t scaling_factor) {
    size_t i;
    int scale_shift = scaling_factor - bits_coeff;
    uint16_t val;
    size_t bits_done = 0;
    size_t idx;
    size_t bit_idx;

    /* Initialize result with coefficients in matrix */
    memcpy(result, matrix, len * sizeof (*matrix));

    for (i = 0; i < len; ++i) {
        idx = bits_done >> 3;
        bit_idx = bits_done & 7;
        val = (uint16_t) (m[idx] >> bit_idx);
        if (bit_idx + bits_coeff > 8) {
            /* Get spill over from next message byte */
            val = (uint16_t) (val | (m[idx + 1] << (8 - bit_idx)));
        }
        result[i] = (uint16_t) ((result[i] + (val << scale_shift)) & ((1 << scaling_factor) - 1));
        bits_done += bits_coeff;
    }

    return 0;
}


int r5_cpa_kem_encrypt(uint8_t *ct, uint8_t *pk, uint8_t *m, uint8_t *rho){

    /* Length of matrices, vectors, bit strings */
    size_t len_a = (size_t) (k * k * n);
    size_t len_r = (size_t) (k * m_bar * n);
    size_t len_u = (size_t) (k * m_bar * n);
    size_t len_b = (size_t) (k * n_bar * n);
    size_t len_x = (size_t) (n_bar * m_bar * n);
    size_t len_m1 = (size_t) BITS_TO_BYTES(mu * b_bits);
    size_t unpack_pk_return;

    /* Matrices, vectors, bit strings */
    uint16_t A[k * k * n];
    uint16_t A_T[k * k * n];
    uint16_t R[k * m_bar * n];
    uint16_t R_T[m_bar * k * n];
    uint16_t U[k * m_bar * n];
    uint16_t U_T[k * m_bar * n];
    uint16_t B[k * n_bar * n];
    uint16_t B_T[k * n_bar * n];
    uint16_t X[n_bar * m_bar * n];
    uint16_t v[mu];
    uint8_t m1[BITS_TO_BYTES(mu * b_bits)];

    //rounding constant
    uint16_t z_bits = (uint16_t) (q_bits - p_bits + t_bits);
    uint16_t h2 = (uint16_t) (1 << (q_bits - z_bits - 1));

    uint8_t sigma2[kappa_bytes];

    /* Unpack received public key into sigma and B */
    unpack_pk_return = unpack_pk(sigma2, B, pk, kappa_bytes, len_b, p_bits);

    // /* Create A from sigma */
    create_A(sigma2, A);

    // /* Create R_T from rho */
    create_R_T(R_T, rho);

    // /* Transpose A */
    transpose(A_T, A, k, k);

    // /* Transpose R_T to get R */
    // transpose((uint16_t *) R, (uint16_t *) R_T, params->m_bar, params->k, params->n);
    transpose(R, R_T, m_bar, k);

    // /* U = A^T * R */
    // mult_matrix(U, (int16_t *) A_T, params->k, params->k, R, params->k, params->m_bar, params->n, params->q, 0);
    dotMatMat(U, A_T, k, k, R, k, m_bar);

    //     /* Compress U q_bits -> p_bits with flooring */
    // round_matrix(U, (size_t) (params->k * params->m_bar), params->n, params->q_bits, params->p_bits, params->h2);
    round_matrix(U, (size_t) (k * m_bar), n, q_bits, p_bits, h2);

    // /* Transpose U */
    // transpose_matrix(U_T, U, params->k, params->m_bar, params->n);
    transpose(U_T, U, k, m_bar);

    // /* Transpose B */
    // transpose_matrix(B_T, B, params->k, params->n_bar, params->n);
    transpose(B_T, B, k, n_bar);

    // /* X = B^T * R */
    // mult_matrix(X, (int16_t *) B_T, params->n_bar, params->k, R, params->k, params->m_bar, params->n, params->p, params->xe != 0 || params->f != 0);
    dotMatMat(X, B_T, n_bar, k, R, k, m_bar);

    // /* v is a matrix of scalars, so we use 1 as the number of coefficients */
    // round_matrix(X, params->mu, 1, params->p_bits, params->t_bits, params->h2);
    round_matrix(X, mu, 1, p_bits, t_bits, h2);

    // /* Compute codeword */
    memcpy(m1, m, kappa_bytes);
    memset(m1 + kappa_bytes, 0, (size_t) (len_m1 - kappa_bytes));

    // /* Add message (mod t) */
    add_msg(v, mu, X, m1, b_bits, t_bits);

    // /* Transpose U */
    // transpose_matrix(U_T, U, params->k, params->m_bar, params->n);
    transpose(U_T, U, k, m_bar);

    // /* Pack ciphertext */
    pack_ct(ct, U_T, len_u, p_bits, v, mu, t_bits);

    return 0;
}



int r5_cpa_kem_encap(uint8_t *ct, uint8_t *key, uint8_t *pk, uint8_t sigma[16]){
    uint8_t m[kappa_bytes] = {0};
    uint8_t rho[kappa_bytes] = {0};
    uint8_t buf[kappa_bytes + 1];

    // copy sigma[0..kappa_bytes-1]
    memcpy(buf, sigma, kappa_bytes);

    // random m and rho generation
    buf[kappa_bytes] = 0x00;
    shake128(m, kappa_bytes, buf, kappa_bytes + 1);

    buf[kappa_bytes] = 0x01;
    shake128(rho, kappa_bytes, buf, kappa_bytes + 1);

    r5_cpa_kem_encrypt(ct, pk, m, rho);

    /* k = H(m, ct) */
    return 0;
}