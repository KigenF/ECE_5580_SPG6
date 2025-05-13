#include <stdlib.h>
#include <stdio.h>

size_t pack(uint8_t *packed, const uint16_t *m, const size_t els, const uint8_t nr_bits);

void pack_pk(uint8_t *sigma, uint16_t *b, uint8_t *out);

size_t unpack_pk(uint8_t *sigma, uint16_t *B, const uint8_t *packed_pk, size_t sigma_len, size_t elements, uint8_t nr_bits);

size_t pack_ct(uint8_t *packed_ct, const uint16_t *U_T, size_t U_els, uint8_t U_bits, const uint16_t *v, size_t v_els, uint8_t v_bits);

size_t unpack_ct(uint16_t *U_T, uint16_t *v, const uint8_t *packed_ct, const size_t U_els, const uint8_t U_bits, const size_t v_els, const uint8_t v_bits);