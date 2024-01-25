/**
 * @file bit_manip_syuzuk81.cpp
 * @author ysuzuk81
 * SPDX-FileCopyrightText: © 2023 ysuzuk81 <https: //qiita.com/ysuzuk81/>
 * original code from https: //qiita.com/ysuzuk81/items/453b08a14d23fb8c6c11
 * modified by Gold smith
 */
#include "pch.h"
#include "bit_manip.h"
#include "bit_manip_syuzuk81.h"

uint64_t bit_manip::syuzuk81::delta_swap(uint64_t x, uint64_t mask, int delta) {
	uint64_t t = (x ^ (x >> delta)) & mask;
	return x ^ t ^ (t << delta);
}

__m128i _vectorcall bit_manip::syuzuk81::delta_swap128(__m128i xs, __m128i masks, int delta)
{
	__m256i res = delta_swap256(_mm256_set_m128i(xs, xs), _mm256_set_m128i(masks, masks), delta);
	return _mm256_extracti128_si256(res, 0);
}

__m256i  bit_manip::syuzuk81::delta_swap256(__m256i xs, __m256i masks, int delta)
{
	__m256i ts = _mm256_and_si256(_mm256_xor_si256(_mm256_srli_epi64(xs, delta), xs), masks);
	return _mm256_xor_si256(_mm256_xor_si256(ts, _mm256_slli_epi64(ts, delta)), xs);
}

uint64_t bit_manip::syuzuk81::rotateLeft(uint64_t x, int n)
{
	return (x << n) | (x >> (64 - n));
}

__m128i _vectorcall bit_manip::syuzuk81::rotateLeft128(__m128i xs, int n)
{
	return _mm256_extracti128_si256(rotateLeft256(_mm256_set_m128i(xs, xs), n), 0);
}

__m256i _vectorcall bit_manip::syuzuk81::rotateLeft256(__m256i xs, int n)
{
	return _mm256_or_si256(_mm256_slli_epi64(xs, n), _mm256_srli_epi64(xs, 64 - n));
}

uint64_t bit_manip::syuzuk81::rotateRight(uint64_t x, int n)
{
	return (x >> n) | (x << (64 - n));
}

__m128i _vectorcall bit_manip::syuzuk81::rotateRight128(__m128i xs, int n)
{
	return _mm256_extracti128_si256(rotateRight256(_mm256_set_m128i(xs, xs), n), 0);
}

__m256i _vectorcall bit_manip::syuzuk81::rotateRight256(__m256i xs, int n)
{
	return _mm256_or_si256(_mm256_srli_epi64(xs, n), _mm256_slli_epi64(xs, 64 - n));
}

uint64_t bit_manip::syuzuk81::flipHorizontal(uint64_t x)
{
	x = delta_swap(x, 0x5555555555555555ULL, 1);
	x = delta_swap(x, 0x3333333333333333ULL, 2);
	return delta_swap(x, 0x0F0F0F0F0F0F0F0FULL, 4);
}

__m128i _vectorcall bit_manip::syuzuk81::flipHorizontal128(__m128i x)
{
	return _mm256_extracti128_si256(flipHorizontal256(_mm256_set_m128i(x, x)), 0);
}

__m256i _vectorcall bit_manip::syuzuk81::flipHorizontal256(__m256i x)
{
	x = delta_swap256(x, _mm256_set1_epi64x(0x5555555555555555ULL), 1);
	x = delta_swap256(x, _mm256_set1_epi64x(0x3333333333333333ULL), 2);
	return delta_swap256(x, _mm256_set1_epi64x(0x0F0F0F0F0F0F0F0FULL), 4);
}

uint64_t bit_manip::syuzuk81::flipVertical(uint64_t x)
{
	x = delta_swap(x, 0x00FF00FF00FF00FFULL, 8);
	x = delta_swap(x, 0x0000FFFF0000FFFFULL, 16);
	return delta_swap(x, 0x00000000FFFFFFFFULL, 32);
}

__m128i _vectorcall bit_manip::syuzuk81::flipVertical128(__m128i x)
{
	return _mm256_extracti128_si256(flipVertical256(_mm256_set_m128i(x, x)), 0);
}

