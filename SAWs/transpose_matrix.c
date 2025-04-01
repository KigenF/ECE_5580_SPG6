#include <stdlib.h>
#include <stdio.h>

int transpose_matrix(uint16_t *matrix_t, const uint16_t *matrix, const size_t rows, const size_t cols, const size_t els) {
    size_t i, j, k;

    for (i = 0; i < rows; ++i) {
        for (j = 0; j < cols; ++j) {
            for (k = 0; k < els; ++k) {
                matrix_t[j * (rows * els) + (i * els) + k] = matrix[i * (cols * els) + (j * els) + k];
            }
        }
    }

    return 0;
}