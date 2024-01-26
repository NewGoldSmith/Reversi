/**
 * @file debug_fnc.cpp
 * @brief デバッグ関数の実装
 * @author Gold Smith
 * @date 2023
 *
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 */
#include "pch.h"
#include "debug_fnc.h"

std::string debug_fnc::binary_to_string(uint8_t b)
{
	std::stringstream ss;
	for (int j = 7; j >= 0; --j) {
		ss << (b >> j & 1) << " ";
	}
	return ss.str().c_str();
}

void debug_fnc::print_binary(uint8_t b)
{
	std::cout << binary_to_string(b) << std::endl;
}

void debug_fnc::dout_binary(uint8_t b)
{
	OutputDebugStringA((binary_to_string(b) + "\r\n").c_str());
}

std::string debug_fnc::binary_to_string(uint16_t b)
{
	std::stringstream ss;
	ss << binary_to_string(((uint8_t*)&b)[1]);
	ss << std::endl;
	ss << binary_to_string(((uint8_t*)&b)[0]);
	return ss.str().c_str();
}

void debug_fnc::print_binary(uint16_t b)
{
	std::cout << binary_to_string(b) << std::endl;
}

void debug_fnc::dout_binary(uint16_t b)
{
	OutputDebugStringA((binary_to_string(b) + "\r\n").c_str());
}

std::string debug_fnc::binary_to_string(uint64_t b)
{
	std::stringstream ss;
	for (int j = 7; j >= 0; --j) {
		ss << binary_to_string(((uint8_t*)&b)[j]);
		if (j == 0) {
			break;
		}
		ss << std::endl;
	}
	return ss.str().c_str();
}

void debug_fnc::print_binary(uint64_t b)
{
	std::cout << binary_to_string(b) << std::endl;
}

void debug_fnc::dout_binary(uint64_t b)
{
	OutputDebugStringA((binary_to_string(b) + "\r\n").c_str());
}

std::string debug_fnc::binary_to_string(__m128i b)
{
	std::stringstream ss;
	for (int i = 0; i <= 7; ++i) {
		ss << binary_to_string(((uint8_t*)&b.m128i_u64[0])[7 - i]);
		ss << " ";
		ss << binary_to_string(((uint8_t*)&b.m128i_u64[1])[7 - i]);
		if (i == 7) {
			break;
		}
		ss << std::endl;
	}
	return ss.str().c_str();
}

void debug_fnc::print_binary(__m128i b)
{
	std::cout << binary_to_string(b) << std::endl;
}

void debug_fnc::dout_binary(__m128i b)
{
	OutputDebugStringA((binary_to_string(b) + "\r\n").c_str());
}

std::string debug_fnc::binary_to_string(__m256i b)
{
	std::stringstream ss;
	for (int i = 0; i <= 7; ++i) {
		ss << binary_to_string(((uint8_t*)&b.m256i_u64[0])[7 - i]);
		ss << " ";
		ss << binary_to_string(((uint8_t*)&b.m256i_u64[1])[7 - i]);
		ss << " ";
		ss << binary_to_string(((uint8_t*)&b.m256i_u64[2])[7 - i]);
		ss << " ";
		ss << binary_to_string(((uint8_t*)&b.m256i_u64[3])[7 - i]);
		if (i == 7) {
			break;
		}
		ss << std::endl;
	}
	return ss.str().c_str();
}

void debug_fnc::print_binary(__m256i b)
{
	std::cout << binary_to_string(b) << std::endl;
}

void debug_fnc::dout_binary(__m256i b)
{
	OutputDebugStringA((binary_to_string(b) + "\r\n").c_str());
}

void debug_fnc::dout(const std::string& str)
{
	OutputDebugStringA((str + "\r\n").c_str());
}

std::string debug_fnc::GetErrString(DWORD dw)
{
		LPVOID lpMsgBuf;

		FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER
			| FORMAT_MESSAGE_FROM_SYSTEM
			| FORMAT_MESSAGE_IGNORE_INSERTS
			| FORMAT_MESSAGE_MAX_WIDTH_MASK
			, NULL
			, dw
			, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
			, (LPSTR)&lpMsgBuf
			, 0, NULL);
		std::string str=(char*)lpMsgBuf;
		LocalFree(lpMsgBuf);
		return str;
}

const std::string debug_fnc::ErrOut_(DWORD dw, LPCSTR lpcszFile, LPCSTR lpcszFunction, DWORD dwLine, const std::string& lpszOpMessage)
{
	const std::string strOpMessage = "User Message:\"" + lpszOpMessage + "\"";
	std::stringstream ss;
	ss << lpcszFile << "(" << dwLine << "): error C" << dw << ": "\
		<< GetErrString(dw)
		<< "function name: " << lpcszFunction
		<< strOpMessage << "\r\n";
	::OutputDebugStringA(ss.str().c_str());
	std::cerr << ss.str();
	return ss.str().c_str();
}
