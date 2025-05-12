#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "cycles.h"
#include <time.h>

#define MAX_BITS 16

void generateRandomMatrix(uint16_t *matrix, uint16_t rows, uint16_t cols, uint16_t max_val) {
    for (uint16_t i = 0; i < rows; ++i) {
        for (uint16_t j = 0; j < cols; ++j) {
            matrix[i * cols + j] = rand() % (max_val + 1);
        }
    }
}

void dotMatMatBitsliced(uint16_t *out, const uint16_t *a, uint16_t m1_rows, uint16_t m1_cols, const uint16_t *b, uint16_t m2_rows, uint16_t m2_cols) {
    memset(out, 0, m1_rows * m2_cols * sizeof(uint16_t)); 

    for (uint16_t i = 0; i < m1_rows; ++i) {
        for (uint16_t j = 0; j < m2_cols; ++j) {
            uint32_t result = 0;

            // Bitwise multiplication.
            for (uint16_t k = 0; k < m1_cols; ++k) {
                uint16_t A_val = a[i * m1_cols + k];
                uint16_t B_val = b[k * m2_cols + j];

                for (uint16_t bitA = 0; bitA < MAX_BITS; ++bitA) {
                    if ((A_val >> bitA) & 1) {
                        for (uint16_t bitB = 0; bitB < MAX_BITS; ++bitB) {
                            if ((B_val >> bitB) & 1) {
                                result += (1U << (bitA + bitB));
                            }
                        }
                    }
                }
            }

            // Store truncated result.
			// Overflow issue?
            out[i * m2_cols + j] = (uint16_t) result;
        }
    }
}

void printMatrix(uint16_t *matrix, uint16_t rows, uint16_t cols) {
    for (uint16_t i = 0; i < rows; ++i) {
        for (uint16_t j = 0; j < cols; ++j)
            printf("%5u ", matrix[i * cols + j]);
        printf("\n");
    }
}

int main() {
    uint16_t A[4] = {1, 2, 3, 4}; 
    uint16_t B[4] = {5, 6, 7, 8};
    uint16_t C[4] = {0};

    dotMatMatBitsliced(C, A, 2, 2, B, 2, 2);

    printf("Bitsliced Matrix Multiplication Result:\n");
    printMatrix(C, 2, 2);

    return 0;
}

