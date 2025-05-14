#include <stdlib.h>
#include <stdint.h>
#include "shake.h"
#include "oryx/xof/cshake.h"

#define len_tau_2 2048
#define d 594
#define n 1
#define k 594 // k = d/n.
#define kappa_bits 16
#define q 8192
#define n_bar 7
#define m_bar 7
#define h 238
#define p_bits 10
#define t_bits 7
#define _b_bits 3
#define cipher_bits 5236
#define mu 43
#define sk_bits 16

#define SHAKE128_RATE 168
#define SHAKE256_RATE 136

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define LITTLE_ENDIAN16(x) (x)
#else
#define LITTLE_ENDIAN16(x) ( \
    (((x) & 0xFF00U) >> 8) |  \
    (((x) & 0x00FFU) << 8)    \
)
#endif

typedef CshakeContext cshake_ctx;
// Context for Oryx 
typedef struct {
    cshake_ctx generator_ctx; 
    uint8_t output[SHAKE128_RATE > SHAKE256_RATE ? SHAKE128_RATE : SHAKE256_RATE]; 
    size_t index;
    void (* generate)(uint8_t *output, const size_t output_len);
} drbg_ctx;


int dotMatMatBitslice(uint16_t *matrix_out, uint16_t *matrix1, const uint16_t m1_rows, const uint16_t m1_cols, uint16_t *matrix2, const uint16_t m2_rows, const uint16_t m2_cols, const uint16_t sliceLength)
//This function is an optimized version of dotMatMat using bitslicing to benefit from splitting up the math accross 
{
    uint16_t m1_ct, m1_rt, m2_ct, short_row_sum;
    uint32_t row_sum;

    //need to change the for loops to facilitate the slicing (change the innermost for loop to go through every 32 variables) then introduce a last for loop that will loop through the length of the sliceLength
    for(m1_rt = 0; m1_rt < m1_rows; m1_rt++)
    {
        for(m2_ct = 0; m2_ct < m2_cols; m2_ct++)
        {
            row_sum = 0;
            //it is likely to require an array to store each sliced section in
            for(m1_ct = 0; m1_ct < m1_cols; m1_ct++){
                for (uint16_t bitA = 0; bitA < sliceLength; ++bitA) {
                    if ((matrix1[(m1_rt * m1_cols) + m1_ct] >> bitA) & 1) {
                        for (uint16_t bitB = 0; bitB < sliceLength; ++bitB) {
                            if ((matrix2[(m1_ct * m2_cols) + m2_ct] >> bitB) & 1) {
                                row_sum += (1U << (bitA + bitB));
                            }
                        }
                    }
                }
            }
            //Will need another array to cover the values between the last group and 32, which not previously covered 
            short_row_sum = ((uint16_t)row_sum);
            matrix_out[(m1_rt * m2_cols) + m2_ct] = short_row_sum - ((short_row_sum >> sliceLength) << sliceLength);
        }
    }
    return 0;
}
