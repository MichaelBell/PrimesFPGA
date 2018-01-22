#include "prime.h"

#include "ap_int.h"

uint8_t one_test(const uint M[N_Size], const uint mi, uint R[N_Size])
{
	const uint shift = __builtin_clz(M[N_Size - 1]);
	const uint highbit = ((uint)1) << 31;
	uint startbit = highbit >> shift;

	int en = N_Size;
	one_test_label0:while (en-- > 0)
	{
		uint bit = startbit;
		startbit = highbit;
		uint E = M[en];
		if (en == 0) E--;

		do
		{
			p_array_label:
			{
				uint P[N_Size * 2];
				//mpn_sqr(pp, rp, mn);
				{
					uint T[(N_Size - 1) * 2];

					{
						uint cy = 0;
						uint v = R[0];
						mul1_label:for (int i = 0; i < N_Size - 1; ++i)
						{
							ulong p = ulong(R[i + 1]) * ulong(v) + cy;
							T[i] = uint(p);
							cy = uint(p >> 32);
							T[N_Size + i] = 0;
						}
						T[N_Size - 1] = cy;
					}

					for (int j = 2; j < N_Size; ++j)
					{
						uint cy = 0;
						fermat_test_label0:for (int i = j; i < N_Size; ++i)
						{
							ulong p = ulong(R[i]) * ulong(R[j - 1]) + T[i + j - 2];
							p += cy;
							T[i + j - 2] = uint(p);
							cy = uint(p >> 32);
						}
						T[N_Size + j - 2] = cy;
					}

					{
						ulong p = ulong(R[0]) * ulong(R[0]);
						P[0] = uint(p);

						uint cy = 0;
						one_test_label1:for (int i = 0; i < N_Size - 1; ++i)
						{
							uint t = T[2 * i] & highbit;
							ulong a = (p >> 32) + cy;
							a += T[2 * i] << 1;
							P[2 * i + 1] = uint(a);
							cy = (t >> 31) + uint(a >> 32);

							p = ulong(R[i+1]) * ulong(R[i+1]);
							t = T[2 * i + 1] & highbit;
							a = (p & 0xFFFFFFFF) + cy;
							a += T[2 * i + 1] << 1;
							P[2 * i + 2] = uint(a);
							cy = (t >> 31) + uint(a >> 32);
						}
						P[2 * N_Size - 1] = uint(p >> 32) + cy;
					}
				}

				//if (mpn_redc_1(rp, pp, mp, mn, mi) != 0)
				//  mpn_sub_n(rp, rp, mshifted, n);
				{
					one_test_label6:for (int j = 0; j < N_Size; ++j)
					{
						uint cy = 0;
						uint v = P[j] * mi;
						fermat_test_label1:for (int i = 0; i < N_Size; ++i)
						{
							ulong p = ulong(M[i]) * ulong(v) + cy;
							p += P[i + j];
							P[i + j] = uint(p);
							cy = uint(p >> 32);
						}
						R[j] = cy;
					}
				}

				{
					uint cy = 0;
					sum1_label:for (int i = 0; i < N_Size; ++i)
					{
						ulong a = ulong(R[i]) + cy;
						a += P[i + N_Size];
						R[i] = uint(a);
						cy = uint(a >> 32);
					}

					if (cy != 0)
					{
						int32_t borrow = 0;
						uint last_shifted = 0;
						sub1_label:for (int i = 0; i < N_Size; ++i)
						{
							int64_t a = R[i];
							uint b = (M[i] << shift) | last_shifted;
							last_shifted = M[i] >> (32 - shift);
							a = a - int64_t(b) + borrow;
							R[i] = uint(a);
							borrow = int32_t(a >> 32);
						}
					}
				}
			}

			if (E & bit)
			{
				//mp_limb_t carry = mpn_lshift(rp, rp, mn, 1);
				uint carry = 0;
				shift1_label:for (int i = 0; i < N_Size; ++i)
				{
					uint t = R[i] & highbit;
					R[i] <<= 1;
					R[i] |= carry;
					carry = t >> 31;
				}
				while (carry)
				{
					//carry -= mpn_sub_n(rp, rp, mshifted, mn);
					int32_t borrow = 0;
					uint last_shifted = 0;
					sub2_label:for (int i = 0; i < N_Size; ++i)
					{
						int64_t a = R[i];
						uint b = (M[i] << shift) | last_shifted;
						last_shifted = M[i] >> (32 - shift);
						a = a - int64_t(b) + borrow;
						R[i] = uint(a);
						borrow = int32_t(a >> 32);
					}
					carry += borrow;
				}
			}
			bit >>= 1;
		} while (bit > 0);

	}

	// DeREDCify - necessary as rp can have a large
	//             multiple of m in it (although I'm not 100% sure
	//             why it can't after this redc!)
	{
		uint T[N_Size * 2];
		for (int i = 0; i < N_Size; ++i)
		{
			T[i] = R[i];
			T[N_Size + i] = 0;
		}

		// MPN_REDC_1(rp, tp, mp, mn, mi);
		for (int j = 0; j < N_Size; ++j)
		{
			uint cy = 0;
			uint v = T[j] * mi;
			for (int i = 0; i < N_Size; ++i)
			{
				ulong p = ulong(M[i]) * ulong(v) + cy;
				p += T[i + j];
				T[i + j] = uint(p);
				cy = uint(p >> 32);
			}
			R[j] = cy;
		}

		{
			uint cy = 0;
			for (int i = 0; i < N_Size; ++i)
			{
				ulong a = ulong(R[i]) + cy;
				a += T[i + N_Size];
				R[i] = uint(a);
				cy = uint(a >> 32);
			}

			if (cy != 0)
			{
				int32_t borrow = 0;
				uint last_shifted = 0;
				for (int i = 0; i < N_Size; ++i)
				{
					int64_t a = R[i];
					uint b = (M[i] << shift) | last_shifted;
					last_shifted = M[i] >> (32 - shift);
					a = a - int64_t(b) + borrow;
					R[i] = uint(a);
					borrow = int32_t(a >> 32);
				}
			}
		}
	}

	uint8_t result = 1;
	if (R[N_Size - 1] != 0)
	{
		// Compare to m+1
		uint cy = 1;
		for (int i = 0; i < N_Size && result; ++i)
		{
			uint a = M[i] + cy;
			cy = a < M[i];
			if (R[i] != a) result = 0;
		}
	}
	else
	{
		// Compare to 1
		result = R[0] == 1;
		for (int i = 1; i < N_Size && result; ++i)
		{
			if (R[i] != 0) result = 0;
		}
	}

	return result;
}

