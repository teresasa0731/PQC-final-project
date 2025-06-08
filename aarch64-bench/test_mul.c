#include <stdio.h>
#include <stdint.h>
#include <gmp.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

extern void karatsuba_512_mul(const uint32_t *a, const uint32_t *b, uint32_t *result);

void random_512(uint32_t *x) {
    for (int i = 0; i < 8; i++) {
        x[i] = ((uint32_t)rand() << 16) ^ (rand() & 0xFFFF);
    }
}

void print_hex_512(const char *label, const uint32_t *val) {
    printf("%s: ", label);
    for (int i = 7; i >= 0; i--)
        printf("%08x", val[i]);
    printf("\n");
}

void print_diff_bits(const mpz_t GMP_result, const mpz_t My_result) {
    char gmp_str[1025], my_str[1025];
    gmp_sprintf(gmp_str, "%Zx", GMP_result);
    gmp_sprintf(my_str, "%Zx", My_result);

    int gmp_len = strlen(gmp_str);
    int my_len = strlen(my_str);
    int max_len = gmp_len > my_len ? gmp_len : my_len;
    if (max_len < 128) max_len = 128;

    char gmp_padded[129] = {0};
    char my_padded[129] = {0};

    snprintf(gmp_padded, 129, "%0*.*s", max_len, max_len, gmp_str);
    snprintf(my_padded, 129, "%0*.*s", max_len, max_len, my_str);

    printf("❌ Diff  : ");
    for (int i = 0; i < max_len; i++) {
        putchar(gmp_padded[i] == my_padded[i] ? '0' : '1');
    }
    putchar('\n');
}

int main(void)
{
    srand((unsigned)time(NULL));

    const int NUM_TESTS = 10;
    int cnt = 0;
    for (int test = 1; test <= NUM_TESTS; ++test) {
        uint32_t a[8], b[8];
        uint32_t result[16] = {0};
        random_512(a);
        random_512(b);

        karatsuba_512_mul(a, b, result);

        mpz_t A, B, GMP_result, My_result;
        mpz_inits(A, B, GMP_result, My_result, NULL);
        mpz_import(A, 8, -1, sizeof(uint32_t), 0, 0, a);
        mpz_import(B, 8, -1, sizeof(uint32_t), 0, 0, b);
        mpz_mul(GMP_result, A, B);
        mpz_import(My_result, 16, -1, sizeof(uint32_t), 0, 0, result);

        printf("==== Test Case %d ====\n", test);
        print_hex_512("A     ", a);
        print_hex_512("B     ", b);

        if (mpz_cmp(GMP_result, My_result) == 0) {
            printf("Passed\n\n");
            cnt++;
        } else {
            gmp_printf("❌ GMP   : %Zx\n", GMP_result);
            gmp_printf("❌ Your  : %Zx\n", My_result);
            print_diff_bits(GMP_result, My_result);
            printf("❌ Mismatch at Test Case %d!\n\n", test);
            
        }

        mpz_clears(A, B, GMP_result, My_result, NULL);

        
    }
    printf("Pass rate %d%% (%d/%d)\n", cnt*100/NUM_TESTS, cnt, NUM_TESTS);

    return 0;
}
