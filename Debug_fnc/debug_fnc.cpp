/**
 * @file debug_fnc.cpp
 * @brief ƒfƒoƒbƒOŠÖ”‚ÌŽÀ‘•
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

std::vector<std::string> debug_fnc::boardToString(uint64_t p, uint64_t o, char cp, char co, char cv) {
	std::vector<std::string> result(8);
	for (int i = 63; i >= 0; --i) {
		if (p & (1ULL << i) && o & (1ULL << i)) {
			std::stringstream ss;
			ss << __FILE__ << "(" << std::to_string(__LINE__) << "):"
				<< "invalid_argument:(" << std::to_string(7 - i % 8) << ","
				<< std::to_string(7 - i / 8) << ")" ;
			throw std::invalid_argument(ss.str().c_str());
		}
		else if (p & (1ULL << i)) {
			result[i / 8] += cp;
		}
		else if (o & (1ULL << i)) {
			result[i / 8] += co;
		}
		else {
			result[i / 8] += cv;
		}
		if (i % 8 != 0) {
			result[i / 8] += ' ';
		}
	}
	return result;
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
		ss << binary_to_string(((uint8_t*)&b.m128i_u64[ReversiEngine::mIndex::BB_P64])[7 - i]);
		ss << " ";
		ss << binary_to_string(((uint8_t*)&b.m128i_u64[ReversiEngine::mIndex::BB_O64])[7 - i]);
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

void debug_fnc::dout_status(__m256i m)
{
	OutputDebugStringA(status_to_string(m).c_str());
}

std::string debug_fnc::status_to_string(__m256i m)
{
	using namespace ReversiEngine;
	using namespace bit_manip;
	using namespace std;
	std::stringstream ss;
	uint64_t C;
	uint64_t X;
	if (CHECK_BIT(m.m256i_u8[mIndex::ST1_8], ST1::IS_MY_TURN_NOW)) {
		if (CHECK_BIT(m.m256i_u8[mIndex::ST1_8], ST1::IS_X)) {
			C = m.m256i_u64[mIndex::BB_P64];
			X = m.m256i_u64[mIndex::BB_O64];
		}
		else {
			X = m.m256i_u64[mIndex::BB_P64];
			C = m.m256i_u64[mIndex::BB_O64];
		}
	}
	else {
		if (CHECK_BIT(m.m256i_u8[mIndex::ST1_8], ST1::IS_X)) {
			X = m.m256i_u64[mIndex::BB_P64];
			C = m.m256i_u64[mIndex::BB_O64];
		}
		else {
			C = m.m256i_u64[mIndex::BB_P64];
			X = m.m256i_u64[mIndex::BB_O64];
		}
	}

	vector<string> strings = boardToString(C, X);
	for (int i = 0; i <= 7; ++i) {
		ss << strings[7 - i];

		switch (i)
		{
		case 0: {
			DWORD index;
			unsigned char valid;
			string str;
			ss << " move:\t (";
			if (_BitScanReverse64(&index, m.m256i_u64[mIndex::BB_M64]) &&
				__popcnt64(m.m256i_u64[mIndex::BB_M64]) == 1) {
				unsigned x = 7 - index % 8;
				unsigned y = 7 - index / 8;
				ss << to_string((int)x) << "," << to_string((int)y);
			}
			else {
				ss << "0";
			}
			ss << ")";
			ss << "\t" << "Œ»Ý‚ÌŽè”Ô:\t"
				<< std::string(CHECK_BIT(m.m256i_u8[mIndex::ST1_8], ST1::IS_MY_TURN_NOW)
					? "ƒGƒ“ƒWƒ“" : "‘ŠŽè");
			break;
		}
		case 1: {
			ss << " ALPHA:\t  " << to_string(m.m256i_i16[mIndex::ALPHAi16]);
			ss << "\t\t" << "æUŒãU:\t"
				<< string(CHECK_BIT(m.m256i_u8[mIndex::ST1_8], ST1::IS_X) ? "æU(X)" : "ŒãU(C)");
			break;
		}
		case 2: {
			ss << " BETA:\t  " << to_string(m.m256i_i16[mIndex::BETAi16]);
			ss << "\tC vs C:\t\t"
				<< string(CHECK_BIT(m.m256i_u8[mIndex::ST1_8], ST1::IS_C2C) ? "YES" : "NO");
			break;
		}
		case 3: {
			ss << " DEPTH:\t  " << to_string(m.m256i_u8[mIndex::DEPTH8]);
			break;
		}
		case 4: {
			ss << " NUM_TURN: " << to_string(m.m256i_u8[mIndex::NUM_TURN8]);
			break;
		}
		case 5: {
			break;
		}
		default:
			break;
		}
		ss << std::endl;
	}
	return ss.str().c_str();
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
