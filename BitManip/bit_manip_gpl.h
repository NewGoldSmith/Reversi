/*
*  SPDX-FileCopyrightText: GPL
*  modified by Gold Smith */
#pragma once
#include "pch.h"
#include "bit_manip_syuzuk81.h"

namespace bit_manip::gpl {
	inline __m256i andnot(const __m256i lhs, const __m256i rhs);
	inline __m256i nonzero(const __m256i lhs);
	inline __m256i upper_bit(__m256i p);
	inline __m128i hor(const __m256i lhs);
	__m128i flip(uint64_t p, uint64_t o, int pos);
}