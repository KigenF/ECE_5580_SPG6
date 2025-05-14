#ifndef SIGMA_SK_IO_H
#define SIGMA_SK_IO_H

#include <stddef.h>

#define CNT 100
#define SIGMA_LEN 16
#define SK_LEN 16

void load_sigma_sk_from_file(unsigned char sigmas[CNT][SIGMA_LEN], unsigned char sks[CNT][SK_LEN]);

#endif // SIGMA_SK_IO_H