__m256i _vectorcall bit_manip::syuzuk81::flipVertical256(__m256i x)
{
	x = delta_swap256(x, _mm256_set1_epi64x(0x00FF00FF00FF00FFULL), 8);
	x = delta_swap256(x, _mm256_set1_epi64x(0x0000FFFF0000FFFFULL), 16);
	return delta_swap256(x, _mm256_set1_epi64x(0x00000000FFFFFFFFULL), 32);
}

uint64_t bit_manip::syuzuk81::flipDiagonalA1H8(uint64_t x)
{
	x = delta_swap(x, 0x00AA00AA00AA00AAULL, 7);
	x = delta_swap(x, 0x0000CCCC0000CCCCULL, 14);
	return delta_swap(x, 0x00000000F0F0F0F0ULL, 28);
}

__m128i _vectorcall bit_manip::syuzuk81::flipDiagonalA1H8128(__m128i x)
{
	return _mm256_extracti128_si256(flipDiagonalA1H8256(_mm256_set_m128i(x, x)), 0);
}

__m256i _vectorcall bit_manip::syuzuk81::flipDiagonalA1H8256(__m256i x)
{
	x = delta_swap256(x, _mm256_set1_epi64x(0x00AA00AA00AA00AAULL), 7);
	x = delta_swap256(x, _mm256_set1_epi64x(0x0000CCCC0000CCCCULL), 14);
	return delta_swap256(x, _mm256_set1_epi64x(0x00000000F0F0F0F0ULL), 28);
}

uint64_t bit_manip::syuzuk81::flipDiagonalA8H1(uint64_t x)
{
	x = delta_swap(x, 0x0055005500550055ULL, 9);
	x = delta_swap(x, 0x0000333300003333ULL, 18);
	return delta_swap(x, 0x000000000F0F0F0FULL, 36);
}

__m128i _vectorcall bit_manip::syuzuk81::flipDiagonalA8H1128(__m128i x)
{
	return _mm256_extracti128_si256(flipDiagonalA8H1256(_mm256_set_m128i(x, x)), 0);
}

__m256i _vectorcall bit_manip::syuzuk81::flipDiagonalA8H1256(__m256i x)
{
	x = delta_swap256(x, _mm256_set1_epi64x(0x0055005500550055ULL), 9);
	x = delta_swap256(x, _mm256_set1_epi64x(0x0000333300003333ULL), 18);
	return delta_swap256(x, _mm256_set1_epi64x(0x000000000F0F0F0FULL), 36);
}

uint64_t bit_manip::syuzuk81::rotateC90(uint64_t x)
{
	return flipHorizontal(flipDiagonalA1H8(x));
}

__m128i _vectorcall bit_manip::syuzuk81::rotateC90128(__m128i x)
{
	return _mm256_extracti128_si256(rotateC90256(_mm256_set_m128i(x, x)), 0);
}

__m256i _vectorcall bit_manip::syuzuk81::rotateC90256(__m256i x)
{
	return flipHorizontal256(flipDiagonalA1H8256(x));
}

uint64_t bit_manip::syuzuk81::rotateAC90(uint64_t x)
{
	return flipVertical(flipDiagonalA1H8(x));
}

__m128i _vectorcall bit_manip::syuzuk81::rotateAC90128(__m128i x)
{
	return flipVertical128(flipDiagonalA1H8128(x));
}

__m256i _vectorcall bit_manip::syuzuk81::rotateAC90256(__m256i x)
{
	return flipVertical256(flipDiagonalA1H8256(x));
}

uint64_t bit_manip::syuzuk81::rotate180(uint64_t x)
{
	return flipVertical(flipHorizontal(x));
}

__m128i _vectorcall bit_manip::syuzuk81::rotate180128(__m128i x)
{
	return _mm256_extracti128_si256(rotate180256(_mm256_set_m128i(x, x)), 0);
}

__m256i _vectorcall bit_manip::syuzuk81::rotate180256(__m256i x)
{
	return flipVertical256(flipHorizontal256(x));
}

uint64_t bit_manip::syuzuk81::rotateC45U(uint64_t x)
{
	x ^= 0xAAAAAAAAAAAAAAAAULL & (x ^ rotateLeft(x, 8));
	x ^= 0xCCCCCCCCCCCCCCCCULL & (x ^ rotateLeft(x, 16));
	return (x ^ 0xF0F0F0F0F0F0F0F0ULL & (x ^ rotateLeft(x, 32))) & mask_CW45U;
}

