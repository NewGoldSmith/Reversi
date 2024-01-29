/**
 * @file m256_Status.cpp
 * @brief m_256のステータス関数の実装
 * @author Gold Smith
 * @date 2024
 * SPDX-License-Identifier: MIT
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 *
 * このファイル内のすべてのコードは、特に明記されていない限り、MITライセンスに従います。
 */
#include "pch.h"
#include "m256_Status.h"
using namespace ReversiEngine;
__m256i __vectorcall ReversiEngine::make_next_turn_m(const __m256i m)noexcept
{
	__m256i tmp = _mm256_permute4x64_epi64(m, _MM_SHUFFLE(3, 2, 0, 1));
	tmp.m256i_u8[mIndex::ST1_8] = bit_manip::TOGGLE_BIT(m.m256i_u8[mIndex::ST1_8], ST1::IS_MY_TURN_NOW);
	return tmp;
}

int ReversiEngine::evaluate_by_turn(const __m256i m) noexcept
{
	if (bit_manip::CHECK_BIT(m.m256i_u8[mIndex::ST1_8], ST1::IS_MY_TURN_NOW)) {
		return (int)__popcnt64(m.m256i_u64[mIndex::BB_P64]) - (int)__popcnt64(m.m256i_u64[mIndex::BB_O64]);
	}
	else {
		return (int)__popcnt64(m.m256i_u64[mIndex::BB_O64]) - (int)__popcnt64(m.m256i_u64[mIndex::BB_P64]);
	}
}
