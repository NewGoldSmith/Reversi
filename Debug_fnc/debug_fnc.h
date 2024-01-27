/**
 * @file debug_fnc.h
 * @brief デバッグ関数の宣言
 * @author Gold Smith
 * @date 2023 2024
 *
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 */
#pragma once
#include "pch.h"

namespace debug_fnc {
	std::string binary_to_string(uint8_t b);
	std::vector<std::string> boardToString(uint64_t p, uint64_t o,char cp='X',char co='C',char cv='#');
	void print_binary(uint8_t b);
	void dout_binary(uint8_t b);
	std::string binary_to_string(uint16_t b);
	void print_binary(uint16_t b);
	void dout_binary(uint16_t b);
	std::string binary_to_string(uint64_t b);
	void print_binary(uint64_t b);
	void dout_binary(uint64_t b);
	std::string binary_to_string(__m128i b);
	void print_binary(__m128i b);
	void dout_binary(__m128i b);
	std::string binary_to_string(__m256i b);
	void print_binary(__m256i b);
	void dout_binary(__m256i m);
	void dout_status(__m256i m);
	std::string status_to_string(__m256i m);
	void chk_bb(__m256i m);
	void dout(const std::string& str);
	std::string GetErrString(DWORD dw);
	const std::string ErrOut_(
		DWORD dw
		, LPCSTR lpcszFile
		, LPCSTR lpcszFunction
		, DWORD dwLine
		, const std::string& lpszOpMessage = "");
#define EOut ErrOut_(GetLastError(),__FILE__,__FUNCTION__,__LINE__)

#ifdef _DEBUG
#define _D(s) {::OutputDebugStringA((std::string(__FILE__ "(" _STRINGIZE(__LINE__) "):")+s+"\r\n").c_str());}
#define _DOB(b){debug_fnc::dout({__FILE__ "(" _STRINGIZE(__LINE__)"):" #b});debug_fnc::dout_binary(b);}
#define _DOS(b){debug_fnc::dout({__FILE__ "(" _STRINGIZE(__LINE__)"):" #b});debug_fnc::dout_status(b);}
#define _CHKBB(b){chk_bb(b);}
#else
#define _D(s) __noop
#define _DOB(b) __noop
#define _DOS(b) __noop
#define _CHKBB(b) __noop
#endif 
	// _DEBUG
}