__m128i _vectorcall bit_manip::syuzuk81::rotateC45U128(__m128i x)
{
	return _mm256_extracti128_si256(rotateC45U256(_mm256_set_m128i(x, x)), 0);
}

__m256i _vectorcall bit_manip::syuzuk81::rotateC45U256(__m256i x)
{
	x = _mm256_xor_si256(x, _mm256_and_si256(_mm256_set1_epi64x(0xAAAAAAAAAAAAAAAAULL)
		, _mm256_xor_si256(x, rotateLeft256(x, 8))));
	x = _mm256_xor_si256(x, _mm256_and_si256(_mm256_set1_epi64x(0xCCCCCCCCCCCCCCCCULL)
		, _mm256_xor_si256(x, rotateLeft256(x, 16))));
	return _mm256_and_si256(_mm256_xor_si256(x, _mm256_and_si256(_mm256_set1_epi64x(0xF0F0F0F0F0F0F0F0ULL)
		, _mm256_xor_si256(x, rotateLeft256(x, 32)))), _mm256_set1_epi64x(mask_CW45U));
}

uint64_t bit_manip::syuzuk81::rotateC45L(uint64_t x)
{
	x ^= 0x5555555555555555ULL & (x ^ rotateRight(x, 8));
	x ^= 0x3333333333333333ULL & (x ^ rotateRight(x, 16));
	return (x ^ 0x0F0F0F0F0F0F0F0FULL & (x ^ rotateRight(x, 32))) & mask_CW45L;
}

__m128i _vectorcall bit_manip::syuzuk81::rotateC45L128(__m128i x)
{
	return _mm256_extracti128_si256(rotateC45L256(_mm256_set_m128i(x, x)), 0);
}

__m256i _vectorcall bit_manip::syuzuk81::rotateC45L256(__m256i x)
{
	x = _mm256_xor_si256(x, _mm256_and_si256(_mm256_set1_epi64x(0x5555555555555555ULL)
		, _mm256_xor_si256(x, rotateRight256(x, 8))));
	x = _mm256_xor_si256(x, _mm256_and_si256(_mm256_set1_epi64x(0x3333333333333333ULL)
		, _mm256_xor_si256(x, rotateRight256(x, 16))));
	return _mm256_and_si256(_mm256_xor_si256(x, _mm256_and_si256(_mm256_set1_epi64x(0x0F0F0F0F0F0F0F0FULL)
		, _mm256_xor_si256(x, rotateRight256(x, 32)))), _mm256_set1_epi64x(mask_CW45L));
}

__m256i _vectorcall bit_manip::syuzuk81::rotateC45UULL256(__m256i xo)
{
	return _mm256_set_m128i(_mm256_extracti128_si256(rotateC45L256(xo), 1), _mm256_extracti128_si256(rotateC45U256(xo), 0));
}

uint64_t bit_manip::syuzuk81::rotateAC45U(uint64_t x)
{
	x ^= 0x5555555555555555ULL & (x ^ rotateLeft(x, 8));
	x ^= 0x3333333333333333ULL & (x ^ rotateLeft(x, 16));
	return (x ^ 0x0F0F0F0F0F0F0F0FULL & (x ^ rotateLeft(x, 32))) & mask_ACW45U;
}

__m128i _vectorcall bit_manip::syuzuk81::rotateAC45U128(__m128i x)
{
	return _mm256_extracti128_si256(rotateAC45U256(_mm256_set_m128i(x, x)), 0);
}

__m256i _vectorcall bit_manip::syuzuk81::rotateAC45U256(__m256i x)
{
	x = _mm256_xor_si256(x, _mm256_and_si256(_mm256_set1_epi64x(0x5555555555555555ULL)
		, _mm256_xor_si256(x, rotateLeft256(x, 8))));
	x = _mm256_xor_si256(x, _mm256_and_si256(_mm256_set1_epi64x(0x3333333333333333ULL)
		, _mm256_xor_si256(x, rotateLeft256(x, 16))));
	return _mm256_and_si256(_mm256_xor_si256(x, _mm256_and_si256(_mm256_set1_epi64x(0x0F0F0F0F0F0F0F0FULL)
		, _mm256_xor_si256(x, rotateLeft256(x, 32)))), _mm256_set1_epi64x(mask_ACW45U));
}

