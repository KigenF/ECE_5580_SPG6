#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sigma_sk_io.h"

// Converts hex string to byte array.
void hex_to_bytes(const char *hex_str, unsigned char *out, size_t len) {
    for (size_t i = 0; i < len; i++) {
        sscanf(hex_str + 2 * i, "%2hhx", &out[i]);
    }
}

// Loads 100 entries from sigma_sk.txt.
void load_sigma_sk_from_file(unsigned char sigmas[CNT][SIGMA_LEN], unsigned char sks[CNT][SK_LEN]) {
    FILE *fp = fopen("sigma_sk.txt", "r");
    if (!fp) {
        perror("Failed to open sigma_sk.txt");
		return;
    }

    char line[4096];
    int count = 0;

    while (fgets(line, sizeof(line), fp) && count < CNT) {
        if (strncmp(line, "cnt = ", 6) == 0) {
            // Read sigma.
            if (!fgets(line, sizeof(line), fp) || strncmp(line, "sigma = ", 8) != 0) break;
            hex_to_bytes(line + 8, sigmas[count], SIGMA_LEN);

            // Read sk.
            if (!fgets(line, sizeof(line), fp) || strncmp(line, "sk = ", 5) != 0) break;
            hex_to_bytes(line + 5, sks[count], SK_LEN);

            count++;
        }
    }

    fclose(fp);
	return;
}

/*
// Example usage:
int main() {
	unsigned char sigmas[100][16];
	unsigned char sks[100][16];

	load_sigma_sk_from_file(sigmas, sks);
	
	// Processing...
	//
	//

	return 0;
}
*/
