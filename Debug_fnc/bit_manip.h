/**
 * @file bit_manip.h
 * @brief ビットマニピュレータの宣言
 * @author Gold Smith
 * @date 2023
 * SPDX-License-Identifier: MIT * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 *
 * このファイル内のすべてのコードは、特に明記されていない限り、MITライセンスに従います。
 */
#pragma once
#include "pch.h"
#include "debug_fnc.h"

#if !defined(_M_X64) && !defined(__amd64__)
#error "This code must be compiled in x64 configuration."
#endif
#ifndef __AVX2__
#error "This code requires AVX2."
#endif

/** namespace bit_manip:ビット操作関数群 */
namespace bit_manip {


}
