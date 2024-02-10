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
#include "../Engine/Engine8.h"

using namespace std;

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
		if (p & (1ULL << i)) {
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

std::string debug_fnc::binary_to_string(m128 b)
{
	std::stringstream ss;
	for (int i = 0; i <= 7; ++i) {
		ss << binary_to_string(((uint8_t*)&b.P)[7 - i]);
		ss << " ";
		ss << binary_to_string(((uint8_t*)&b.O)[7 - i]);
		if (i == 7) {
			break;
		}
		ss << std::endl;
	}
	return ss.str().c_str();
}

void debug_fnc::print_binary(__m128i b)
{
	std::cout << binary_to_string((m128)b) << std::endl;
}

void debug_fnc::dout_binary(__m128i b)
{
	OutputDebugStringA((binary_to_string((m128)b) + "\r\n").c_str());
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

void debug_fnc::dout_binary(m256 m)
{
	dout_binary(m.m);
}

void debug_fnc::dout_status(m256 m)
{
	OutputDebugStringA(status_to_string(m).c_str());
}

std::string debug_fnc::status_to_string(m256 m)
{
	std::stringstream ss;
	uint64_t C;
	uint64_t X;
	if (m.ST.is_my_turn_now) {
		if (m.ST.is_x) {
			C = m.P;
			X = m.O;
		}
		else {
			X = m.P;
			C = m.O;
		}
	}
	else {
		if (m.ST.is_x) {
			X = m.P;
			C = m.O;
		}
		else {
			C = m.P;
			X = m.O;
		}
	}

	vector<string> strings = boardToString(C, X);
	for (int i = 0; i <= 7; ++i) {
		ss << strings[7 - i];

		switch (i)
		{
		case 0: {
			DWORD index;
			string str;
			ss << " move:\t (";
			if (_BitScanReverse64(&index, m.M) &&
				__popcnt64(m.M) == 1) {
				unsigned x = 8 - index % 8;
				unsigned y = 8 - index / 8;
				ss << to_string((int)x) << "," << to_string((int)y);
			}
			else {
				ss << "0";
			}
			ss << ")";
			ss << "\t" << "現在の手番:\t\t"
				<< std::string(m.ST.is_my_turn_now
					? "エンジン" : "相手");
			break;
		}
		case 1: {
			ss << " ALPHA:\t  " << to_string(m.ST.alpha);
			ss << "\t\t" << "先攻後攻:\t\t"
				<< string(m.ST.is_x ? "先攻(X)" : "後攻(C)");
			break;
		}
		case 2: {
			ss << " BETA:\t  " << to_string(m.ST.beta);
			ss << "\t\tC vs C:\t\t\t"
				<< string(m.ST.is_c2c ? "YES" : "NO");
			break;
		}
		case 3: {
			ss << " DEPTH:\t  " << to_string(m.ST.depth);
			m256 tmp;
			if (m.ST.is_my_turn_now) {
				tmp = m;
			}
			else {
				tmp = bit_manip::make_next_turn_m(m);
			}
			int result = (int)(__popcnt64(m.P) - __popcnt64(m.O));
			int val = m.ST.is_my_turn_now ? result : -result;
			ss << "\t\t石差:\t\t\t" << to_string(val);
			break;
		}
		case 4: {
			ss << " NUM_TURN: " << to_string(m.ST.num_turn);
			break;
		}
		case 5: {
			ss << " ID:\t\t  " << to_string(m.ST.ID);
			break;
		}
		default:
			break;
		}
		ss << std::endl;
	}
	return ss.str().c_str();
}

bool debug_fnc::chk_bb(m256 m)
{
	if (m.P & m.O) {
		return false;
	}
	return true;
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