uint64_t bit_manip::syuzuk81::rotateAC45L(uint64_t x)
{
	x ^= 0xAAAAAAAAAAAAAAAAULL & (x ^ rotateRight(x, 8));
	x ^= 0xCCCCCCCCCCCCCCCCULL & (x ^ rotateRight(x, 16));
	return (x ^ 0xF0F0F0F0F0F0F0F0ULL & (x ^ rotateRight(x, 32))) & mask_ACW45L;
}

__m256i _vectorcall bit_manip::syuzuk81::rotateAC45L256(__m256i x)
{
	x = _mm256_xor_si256(x, _mm256_and_si256(_mm256_set1_epi64x(0xAAAAAAAAAAAAAAAAULL)
		, _mm256_xor_si256(x, rotateRight256(x, 8))));
	x = _mm256_xor_si256(x, _mm256_and_si256(_mm256_set1_epi64x(0xCCCCCCCCCCCCCCCCULL)
		, _mm256_xor_si256(x, rotateRight256(x, 16))));
	return _mm256_and_si256(_mm256_xor_si256(x, _mm256_and_si256(_mm256_set1_epi64x(0xF0F0F0F0F0F0F0F0ULL)
		, _mm256_xor_si256(x, rotateRight256(x, 32)))), _mm256_set1_epi64x(mask_ACW45L));
}

__m128i _vectorcall bit_manip::syuzuk81::rotateAC45L128(__m128i x)
{
	__m256i LUUL = rotateAC45UULL256(_mm256_set_m128i(x, x));
	return _mm256_extracti128_si256(_mm256_setr_epi64x(_mm256_extract_epi64(LUUL, 0), _mm256_extract_epi64(LUUL, 3), 0ULL, 0ULL), 0);
}

__m256i _vectorcall bit_manip::syuzuk81::rotateAC45UULL256(__m256i xo)
{
	// LUとULを結合して返す。
	return _mm256_set_m128i(_mm256_extracti128_si256(rotateAC45L256(xo), 1), _mm256_extracti128_si256(rotateAC45U256(xo), 0));
}

uint8_t bit_manip::syuzuk81::mobilityL8(uint8_t p, uint8_t o)
{
	uint8_t p1 = p << 1;
	uint8_t mask = ~(p | o);
	return /*~(p1 | o) &*/ (p1 + o) & mask;
}

uint8_t bit_manip::syuzuk81::mobilityR8(uint8_t p, uint8_t o)
{
	uint8_t  p1{}, p2{}, p3{}, o1{}, o2{}, o3{}, mask{}, mask2{}, rm{}, t{};
	mask = ~(p | o);
	p1 = ((p | mask) - o);
	p2 = p1 & o;
	p3 = p2 >> 1;

	rm = p3 & (~mask);
	return rm;

}

uint64_t bit_manip::syuzuk81::mobility_line64(uint64_t p, uint64_t o)
{
	uint64_t p1 = (0x7F7F7F7F7F7F7F7FULL & p) << 1;
	return ~(p1 | o) & (p1 + o);
}

__m256i _vectorcall bit_manip::syuzuk81::mobility_line256(__m256i p, __m256i o)
{
	__m256i mask = _mm256_set1_epi64x(0x7F7F7F7F7F7F7F7FULL);
	__m256i p1 = _mm256_slli_epi64(_mm256_and_si256(p, mask), 1);
	return _mm256_andnot_si256(_mm256_or_si256(p1, o), _mm256_add_epi64(p1, o));
}

uint64_t bit_manip::syuzuk81::mobility_BB64(uint64_t p, uint64_t o)
{
	// 時計短針0時、2時30分:、･･･、10時30分と探していく。見つけたい方向を左にする。
	uint64_t pos_result(0ULL);
	uint64_t pACW90 = rotateAC90(p);
	uint64_t oACW90 = rotateAC90(o);
	uint64_t p180 = rotate180(p);
	uint64_t o180 = rotate180(o);
	pos_result = rotateC90(mobility_line64(pACW90, oACW90));
	pos_result |= rotateC90(rotateC45L(mobility_line64(rotateAC45U(pACW90), rotateAC45U(oACW90))));
	pos_result |= rotateC90(rotateC45U(mobility_line64(rotateAC45L(pACW90), rotateAC45L(oACW90))));
	pos_result |= rotate180(mobility_line64(p180, o180));
	pos_result |= rotate180(rotateC45L(mobility_line64(rotateAC45U(p180), rotateAC45U(o180))));
	pos_result |= rotate180(rotateC45U(mobility_line64(rotateAC45L(p180), rotateAC45L(o180))));
	pos_result |= rotateAC90(mobility_line64(rotateC90(p), rotateC90(o)));
	pos_result |= rotateAC45L(mobility_line64(rotateC45U(p), rotateC45U(o)));
	pos_result |= rotateAC45U(mobility_line64(rotateC45L(p), rotateC45L(o)));
	pos_result |= mobility_line64(p, o);
	pos_result |= rotateC45L(mobility_line64(rotateAC45U(p), rotateAC45U(o)));
	pos_result |= rotateC45U(mobility_line64(rotateAC45L(p), rotateAC45L(o)));
	return pos_result;
}

