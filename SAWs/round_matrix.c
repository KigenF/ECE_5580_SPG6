#include <stdlib.h>
#include <stdio.h>


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