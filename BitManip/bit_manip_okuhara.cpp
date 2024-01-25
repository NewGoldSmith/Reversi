/**
 * @file bit_manip_okuhara.cpp
 *  SPDX-FileCopyrightText: © 2020 奥原 俊彦 <okuhara＠amy.hi-ho.ne.jp>
 *  original code from http: //www.amy.hi-ho.ne.jp/okuhara/bitboard.htm
 *  modified by Gold Smith
 */
#include "pch.h"
#include "bit_manip_okuhara.h"

__m256i __vectorcall bit_manip::okuhara::get_moves256(const __m256i poms)
{
	__m256i	PP, mOO, MM, flip_l, flip_r, pre_l, pre_r, shift1897;
	__m128i	M;
	shift1897 = _mm256_set_epi64x(7, 9, 8, 1);
	__m256i mflipH = _mm256_set_epi64x(0x7e7e7e7e7e7e7e7e, 0x7e7e7e7e7e7e7e7e, -1, 0x7e7e7e7e7e7e7e7e);

	PP = _mm256_broadcastq_epi64(_mm_cvtsi64_si128(poms.m256i_u64[0]));
	mOO = _mm256_and_si256(_mm256_broadcastq_epi64(_mm_cvtsi64_si128(poms.m256i_u64[1])), mflipH);

	flip_l = _mm256_and_si256(mOO, _mm256_sllv_epi64(PP, shift1897));
	flip_r = _mm256_and_si256(mOO, _mm256_srlv_epi64(PP, shift1897));
	flip_l = _mm256_or_si256(flip_l, _mm256_and_si256(mOO, _mm256_sllv_epi64(flip_l, shift1897)));
	flip_r = _mm256_or_si256(flip_r, _mm256_and_si256(mOO, _mm256_srlv_epi64(flip_r, shift1897)));
	pre_l = _mm256_and_si256(mOO, _mm256_sllv_epi64(mOO, shift1897));
	pre_r = _mm256_srlv_epi64(pre_l, shift1897);
	__m256i shiftx2 = _mm256_add_epi64(shift1897, shift1897);
	flip_l = _mm256_or_si256(flip_l, _mm256_and_si256(pre_l, _mm256_sllv_epi64(flip_l, shiftx2)));
	flip_r = _mm256_or_si256(flip_r, _mm256_and_si256(pre_r, _mm256_srlv_epi64(flip_r, shiftx2)));
	flip_l = _mm256_or_si256(flip_l, _mm256_and_si256(pre_l, _mm256_sllv_epi64(flip_l, shiftx2)));
	flip_r = _mm256_or_si256(flip_r, _mm256_and_si256(pre_r, _mm256_srlv_epi64(flip_r, shiftx2)));
	MM = _mm256_sllv_epi64(flip_l, shift1897);
	MM = _mm256_or_si256(MM, _mm256_srlv_epi64(flip_r, shift1897));

	M = _mm_or_si128(_mm256_castsi256_si128(MM), _mm256_extracti128_si256(MM, 1));
	M = _mm_or_si128(M, _mm_unpackhi_epi64(M, M));
	uint64_t MoveResult = _mm_cvtsi128_si64(M) & ~(poms.m256i_u64[0] | poms.m256i_u64[1]);
	return _mm256_insert_epi64(poms, MoveResult, 2);
}

