/**
 * @file bit_manip_okuhara.h
 *  SPDX-FileCopyrightText: © 2020 奥原 俊彦 <okuhara＠amy.hi-ho.ne.jp>
 *  original code from http: //www.amy.hi-ho.ne.jp/okuhara/bitboard.htm
 *  modified by Gold Smith
 */
#include "pch.h"
#include "bit_manip.h"
#pragma once
#if !defined(_M_X64) && !defined(__amd64__)
#error "This code must be compiled in x64 configuration."
#endif
#ifndef __AVX2__
#error "This code requires AVX2."
#endif

namespace bit_manip::okuhara {
	/// <summary>
	/// 有効な手を探す。
	/// </summary>
	/// <param name="poms">Player,Opponent,Move,Status</param>
	/// <returns>Player,Opponent,Move,StatusのMoveに有効な手（複数）。</returns>
	__m256i __vectorcall get_moves256(const __m256i poms);
	uint64_t flip_shift_line(uint64_t p, uint64_t o, uint64_t m);
	uint64_t flip_para_line(uint64_t p, uint64_t o, uint64_t m);
	__m128i flip_DiagonalA1H8_line(uint64_t p, uint64_t o, uint64_t X);
	/// <summary>
	/// ひっくり返す。
	/// </summary>
	/// <param name="poms">m256i_u64[0]:Player、m256i_u64[1]:Opponent、m256i_u64[2]:Move、m256i_u64[3]:Status。</param>
	/// <returns>m256i_u64[0]とm256i_u64[1]は反映された結果が入り、他はそのまま。</returns>
	__m256i __vectorcall flip256(const __m256i poms);
}