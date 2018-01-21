#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

#include <algorithm>
#include <cstring>

#include "prime-gmp.h"

#include "prime.h"

#define DEBUG 0

#define MAX_JOB_SIZE LIST_SIZE

#define MAX_SOURCE_SIZE (0x100000)

const unsigned char  binvert_limb_table[128] = {
	0x01, 0xAB, 0xCD, 0xB7, 0x39, 0xA3, 0xC5, 0xEF,
	0xF1, 0x1B, 0x3D, 0xA7, 0x29, 0x13, 0x35, 0xDF,
	0xE1, 0x8B, 0xAD, 0x97, 0x19, 0x83, 0xA5, 0xCF,
	0xD1, 0xFB, 0x1D, 0x87, 0x09, 0xF3, 0x15, 0xBF,
	0xC1, 0x6B, 0x8D, 0x77, 0xF9, 0x63, 0x85, 0xAF,
	0xB1, 0xDB, 0xFD, 0x67, 0xE9, 0xD3, 0xF5, 0x9F,
	0xA1, 0x4B, 0x6D, 0x57, 0xD9, 0x43, 0x65, 0x8F,
	0x91, 0xBB, 0xDD, 0x47, 0xC9, 0xB3, 0xD5, 0x7F,
	0x81, 0x2B, 0x4D, 0x37, 0xB9, 0x23, 0x45, 0x6F,
	0x71, 0x9B, 0xBD, 0x27, 0xA9, 0x93, 0xB5, 0x5F,
	0x61, 0x0B, 0x2D, 0x17, 0x99, 0x03, 0x25, 0x4F,
	0x51, 0x7B, 0x9D, 0x07, 0x89, 0x73, 0x95, 0x3F,
	0x41, 0xEB, 0x0D, 0xF7, 0x79, 0xE3, 0x05, 0x2F,
	0x31, 0x5B, 0x7D, 0xE7, 0x69, 0x53, 0x75, 0x1F,
	0x21, 0xCB, 0xED, 0xD7, 0x59, 0xC3, 0xE5, 0x0F,
	0x11, 0x3B, 0x5D, 0xC7, 0x49, 0x33, 0x55, 0xFF
};

#define binvert_limb(inv,n)                                             \
  do {                                                                  \
    mp_limb_t  __n = (n);                                               \
    mp_limb_t  __inv;                                                   \
    assert ((__n & 1) == 1);                                            \
                                                                        \
    __inv = binvert_limb_table[(__n/2) & 0x7F]; /*  8 */                \
    if (GMP_LIMB_BITS > 8)   __inv = 2 * __inv - __inv * __inv * __n;   \
    if (GMP_LIMB_BITS > 16)  __inv = 2 * __inv - __inv * __inv * __n;   \
    if (GMP_LIMB_BITS > 32)  __inv = 2 * __inv - __inv * __inv * __n;   \
                                                                        \
    assert ((__inv * __n) == 1);                        \
    (inv) = __inv;                                      \
  } while (0)

static void setup_fermat(int num, const mp_limb_t* M, mp_limb_t* MI, mp_limb_t* R)
{
	for (int j = 0; j < num; ++j)
	{
		mp_size_t mn = N_Size;
		mp_limb_t mshifted[N_Size];
		mp_srcptr mp;
		mp_ptr rp;
		struct gmp_div_inverse minv;

		// REDCify: r = B^n * 2 % M
		mp = &M[j*N_Size];
		rp = &R[j*N_Size];
		mpn_div_qr_invert(&minv, mp, mn);

		if (minv.shift > 0)
		{
			mpn_lshift(mshifted, mp, mn, minv.shift);
			mp = mshifted;
		}
		else
		{
			for (size_t i = 0; i < mn; ++i) mshifted[i] = mp[i];
		}

		for (size_t i = 0; i < mn; ++i) rp[i] = 0;
		rp[mn] = 1 << minv.shift;
		mpn_div_r_preinv_ns(rp, mn + 1, mp, mn, &minv);

		if (minv.shift > 0)
		{
			mpn_rshift(rp, rp, mn, minv.shift);
			mp = &M[j*N_Size];
		}

		mp_limb_t mi;
		binvert_limb(mi, mp[0]);
		MI[j] = -mi;
	}
}

#if DEBUG
#define DPRINTF(fmt, args...) do { printf("line %d: " fmt, __LINE__, ##args); fflush(stdout); } while(0)
#else
#define DPRINTF(fmt, ...) do { } while(0)
#endif

typedef struct PrimeTestCxt
{
	uint *R;
	uint *MI;
	uint8_t* is_prime;
} PrimeTestCxt;

PrimeTestCxt* primeTestInit()
{
	PrimeTestCxt* cxt = new PrimeTestCxt;

	// Create buffers on host
	cxt->R = (uint*)malloc(sizeof(uint)*(N_Size*MAX_JOB_SIZE + 1));
	cxt->MI = (uint*)malloc(sizeof(uint)*MAX_JOB_SIZE);
	cxt->is_prime = (uint8_t*)malloc(sizeof(uint8_t)*MAX_JOB_SIZE);

	return cxt;
}

void primeTest(PrimeTestCxt* cxt, int in_N_Size, int listSize, const uint* M, uint8_t* is_prime)
{
	if (in_N_Size != N_Size)
	{
		printf("N Size out of bounds\n");
		abort();
	}

	int nextJobSize = std::min(MAX_JOB_SIZE, listSize);
	int jobSize = 0;
	int lastJobSize = 0;

	if (nextJobSize > 0)
	{
		setup_fermat(nextJobSize, M, cxt->MI, cxt->R);
	}

	while (nextJobSize == MAX_JOB_SIZE)
	{
		lastJobSize = jobSize;
		jobSize = nextJobSize;
		listSize -= jobSize;
		nextJobSize = std::min(MAX_JOB_SIZE, listSize);

		// Copy the lists A and B to their respective memory buffers
		DPRINTF("before execution\n");
		fermat_test(M, cxt->MI, cxt->R, cxt->is_prime);
		
		memcpy(is_prime, cxt->is_prime, jobSize * sizeof(uint8_t));
		is_prime += jobSize;

		if (nextJobSize > 0)
		{
			M += jobSize*N_Size;
			setup_fermat(nextJobSize, M, cxt->MI, cxt->R);
		}
	}
}

void primeTestTerm(PrimeTestCxt* cxt)
{

	free(cxt->R);
	free(cxt->MI);
	free(cxt->is_prime);
	delete cxt;
}