__m256i __vectorcall bit_manip::okuhara::flip256(const __m256i poms)
{
	const __m256i shift1897 = _mm256_set_epi64x(7, 9, 8, 1);
	// masked opponent 
	const __m256i mOO = _mm256_and_si256(_mm256_permute4x64_epi64(poms, _MM_SHUFFLE(1, 1, 1, 1)),
		_mm256_set_epi64x(0x007e7e7e7e7e7e00, 0x007e7e7e7e7e7e00, 0x00ffffffffffff00, 0x7e7e7e7e7e7e7e7e));	// (sentinel on the edge)
	const __m256i shift1897x2 = _mm256_add_epi64(shift1897, shift1897);
	const __m256i XX = _mm256_set1_epi64x(poms.m256i_u64[2]);

	// 左上方向
//	flip = (X << shift) & mO;
	__m256i flipLU256 = _mm256_and_si256(_mm256_sllv_epi64(XX, shift1897), mOO);
	//flip |= (flip << shift) & mO;// 0 0 0 0 0 F&G G 0
	flipLU256 = _mm256_or_si256(flipLU256, _mm256_and_si256(_mm256_sllv_epi64(flipLU256, shift1897), mOO));
	// pre = mO & (mO << shift);// A&B B&C C&D D&E E&F F&G G&H 0
 	 __m256i preLU256 = _mm256_and_si256(mOO, _mm256_sllv_epi64(mOO, shift1897));
	//flip |= (flip << shift2) & pre;
	flipLU256 = _mm256_or_si256(flipLU256, _mm256_and_si256(_mm256_sllv_epi64(flipLU256, shift1897x2), preLU256));
	//flip |= (flip << shift2) & pre; 0 0 0 D&E&F&G E&F&G F&G G 0
	flipLU256 = _mm256_or_si256(flipLU256, _mm256_and_si256(_mm256_sllv_epi64(flipLU256, shift1897x2), preLU256));
	//outflank = p & (flip << shift);
	__m256i PP = _mm256_permute4x64_epi64(poms, _MM_SHUFFLE(0, 0, 0, 0));
	__m256i outflankLU256 = _mm256_and_si256(PP, _mm256_sllv_epi64(flipLU256, shift1897));
	//flip &= -(int)(outflank != 0);
	// Create a maskLU where outflank is not zero
	__m256i maskLU = _mm256_cmpeq_epi64(outflankLU256, _mm256_setzero_si256());
	// Invert the maskLU
	maskLU = _mm256_xor_si256(maskLU, _mm256_set1_epi64x(-1));
	// Perform bitwise AND on flip and maskLU
	flipLU256 = _mm256_and_si256(flipLU256, maskLU);
	//uint64_t rp = p | X | flip;
	__m256i rPPLU = _mm256_or_si256(PP, _mm256_or_si256(XX, flipLU256));

	// 右下方向
	__m256i flipRD256 = _mm256_and_si256(_mm256_srlv_epi64(XX, shift1897), mOO);
	//flip |= (flip << shift) & mO;// 0 0 0 0 0 F&G G 0
	flipRD256 = _mm256_or_si256(flipRD256, _mm256_and_si256(_mm256_srlv_epi64(flipRD256, shift1897), mOO));
	// pre = mO & (mO << shift);// A&B B&C C&D D&E E&F F&G G&H 0
	__m256i preRD256 = _mm256_and_si256(mOO, _mm256_srlv_epi64(mOO, shift1897));
	//flip |= (flip << shift2) & pre;
	flipRD256 = _mm256_or_si256(flipRD256, _mm256_and_si256(_mm256_srlv_epi64(flipRD256, shift1897x2), preRD256));
	//flip |= (flip << shift2) & pre; 0 0 0 D&E&F&G E&F&G F&G G 0
	flipRD256 = _mm256_or_si256(flipRD256, _mm256_and_si256(_mm256_srlv_epi64(flipRD256, shift1897x2), preRD256));
	//outflank = p & (flip << shift);
	__m256i outflankRD256 = _mm256_and_si256(PP, _mm256_srlv_epi64(flipRD256, shift1897));
	//flip &= -(int)(outflank != 0);
	// Create a mask where outflank is not zero
	__m256i maskRD = _mm256_cmpeq_epi64(outflankRD256, _mm256_setzero_si256());
	// Invert the mask
	maskRD = _mm256_xor_si256(maskRD, _mm256_set1_epi64x(-1));
	// Perform bitwise AND on flip and maskLU
	flipRD256 = _mm256_and_si256(flipRD256, maskRD);
	//uint64_t rp = p | X | flip;
	__m256i rPPRD = _mm256_or_si256(PP, _mm256_or_si256(XX, flipRD256));

	// すべてのプレーヤーボードのorを取る。
	__m256i PPLURD = _mm256_or_si256(rPPLU, rPPRD);
	__m128i PPLURD2 = _mm_or_si128(_mm256_extracti128_si256(PPLURD,1), _mm256_extracti128_si256(PPLURD, 0));
	//uint64_t rp = PPLURD2.m128i_u64[0] | PPLURD2.m128i_u64[1] ;
	__m128i rp128 = _mm_or_si128(PPLURD2, _mm_shuffle_epi32(PPLURD2, _MM_SHUFFLE(1, 0, 3, 2)));
	//uint64_t ro = ~rp & o;
	__m128i ro128 = _mm_andnot_si128(rp128, _mm256_extractf128_si256(poms, 0));
	return _mm256_inserti128_si256(poms, _mm_set_epi64x(ro128.m128i_u64[1], rp128.m128i_u64[0]),0);
}


