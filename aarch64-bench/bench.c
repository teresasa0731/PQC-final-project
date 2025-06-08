/*
 * Copyright (c) 2024-2025 The mlkem-native project authors
 * SPDX-License-Identifier: Apache-2.0
 */
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hal.h"

#include <gmp.h>
#include <time.h>
extern void karatsuba_512_mul(const uint32_t *a, const uint32_t *b, uint32_t *result);
extern void karatsuba_512_mul_asm(const uint32_t *a, const uint32_t *b, uint32_t *result);

extern void mod_2to512_minus1(uint32_t *x);
extern void mod_2to256_minus1(uint32_t *x);

// #define checkAB
#define checkKARA
#define checkMOD
#define KARA_ASM

#define NWARMUP 50
#define NITERATIONS 300
#define NTESTS 500

#define LIMBS 8

const uint32_t MOD_P[LIMBS] = {
    // revised
    // 0xffffffffffffffffULL, 0xffffffffffffffffULL,
    // 0xffffffffffffffffULL, 0xffffffffffffffffULL,
    // 0xffffffffffffffffULL, 0xffffffffffffffffULL,
    // 0xffffffffffffffffULL, 0xffffffffffffffffULL // = 2^512 - 1
    0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF

};

static int cmp_uint64_t(const void *a, const void *b)
{
  return (int)((*((const uint64_t *)a)) - (*((const uint64_t *)b)));
}

static void print_median(const char *txt, uint64_t cyc[NTESTS])
{
  printf("%10s cycles = %" PRIu64 "\n", txt, cyc[NTESTS >> 1] / NITERATIONS);
}

static int percentiles[] = {1, 10, 20, 30, 40, 50, 60, 70, 80, 90, 99};

static void print_percentile_legend(void)
{
  unsigned i;
  printf("%21s", "percentile");
  for (i = 0; i < sizeof(percentiles) / sizeof(percentiles[0]); i++)
  {
    printf("%7d", percentiles[i]);
  }
  printf("\n");
}

static void print_percentiles(const char *txt, uint64_t cyc[NTESTS])
{
  unsigned i;
  printf("%10s percentiles:", txt);
  for (i = 0; i < sizeof(percentiles) / sizeof(percentiles[0]); i++)
  {
    printf("%7" PRIu64, (cyc)[NTESTS * percentiles[i] / 100] / NITERATIONS);
  }
  printf("\n");
}

/* Test case & initialization */

static uint32_t a[LIMBS]; // 32->64
static uint32_t b[LIMBS]; // 32->64

static void random_input(void)
{
  for (int i = 0; i < LIMBS; ++i)
  {
    a[i] = ((uint32_t)rand() << 16) | rand();
    b[i] = ((uint32_t)rand() << 16) | rand();
  }
}

