#include <stdlib.h>
#include <stdio.h>

int transpose_matrix(uint16_t *matrix_t, const uint16_t *matrix, const size_t rows, const size_t cols) {
    size_t i, j;

    for (i = 0; i < rows; ++i) {
        for (j = 0; j < cols; ++j) {
            matrix_t[j * rows + i] = matrix[i * cols + j];
        }
    }
    return 0;
}