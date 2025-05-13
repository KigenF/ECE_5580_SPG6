#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

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
#define CEIL_DIV(a,b) ((a+b-1)/b)
#define BITS_TO_BYTES(b) (CEIL_DIV(b,8))

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

size_t unpack(uint16_t *m, const uint8_t *packed, const size_t els, const uint8_t nr_bits) {
    const size_t unpacked_len = (size_t) (BITS_TO_BYTES(els * nr_bits));
    size_t i;
    uint16_t val;
    size_t bits_done = 0;
    size_t idx;
    size_t bit_idx;
    uint16_t bitmask = (uint16_t) ((1 << nr_bits) - 1);

    if (nr_bits == 8) {
        for (i = 0; i < els; ++i) {
            m[i] = packed[i];
        }
    } else {
        for (i = 0; i < els; ++i) {
            idx = bits_done >> 3;
            bit_idx = bits_done & 7;
            val = (uint16_t) (packed[idx] >> bit_idx);
            if (bit_idx + nr_bits > 8) {
                /* Get spill over from next packed byte */
                val = (uint16_t) (val | (packed[idx + 1] << (8 - bit_idx)));
                if (bit_idx + nr_bits > 16) {
                    /* Get spill over from next packed byte */
                    val = (uint16_t) (val | (packed[idx + 2] << (16 - bit_idx)));
                }
            }
            m[i] = val & bitmask;
            bits_done += nr_bits;
        }
    }

    return unpacked_len;
}

void pack_pk(uint8_t *sigma, uint16_t *b, uint8_t *out){
    size_t packing_index = 0;
    uint8_t *packed_pk = out;

    memcpy(packed_pk, sigma, kappa_bytes);
    packing_index += kappa_bytes;

    packing_index += pack((packed_pk + packing_index), b, d*n_bar, p_bits);
}

size_t unpack_pk(uint8_t *sigma, uint16_t *B, const uint8_t *packed_pk, size_t sigma_len, size_t elements, uint8_t nr_bits) {
    size_t unpacked_idx = 0;

    /* Unpack sigma */
    memcpy(sigma, packed_pk + unpacked_idx, sigma_len);
    unpacked_idx += sigma_len;
    /* Unpack B */
    unpacked_idx += unpack(B, packed_pk + unpacked_idx, elements, nr_bits);

    return unpacked_idx;
}

size_t pack_ct(uint8_t *packed_ct, const uint16_t *U_T, size_t U_els, uint8_t U_bits, const uint16_t *v, size_t v_els, uint8_t v_bits) {
    size_t idx = 0;

    /* Pack U_T */
    idx += pack(packed_ct, U_T, U_els, U_bits);
    /* Pack v */
    idx += pack((packed_ct + idx), v, v_els, v_bits);

    return idx;
}

size_t unpack_ct(uint16_t *U_T, uint16_t *v, const uint8_t *packed_ct, const size_t U_els, const uint8_t U_bits, const size_t v_els, const uint8_t v_bits) {
    size_t idx = 0;

    /* Unpack U */
    idx += unpack(U_T, packed_ct, U_els, U_bits);
    /* Unpack v */
    idx += unpack(v, (packed_ct + idx), v_els, v_bits);

    return idx;
}