uint64_t _vectorcall bit_manip::syuzuk81::mobility_BB128(__m128i po)
{
	using namespace debug_fnc;
	__m256i p0o0p0o0 = _mm256_setr_m128i(po, po);
	uint64_t result = {};
	__m256i pAC90oAC90pAC90oAC90 = rotateAC90256(p0o0p0o0);
	__m256i pAC135UoAC135UpAC135LoAC135L = rotateAC45UULL256(pAC90oAC90pAC90oAC90);
	__m256i pAC45UoAC45UpAC45LoAC45L = rotateAC45UULL256(p0o0p0o0);

	__m256i pAC45UpAC135UpAC45LpAC135L = _mm256_unpacklo_epi64(pAC45UoAC45UpAC45LoAC45L, pAC135UoAC135UpAC135LoAC135L);
	__m256i oAC45UoAC135UoAC45LoAC135L = _mm256_unpackhi_epi64(pAC45UoAC45UpAC45LoAC45L, pAC135UoAC135UpAC135LoAC135L);
	__m256i rAC45UrAC135UrAC45LrAC135L = mobility_line256(pAC45UpAC135UpAC45LpAC135L, oAC45UoAC135UoAC45LoAC135L);
	__m256i rAC45LrAC135LrAC45UrAC135U = _mm256_permute4x64_epi64(rAC45UrAC135UrAC45LrAC135L, mm_shuffle_1_0_3_2);
	__m256i r0rAC90r0rAC90 = rotateC45UULL256(rAC45LrAC135LrAC45UrAC135U);

	__m256i pC90oC90pC90oC90 = rotateC90256(p0o0p0o0);
	__m256i pC135UoC135UpC135LoC135L = rotateC45UULL256(pC90oC90pC90oC90);
	__m256i pC45UoC45UpC45LoC45L = rotateC45UULL256(p0o0p0o0);
	__m256i pC45UpC135UpC45LpC135L = _mm256_unpacklo_epi64(pC45UoC45UpC45LoC45L, pC135UoC135UpC135LoC135L);
	__m256i oC45UoC135UoC45LoC135L = _mm256_unpackhi_epi64(pC45UoC45UpC45LoC45L, pC135UoC135UpC135LoC135L);
	__m256i rC45UrC135UrC45LrC135L = mobility_line256(pC45UpC135UpC45LpC135L, oC45UoC135UoC45LoC135L);
	__m256i rC45LrC135LrC45UrC135U = _mm256_permute4x64_epi64(rC45UrC135UrC45LrC135L, mm_shuffle_1_0_3_2);
	__m256i r0rC90r0rC90 = rotateAC45UULL256(rC45LrC135LrC45UrC135U);

	__m256i p180o180p180o180 = rotate180256(p0o0p0o0);
	__m256i p0o0p180o180 = _mm256_set_m128i(_mm256_extracti128_si256(p180o180p180o180, 0), _mm256_extracti128_si256(p0o0p0o0, 0));
	__m256i pC90oC90pAC90oAC90 = _mm256_set_m128i(_mm256_extracti128_si256(pAC90oAC90pAC90oAC90, 0), _mm256_extracti128_si256(pC90oC90pC90oC90, 0));
	__m256i p0pC90p180pAC90 = _mm256_unpacklo_epi64(p0o0p180o180, pC90oC90pAC90oAC90);
	__m256i o0oC90o180oAC90 = _mm256_unpackhi_epi64(p0o0p180o180, pC90oC90pAC90oAC90);
	__m256i r0rC90r180rAC90 = mobility_line256(p0pC90p180pAC90, o0oC90o180oAC90);

	__m256i r0r0r0r0 = _mm256_unpacklo_epi64(r0rAC90r0rAC90, r0rC90r0rC90);
	for (int i(0); i < 3; ++i) {
		result |= r0r0r0r0.m256i_u64[i];
	}

	__m256i rAC90rAC90rAC90_0 = _mm256_setr_epi64x(r0rAC90r0rAC90.m256i_u64[1], r0rAC90r0rAC90.m256i_u64[3], r0rC90r180rAC90.m256i_u64[3], 0);
	__m256i r0r0r0_0 = rotateC90256(rAC90rAC90rAC90_0);
	for (int i(0); i <= 2; ++i) {
		result |= r0r0r0_0.m256i_u64[i];
	}

	__m256i rC90rC90rC90_0 = _mm256_setr_epi64x(r0rC90r0rC90.m256i_u64[1], r0rC90r0rC90.m256i_u64[3], r0rC90r180rAC90.m256i_u64[1], 0);
	r0r0r0_0 = rotateAC90256(rC90rC90rC90_0);
	for (int i(0); i <= 3; ++i) {
		result |= r0r0r0_0.m256i_u64[i];
	}

	result |= rotate180(r0rC90r180rAC90.m256i_u64[2]);
	result |= r0rC90r180rAC90.m256i_u64[0];
	return result;
}

