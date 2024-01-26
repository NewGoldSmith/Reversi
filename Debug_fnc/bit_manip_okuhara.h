/* 
* @file bit_manip_okuhara.h
*  SPDX-FileCopyrightText: © 2020 奥原 俊彦 <okuhara＠amy.hi-ho.ne.jp>
*  original code from http: //www.amy.hi-ho.ne.jp/okuhara/bitboard.htm
*  modified by Gold Smith */
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
	/// <param name="p">プレイヤーボード。</param>
	/// <param name="o">相手ボード。</param>
	/// <returns>有効な着手手のビットが全て立っている。</returns>
	uint64_t get_movesBB(uint64_t p, uint64_t o);
	uint64_t flip_shift_line(uint64_t p, uint64_t o, uint64_t m);
	uint64_t flip_para_line(uint64_t p, uint64_t o, uint64_t m);
	__m128i flip_DiagonalA1H8_line(uint64_t p, uint64_t o, uint64_t X);
	/// <summary>
	/// ひっくり返す。
	/// </summary>
	/// <param name="pomv">m256i_u64[0]:プレイヤー、m256i_u64[1]:相手、m256i_u64[2]:指し手、m256i_u64[3]:ボイド</param>
	/// <returns>結果__m128.m128i_u64[0プレイヤー:1相手]。</returns>
	__m128i flip_BB(__m256i pomv);
}