/**
 * @file bit_manip.h
 * @brief ビットマニピュレータの宣言
 * @author Gold Smith
 * @date 2023
 * SPDX-License-Identifier: MIT
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 *
 * このファイル内のすべてのコードは、特に明記されていない限り、MITライセンスに従います。
 */
#pragma once
#include "pch.h"

#if !defined(_M_X64) && !defined(__amd64__)
#error "This code must be compiled in x64 configuration."
#endif
#ifndef __AVX2__
#error "This code requires AVX2."
#endif
#pragma comment(lib,  "../Debug_fnc/" _STRINGIZE($CONFIGURATION) "/Debug_fnc-" _STRINGIZE($CONFIGURATION) ".lib")

/** namespace bit_manip:ビット操作関数群 */
namespace bit_manip {
   /// <summary>
   /// n番目の位置のビットが1のunsignedを返す。
   /// </summary>
   /// <param name="n">LSBからの位置。</param>
   /// <returns></returns>
   template<typename T>
   inline T BIT(unsigned n) { return 1ULL << n; }
   /// <summary>
   /// n番目の位置のvalのビットを1にする。
   /// </summary>
   /// <typeparam name="T">符号なし整数の型</typeparam>
   /// <param name="val">変更する変数。</param>
   /// <param name="bit">LSBからの位置。</param>
   template<typename T>
   inline T SET_BIT(const T val, unsigned bit) { return val | BIT<T>(bit); }
   /// <summary>
   /// n番目の位置のvalのビットを0にする。
   /// </summary>
   /// <typeparam name="T">符号なし整数の型</typeparam>
   /// <param name="val">変更する変数。</param>
   /// <param name="bit">LSBからの位置。</param>
   template<typename T>
   inline T RESET_BIT(const T val, unsigned bit) {return val & ~BIT<T>(bit); }
   /// <summary>
    /// n番目の位置のvalのビットを0にする。
    /// </summary>
    /// <typeparam name="T">符号なし整数の型</typeparam>
    /// <param name="val">変更する変数。</param>
    /// <param name="bit">LSBからの位置。</param>
   template<typename T>
   inline T CLEAR_BIT(const T val, unsigned bit) { T result = val & ~BIT<T>(bit); return result; }
   /// <summary>
   /// n番目の位置のvalのビットを反転させる。
   /// </summary>
   /// <typeparam name="T">符号なし整数の型</typeparam>
   /// <param name="val">変更する変数。</param>
   /// <param name="bit">LSBからの位置。</param>
   /// <returns>変更後のvalを返す。</returns>
   template<typename T>
   inline T TOGGLE_BIT(const T val, unsigned bit) {
      return val ^ BIT<T>(bit); // valのbit番目のビットを反転
    }
   /// <summary>
    /// n番目の位置のvalのビットをxorさせ、変更する。
    /// </summary>
    /// <typeparam name="T">符号なし整数の型</typeparam>
    /// <param name="val">変更する変数。</param>
    /// <param name="bit">LSBからの位置。</param>
    /// <param name="cmp">比較する値。</param>
    /// <returns>変更後のvalを返す。</returns>
   template<typename T>
   inline T XOR_BIT(const T val, unsigned bit, bool cmp) {
      return CHANGE_BIT(val, bit, CHECK_BIT<T>(val, bit) ^ cmp);
   }
   /// <summary>
	/// valの指定されたbitをset値にする。
	/// </summary>
	/// <typeparam name="T">valの型。</typeparam>
	/// <param name="val">変更する変数。</param>
	/// <param name="bit">LSBからの変更する位置。</param>
	/// <param name="set">true:1にする。false:0にする。</param>
	/// <returns>set値。</returns>
	template<typename T>
   inline T CHANGE_BIT(const T val, unsigned bit, bool set) {
		if (set) {
			return SET_BIT(val, bit); // valのbit番目のビットをセット
		}
		else {
			return RESET_BIT(val, bit); // valのbit番目のビットをリセット
		}
	}
   /// <summary>
   /// 指定されたビットの値を返す。
   /// </summary>
   /// <typeparam name="T">valの型。</typeparam>
   /// <param name="val">調べる変数。</param>
   /// <param name="bit">LSBからの位置。</param>
   /// <returns>指定された位置のbit値。</returns>
   template<typename T>
   inline bool CHECK_BIT(T val, unsigned bit) { return val & BIT<T>(bit); }
   template<typename T>
   inline T ANDNOT_BIT(const T val1, const T val2) { return ~val1 & val2; }

}