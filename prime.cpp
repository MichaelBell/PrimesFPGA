#include "prime.h"

#include "ap_int.h"

typedef ap_uint<7> nsize_t;

template<uint foo>
uint8_t one_test(const nsize_t N_Size, const uint M[MAX_N_SIZE], const uint mi, uint R[MAX_N_SIZE])
{
	const uint shift = __builtin_clz(M[N_Size - 1]);
	const uint highbit = ((uint)1) << 31;
	uint startbit = highbit >> shift;

	nsize_t en = N_Size;
	one_test_label0:while (en-- > 0)
	{
		ap_uint<33> bit = startbit;
		startbit = highbit;
		ap_uint<33> E = M[en];
		if (en == 0)
		{
			E = (E - 1) << 1;
			bit <<= 1;
		}

		do
		{
			p_array_label:
			{
				uint P[MAX_N_SIZE * 2];
				//mpn_sqr(pp, rp, mn);
				if (en != 0 || bit != 1)
				{
					uint T[MAX_N_SIZE * 2 - 1];

					{
						uint cy = 0;
						uint v = R[0];
						mul1_label:for (nsize_t i = 0; i < N_Size - 1; ++i)
						{
							ulong p = ulong(R[i + 1]) * ulong(v) + cy;
							T[i] = uint(p);
							cy = uint(p >> 32);
							T[N_Size + i] = 0;
						}
						T[N_Size - 1] = cy;
					}

					for (nsize_t j = 2; j < N_Size; ++j)
					{
						uint cy = 0;
						fermat_test_label0:for (nsize_t i = j; i < N_Size; ++i)
						{
							ulong p = ulong(R[i]) * ulong(R[j - 1]) + (ulong(T[i + j - 2]) + cy);
							T[i + j - 2] = uint(p);
							cy = uint(p >> 32);
						}
						T[N_Size + j - 2] = cy;
					}

					{
						ulong p = ulong(R[0]) * ulong(R[0]);
						P[0] = uint(p);

						uint cy = 0;
						one_test_label1:for (nsize_t i = 0; i < N_Size - 1; ++i)
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
				else
				{
					one_test_label4:for (nsize_t i = 0; i < N_Size; ++i)
					{
						P[i] = R[i];
						P[N_Size + i] = 0;
					}
				}

				//if (mpn_redc_1(rp, pp, mp, mn, mi) != 0)
				//  mpn_sub_n(rp, rp, mshifted, n);
				{
					one_test_label6:for (nsize_t j = 0; j < N_Size; ++j)
					{
						uint cy = 0;
						uint v = P[j] * mi;
						fermat_test_label1:for (nsize_t i = 0; i < N_Size; ++i)
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
					ap_uint<1> cy = 0;
					sum1_label:for (nsize_t i = 0; i < N_Size - 1; i += 2)
					{
						ap_uint<65> a = (ap_uint<65>(R[i]) | (ap_uint<65>(R[i+1]) << 32)) + cy;
						a += (ap_uint<65>(P[i + N_Size]) | (ap_uint<65>(P[i+1 + N_Size]) << 32));
						R[i] = uint(a);
						R[i+1] = uint(a >> 32);
						cy = a >> 64;
					}
					if (N_Size & 1)
					{
						ap_uint<33> a = ap_uint<33>(R[N_Size - 1]) + ap_uint<33>(P[2*N_Size - 1]) + cy;
						R[N_Size - 1] = uint(a);
						cy = a >> 32;
					}

					if (cy != 0)
					{
						ap_int<1> borrow = 0;
						uint last_shifted = 0;
						sub1_label:for (nsize_t i = 0; i < N_Size - 1; i += 2)
						{
							ap_int<65> a = (ap_int<65>(R[i]) | (ap_int<65>(R[i+1]) << 32));
							ap_uint<64> b = (ap_uint<64>(M[i+1]) << (shift + 32)) | (ap_uint<64>(M[i]) << shift) | last_shifted;
							last_shifted = M[i+1] >> (32 - shift);
							a = a - ap_int<65>(b) + borrow;
							R[i] = uint(a);
							R[i+1] = uint(a >> 32);
							borrow = a >> 64;
						}
						if (N_Size & 1)
						{
							ap_int<33> a = R[N_Size - 1];
							uint b = (M[N_Size - 1] << shift) | last_shifted;
							R[N_Size - 1] = a - ap_int<33>(b) + borrow;
						}
					}
				}
			}

			if (E & bit)
			{
				//mp_limb_t carry = mpn_lshift(rp, rp, mn, 1);
				ap_uint<1> carry = 0;
				shift1_label:for (nsize_t i = 0; i < N_Size - 1; i+=2)
				{
					ap_uint<1> t = R[i+1] >> 31;
					R[i+1] = (R[i+1] << 1) | (R[i] >> 31);
					R[i] = (R[i] << 1) | carry;
					carry = t;
				}
				if (N_Size & 1)
				{
					ap_uint<1> t = R[N_Size - 1] >> 31;
					R[N_Size - 1] = (R[N_Size - 1] << 1) | carry;
					carry = t;
				}
				while (carry)
				{
					//carry -= mpn_sub_n(rp, rp, mshifted, mn);
					ap_int<1> borrow = 0;
					uint last_shifted = 0;
					sub2_label:for (nsize_t i = 0; i < N_Size - 1; i += 2)
					{
						ap_int<65> a = (ap_int<65>(R[i]) | (ap_int<65>(R[i+1]) << 32));
						ap_uint<64> b = (ap_uint<64>(M[i+1]) << (shift + 32)) | (ap_uint<64>(M[i]) << shift) | last_shifted;
						last_shifted = M[i+1] >> (32 - shift);
						a = a - ap_int<65>(b) + borrow;
						R[i] = uint(a);
						R[i+1] = uint(a >> 32);
						borrow = a >> 64;
					}
					if (N_Size & 1)
					{
						ap_int<33> a = R[N_Size - 1];
						uint b = (M[N_Size - 1] << shift) | last_shifted;
						a = a - ap_int<33>(b) + borrow;
						R[N_Size - 1] = uint(a);
						borrow = a >> 32;
					}
					carry += borrow;
				}
			}
			bit >>= 1;
		} while (bit > 0);

	}

	uint8_t result = 1;
	if (R[N_Size - 1] != 0)
	{
		// Compare to m+1
		ap_uint<1> cy = 1;
		check1_label:for (nsize_t i = 0; i < N_Size && result; ++i)
		{
			ap_uint<33> a = M[i] + cy;
			cy = a >> 32;
			if (R[i] != uint(a)) result = 0;
		}
	}
	else
	{
		// Compare to 1
		result = R[0] == 1;
		check2_label:for (nsize_t i = 1; i < N_Size && result; ++i)
		{
			if (R[i] != 0) result = 0;
		}
	}

	return result;
}

#define TESTS_PER_GROUP 16

void fermat_test(const uint N_Size, const uint M_in[MAX_N_SIZE * LIST_SIZE], const uint Mi_in[LIST_SIZE], const uint R_in[MAX_N_SIZE * LIST_SIZE], uint8_t is_prime[LIST_SIZE])
{
#pragma HLS INTERFACE s_axilite register port=N_Size bundle=axis
#pragma HLS INTERFACE s_axilite port=return bundle=axis
#pragma HLS INTERFACE s_axilite port=is_prime bundle=axis
#pragma HLS INTERFACE s_axilite port=R_in bundle=axis
#pragma HLS INTERFACE s_axilite port=Mi_in bundle=axis
#pragma HLS INTERFACE s_axilite port=M_in bundle=axis

	fermat_test_outer_loop:
	for (int idx = 0; idx < LIST_SIZE; idx += TESTS_PER_GROUP)
	{
		int offset;

#define PREP_TEST(NUM) \
		offset = (idx + NUM) * MAX_N_SIZE; \
		uint M##NUM[MAX_N_SIZE]; \
		uint mi##NUM = Mi_in[idx]; \
		uint R##NUM[MAX_N_SIZE]; \
 \
		copy_label##NUM:for (nsize_t i = 0; i < N_Size; ++i) \
		{ \
			M##NUM[i] = M_in[offset + i]; \
			R##NUM[i] = R_in[offset + i]; \
		}

		PREP_TEST(0);
		PREP_TEST(1);
		PREP_TEST(2);
		PREP_TEST(3);
		PREP_TEST(4);
		PREP_TEST(5);
		PREP_TEST(6);
		PREP_TEST(7);
		PREP_TEST(8);
		PREP_TEST(9);
		PREP_TEST(10);
		PREP_TEST(11);
		PREP_TEST(12);
		PREP_TEST(13);
		PREP_TEST(14);
		PREP_TEST(15);

#define RUN_TEST(NUM) \
		is_prime[idx + NUM] = one_test<NUM>(N_Size, M##NUM, mi##NUM, R##NUM)

		RUN_TEST(0);
		RUN_TEST(1);
		RUN_TEST(2);
		RUN_TEST(3);
		RUN_TEST(4);
		RUN_TEST(5);
		RUN_TEST(6);
		RUN_TEST(7);
		RUN_TEST(8);
		RUN_TEST(9);
		RUN_TEST(10);
		RUN_TEST(11);
		RUN_TEST(12);
		RUN_TEST(13);
		RUN_TEST(14);
		RUN_TEST(15);
	}
}
