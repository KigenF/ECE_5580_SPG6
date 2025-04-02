#include <stdlib.h>
#include <stdio.h>


int round_element(uint16_t *x, const uint16_t a_bits, const uint16_t b_bits, const uint16_t b_mask, const uint16_t rounding_constant);

int round_matrix(uint16_t *matrix, const size_t len, const size_t els, const uint16_t a, const uint16_t b, const uint16_t rounding_constant);

int dotMatMat(uint16_t *matrix_out, uint16_t *matrix1, const uint16_t m1_rows, const uint16_t m1_cols, uint16_t *matrix2, const uint16_t m2_rows, const uint16_t m2_cols);
