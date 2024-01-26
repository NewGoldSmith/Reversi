/**
 * @file MemoryRental.h
 * @brief �e���v���[�g�������v�[���N���X�̎���
 * @author Gold Smith
 * @date 2024
 * SPDX-License-Identifier: MIT
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 *
 * ���̃t�@�C�����̂��ׂẴR�[�h�́A���ɖ��L����Ă��Ȃ�����AMIT���C�Z���X�ɏ]���܂��B
 */

#pragma once
// ���̃N���X��Windows��p�ł��B
// ********�g�p������ݒ�***********
#define USING_CRITICAL_SECTION// �N���e�B�J���Z�N�V�������g�p����ꍇ
#define CONFIRM_POINT    // �͈͊O�m�F���K�v�̏ꍇ
#define USING_DEBUG_STRING		// �f�X�g���N�^�Ő������m�F������ꍇ
// ******�����ݒ�I���*************

#ifdef USING_DEBUG_STRING
#define NOMINMAX
#include <Windows.h>
#include < algorithm >
#include <string>
#endif // USING_DEBUG_STRING

#ifdef USING_CRITICAL_SECTION
#include <synchapi.h>
#endif // USING_CRITICAL_SECTION


#include <exception>
#include <atomic>
#include <iostream>
#include <sstream>

template <class T>class MemoryRental
{
public:
	MemoryRental() = delete;
	//sizeIn��2�ׂ̂���Ŗ����Ă͂Ȃ�܂���B
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
			std::cerr << estr;
			throw std::invalid_argument(estr);
		}

#ifdef USING_CRITICAL_SECTION
		(void)InitializeCriticalSectionAndSpinCount(&cs, 0);
#endif // USING_CRITICAL_SECTION
		ppBuf = new T * [sizeIn];
		for (size_t i(0); i < size; ++i)
		{
			ppBuf[i] = &pBufIn[i];
		}
	}
	MemoryRental(const MemoryRental&) = delete;
	MemoryRental(const MemoryRental&&)noexcept = delete;
	MemoryRental operator ()(const MemoryRental&) = delete;
	MemoryRental operator =(const MemoryRental&) = delete;
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
			<< "Each unit size:"<< sizeof(T)<<"bytes "
			<< "front:" << std::to_string(front)
			<< " end:" << std::to_string(end)
			<< " Difference:" << std::to_string((long long)end - (long long)front)
			<< " Number of available units:" << std::to_string(size)
			<< " MaximumPeakUsage:" << std::to_string(max_using)
			<< "\r\n";
		OutputDebugStringA(ss.str().c_str());
#endif // USING_DEBUG_STRING
		delete[]ppBuf;
	}

	//sizeIn��2�ׂ̂���Ŗ����Ă͂Ȃ�܂���B
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
			std::cerr << estr;
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
				<< "front:" << std::to_string(front)
				<< " end:" << std::to_string(end)
				<< " Difference:" << std::to_string((long long)end - (long long)front)
				<< " AvailableUnitSize:" << std::to_string(size)
				<< " MaximumPeakUsage:" << std::to_string(max_using)
				<< "\r\n";
			std::cerr << ss.str();
#ifdef USING_DEBUG_STRING
			::OutputDebugStringA(ss.str().c_str());
#endif// USING_DEBUG_STRING
			throw std::runtime_error(ss.str().c_str()); // ��O���o
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
				<< "front:" << std::to_string(front)
				<< " end:" << std::to_string(end)
				<< " Difference:" << std::to_string((long long)end - (long long)front)
				<< " AvailableUnitSize:" << std::to_string(size)
				<< " MaximumPeakUsage:" << std::to_string(max_using)
				<< "\r\n";
			std::cerr << ss.str();
#ifdef USING_DEBUG_STRING
			::OutputDebugStringA(ss.str().c_str());
#endif// USING_DEBUG_STRING
			throw std::runtime_error(ss.str()); // ��O���o
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
#undef NOMINMAX
#undef USING_DEBUG_STRING
#endif // USING_DEBUG_STRING
