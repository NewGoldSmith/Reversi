/**
 * @file bit_manip_okuhara.cpp
 *  SPDX-FileCopyrightText: © 2020 奥原 俊彦 <okuhara＠amy.hi-ho.ne.jp>
 *  original code from http: //www.amy.hi-ho.ne.jp/okuhara/bitboard.htm
 *  modified by Gold Smith
 */
#include "pch.h"
#include "bit_manip_okuhara.hpp"

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
