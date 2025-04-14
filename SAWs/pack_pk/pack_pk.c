#include <stdlib.h>
#include <stdio.h>

size_t pack(unsigned char *packed, const uint16_t *m, const size_t els, const uint8_t nr_bits) {
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

size_t pack_pk(unsigned char *packed_pk, const unsigned char *sigma, size_t sigma_len, const uint16_t *B, size_t elements, uint8_t nr_bits) {
    size_t packed_idx = 0;

    /* Pack sigma */
    memcpy(packed_pk + packed_idx, sigma, sigma_len);
    packed_idx += sigma_len;
    /* Pack B */
    packed_idx += pack((packed_pk + packed_idx), B, elements, nr_bits);

    return packed_idx;
}

