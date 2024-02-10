/**
 * @file MemoryRental.h
 * @brief テンプレートメモリプールクラスの実装
 * @author Gold Smith
 * @date 2022-2024
 * SPDX-License-Identifier: MIT
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 *
 * このファイル内のすべてのコードは、特に明記されていない限り、MITライセンスに従います。
 */

#pragma once
 // このクラスはWindows専用です。
 // 異常が検出された場合、必ず例外が発生します。
 // ********使用条件を設定***********
#define USING_CRITICAL_SECTION// クリティカルセクションを使用する場合
#define CONFIRM_POINT			// 範囲外確認が必要の場合。
#define USING_STD_ERROR			// std::cerr出力をする場合。
#define USING_DEBUG_STRING		// デバッグ出力をする場合。
// ******条件設定終わり*************

#ifdef USING_DEBUG_STRING
#include < algorithm >
#include <string>
#endif // USING_DEBUG_STRING

#ifdef USING_CRITICAL_SECTION
#include <synchapi.h>
#endif // USING_CRITICAL_SECTION

#include <exception>
#include <iostream>
#include <sstream>

template <class T>class MemoryRental
{
public:
	MemoryRental() = delete;
	//sizeInは2のべき乗で無くてはなりません。
	MemoryRental(T* const pBufIn, size_t sizeIn)
		:ppBuf(nullptr)
		, size(sizeIn)
		, front(0)
		, end(0)
#ifdef USING_DEBUG_STRING
		, max_using(0)
#endif // USING_DEBUG_STRING
		, mask(sizeIn - 1)
#ifdef USING_CRITICAL_SECTION
		, cs{}
#endif // USING_CRITICAL_SECTION
	{
		if ((sizeIn & (sizeIn - 1)) != 0)
		{
			std::string estr("Err! The MemoryRental must be Power of Two.\r\n");
#ifdef USING_DEBUG_STRING
			::OutputDebugStringA(estr.c_str());
#endif// USING_DEBUG_STRING
#ifdef USING_STD_ERROR
			std::cerr << estr;
#endif // USING_STD_ERROR
			throw std::invalid_argument(estr);
		}

#ifdef USING_CRITICAL_SECTION
		(void)InitializeCriticalSection(&cs);
#endif // USING_CRITICAL_SECTION
		ppBuf = new T * [sizeIn];
		for (size_t i(0); i < size; ++i)
		{
			ppBuf[i] = &pBufIn[i];
		}
	}
	MemoryRental(const MemoryRental&) = delete;
	MemoryRental(const MemoryRental&&)noexcept = delete;
	MemoryRental& operator ()(const MemoryRental&) = delete;
	MemoryRental& operator =(const MemoryRental&) = delete;
	MemoryRental& operator ()(MemoryRental&&) = delete;
	MemoryRental& operator =(MemoryRental&&) = delete;
	~MemoryRental()
	{
#ifdef USING_CRITICAL_SECTION
		DeleteCriticalSection(&cs);
#endif // USING_CRITICAL_SECTION

#ifdef USING_DEBUG_STRING
		std::stringstream ss;
		ss << "MemoryRental "
			<< "DebugMessage:"
			<< "\"" << strDebug.c_str() << "\" "
			<< "TypeName:"<<"\""<< typeid(T).name() << "\" "
			<< "SizeOfUnit:"<< sizeof(T)<<"bytes "
			<< "TotalLoans:" << std::to_string(front)
			<< " TotalReturns:" << std::to_string(end)
			<< " OutstandingLoans:" << std::to_string((long long)end - (long long)front)
			<< " NumberOfUnits:" << std::to_string(size)
			<< " MaximumPeakLoans:" << std::to_string(max_using)
			<< "\r\n";
		OutputDebugStringA(ss.str().c_str());
#endif // USING_DEBUG_STRING
		delete[]ppBuf;
	}

	//sizeInは2のべき乗で無くてはなりません。
	void ReInitialize(T* pBufIn, size_t sizeIn)
	{
		delete[]ppBuf;
		ppBuf = nullptr;
		size = sizeIn;
		front = 0;
		end = 0;
		const_cast<size_t>(mask) = sizeIn - 1;

		if ((sizeIn & (sizeIn - 1)) != 0)
		{
			std::string estr("Err! The MemoryRental must be Power of Two.\r\n");
#ifdef USING_DEBUG_STRING
			::OutputDebugStringA(estr.c_str());
#endif// USING_DEBUG_STRING
#ifdef USING_STD_ERROR
			std::cerr << estr;
#endif // USING_STD_ERROR
			throw std::invalid_argument(estr);
		}

		ppBuf = new T * [sizeIn];

		for (size_t i(0); i < size; ++i)
		{
			ppBuf[i] = &pBufIn[i];
		}
	}

	inline T* Lend()
	{
#ifdef USING_CRITICAL_SECTION
		std::unique_ptr< CRITICAL_SECTION, void(*)(CRITICAL_SECTION*)> qcs
			= { [&]() {EnterCriticalSection(&cs); return &cs; }()
			,[](CRITICAL_SECTION* pcs) {LeaveCriticalSection(pcs); } };
#endif // USING_CRITICAL_SECTION
#ifdef CONFIRM_POINT
		if (front + size < end)
		{
			std::stringstream ss;
			ss << __FILE__ << "(" << __LINE__ << "): "
				<< "MemoryRental is underflow. "
#ifdef USING_DEBUG_STRING
				<< "DebugMessage:"
				<< "\"" << strDebug << "\"" << " "
#endif// USING_DEBUG_STRING
				<< "TypeName:" << "\"" << typeid(T).name() << "\" "
				<< "SizeOfUnit:" << sizeof(T) << " "
				<< "TotalLoans:" << std::to_string(front)
				<< " TotalReturns:" << std::to_string(end)
				<< " OutstandingLoans:" << std::to_string((long long)end - (long long)front)
				<< " NumberOfUnits:" << std::to_string(size)
				<< " MaximumPeakLoans:" << std::to_string(max_using)
				<< "\r\n";
#ifdef USING_STD_ERROR
			std::cerr << ss.str();
#endif // USING_STD_ERROR
#ifdef USING_DEBUG_STRING
			::OutputDebugStringA(ss.str().c_str());
#endif// USING_DEBUG_STRING
			throw std::out_of_range(ss.str().c_str()); // 例外送出
		}
#endif // !CONFIRM_POINT
		T** ppT = &ppBuf[end & mask];
		++end;
#ifdef USING_DEBUG_STRING
		max_using=std::max<size_t>(end - front, max_using);
#endif // USING_DEBUG_STRING
		return *ppT;
	}

	inline void Return(T* pT)
	{
#ifdef USING_CRITICAL_SECTION
		std::unique_ptr< CRITICAL_SECTION, void(*)(CRITICAL_SECTION*)> qcs
			= { [&]() {EnterCriticalSection(&cs); return &cs; }()
			,[](CRITICAL_SECTION* pcs) {LeaveCriticalSection(pcs); }};
#endif // USING_CRITICAL_SECTION
#ifdef CONFIRM_POINT
		if (front == end + size)
		{
			std::stringstream ss;
			ss << __FILE__ << "(" << __LINE__ << "): " 
				<< "MemoryRental is overflow. "

#ifdef USING_DEBUG_STRING
				<< "DebugMessage:"
				<< "\"" << strDebug << "\"" << " "
#endif// USING_DEBUG_STRING
				<< "TypeName:" << "\"" << typeid(T).name() << "\" "
				<< "SizeOfUnit:" << sizeof(T) << " "
				<< "TotalLoans:" << std::to_string(front)
				<< " TotalReturns:" << std::to_string(end)
				<< " OutstandingLoans:" << std::to_string((long long)end - (long long)front)
				<< " NumberOfUnits:" << std::to_string(size)
				<< " MaximumPeakLoans:" << std::to_string(max_using)
				<< "\r\n";
#ifdef USING_STD_ERROR
			std::cerr << ss.str();
#endif // USING_STD_ERROR
#ifdef USING_DEBUG_STRING
			::OutputDebugStringA(ss.str().c_str());
#endif// USING_DEBUG_STRING
			throw std::out_of_range(ss.str()); // 例外送出
		}

#endif // !CONFIRM_POINT
		ppBuf[front & mask] = pT;
		++front;
	}

	void DebugString(const std::string str)
	{
#ifdef USING_DEBUG_STRING
		strDebug = str;
#endif // USING_DEBUG_STRING
	}

protected:
	T * *ppBuf; 
	size_t size;
	size_t front;
	size_t end;
	const size_t mask;
#ifdef USING_CRITICAL_SECTION
	CRITICAL_SECTION cs;
#endif // USING_CRITICAL_SECTION

#ifdef USING_DEBUG_STRING
	size_t max_using;
	std::string strDebug;
#endif // USING_DEBUG_STRING
};


#ifdef USING_CRITICAL_SECTION
#undef USING_CRITICAL_SECTION
#endif // USING_CRITICAL_SECTION

#ifdef CONFIRM_POINT
#undef CONFIRM_POINT
#endif // CONFIRM_POINT

#ifdef USING_DEBUG_STRING
#undef USING_DEBUG_STRING
#endif // USING_DEBUG_STRING

#ifdef USING_STD_ERROR
#undef USING_STD_ERROR
#endif // USING_STD_ERROR
