/**
 * @file bit_manip_syuzuk81.h
 * @author ysuzuk81
 * SPDX-FileCopyrightText: © 2023 ysuzuk81 <https: //qiita.com/ysuzuk81/>
 * original code from https: //qiita.com/ysuzuk81/items/453b08a14d23fb8c6c11
 * modified by Gold smith
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

#define m_ACW45L \
0b\
00000001\
00000011\
00000111\
00001111\
00011111\
00111111\
01111111\
11111111ULL
#define m_ACW45U \
0b\
11111111\
11111110\
11111100\
11111000\
11110000\
11100000\
11000000\
10000000ULL

#define m_CW45L \
0b\
10000000\
11000000\
11100000\
11110000\
11111000\
11111100\
11111110\
11111111ULL
#define m_CW45U \
0b\
11111111\
01111111\
00111111\
00011111\
00001111\
00000111\
00000011\
00000001ULL

namespace bit_manip::syuzuk81{
	constexpr uint64_t mask_ACW45L = m_ACW45L ;
	constexpr uint64_t mask_ACW45U = m_ACW45U ;
	constexpr uint64_t mask_CW45L =	m_CW45L  ;
	constexpr uint64_t mask_CW45U =	m_CW45U;		constexpr int mm_shuffle_1_0_3_2 = _MM_SHUFFLE(1, 0, 3, 2);
	constexpr int mm_shuffle_1_3_0_2 = _MM_SHUFFLE(1, 3, 0, 2);
	/*
	 * delta_swapからrotateAC45UULL256までは以下の著作者となっています。
	 * @author ysuzuk81
	 * SPDX-FileCopyrightText: © 2023 ysuzuk81 <https: //qiita.com/ysuzuk81/>
	 * modified by Gold smith
	 */
	uint64_t delta_swap(uint64_t x, uint64_t mask, int delta);
	__m128i _vectorcall delta_swap128(__m128i x, __m128i mask, int delta);
	__m256i  delta_swap256(__m256i xs, __m256i masks, int delta);
	uint64_t rotateLeft(uint64_t x, int n);
	__m128i _vectorcall rotateLeft128(__m128i xs, int n);
	__m256i _vectorcall rotateLeft256(__m256i xs, int n);
	uint64_t rotateRight(uint64_t x, int n);
	__m128i _vectorcall rotateRight128(__m128i xs, int n);
	__m256i _vectorcall rotateRight256(__m256i xs, int n);
	uint64_t flipHorizontal(uint64_t x);
	__m128i _vectorcall flipHorizontal128(__m128i x);
	__m256i _vectorcall flipHorizontal256(__m256i x);
	uint64_t flipVertical(uint64_t x);
	__m128i _vectorcall flipVertical128(__m128i x);
	__m256i _vectorcall flipVertical256(__m256i x);
	uint64_t flipDiagonalA1H8(uint64_t x);
	__m128i _vectorcall flipDiagonalA1H8128(__m128i x);
	__m256i _vectorcall flipDiagonalA1H8256(__m256i x);
	uint64_t flipDiagonalA8H1(uint64_t x);
	__m128i _vectorcall flipDiagonalA8H1128(__m128i x);
	__m256i _vectorcall flipDiagonalA8H1256(__m256i x);
	uint64_t rotateC90(uint64_t x);
	__m128i _vectorcall rotateC90128(__m128i x);
	__m256i _vectorcall rotateC90256(__m256i x);
	uint64_t rotateAC90(uint64_t x);
	__m128i _vectorcall rotateAC90128(__m128i x);
	__m256i _vectorcall rotateAC90256(__m256i x);
	uint64_t rotate180(uint64_t x);
	__m128i _vectorcall rotate180128(__m128i x);
	__m256i _vectorcall rotate180256(__m256i x);
	uint64_t rotateC45U(uint64_t x);
	__m128i _vectorcall rotateC45U128(__m128i x);
	__m256i _vectorcall rotateC45U256(__m256i x);
	uint64_t rotateC45L(uint64_t x);
	__m128i _vectorcall rotateC45L128(__m128i x);
	__m256i _vectorcall rotateC45L256(__m256i x);
	/// <summary>
	/// rotateCW45のLUとULを両方変換する。
	/// </summary>
	/// <param name="x">m256i_u64[0,1]LU、m256i_u64[2,3]ULに変換したい物を入れる。</param>
	/// <returns>m256i_u64[0,1]CW45LU、m256i_u64[2,3]CW45UL</returns>
	__m256i _vectorcall rotateC45UULL256(__m256i x);
	uint64_t rotateAC45U(uint64_t x);
	__m128i _vectorcall rotateAC45U128(__m128i x);
	__m256i _vectorcall rotateAC45U256(__m256i x);
	uint64_t rotateAC45L(uint64_t x);
	__m128i _vectorcall rotateAC45L128(__m128i x);
	__m256i _vectorcall rotateAC45L256(__m256i x);
	/// <summary>
	/// rotateACW45のLUとULを両方変換する。
	/// </summary>
	/// <param name="x">m256i_u64[0,1]LU、m256i_u64[2,3]ULに変換したい物を入れる。</param>
	/// <returns>m256i_u64[0,1]ACW45LU、m256i_u64[2,3]ACW45UL</returns>
	__m256i _vectorcall rotateAC45UULL256(__m256i x);
	/// <summary>
	/// ラインの左側の着手可能な位置を探す。キャリー伝搬を利用している。
	/// </summary>
	/// <param name="p">プレイヤーライン。</param>
	/// <param name="o">相手側ライン。</param>
	/// <returns>着手可能なビットが立っている。</returns>
	uint8_t mobilityL8(uint8_t p, uint8_t o);
	uint8_t mobilityR8(uint8_t p, uint8_t o);
	/// <summary>
	/// mobility_line8のビットボード版。左側の着手可能な位置をまとめて探す。
	/// </summary>
	/// <param name="p">プレイヤーボード。</param>
	/// <param name="o">相手ボード。</param>
	/// <returns>着手可能な左側のビットが立っている。</returns>
	uint64_t mobility_line64(uint64_t p, uint64_t o);
	/// <summary>
	/// 4つのビットボードの左側の着手可能な位置を探す。mobility_line64のm256i版。
	/// </summary>
	/// <param name="p">m256i_u64[0-3]:p[0-3]。</param>
	/// <param name="o">m256i_u64[0-3]:o[0-3]。</param>
	/// <returns>m256i_u64[0-3]:pos[0-3]。</returns>
	__m256i _vectorcall mobility_line256(__m256i p, __m256i o);
	uint64_t mobility_BB64(uint64_t p, uint64_t o);
	/// <summary>
	/// 2つのビットボードの着手可能な手を探す。
	/// </summary>
	/// <param name="po">m256i_u64[0]:p0。m256i_u64[1]:o0。</param>
	/// <param name="po">m256i_u64[0]:p0。m256i_u64[1]:o0。</param>
	/// <returns>m256i_u64[0]:pos。</returns>
	uint64_t _vectorcall mobility_BB128(__m128i po);

	uint16_t flipDiscsLine8(uint8_t p, uint8_t o, uint8_t pos);
	/// <summary>
	/// 8x8ビットボードの各8ビットラインを反転させる。
	/// 置いた石の左側が反転する。正しい位置を指定する必要がある。
	/// </summary>
	/// <param name="p">プレイヤーボード。</param>
	/// <param name="o">相手ボード。</param>
	/// <param name="pos">move(指し手)</param>
	/// <returns>m128i_u64[0,1]フリップした後のp,oのボード。</returns>
	uint64_t _vectorcall flipDiscsLine64(uint64_t p, uint64_t o, uint64_t m);
	/// <summary>
	/// ２つのビットボードをラインフリップする。
	/// </summary>
	/// <param name="p1p2">プレイヤーボード。</param>
	/// <param name="o1o2">相手ボード。</param>
	/// <param name="pos1pos2">指し手</param>
	/// <returns>m256i_u64[0,1]p1o1の結果。m256i_u64[2,3]p2o2の結果。</returns>
	void _vectorcall flipDiscs2Line(__m128i p0p1, __m128i o0o1, __m128i m0m1, __m128i* prp0rp1, __m128i* pro0ro1);
	/// <summary>
	/// 4つのビットボードをラインフリップする。
	/// </summary>
	/// <param name="p1p2p3p4">プレイヤーボード。</param>
	/// <param name="o1o2o3o4">相手ボード。</param>
	/// <param name="pos1pos2pos3pos4">指し手</param>
	/// <param name="p_rp">結果を保存するプレイヤーボードへのポインタ。</param>
	/// <param name="p_ro">結果を保存する相手ボードへのポインタ。</param>
	/// <returns>void</returns>
	void _vectorcall flipDiscs4Lines(__m256i p0p1p2p3, __m256i o0o1o2o3, __m256i m0m1m2m3, __m256i* p_rp0rp1rp2rp3, __m256i* p_ro0ro1ro2ro3);
	void flipDiscsBB(uint64_t p, uint64_t o, uint64_t m, uint64_t* pro, uint64_t* prp);

}
#undef m_ACW45L 
#undef m_ACW45U 
#undef m_CW45L 
#undef m_CW45U
