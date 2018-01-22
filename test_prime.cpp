#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <climits>

#include "mini-gmp-impl.h"

struct PrimeTestCxt;
PrimeTestCxt* primeTestInit();
void primeTest(PrimeTestCxt* cxt, uint32_t N_Size, int listSize, const uint32_t* M, uint8_t* is_prime);
void primeTestTerm(PrimeTestCxt* cxt);

int main()
{
	PrimeTestCxt* cxt = primeTestInit();

	const int LIST_SIZE = 16;
	const int N_Size = 9;
	const int MAX_N_SIZE = 64;

	int errors = 0;

	srand(0);

	uint32_t *M = (uint32_t*)malloc(sizeof(uint32_t)*MAX_N_SIZE*LIST_SIZE);
	uint8_t *is_prime = (uint8_t*)malloc(sizeof(uint8_t)*LIST_SIZE);

	uint32_t k = 0;

	mpz_t z_ft_r, z_ft_n, z_m;
	mpz_init(z_ft_r);
	mpz_init(z_ft_n);
	mpz_init_set_ui(z_m, 2);
	mpz_pow_ui(z_m, z_m, (N_Size - 1) * 32);
	mpz_t z_ft_b;
	mpz_init_set_ui(z_ft_b, 2);

	while (k < 1024)
	{
		for (int j = 1; j < N_Size; ++j)
		{
			M[j] = (rand() << 16) + rand();
		}
		if (k != 0x60)
		{
			k += LIST_SIZE;
			continue;
		}

		for (uint32_t i = 0; i < LIST_SIZE; i++) {
			M[i*MAX_N_SIZE] = (i + k) * 2 + 1;
			for (int j = 1; j < N_Size; ++j)
			{
				M[i*MAX_N_SIZE + j] = M[j];
			}
		}
		k += LIST_SIZE;

		primeTest(cxt, N_Size, LIST_SIZE, M, is_prime);

		for (int j = 1; j < (N_Size / 2); ++j)
		{
			z_m->_mp_d[j] = (mp_limb_t(M[j * 2 + 1]) << 32) + M[j * 2];
		}
		if (N_Size & 1)
		{
			z_m->_mp_d[N_Size / 2] = M[N_Size - 1];
		}

		mp_limb_t high_m = mp_limb_t(M[1]) << 32;
		for (uint32_t i = 0; i < LIST_SIZE; i+=3) {
			z_m->_mp_d[0] = high_m + M[i*MAX_N_SIZE];

			mpz_sub_ui(z_ft_n, z_m, 1);
			mpz_powm(z_ft_r, z_ft_b, z_ft_n, z_m);
			bool gmp_is_prime = mpz_cmp_ui(z_ft_r, 1) == 0;
			if (gmp_is_prime != bool(is_prime[i])) {
				printf("GMP %d disagrees with FPGA %d Index: %d\n", gmp_is_prime, is_prime[i], i);
				++errors;
				//abort();
			}

			if (is_prime[i]) printf("%d\n", M[i*MAX_N_SIZE]);
		}
	}

	primeTestTerm(cxt);

	free(M);
	free(is_prime);

	return errors;
}
