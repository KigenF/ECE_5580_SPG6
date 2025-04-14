#include <stdlib.h>

void *checked_malloc(size_t size);

void *checked_calloc(size_t count, size_t size);

void transpose(uint16_t *input, uint16_t *output, uint16_t rows, uint16_t cols);

int dotMatMat(uint16_t *matrix_out, uint16_t *matrix1, const uint16_t m1_rows, const uint16_t m1_cols, uint16_t *matrix2, const uint16_t m2_rows, const uint16_t m2_cols);
