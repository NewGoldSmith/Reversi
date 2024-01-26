/*
*  SPDX-FileCopyrightText: GPL
*  modified by Gold Smith */

#include "pch.h"
#include "bit_manip_gpl.h"
// (NOT lhs) AND rhs


inline __m256i bit_manip::gpl::andnot(const __m256i lhs, const __m256i rhs) {
   return _mm256_andnot_si256(lhs, rhs);
}

inline __m256i bit_manip::gpl::nonzero(const __m256i lhs) {
   return _mm256_add_epi64(_mm256_cmpeq_epi64(lhs, _mm256_setzero_si256()), _mm256_set1_epi64x(1));
}

inline __m256i bit_manip::gpl::upper_bit(__m256i p) {
   p = _mm256_or_si256(p, _mm256_srli_epi64(p, 1));
   p = _mm256_or_si256(p, _mm256_srli_epi64(p, 2));
   p = _mm256_or_si256(p, _mm256_srli_epi64(p, 4));
   p = _mm256_andnot_si256(_mm256_srli_epi64(p, 1), p);
   p = bit_manip::syuzuk81::flipVertical256(p);
   p = _mm256_andnot_si256(p, p);
   return bit_manip::syuzuk81::flipVertical256(p);
}

inline __m128i bit_manip::gpl::hor(const __m256i lhs) {
   __m128i lhs_xz_yw = _mm_or_si128(_mm256_castsi256_si128(lhs),
      _mm256_extractf128_si256(lhs, 1));
   return _mm_or_si128(lhs_xz_yw, _mm_alignr_epi8(lhs_xz_yw, lhs_xz_yw, 8));
}

__m128i bit_manip::gpl::flip(uint64_t p, uint64_t o, int pos)
{
   __m256i black = _mm256_set1_epi64x(p);
   __m256i white = _mm256_set1_epi64x(o);
   __m256i flipped, OM, outflank, mask;
   __m256i yzw = _mm256_setr_epi64x(
      UINT64_C(0xFFFFFFFFFFFFFFFF),
      UINT64_C(0x7E7E7E7E7E7E7E7E),
      UINT64_C(0x7E7E7E7E7E7E7E7E),
      UINT64_C(0x7E7E7E7E7E7E7E7E)
   );
   OM = _mm256_and_si256(white, yzw);
   mask = _mm256_setr_epi64x(
      UINT64_C(0x0080808080808080),
      UINT64_C(0x7F00000000000000),
      UINT64_C(0x0102040810204000),
      UINT64_C(0x0040201008040201)
   );
   mask = _mm256_srli_epi64(mask, (63 - pos));

   outflank = _mm256_and_si256(upper_bit(_mm256_andnot_si256(OM, mask)), black);
   flipped = _mm256_andnot_si256(_mm256_slli_epi64(outflank, 1), mask);
   mask = _mm256_setr_epi64x(
      UINT64_C(0x0101010101010100),
      UINT64_C(0x00000000000000FE),
      UINT64_C(0x0002040810204080),
      UINT64_C(0x8040201008040200)
   );
   mask = _mm256_slli_epi64(mask, pos);
   outflank = _mm256_and_si256(_mm256_and_si256(mask, _mm256_add_epi64(_mm256_or_si256(OM, _mm256_xor_si256(mask, _mm256_set1_epi64x(-1))), _mm256_set1_epi64x(1))), black);
   flipped = _mm256_or_si256(flipped, _mm256_and_si256(_mm256_sub_epi64(outflank, nonzero(outflank)), mask));
   return hor(flipped);
}
