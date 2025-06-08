// group7_test/mod_2to512_minus1.c
#include <stdint.h>

void mod_2to256_minus1(uint32_t *x); // prototype

void mod_2to256_minus1(uint32_t *x)
{
    uint64_t carry = 0;

    // Add upper 256 bits (x[8]~x[15]) into lower 256 bits (x[0]~x[7])
    for (int i = 0; i < 8; i++)
    {
        uint64_t sum = (uint64_t)x[i] + x[i + 8] + carry;
        x[i] = (uint32_t)sum;
        carry = sum >> 32;
    }

    // Wraparound carry
    while (carry)
    {
        uint64_t sum = (uint64_t)x[0] + carry;
        x[0] = (uint32_t)sum;
        carry = sum >> 32;

        for (int i = 1; carry && i < 8; i++)
        {
            sum = (uint64_t)x[i] + carry;
            x[i] = (uint32_t)sum;
            carry = sum >> 32;
        }
    }

    // Clear upper limbs
    for (int i = 8; i < 16; i++)
    {
        x[i] = 0;
    }
}