uint16_t bit_manip::syuzuk81::flipDiscsLine8(uint8_t p, uint8_t o, uint8_t m)
{
	__m256i zero_vec = _mm256_setzero_si256();
	uint8_t m1 = m << 1;
	uint8_t o1 = o + m1;
	uint8_t p1 = p & o1;
	uint8_t o3 = p1 - m1;
	uint8_t o4 = o & ~o3;
	_DOB(o4);
	__m256i cmp_result = _mm256_cmpeq_epi64(_mm256_set1_epi8(p1), zero_vec);
	_DOB(cmp_result);
	// aが0でない場合、マスクは全ビットが1になります
	__m256i mask = _mm256_andnot_si256(cmp_result, _mm256_set1_epi64x(-1));
	_DOB(mask);
	uint8_t p3 = p | o3 | m;
	_DOB(p3);
	// aが0以外ならbにcを、それ以外ならbにdを入れます
	__m256i o5 = _mm256_blendv_epi8(_mm256_set1_epi64x(o), _mm256_set1_epi64x(o4), mask);
	_DOB(o5);
	__m256i p4 = _mm256_blendv_epi8(_mm256_set1_epi64x(p), _mm256_set1_epi64x(p3), mask);
	_DOB(p4);
	uint16_t result = p4.m256i_u8[0] << 8 | o5.m256i_u8[0];
	_DOB(result);
	return result;
}

uint64_t _vectorcall bit_manip::syuzuk81::flipDiscsLine64(uint64_t p, uint64_t o, uint64_t m)
{
	uint64_t m1 = m << 1;
	uint64_t omask = o & 0x7F7F7F7F7F7F7F7F;
	uint64_t p1 = omask + m1;
	uint64_t p2 = p & p1;
	if (!p2) {
		return p;
	}
	uint64_t p3 = p2 - m1;
	uint64_t o3 = o & ~p3;
	uint64_t p4 = p | p3 | m;
	return p4;
}

void _vectorcall bit_manip::syuzuk81::flipDiscs2Line(__m128i p0p1, __m128i o0o1, __m128i m0m1, __m128i* p_rp0rp1, __m128i* p_ro0ro1)
{
	__m128i m01m11 = _mm_slli_epi64(m0m1, 1);
	__m128i masko0o1 = _mm_and_si128(o0o1, _mm_set1_epi64x(0x7F7F7F7F7F7F7F7F));
	__m128i p01p11 = _mm_add_epi64(masko0o1, m01m11);
	__m128i p02p12 = _mm_and_si128(p0p1, p01p11);
	__m128i p03p13 = _mm_sub_epi64(p02p12, m01m11);
	__m128i o03o13 = _mm_andnot_si128(p03p13, o0o1);
	__m128i p04p14 = _mm_or_si128(p0p1, p03p13);
	__m128i p05p15 = _mm_or_si128(p04p14, m0m1);

	__m128i zero_vec = _mm_setzero_si128();
	__m128i cmp_result = _mm_cmpeq_epi64(p02p12, zero_vec);
	// p02p12が0でない場合、マスクは全ビットが1になります
	__m128i mask = _mm_andnot_si128(cmp_result, _mm_set1_epi64x(-1));
	__m128i p06p16 = _mm_blendv_epi8(p0p1, p05p15, mask);
	__m128i o04o14 = _mm_blendv_epi8(o0o1, o03o13, mask);
	*p_rp0rp1 = p06p16;
	*p_ro0ro1 = o04o14;
	return;
}

