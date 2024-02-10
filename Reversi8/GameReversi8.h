/**
 * @file GameReversi8.h
 * @brief �Q�[���N���X�̐錾
 * @author Gold Smith
 * @date 2023 2024
 * SPDX-License-Identifier: MIT
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 */
#pragma once
#if !defined(_M_X64) && !defined(__amd64__)
#error "This code must be compiled in x64 configuration."
#endif
#include "pch.h"
#include "../Debug_fnc/debug_fnc.h"
#include "../BitManip/bit_manip.h"
#include "../BitManip/bit_manip_okuhara.hpp"
#include "../BitManip/m256.h"
#include "../Engine/Engine8.h"
#pragma comment(lib,  "../BitManip/" _STRINGIZE($CONFIGURATION) "/BitManip-" _STRINGIZE($CONFIGURATION) ".lib")
#pragma comment(lib,  "../Debug_fnc/" _STRINGIZE($CONFIGURATION) "/Debug_fnc-" _STRINGIZE($CONFIGURATION) ".lib")
#pragma comment(lib,  "../Engine/" _STRINGIZE($CONFIGURATION) "/Engine-" _STRINGIZE($CONFIGURATION) ".lib")

#define CONFIRM_AVX2
namespace Reversi8 {
	using namespace std;
	using namespace bit_manip;
	constexpr unsigned N = 8U;
	constexpr unsigned FIRST_DEPTH = 1U;
	constexpr unsigned SECOND_DEPTH = 10U;
	constexpr unsigned MAX_DEPTH = FIRST_DEPTH + SECOND_DEPTH;
	constexpr unsigned MAX_THREADS = 6U;
	constexpr unsigned NUMBER_OF_NODE_UNIT = 0x100U;
#if (FIRST_DEPTH > 1 ) 
#error FIRST_DEPTH must be 1 or greater.
#endif
	class Game
	{
		CRITICAL_SECTION cs = {};
		const unique_ptr<TP_POOL, decltype(CloseThreadpool)*>ptpp;
		const unique_ptr<TP_CALLBACK_ENVIRON
			, void(*)(TP_CALLBACK_ENVIRON*)>pcbe;
		const unique_ptr<TP_CLEANUP_GROUP
			, void(*)(PTP_CLEANUP_GROUP)>ptpcg;
		PTP_CLEANUP_GROUP_CANCEL_CALLBACK const pfng;
		const unique_ptr< CRITICAL_SECTION
			, decltype(DeleteCriticalSection)*> pcs;

		HANDLE hWaitEvent = NULL;
		const unique_ptr<remove_pointer_t<HANDLE>, decltype(CloseHandle)*> uhWaitEvent;

		LARGE_INTEGER TotalTime = {};
		LARGE_INTEGER StartingTime{}, EndingTime{}, ElapsedMicroseconds{};
		const LARGE_INTEGER Frequency{};
		bit_manip::m256 m_board{};
		std::vector<m256>undo_buf{};
	public:
		Game();
		~Game();
		Game(const Game&) = delete;
		Game(Game&&) = delete;
		Game& operator=(const Game&) = delete;
		Game& operator()(const Game&) = delete;
		Game& operator=(Game&&) = delete;
		Game& operator()(Game&&) = delete;
		bool operator==(const Game&) const = delete;
		bool operator!=(const Game&) const = delete;
		/// <summary>
		/// ���ۂ̃Q�[���X�^�[�g�B
		/// </summary>
		/// <param name="human_first">�l����U���B</param>
		/// <param name="two_player">2�v���[���[�ΐ킩�B</param>
		void play_game(bool human_first, bool two_player);
	private:
		void display_board_m(const bit_manip::m256 m_board) const;
		void init_game_m();
		bool is_draw_m(const m256 m);
		/// <summary>
		/// �����������ǂ������ׂ�B
		/// </summary>
		/// <param name="bb_X">�r�b�g�{�[�hX</param>
		/// <param name="bb_C">�r�b�g�{�[�hC</param>
		/// <returns>���������Ȃ�true�B����ȊOfalse�B</returns>
		bool is_win_m(const m256 m);
		bool is_lose_m(const m256 m);
		/// <summary>
		/// �Ώۂ̃v���C���[���������ǂ������ׂ�B
		/// </summary>
		/// <param name="p_board">���ׂ�Ֆʂ̃|�C���^�B</param>
		/// <param name="player">�Ώۂ̃v���C���[�B</param>
		/// <returns>�����Ȃ�true�A����ȊO��false�B</returns>
		bool is_game_over_m(bit_manip::m256 m);
	};

}