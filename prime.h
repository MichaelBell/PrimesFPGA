#pragma once
#define MAX_N_SIZE	64
#define LIST_SIZE 360

#include <string.h>
#include <stdint.h>

typedef uint32_t uint;
typedef uint64_t ulong;

void fermat_test(const uint N_Size, const uint M_in[MAX_N_SIZE * LIST_SIZE], const uint Mi_in[LIST_SIZE], const uint R_in[MAX_N_SIZE * LIST_SIZE], uint8_t is_prime[LIST_SIZE]);

