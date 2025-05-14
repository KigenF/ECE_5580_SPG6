#include <stdlib.h>
#include <stdbool.h>
#include "shake.h"
#include "oryx/xof/cshake.h"
#include "generalFunctions.h"


int dotMatMatBitslice(uint16_t *matrix_out, uint16_t *matrix1, const uint16_t m1_rows, const uint16_t m1_cols, uint16_t *matrix2, const uint16_t m2_rows, const uint16_t m2_cols, const uint16_t sliceLength);

