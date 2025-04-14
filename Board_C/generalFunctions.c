#include <stdlib.h>
#include <stdint.h>

void *checked_malloc(size_t size) {
	void *tmp = malloc(size);
	if(tmp == NULL) abort();
	return tmp;
}

void *checked_calloc(size_t count, size_t size) {
	void *temp = calloc(count, size);
	if (temp == NULL) {
		abort();
	}
	return temp;
}

void transpose(uint16_t *input, uint16_t *output, uint16_t rows, uint16_t cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            output[j * rows + i] = input[i * cols + j];
        }
    }
}

int dotMatMat(uint16_t *matrix_out, uint16_t *matrix1, const uint16_t m1_rows, const uint16_t m1_cols, uint16_t *matrix2, const uint16_t m2_rows, const uint16_t m2_cols)
{
    uint16_t m1_ct, m1_rt, m2_ct, row_sum;

    for(m1_rt = 0; m1_rt < m1_rows; m1_rt++)
    {
        for(m2_ct = 0; m2_ct < m2_cols; m2_ct++)
        {
            row_sum = 0;
            for(m1_ct = 0; m1_ct < m1_cols; m1_ct++){
                row_sum += matrix1[(m1_rt * m1_cols) + m1_ct] * matrix2[(m1_ct * m2_cols) + m2_ct];
            }
            matrix_out[(m1_rt * m2_cols) + m2_ct] = row_sum;
        }
    }
    return 0;
}

