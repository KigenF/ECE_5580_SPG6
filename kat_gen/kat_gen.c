#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "rng.h"
#include "api.h"

#define SEED_HEX_LEN 96
#define SEED_LEN     48
#define SIGMA_LEN    16
#define SK_LEN       16
#define M_LEN        16
#define RHO_LEN      16

#define	MAX_MARKER_LEN		50
#define KAT_SUCCESS          0
#define KAT_FILE_OPEN_ERROR -1
#define KAT_DATA_ERROR      -3
#define KAT_CRYPTO_FAILURE  -4

int		FindMarker(FILE *infile, const char *marker);
int		ReadHex(FILE *infile, unsigned char *A, int Length, char *str);
void	fprintBstr(FILE *fp, char *S, unsigned char *A, unsigned long long L);

// hex to byte
void hexstr_to_bytes(const char *hex_str, unsigned char *bytes, size_t bytes_len) {
    for (size_t i = 0; i < bytes_len; i++) {
        unsigned char hi, lo;
        char c1 = hex_str[2 * i];
        char c2 = hex_str[2 * i + 1];
        if (c1 >= '0' && c1 <= '9')
            hi = c1 - '0';
        else if (c1 >= 'A' && c1 <= 'F')
            hi = c1 - 'A' + 10;
        else if (c1 >= 'a' && c1 <= 'f')
            hi = c1 - 'a' + 10;
        else
            hi = 0;
        
        if (c2 >= '0' && c2 <= '9')
            lo = c2 - '0';
        else if (c2 >= 'A' && c2 <= 'F')
            lo = c2 - 'A' + 10;
        else if (c2 >= 'a' && c2 <= 'f')
            lo = c2 - 'a' + 10;
        else
            lo = 0;
        
        bytes[i] = (hi << 4) | lo;
    }
}

// byte to hex
// hex_str のサイズは最低 (len*2 + 1) バイト必要
void bytes_to_hexstr(const unsigned char *bytes, size_t len, char *hex_str) {
    for (size_t i = 0; i < len; i++) {
        sprintf(hex_str + (i * 2), "%02X", bytes[i]);
    }
    hex_str[len * 2] = '\0';
}

int main(void) {
    char                fn_req[32], fn_rsp[32];
    FILE                *fp_req, *fp_rsp;
    unsigned char       seed[48];
    unsigned char       entropy_input[48];
    unsigned char       ct[CRYPTO_CIPHERTEXTBYTES], ss[CRYPTO_BYTES], ss1[CRYPTO_BYTES];
    int                 done;
    unsigned char       pk[CRYPTO_PUBLICKEYBYTES], sk[CRYPTO_SECRETKEYBYTES];
    int                 ret_val;

    FILE *fin = fopen("PQCkemKAT_16-seed.txt", "r");
    if (!fin) {
        perror("Error opening input file PQCkemKAT_16-seed.txt");
        return 1;
    }
    
    // Create the REQUEST file
    sprintf(fn_req, "PQCkemKAT_%d.req", CRYPTO_SECRETKEYBYTES);
    if ( (fp_req = fopen(fn_req, "w")) == NULL ) {
        printf("Couldn't open <%s> for write\n", fn_req);
        return KAT_FILE_OPEN_ERROR;
    }
    // sprintf(fn_rsp, "PQCkemKAT_%d.rsp", CRYPTO_SECRETKEYBYTES);
    // if ( (fp_rsp = fopen(fn_rsp, "w")) == NULL ) {
    //     printf("Couldn't open <%s> for write\n", fn_rsp);
    //     return KAT_FILE_OPEN_ERROR;
    // }
    
    char seed_hex[SEED_HEX_LEN + 2]; // +2 で改行やNULL用
    int count = 0;
    
    // get seed
    while (fgets(seed_hex, sizeof(seed_hex), fin) != NULL) {
        // remove indent
        char *newline = strchr(seed_hex, '\n');
        if (newline) *newline = '\0';
        
        // generate output block
        fprintf(fp_req, "count = %d\n", count);
        fprintf(fp_req, "seed = %s\n", seed_hex);
        
        // seed_hex to bytes
        unsigned char seed_bytes[SEED_LEN];
        // 入力の seed は SEED_HEX_LEN 桁の16進文字列であることを想定
        if (strlen(seed_hex) < SEED_HEX_LEN) {
            fprintf(stderr, "Seed string too short on line %d\n", count);
            continue;
        }
        hexstr_to_bytes(seed_hex, seed_bytes, SEED_LEN);
        
        // initialize randombytes with seed
        randombytes_init(seed_bytes, NULL, 256);
        
        // randombytes() を呼び出して各フィールドを生成
        unsigned char sigma[SIGMA_LEN], sk[SK_LEN], m[M_LEN], rho[RHO_LEN];
        randombytes(sigma, SIGMA_LEN);
        randombytes(sk, SK_LEN);
        randombytes(m, M_LEN);
        randombytes(rho, RHO_LEN);
        
        
        // 出力フォーマットに従い、各フィールドを書き出す
        fprintBstr(fp_req, "sigma = ", sigma, SIGMA_LEN);
        fprintf(fp_req, "pk =\n");
        fprintBstr(fp_req, "sk = ", sk, SK_LEN);
        fprintBstr(fp_req, "m = ", m, M_LEN);
        fprintBstr(fp_req, "rho = ", rho, RHO_LEN);
        fprintf(fp_req, "ct =\n");
        fprintf(fp_req, "ss =\n\n");
        
        count++;
    }
    
    fclose(fin);
    fclose(fp_req);
    
    printf("Generated %d test blocks in Modified_PQCkemKAT_16.req\n", count);
    return 0;
}


void
fprintBstr(FILE *fp, char *S, unsigned char *A, unsigned long long L)
{
	unsigned long long  i;

	fprintf(fp, "%s", S);

	for ( i=0; i<L; i++ )
		fprintf(fp, "%02X", A[i]);

	if ( L == 0 )
		fprintf(fp, "00");

	fprintf(fp, "\n");
}
