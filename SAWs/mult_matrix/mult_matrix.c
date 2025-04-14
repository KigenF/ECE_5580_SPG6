#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

int mult_matrix(uint16_t *matrix_out, uint16_t *matrix1, const uint16_t m1_rows, const uint16_t m1_cols, uint16_t *matrix2, const uint16_t m2_rows, const uint16_t m2_cols)
{
    uint16_t m1_ct, m1_rt, m2_ct;
    uint32_t row_sum;

    for(m1_rt = 0; m1_rt < m1_rows; m1_rt++)
    {
        for(m2_ct = 0; m2_ct < m2_cols; m2_ct++)
        {
            row_sum = 0;
            for(m1_ct = 0; m1_ct < m1_cols; m1_ct++){
                row_sum += (uint32_t)matrix1[(m1_rt * m1_cols) + m1_ct] * (uint32_t)matrix2[(m1_ct * m2_cols) + m2_ct];
            }
            matrix_out[(m1_rt * m2_cols) + m2_ct] = (uint16_t)row_sum;
        }
    }
    return 0;
}

// int mult_matrix(int *matrix_out, int *matrix1, const int m1_rows, const int m1_cols, int *matrix2, const int m2_rows, const int m2_cols)
// {
//     int m1_ct, m1_rt, m2_ct, row_sum;

//     for(m1_rt = 0; m1_rt < m1_rows; m1_rt++)
//     {
//         for(m2_ct = 0; m2_ct < m2_cols; m2_ct++)
//         {
//             row_sum = 0;
//             for(m1_ct = 0; m1_ct < m1_cols; m1_ct++){
//                 row_sum += matrix1[(m1_rt * m1_cols) + m1_ct] * matrix2[(m1_ct * m2_cols) + m2_ct];
//             }
//             matrix_out[(m1_rt * m2_cols) + m2_ct] = row_sum;
//         }
//     }
//     return 0;
// }