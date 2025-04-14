#include <stdlib.h>
#include <stdio.h>


int round_element(uint16_t *x, const uint16_t a_bits, const uint16_t b_bits, const uint16_t b_mask, const uint16_t rounding_constant);

int round_matrix(uint16_t *matrix, const size_t len, const size_t els, const uint16_t a, const uint16_t b, const uint16_t rounding_constant);

int kemKeygen(uint8_t sigma[16], uint8_t sk[16], uint8_t pk[5214]);

int add(int a, int b);
