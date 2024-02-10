/**
 * @file m256_Status.h
 * @brief m_256のインデックスの宣言
 * @author Gold Smith
 * @date 2024
 * SPDX-License-Identifier: MIT
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 *
 * このファイル内のすべてのコードは、特に明記されていない限り、MITライセンスに従います。
 */
#pragma once
#include "pch.h"

namespace bit_manip
{
	union m256 {
		__m256i m{};
		struct {
			uint64_t P;
			uint64_t O;
			uint64_t M;
			struct uint64_t {
				int16_t alpha;
				int16_t beta;
				uint8_t depth;
				uint8_t num_turn;
				uint8_t ID;
				uint8_t is_my_turn_now :1;
				uint8_t is_x :1;
				uint8_t is_c2c:1;
			}ST;
		};
	};
	union m128 {
		__m128i m{};
		struct {
			uint64_t P;
			uint64_t O;
		};
	};
}