static int bench(void)
{
  int i, j;
  uint64_t t0, t1;

  mpz_t A, B, R, M;
  mpz_init(A);
  mpz_init(B);
  mpz_init(R);
  mpz_init(M);
  mpz_import(M, LIMBS, -1, sizeof(uint32_t), 0, 0, MOD_P);

  random_input(); // randomly generate and pass the same A, B to both GMP and Karatsuba

  printf("\n========= mpz_mul ==========\n");

  uint64_t cycles_mpz[NTESTS];

  for (i = 0; i < NTESTS; i++)
  {
    for (j = 0; j < NWARMUP; j++)
    {
      mpz_import(A, LIMBS, -1, sizeof(uint32_t), 0, 0, a);
      mpz_import(B, LIMBS, -1, sizeof(uint32_t), 0, 0, b);
      mpz_mul(R, A, B);
      mpz_mod(R, R, M);
    }

    t0 = get_cyclecounter();
    for (j = 0; j < NITERATIONS; j++)
    {
      mpz_import(A, LIMBS, -1, sizeof(uint32_t), 0, 0, a);
      mpz_import(B, LIMBS, -1, sizeof(uint32_t), 0, 0, b);
      mpz_mul(R, A, B);
      mpz_mod(R, R, M);
    }
    t1 = get_cyclecounter();
    cycles_mpz[i] = t1 - t0;
  }
#ifdef checkAB
  gmp_printf("Input A: %Zx\n", A);
  gmp_printf("Input B: %Zx\n", B);
#endif

  qsort(cycles_mpz, NTESTS, sizeof(uint64_t), cmp_uint64_t);
  print_median("mpz_mul", cycles_mpz);
  print_percentile_legend();
  print_percentiles("mpz_mul", cycles_mpz);
  printf("\n");

  printf("========= your_mul (Karatsuba with pure C) ==========\n");

  uint64_t cycles_karatsuba[NTESTS];
  uint32_t result[16], raw_result[16];

  for (i = 0; i < NTESTS; i++)
  {
    for (j = 0; j < NWARMUP; j++)
    {
      karatsuba_512_mul(a, b, raw_result);
      memcpy(result, raw_result, sizeof(result));
      mod_2to256_minus1(result);
    }

    t0 = get_cyclecounter();
    for (j = 0; j < NITERATIONS; j++)
    {
      karatsuba_512_mul(a, b, raw_result);
      memcpy(result, raw_result, sizeof(result));
      mod_2to256_minus1(result);
    }
    t1 = get_cyclecounter();
    cycles_karatsuba[i] = t1 - t0;
  }

  qsort(cycles_karatsuba, NTESTS, sizeof(uint64_t), cmp_uint64_t);
  print_median("Our_mul", cycles_karatsuba);
  print_percentile_legend();
  print_percentiles("Our_mul", cycles_karatsuba);
  printf("\n");

#ifdef KARA_ASM
  printf("========= your_mul (Karatsuba with partial assembly) ==========\n");

  uint64_t asm_cycles_karatsuba[NTESTS];
  uint32_t asm_result[16], asm_raw_result[16];

  for (i = 0; i < NTESTS; i++)
  {
    for (j = 0; j < NWARMUP; j++)
    {
      karatsuba_512_mul_asm(a, b, asm_raw_result);
      memcpy(asm_result, asm_raw_result, sizeof(asm_result));
      mod_2to256_minus1(asm_result);
    }

    t0 = get_cyclecounter();
    for (j = 0; j < NITERATIONS; j++)
    {
      karatsuba_512_mul_asm(a, b, asm_raw_result);
      memcpy(asm_result, asm_raw_result, sizeof(asm_result));
      mod_2to256_minus1(asm_result);
    }
    t1 = get_cyclecounter();
    asm_cycles_karatsuba[i] = t1 - t0;
  }

  qsort(asm_cycles_karatsuba, NTESTS, sizeof(uint64_t), cmp_uint64_t);
  print_median("Our_mul", asm_cycles_karatsuba);
  print_percentile_legend();
  print_percentiles("Our_mul", asm_cycles_karatsuba);
  printf("\n");

#endif

#ifdef checkKARA
  // Compare full multiplication result
  mpz_t RAW_KARA, RAW_GMP;
  mpz_init(RAW_KARA);
  mpz_init(RAW_GMP);
  mpz_import(RAW_KARA, 16, -1, sizeof(uint32_t), 0, 0, raw_result);
  mpz_mul(RAW_GMP, A, B); // GMP full product

  gmp_printf("GMP FullProd : %Zx\n", RAW_GMP);
  gmp_printf("Raw Karatsuba: %Zx\n", RAW_KARA);

  if (mpz_cmp(RAW_KARA, RAW_GMP) != 0)
  {
    printf("Mismatch in full product: Karatsuba is WRONG\n");
  }
  else
  {
    printf("Full product matches: Karatsuba is correct\n");
  }
  mpz_clear(RAW_GMP);
  mpz_clear(RAW_KARA);
#endif

#ifdef checkMOD
  // Compare mod result
  mpz_t TMP;
  mpz_init(TMP);
  mpz_import(TMP, 8, -1, sizeof(uint32_t), 0, 0, result);
  gmp_printf("\nGMP mod P: %Zx\n", R);
  gmp_printf("Our mod P: %Zx\n", TMP);
  if (mpz_cmp(R, TMP) != 0)
  {
    printf("Mismatch after mod: mod_2to512_minus1 is WRONG\n");
  }
  else
  {
    printf("Mod result matches: mod_2to256_minus1 is correct\n");
  }
  mpz_clear(TMP);
#endif



  // clear
  mpz_clear(A);
  mpz_clear(B);
  mpz_clear(R);
  mpz_clear(M);

  return 0;
}

int main(void)
{
  srand(time(NULL));
  enable_cyclecounter();
  bench();
  disable_cyclecounter();

  return 0;
}