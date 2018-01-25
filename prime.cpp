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

#define TESTS_PER_GROUP 360

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
                PREP_TEST(16);
                PREP_TEST(17);
                PREP_TEST(18);
                PREP_TEST(19);
                PREP_TEST(20);
                PREP_TEST(21);
                PREP_TEST(22);
                PREP_TEST(23);
                PREP_TEST(24);
                PREP_TEST(25);
                PREP_TEST(26);
                PREP_TEST(27);
                PREP_TEST(28);
                PREP_TEST(29);
                PREP_TEST(30);
                PREP_TEST(31);
                PREP_TEST(32);
                PREP_TEST(33);
                PREP_TEST(34);
                PREP_TEST(35);
                PREP_TEST(36);
                PREP_TEST(37);
                PREP_TEST(38);
                PREP_TEST(39);
                PREP_TEST(40);
                PREP_TEST(41);
                PREP_TEST(42);
                PREP_TEST(43);
                PREP_TEST(44);
                PREP_TEST(45);
                PREP_TEST(46);
                PREP_TEST(47);
                PREP_TEST(48);
                PREP_TEST(49);
                PREP_TEST(50);
                PREP_TEST(51);
                PREP_TEST(52);
                PREP_TEST(53);
                PREP_TEST(54);
                PREP_TEST(55);
                PREP_TEST(56);
                PREP_TEST(57);
                PREP_TEST(58);
                PREP_TEST(59);
                PREP_TEST(60);
                PREP_TEST(61);
                PREP_TEST(62);
                PREP_TEST(63);
                PREP_TEST(64);
                PREP_TEST(65);
                PREP_TEST(66);
                PREP_TEST(67);
                PREP_TEST(68);
                PREP_TEST(69);
                PREP_TEST(70);
                PREP_TEST(71);
                PREP_TEST(72);
                PREP_TEST(73);
                PREP_TEST(74);
                PREP_TEST(75);
                PREP_TEST(76);
                PREP_TEST(77);
                PREP_TEST(78);
                PREP_TEST(79);
                PREP_TEST(80);
                PREP_TEST(81);
                PREP_TEST(82);
                PREP_TEST(83);
                PREP_TEST(84);
                PREP_TEST(85);
                PREP_TEST(86);
                PREP_TEST(87);
                PREP_TEST(88);
                PREP_TEST(89);
                PREP_TEST(90);
                PREP_TEST(91);
                PREP_TEST(92);
                PREP_TEST(93);
                PREP_TEST(94);
                PREP_TEST(95);
                PREP_TEST(96);
                PREP_TEST(97);
                PREP_TEST(98);
                PREP_TEST(99);
                PREP_TEST(100);
                PREP_TEST(101);
                PREP_TEST(102);
                PREP_TEST(103);
                PREP_TEST(104);
                PREP_TEST(105);
                PREP_TEST(106);
                PREP_TEST(107);
                PREP_TEST(108);
                PREP_TEST(109);
                PREP_TEST(110);
                PREP_TEST(111);
                PREP_TEST(112);
                PREP_TEST(113);
                PREP_TEST(114);
                PREP_TEST(115);
                PREP_TEST(116);
                PREP_TEST(117);
                PREP_TEST(118);
                PREP_TEST(119);
                PREP_TEST(120);
                PREP_TEST(121);
                PREP_TEST(122);
                PREP_TEST(123);
                PREP_TEST(124);
                PREP_TEST(125);
                PREP_TEST(126);
                PREP_TEST(127);
                PREP_TEST(128);
                PREP_TEST(129);
                PREP_TEST(130);
                PREP_TEST(131);
                PREP_TEST(132);
                PREP_TEST(133);
                PREP_TEST(134);
                PREP_TEST(135);
                PREP_TEST(136);
                PREP_TEST(137);
                PREP_TEST(138);
                PREP_TEST(139);
                PREP_TEST(140);
                PREP_TEST(141);
                PREP_TEST(142);
                PREP_TEST(143);
                PREP_TEST(144);
                PREP_TEST(145);
                PREP_TEST(146);
                PREP_TEST(147);
                PREP_TEST(148);
                PREP_TEST(149);
                PREP_TEST(150);
                PREP_TEST(151);
                PREP_TEST(152);
                PREP_TEST(153);
                PREP_TEST(154);
                PREP_TEST(155);
                PREP_TEST(156);
                PREP_TEST(157);
                PREP_TEST(158);
                PREP_TEST(159);
                PREP_TEST(160);
                PREP_TEST(161);
                PREP_TEST(162);
                PREP_TEST(163);
                PREP_TEST(164);
                PREP_TEST(165);
                PREP_TEST(166);
                PREP_TEST(167);
                PREP_TEST(168);
                PREP_TEST(169);
                PREP_TEST(170);
                PREP_TEST(171);
                PREP_TEST(172);
                PREP_TEST(173);
                PREP_TEST(174);
                PREP_TEST(175);
                PREP_TEST(176);
                PREP_TEST(177);
                PREP_TEST(178);
                PREP_TEST(179);
                PREP_TEST(180);
                PREP_TEST(181);
                PREP_TEST(182);
                PREP_TEST(183);
                PREP_TEST(184);
                PREP_TEST(185);
                PREP_TEST(186);
                PREP_TEST(187);
                PREP_TEST(188);
                PREP_TEST(189);
                PREP_TEST(190);
                PREP_TEST(191);
                PREP_TEST(192);
                PREP_TEST(193);
                PREP_TEST(194);
                PREP_TEST(195);
                PREP_TEST(196);
                PREP_TEST(197);
                PREP_TEST(198);
                PREP_TEST(199);
                PREP_TEST(200);
                PREP_TEST(201);
                PREP_TEST(202);
                PREP_TEST(203);
                PREP_TEST(204);
                PREP_TEST(205);
                PREP_TEST(206);
                PREP_TEST(207);
                PREP_TEST(208);
                PREP_TEST(209);
                PREP_TEST(210);
                PREP_TEST(211);
                PREP_TEST(212);
                PREP_TEST(213);
                PREP_TEST(214);
                PREP_TEST(215);
                PREP_TEST(216);
                PREP_TEST(217);
                PREP_TEST(218);
                PREP_TEST(219);
                PREP_TEST(220);
                PREP_TEST(221);
                PREP_TEST(222);
                PREP_TEST(223);
                PREP_TEST(224);
                PREP_TEST(225);
                PREP_TEST(226);
                PREP_TEST(227);
                PREP_TEST(228);
                PREP_TEST(229);
                PREP_TEST(230);
                PREP_TEST(231);
                PREP_TEST(232);
                PREP_TEST(233);
                PREP_TEST(234);
                PREP_TEST(235);
                PREP_TEST(236);
                PREP_TEST(237);
                PREP_TEST(238);
                PREP_TEST(239);
                PREP_TEST(240);
                PREP_TEST(241);
                PREP_TEST(242);
                PREP_TEST(243);
                PREP_TEST(244);
                PREP_TEST(245);
                PREP_TEST(246);
                PREP_TEST(247);
                PREP_TEST(248);
                PREP_TEST(249);
                PREP_TEST(250);
                PREP_TEST(251);
                PREP_TEST(252);
                PREP_TEST(253);
                PREP_TEST(254);
                PREP_TEST(255);
                PREP_TEST(256);
                PREP_TEST(257);
                PREP_TEST(258);
                PREP_TEST(259);
                PREP_TEST(260);
                PREP_TEST(261);
                PREP_TEST(262);
                PREP_TEST(263);
                PREP_TEST(264);
                PREP_TEST(265);
                PREP_TEST(266);
                PREP_TEST(267);
                PREP_TEST(268);
                PREP_TEST(269);
                PREP_TEST(270);
                PREP_TEST(271);
                PREP_TEST(272);
                PREP_TEST(273);
                PREP_TEST(274);
                PREP_TEST(275);
                PREP_TEST(276);
                PREP_TEST(277);
                PREP_TEST(278);
                PREP_TEST(279);
                PREP_TEST(280);
                PREP_TEST(281);
                PREP_TEST(282);
                PREP_TEST(283);
                PREP_TEST(284);
                PREP_TEST(285);
                PREP_TEST(286);
                PREP_TEST(287);
                PREP_TEST(288);
                PREP_TEST(289);
                PREP_TEST(290);
                PREP_TEST(291);
                PREP_TEST(292);
                PREP_TEST(293);
                PREP_TEST(294);
                PREP_TEST(295);
                PREP_TEST(296);
                PREP_TEST(297);
                PREP_TEST(298);
                PREP_TEST(299);
                PREP_TEST(300);
                PREP_TEST(301);
                PREP_TEST(302);
                PREP_TEST(303);
                PREP_TEST(304);
                PREP_TEST(305);
                PREP_TEST(306);
                PREP_TEST(307);
                PREP_TEST(308);
                PREP_TEST(309);
                PREP_TEST(310);
                PREP_TEST(311);
                PREP_TEST(312);
                PREP_TEST(313);
                PREP_TEST(314);
                PREP_TEST(315);
                PREP_TEST(316);
                PREP_TEST(317);
                PREP_TEST(318);
                PREP_TEST(319);
                PREP_TEST(320);
                PREP_TEST(321);
                PREP_TEST(322);
                PREP_TEST(323);
                PREP_TEST(324);
                PREP_TEST(325);
                PREP_TEST(326);
                PREP_TEST(327);
                PREP_TEST(328);
                PREP_TEST(329);
                PREP_TEST(330);
                PREP_TEST(331);
                PREP_TEST(332);
                PREP_TEST(333);
                PREP_TEST(334);
                PREP_TEST(335);
                PREP_TEST(336);
                PREP_TEST(337);
                PREP_TEST(338);
                PREP_TEST(339);
                PREP_TEST(340);
                PREP_TEST(341);
                PREP_TEST(342);
                PREP_TEST(343);
                PREP_TEST(344);
                PREP_TEST(345);
                PREP_TEST(346);
                PREP_TEST(347);
                PREP_TEST(348);
                PREP_TEST(349);
                PREP_TEST(350);
                PREP_TEST(351);
                PREP_TEST(352);
                PREP_TEST(353);
                PREP_TEST(354);
                PREP_TEST(355);
                PREP_TEST(356);
                PREP_TEST(357);
                PREP_TEST(358);
                PREP_TEST(359);
