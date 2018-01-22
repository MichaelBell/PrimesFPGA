#pragma once
#define N_Size	12
#define LIST_SIZE 2

#include <string.h>
#include <stdint.h>

typedef uint32_t uint;
typedef uint64_t ulong;

void fermat_test(const uint M_in[N_Size * LIST_SIZE], const uint Mi_in[LIST_SIZE], const uint R_in[N_Size * LIST_SIZE], uint8_t is_prime[LIST_SIZE]);

