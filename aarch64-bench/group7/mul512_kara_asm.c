#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define C_KARA

void karatsuba_512_mul_asm(const uint32_t *a, const uint32_t *b, uint32_t *result);

void schoolbook_256_mul_asm(const uint32_t *a, const uint32_t *b, uint32_t *res);

static void add_n(uint32_t *dst, const uint32_t *src, int n)
{
    uint64_t carry = 0;
    for (int i = 0; i < n; i++)
    {
        uint64_t sum = (uint64_t)dst[i] + src[i] + carry;
        dst[i] = (uint32_t)sum;
        carry = sum >> 32;
    }
    for (int i = n; carry != 0 && i < 2 * n; i++)
    {
        uint64_t sum = (uint64_t)dst[i] + carry;
        dst[i] = (uint32_t)sum;
        carry = sum >> 32;
    }
}

static void sub_n(uint32_t *dst, const uint32_t *a, const uint32_t *b, int n)
{
    int64_t borrow = 0;
    for (int i = 0; i < n; i++)
    {
        int64_t diff = (int64_t)(uint64_t)a[i] - b[i] - borrow;
        if (diff < 0)
        {
            diff += ((int64_t)1 << 32);
            borrow = 1;
        }
        else
        {
            borrow = 0;
        }
        dst[i] = (uint32_t)diff;
    }
}

// /* 4x4 multiplication */
// static void schoolbook_256_mul(const uint32_t *a, const uint32_t *b, uint32_t *res)
// {
//     memset(res, 0, 8 * sizeof(uint32_t));
//     for (int i = 0; i < 4; i++)
//     {
//         uint64_t carry = 0;
//         for (int j = 0; j < 4; j++)
//         {
//             uint64_t sum = (uint64_t)a[i] * b[j] + res[i + j] + carry;
//             res[i + j] = (uint32_t)sum;
//             carry = sum >> 32;
//         }
//         res[i + 4] += (uint32_t)carry;
//     }
// }

static void schoolbook_512_mul(const uint32_t *a, const uint32_t *b, uint32_t *res)
{
    memset(res, 0, 16 * sizeof(uint32_t));
    for (int i = 0; i < 8; i++)
    {
        uint64_t carry = 0;
        for (int j = 0; j < 8; j++)
        {
            uint64_t sum = (uint64_t)a[i] * b[j] + res[i + j] + carry;
            res[i + j] = (uint32_t)sum;
            carry = sum >> 32;
        }
        res[i + 8] += (uint32_t)carry;
    }
}



/* Implementation with assembly */
void karatsuba_512_mul_asm(const uint32_t *a, const uint32_t *b, uint32_t *result)
{
    // printf("karatsuba_512_mul() has been called!\n");

    const uint32_t *a0 = a, *a1 = a + 4;
    const uint32_t *b0 = b, *b1 = b + 4;

    uint32_t z0[16] = {0}, z2[16] = {0};
    uint32_t z1[16] = {0}; //  z1: 512-bit space
    uint32_t a_sum[8] = {0}, b_sum[8] = {0};

    // z0 = a0 * b0
    schoolbook_256_mul_asm(a0, b0, z0);

    // z2 = a1 * b1
    schoolbook_256_mul_asm(a1, b1, z2);

    // z1 = (a0 + a1) * (b0 + b1)
    memcpy(a_sum, a0, 4 * sizeof(uint32_t));
    add_n(a_sum, a1, 4);
    memcpy(b_sum, b0, 4 * sizeof(uint32_t));
    add_n(b_sum, b1, 4);

    schoolbook_512_mul(a_sum, b_sum, z1);

    // z1 = z1 - z0 - z2
    sub_n(z1, z1, z0, 16);
    sub_n(z1, z1, z2, 16);

    memset(result, 0, 16 * sizeof(uint32_t));
    memcpy(result, z0, 8 * sizeof(uint32_t));
    add_n(result + 4, z1, 12); // add z1 to middle position
    add_n(result + 8, z2, 8);  // Add z2 to high position
}

// /* Implementation with c language */
// void karatsuba_512_mul_c(const uint32_t *a, const uint32_t *b, uint32_t *result)
// {
//     // printf("karatsuba_512_mul() has been called!\n");

//     const uint32_t *a0 = a, *a1 = a + 4;
//     const uint32_t *b0 = b, *b1 = b + 4;

//     uint32_t z0[16] = {0}, z2[16] = {0};
//     uint32_t z1[16] = {0}; //  z1: 512-bit space
//     uint32_t a_sum[8] = {0}, b_sum[8] = {0};

//     // z0 = a0 * b0
//     schoolbook_256_mul(a0, b0, z0);

//     // z2 = a1 * b1
//     schoolbook_256_mul(a1, b1, z2);

//     // z1 = (a0 + a1) * (b0 + b1)
//     memcpy(a_sum, a0, 4 * sizeof(uint32_t));
//     add_n(a_sum, a1, 4);
//     memcpy(b_sum, b0, 4 * sizeof(uint32_t));
//     add_n(b_sum, b1, 4);

//     schoolbook_512_mul(a_sum, b_sum, z1);

//     // z1 = z1 - z0 - z2
//     sub_n(z1, z1, z0, 16);
//     sub_n(z1, z1, z2, 16);

//     memset(result, 0, 16 * sizeof(uint32_t));
//     memcpy(result, z0, 8 * sizeof(uint32_t));
//     add_n(result + 4, z1, 12); // add z1 to middle position
//     add_n(result + 8, z2, 8);  // Add z2 to high position
// }
