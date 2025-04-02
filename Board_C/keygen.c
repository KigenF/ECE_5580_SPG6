#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>


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
int dotMatMat(uint16_t *matrix_out, uint16_t *matrix1, const uint16_t m1_rows, const uint16_t m1_cols, uint16_t *matrix2, const uint16_t m2_rows, const uint16_t m2_cols)
// dotMatMat(const uint16_t m1_rows, const uint16_t m1_cols, uint16_t matrix1[m1_rows][m1_cols], const uint16_t m2_rows, const uint16_t m2_cols, uint16_t matrix2[m2_rows][m2_cols], uint16_t matrix_out[m1_rows][m2_cols])
{
    uint16_t m1_ct, m1_rt, m2_ct, row_sum;

    for(m1_rt = 0; m1_rt < m1_rows; m1_rt++)
    {
        for(m2_ct = 0; m2_ct < m2_cols; m2_ct++)
        {
            row_sum = 0;
            for(m1_ct = 0; m1_ct < m1_cols; m1_ct++){
                // row_sum += matrix1[m1_rt][m1_ct] * matrix2[m2_ct][m1_rt];
                row_sum += matrix1[(m1_rt * m1_cols) + m1_ct] * matrix2[(m1_ct * m2_cols) + m2_ct];
            }
            // matrix_out[m1_rt][m2_ct] = row_sum;
            matrix_out[(m1_rt * m2_cols) + m2_ct] = row_sum;
        }
    }
    return 0;
}