#define TESTS_PER_GROUP 2

void fermat_test(const uint M_in[N_Size * LIST_SIZE], const uint Mi_in[LIST_SIZE], const uint R_in[N_Size * LIST_SIZE], uint8_t is_prime[LIST_SIZE])
{
#pragma HLS INTERFACE s_axilite port=return bundle=axis
#pragma HLS INTERFACE s_axilite port=is_prime bundle=axis
#pragma HLS INTERFACE s_axilite port=R_in bundle=axis
#pragma HLS INTERFACE s_axilite port=Mi_in bundle=axis
#pragma HLS INTERFACE s_axilite port=M_in bundle=axis

	fermat_test_outer_loop:
	for (int idx = 0; idx < LIST_SIZE; idx += TESTS_PER_GROUP)
	{
		// Get the index of the current element to be processed
		const int offset1 = idx * N_Size;
		uint M1[N_Size];
		uint mi1 = Mi_in[idx];
		uint R1[N_Size];

		for (int i = 0; i < N_Size; ++i)
		{
			M1[i] = M_in[offset1 + i];
			R1[i] = R_in[offset1 + i];
		}

		const int offset2 = (idx + 1) * N_Size;
		uint M2[N_Size];
		uint mi2 = Mi_in[idx+1];
		uint R2[N_Size];

		for (int i = 0; i < N_Size; ++i)
		{
			M2[i] = M_in[offset2 + i];
			R2[i] = R_in[offset2 + i];
		}

		is_prime[idx] = one_test(M1, mi1, R1);
		is_prime[idx + 1] = one_test(M2, mi2, R2);
	}
}
