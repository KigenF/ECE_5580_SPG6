#include <stdlib.h>
#include <stdbool.h>
#include "shake.h"

void generate_cshake128(uint8_t *output, const size_t output_len);

void *checked_malloc(size_t size);

void *checked_calloc(size_t count, size_t size);

void transpose(uint16_t *input, uint16_t *output, uint16_t rows, uint16_t cols);

int dotMatMat(uint16_t *matrix_out, uint16_t *matrix1, const uint16_t m1_rows, const uint16_t m1_cols, uint16_t *matrix2, const uint16_t m2_rows, const uint16_t m2_cols);

int round_matrix(uint16_t *matrix, const size_t len, const size_t els, const uint16_t a, const uint16_t b, const uint16_t rounding_constant);

void create_S_T(uint8_t *in, uint16_t *out);

int create_R_T(uint16_t *R_T, uint8_t *rho);

void create_A(uint8_t *sigma, uint16_t *A);