void _vectorcall bit_manip::syuzuk81::flipDiscs4Lines(__m256i p0p1p2p3, __m256i o0o1o2o3, __m256i m0m1m2m3, __m256i* p_rp0rp1rp2rp3, __m256i* p_ro0ro1ro2ro3)
{
	__m256i m01m11 = _mm256_slli_epi64(m0m1m2m3, 1);
	__m256i masko0o1o2o3 = _mm256_and_si256(o0o1o2o3, _mm256_set1_epi64x(0x7F7F7F7F7F7F7F7F));
	__m256i p01p11p21p31 = _mm256_add_epi64(masko0o1o2o3, m01m11);
	__m256i p02p12p22p32 = _mm256_and_si256(p0p1p2p3, p01p11p21p31);
	__m256i p03p13p23p33 = _mm256_sub_epi64(p02p12p22p32, m01m11);
	__m256i o03o13o23o33 = _mm256_andnot_si256(p03p13p23p33, o0o1o2o3);
	__m256i p04p14p24p34 = _mm256_or_si256(p0p1p2p3, p03p13p23p33);
	__m256i p05p15p25p35 = _mm256_or_si256(p04p14p24p34, m0m1m2m3);

	__m256i zero_vec = _mm256_setzero_si256();
	__m256i cmp_result = _mm256_cmpeq_epi64(p02p12p22p32, zero_vec);
	// p02p12が0でない場合、マスクは全ビットが1になります
	__m256i mask = _mm256_andnot_si256(cmp_result, _mm256_set1_epi64x(-1));
	__m256i p06p16p26p36 = _mm256_blendv_epi8(p0p1p2p3, p05p15p25p35, mask);
	__m256i o04o14o24o34 = _mm256_blendv_epi8(o0o1o2o3, o03o13o23o33, mask);
	*p_rp0rp1rp2rp3 = p06p16p26p36;
	*p_ro0ro1ro2ro3 = o04o14o24o34;
	return;
}

void bit_manip::syuzuk81::flipDiscsBB(uint64_t p, uint64_t o, uint64_t m, uint64_t* pro, uint64_t* prp)
{
	uint64_t pAC90 = rotateAC90(p);
	uint64_t oAC90 = rotateAC90(o);
	uint64_t mAC90 = rotateAC90(m);
	uint64_t p180 = rotate180(p);
	uint64_t o180 = rotate180(o);
	uint64_t m180 = rotate180(m);
	uint64_t rp{ p }, ro{ o };
	uint64_t flipped;
	flipped = rotateC90(flipDiscsLine64(pAC90, oAC90, mAC90));
	flipped |= rotateC90(rotateC45L(flipDiscsLine64(rotateAC45U(pAC90), rotateAC45U(oAC90), rotateAC45U(mAC90))));
	flipped |= rotateC90(rotateC45U(flipDiscsLine64(rotateAC45L(pAC90), rotateAC45L(oAC90), rotateAC45L(mAC90))));
	flipped |= rotate180(flipDiscsLine64(p180, o180, m180));
	flipped |= rotate180(rotateC45L(flipDiscsLine64(rotateAC45U(p180), rotateAC45U(o180), rotateAC45U(m180))));
	flipped |= rotate180(rotateC45U(flipDiscsLine64(rotateAC45L(p180), rotateAC45L(o180), rotateAC45L(m180))));
	flipped |= rotateAC90(flipDiscsLine64(rotateC90(p), rotateC90(o), rotateC90(m)));
	flipped |= rotateAC45L(flipDiscsLine64(rotateC45U(p), rotateC45U(o), rotateC45U(m)));
	flipped |= rotateAC45U(flipDiscsLine64(rotateC45L(p), rotateC45L(o), rotateC45L(m)));
	flipped |= flipDiscsLine64(p, o, m);
	flipped |= rotateC45L(flipDiscsLine64(rotateAC45U(p), rotateAC45U(o), rotateAC45U(m)));
	rp |= flipped;
	ro = o & ~flipped;
	*pro = rp;
	*prp = ro;
	return;
}