uint64_t bit_manip::okuhara::flip_shift_line(uint64_t p, uint64_t o, uint64_t m)
{
	uint64_t mO = o & 0x7e7e7e7e7e7e7e7e;	// except for vertical
	_DOB(mO);
	uint64_t flip = (m << 1) & mO;		// 0 0 0 0 0 0 G 0
	_DOB(flip);
	flip |= (flip << 1) & mO;	// 0 0 0 0 0 F&G G 0
	_DOB(flip);
	flip |= (flip << 1) & mO;	// 0 0 0 0 E&F&G F&G G 0
	_DOB(flip);
	flip |= (flip << 1) & mO;
	_DOB(flip);
	flip |= (flip << 1) & mO;
	_DOB(flip);
	flip |= (flip << 1) & mO;
	_DOB(flip);
	flip |= (flip << 1) & mO;	// 0 B&C&D&E&F&G .. F&G G 0
	_DOB(flip);
	uint64_t outflank = p & (flip << 1);
	_DOB(outflank);
	if (outflank == 0)
		flip = 0;
	return flip;
}

uint64_t bit_manip::okuhara::flip_para_line(uint64_t p, uint64_t o, uint64_t X)
{
	uint64_t mO = o & 0x7e7e7e7e7e7e7e7e;
	_DOB(mO);
	uint64_t flip = (X << 1) & mO;		// 0 0 0 0 0 0 G 0
	_DOB(flip);
	flip |= (flip << 1) & mO;	// 0 0 0 0 0 F&G G 0
	_DOB(flip);
	uint64_t pre = mO & (mO << 1);		// A&B B&C C&D D&E E&F F&G G&H 0
	_DOB(pre);
	flip |= (flip << 2) & pre;	// 0 0 0 D&E&F&G E&F&G F&G G 0
	_DOB(flip);
	flip |= (flip << 2) & pre;	// 0 B&C&D&E&F&G .. F&G G 0
	_DOB(flip);
	uint64_t outflank = p & (flip << 1);
	_DOB(outflank);
	flip &= -(int)(outflank != 0);
	_DOB(flip);
	return flip;
}

__m128i bit_manip::okuhara::flip_DiagonalA1H8_line(uint64_t p, uint64_t o, uint64_t X)
{
	uint64_t shift = 0x09ULL;
	uint64_t shift2 = shift + shift;
	uint64_t mO = o & 0x7e7e7e7e7e7e7e7e;
	_DOB(mO);
	_DOB(X);
	uint64_t flip = {};// 0 0 0 0 0 0 G 0
	_DOB((X << shift) & mO);
	flip = (X << shift) & mO;
	_DOB(flip);
	_DOB((flip << shift) & mO);
	flip |= (flip << shift) & mO;// 0 0 0 0 0 F&G G 0
	_DOB(flip);
	_DOB((mO << shift));
	uint64_t pre;
	_DOB(mO << shift);
	pre = mO & (mO << shift);// A&B B&C C&D D&E E&F F&G G&H 0
	_DOB(pre);
	_DOB(flip << shift2);
	_DOB((flip << shift2) & pre);
	flip |= (flip << shift2) & pre;
	_DOB(flip);	// 0 0 0 D&E&F&G E&F&G F&G G 0
	_DOB((flip << shift2));
	_DOB((flip << shift2) & pre);
	flip |= (flip << shift2) & pre;
	_DOB(flip);// 0 B&C&D&E&F&G .. F&G G 0
	uint64_t outflank;
	outflank = p & (flip << shift);
	_DOB(outflank);
	_DOB((uint64_t) - (int)(outflank != 0));
	flip &= -(int)(outflank != 0);
	_DOB(flip);
	uint64_t rp = p | X | flip;
	_DOB(rp);
	uint64_t ro = ~rp & o;
	_DOB(ro);
	return _mm_set_epi64x(ro,rp);
}