#if 0
                PREP_TEST(360);
                PREP_TEST(361);
                PREP_TEST(362);
                PREP_TEST(363);
                PREP_TEST(364);
                PREP_TEST(365);
                PREP_TEST(366);
                PREP_TEST(367);
                PREP_TEST(368);
                PREP_TEST(369);
                PREP_TEST(370);
                PREP_TEST(371);
                PREP_TEST(372);
                PREP_TEST(373);
                PREP_TEST(374);
                PREP_TEST(375);
                PREP_TEST(376);
                PREP_TEST(377);
                PREP_TEST(378);
                PREP_TEST(379);
                PREP_TEST(380);
                PREP_TEST(381);
                PREP_TEST(382);
                PREP_TEST(383);
                PREP_TEST(384);
                PREP_TEST(385);
                PREP_TEST(386);
                PREP_TEST(387);
                PREP_TEST(388);
                PREP_TEST(389);
                PREP_TEST(390);
                PREP_TEST(391);
                PREP_TEST(392);
                PREP_TEST(393);
                PREP_TEST(394);
                PREP_TEST(395);
                PREP_TEST(396);
                PREP_TEST(397);
                PREP_TEST(398);
                PREP_TEST(399);
#endif


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
                RUN_TEST(16);
                RUN_TEST(17);
                RUN_TEST(18);
                RUN_TEST(19);
                RUN_TEST(20);
                RUN_TEST(21);
                RUN_TEST(22);
                RUN_TEST(23);
                RUN_TEST(24);
                RUN_TEST(25);
                RUN_TEST(26);
                RUN_TEST(27);
                RUN_TEST(28);
                RUN_TEST(29);
                RUN_TEST(30);
                RUN_TEST(31);
                RUN_TEST(32);
                RUN_TEST(33);
                RUN_TEST(34);
                RUN_TEST(35);
                RUN_TEST(36);
                RUN_TEST(37);
                RUN_TEST(38);
                RUN_TEST(39);
                RUN_TEST(40);
                RUN_TEST(41);
                RUN_TEST(42);
                RUN_TEST(43);
                RUN_TEST(44);
                RUN_TEST(45);
                RUN_TEST(46);
                RUN_TEST(47);
                RUN_TEST(48);
                RUN_TEST(49);
                RUN_TEST(50);
                RUN_TEST(51);
                RUN_TEST(52);
                RUN_TEST(53);
                RUN_TEST(54);
                RUN_TEST(55);
                RUN_TEST(56);
                RUN_TEST(57);
                RUN_TEST(58);
                RUN_TEST(59);
                RUN_TEST(60);
                RUN_TEST(61);
                RUN_TEST(62);
                RUN_TEST(63);
                RUN_TEST(64);
                RUN_TEST(65);
                RUN_TEST(66);
                RUN_TEST(67);
                RUN_TEST(68);
                RUN_TEST(69);
                RUN_TEST(70);
                RUN_TEST(71);
                RUN_TEST(72);
                RUN_TEST(73);
                RUN_TEST(74);
                RUN_TEST(75);
                RUN_TEST(76);
                RUN_TEST(77);
                RUN_TEST(78);
                RUN_TEST(79);
                RUN_TEST(80);
                RUN_TEST(81);
                RUN_TEST(82);
                RUN_TEST(83);
                RUN_TEST(84);
                RUN_TEST(85);
                RUN_TEST(86);
                RUN_TEST(87);
                RUN_TEST(88);
                RUN_TEST(89);
                RUN_TEST(90);
                RUN_TEST(91);
                RUN_TEST(92);
                RUN_TEST(93);
                RUN_TEST(94);
                RUN_TEST(95);
                RUN_TEST(96);
                RUN_TEST(97);
                RUN_TEST(98);
                RUN_TEST(99);
                RUN_TEST(100);
                RUN_TEST(101);
                RUN_TEST(102);
                RUN_TEST(103);
                RUN_TEST(104);
                RUN_TEST(105);
                RUN_TEST(106);
                RUN_TEST(107);
                RUN_TEST(108);
                RUN_TEST(109);
                RUN_TEST(110);
                RUN_TEST(111);
                RUN_TEST(112);
                RUN_TEST(113);
                RUN_TEST(114);
                RUN_TEST(115);
                RUN_TEST(116);
                RUN_TEST(117);
                RUN_TEST(118);
                RUN_TEST(119);
                RUN_TEST(120);
                RUN_TEST(121);
                RUN_TEST(122);
                RUN_TEST(123);
                RUN_TEST(124);
                RUN_TEST(125);
                RUN_TEST(126);
                RUN_TEST(127);
                RUN_TEST(128);
                RUN_TEST(129);
                RUN_TEST(130);
                RUN_TEST(131);
                RUN_TEST(132);
                RUN_TEST(133);
                RUN_TEST(134);
                RUN_TEST(135);
                RUN_TEST(136);
                RUN_TEST(137);
                RUN_TEST(138);
                RUN_TEST(139);
                RUN_TEST(140);
                RUN_TEST(141);
                RUN_TEST(142);
                RUN_TEST(143);
                RUN_TEST(144);
                RUN_TEST(145);
                RUN_TEST(146);
                RUN_TEST(147);
                RUN_TEST(148);
                RUN_TEST(149);
                RUN_TEST(150);
                RUN_TEST(151);
                RUN_TEST(152);
                RUN_TEST(153);
                RUN_TEST(154);
                RUN_TEST(155);
                RUN_TEST(156);
                RUN_TEST(157);
                RUN_TEST(158);
                RUN_TEST(159);
                RUN_TEST(160);
                RUN_TEST(161);
                RUN_TEST(162);
                RUN_TEST(163);
                RUN_TEST(164);
                RUN_TEST(165);
                RUN_TEST(166);
                RUN_TEST(167);
                RUN_TEST(168);
                RUN_TEST(169);
                RUN_TEST(170);
                RUN_TEST(171);
                RUN_TEST(172);
                RUN_TEST(173);
                RUN_TEST(174);
                RUN_TEST(175);
                RUN_TEST(176);
                RUN_TEST(177);
                RUN_TEST(178);
                RUN_TEST(179);
                RUN_TEST(180);
                RUN_TEST(181);
                RUN_TEST(182);
                RUN_TEST(183);
                RUN_TEST(184);
                RUN_TEST(185);
                RUN_TEST(186);
                RUN_TEST(187);
                RUN_TEST(188);
                RUN_TEST(189);
                RUN_TEST(190);
                RUN_TEST(191);
                RUN_TEST(192);
                RUN_TEST(193);
                RUN_TEST(194);
                RUN_TEST(195);
                RUN_TEST(196);
                RUN_TEST(197);
                RUN_TEST(198);
                RUN_TEST(199);
                RUN_TEST(200);
                RUN_TEST(201);
                RUN_TEST(202);
                RUN_TEST(203);
                RUN_TEST(204);
                RUN_TEST(205);
                RUN_TEST(206);
                RUN_TEST(207);
                RUN_TEST(208);
                RUN_TEST(209);
                RUN_TEST(210);
                RUN_TEST(211);
                RUN_TEST(212);
                RUN_TEST(213);
                RUN_TEST(214);
                RUN_TEST(215);
                RUN_TEST(216);
                RUN_TEST(217);
                RUN_TEST(218);
                RUN_TEST(219);
                RUN_TEST(220);
                RUN_TEST(221);
                RUN_TEST(222);
                RUN_TEST(223);
                RUN_TEST(224);
                RUN_TEST(225);
                RUN_TEST(226);
                RUN_TEST(227);
                RUN_TEST(228);
                RUN_TEST(229);
                RUN_TEST(230);
                RUN_TEST(231);
                RUN_TEST(232);
                RUN_TEST(233);
                RUN_TEST(234);
                RUN_TEST(235);
                RUN_TEST(236);
                RUN_TEST(237);
                RUN_TEST(238);
                RUN_TEST(239);
                RUN_TEST(240);
                RUN_TEST(241);
                RUN_TEST(242);
                RUN_TEST(243);
                RUN_TEST(244);
                RUN_TEST(245);
                RUN_TEST(246);
                RUN_TEST(247);
                RUN_TEST(248);
                RUN_TEST(249);
                RUN_TEST(250);
                RUN_TEST(251);
                RUN_TEST(252);
                RUN_TEST(253);
                RUN_TEST(254);
                RUN_TEST(255);
                RUN_TEST(256);
                RUN_TEST(257);
                RUN_TEST(258);
                RUN_TEST(259);
                RUN_TEST(260);
                RUN_TEST(261);
                RUN_TEST(262);
                RUN_TEST(263);
                RUN_TEST(264);
                RUN_TEST(265);
                RUN_TEST(266);
                RUN_TEST(267);
                RUN_TEST(268);
                RUN_TEST(269);
                RUN_TEST(270);
                RUN_TEST(271);
                RUN_TEST(272);
                RUN_TEST(273);
                RUN_TEST(274);
                RUN_TEST(275);
                RUN_TEST(276);
                RUN_TEST(277);
                RUN_TEST(278);
                RUN_TEST(279);
                RUN_TEST(280);
                RUN_TEST(281);
                RUN_TEST(282);
                RUN_TEST(283);
                RUN_TEST(284);
                RUN_TEST(285);
                RUN_TEST(286);
                RUN_TEST(287);
                RUN_TEST(288);
                RUN_TEST(289);
                RUN_TEST(290);
                RUN_TEST(291);
                RUN_TEST(292);
                RUN_TEST(293);
                RUN_TEST(294);
                RUN_TEST(295);
                RUN_TEST(296);
                RUN_TEST(297);
                RUN_TEST(298);
                RUN_TEST(299);
                RUN_TEST(300);
                RUN_TEST(301);
                RUN_TEST(302);
                RUN_TEST(303);
                RUN_TEST(304);
                RUN_TEST(305);
                RUN_TEST(306);
                RUN_TEST(307);
                RUN_TEST(308);
                RUN_TEST(309);
                RUN_TEST(310);
                RUN_TEST(311);
                RUN_TEST(312);
                RUN_TEST(313);
                RUN_TEST(314);
                RUN_TEST(315);
                RUN_TEST(316);
                RUN_TEST(317);
                RUN_TEST(318);
                RUN_TEST(319);
                RUN_TEST(320);
                RUN_TEST(321);
                RUN_TEST(322);
                RUN_TEST(323);
                RUN_TEST(324);
                RUN_TEST(325);
                RUN_TEST(326);
                RUN_TEST(327);
                RUN_TEST(328);
                RUN_TEST(329);
                RUN_TEST(330);
                RUN_TEST(331);
                RUN_TEST(332);
                RUN_TEST(333);
                RUN_TEST(334);
                RUN_TEST(335);
                RUN_TEST(336);
                RUN_TEST(337);
                RUN_TEST(338);
                RUN_TEST(339);
                RUN_TEST(340);
                RUN_TEST(341);
                RUN_TEST(342);
                RUN_TEST(343);
                RUN_TEST(344);
                RUN_TEST(345);
                RUN_TEST(346);
                RUN_TEST(347);
                RUN_TEST(348);
                RUN_TEST(349);
                RUN_TEST(350);
                RUN_TEST(351);
                RUN_TEST(352);
                RUN_TEST(353);
                RUN_TEST(354);
                RUN_TEST(355);
                RUN_TEST(356);
                RUN_TEST(357);
                RUN_TEST(358);
                RUN_TEST(359);
#if 0
                RUN_TEST(360);
                RUN_TEST(361);
                RUN_TEST(362);
                RUN_TEST(363);
                RUN_TEST(364);
                RUN_TEST(365);
                RUN_TEST(366);
                RUN_TEST(367);
                RUN_TEST(368);
                RUN_TEST(369);
                RUN_TEST(370);
                RUN_TEST(371);
                RUN_TEST(372);
                RUN_TEST(373);
                RUN_TEST(374);
                RUN_TEST(375);
                RUN_TEST(376);
                RUN_TEST(377);
                RUN_TEST(378);
                RUN_TEST(379);
                RUN_TEST(380);
                RUN_TEST(381);
                RUN_TEST(382);
                RUN_TEST(383);
                RUN_TEST(384);
                RUN_TEST(385);
                RUN_TEST(386);
                RUN_TEST(387);
                RUN_TEST(388);
                RUN_TEST(389);
                RUN_TEST(390);
                RUN_TEST(391);
                RUN_TEST(392);
                RUN_TEST(393);
                RUN_TEST(394);
                RUN_TEST(395);
                RUN_TEST(396);
                RUN_TEST(397);
                RUN_TEST(398);
                RUN_TEST(399);
#endif
	}
}
