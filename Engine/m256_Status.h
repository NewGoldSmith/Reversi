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
namespace ReversiEngine
{
	/// <summary>
 /// 盤面のデータを保持する__m256iの頭出し用。
 /// サフィックスの数字は使用するビット幅。
 /// OP64はオプションとして領域を小分けにする。
 /// ST1_8は更に小分けにしてフラグを収める。
 /// </summary>
	const enum mIndex :unsigned {
		BB_P64, BB_O64, BB_M64, OP64, ALPHAi16 = 12, BETAi16
		, DEPTH8 = 28, NUM_TURN8, ST1_8
	};
	/// <summary>
	/// IS_MY_TURN_NOWコンピュータの手番である。
	/// IS_C コンピューターが後手番(C)とする。
	/// F1_IS_C2Cコンピューター同志の対戦。
	/// </summary>
	const enum ST1 :uint8_t { IS_MY_TURN_NOW, IS_X, IS_C2C };
}