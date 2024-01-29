/**
 * @file GameReversi8.h
 * @brief ゲームクラスの宣言
 * @author Gold Smith
 * @date 2023 2024
 * SPDX-License-Identifier: MIT
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 */
#pragma once
#define NOMINMAX
#define _CRTDBG_MAP_ALLOC
#if !defined(_M_X64) && !defined(__amd64__)
#error "This code must be compiled in x64 configuration."
#endif
#include "pch.h"
#pragma comment(lib,  "../BitManip/" _STRINGIZE($CONFIGURATION) "/BitManip-" _STRINGIZE($CONFIGURATION) ".lib")
#pragma comment(lib,  "../Debug_fnc/" _STRINGIZE($CONFIGURATION) "/Debug_fnc-" _STRINGIZE($CONFIGURATION) ".lib")
#pragma comment(lib,  "../Engine/" _STRINGIZE($CONFIGURATION) "/Engine-" _STRINGIZE($CONFIGURATION) ".lib")

#define CONFIRM_AVX2
namespace Reversi8 {
	using namespace std;
	constexpr unsigned N = 8U;
	constexpr unsigned FIRST_DEPTH = 1U;// 1以上に設定要。
	constexpr unsigned SECOND_DEPTH = 14U;
	constexpr unsigned MAX_DEPTH = FIRST_DEPTH + SECOND_DEPTH;
	constexpr unsigned MAX_THREADS = 6U;
	constexpr unsigned NUMBER_OF_NODE_UNIT = 0x1000U;
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
		const LARGE_INTEGER Frequency;
		__m256i m_board{};
	public:
		Game();
		~Game();
		Game(const Game&) = delete;
		Game(Game&&)noexcept = delete;
		Game& operator()(const Game&) = delete;
		Game& operator=(const Game&) = delete;
		bool operator==(const Game&) const = delete;
		/// <summary>
		/// 実際のゲームスタート。
		/// </summary>
		/// <param name="human_first">人が先攻か。</param>
		/// <param name="two_player">2プレーヤー対戦か。</param>
		void play_game(bool human_first, bool two_player);
	private:
		void display_board_m(const __m256i m_board) const;
		void init_game_m();
		bool is_draw_m(const __m256i m);
		/// <summary>
		/// 引き分けかどうか調べる。
		/// </summary>
		/// <param name="bb_X">ビットボードX</param>
		/// <param name="bb_C">ビットボードC</param>
		/// <returns>引き分けならtrue。それ以外false。</returns>
		bool is_win_m(const __m256i m);
		/// <summary>
		/// 対象のプレイヤーが勝ちかどうか調べる。
		/// </summary>
		/// <param name="p_board">調べる盤面のポインタ。</param>
		/// <param name="player">対象のプレイヤー。</param>
		/// <returns>勝ちならtrue、それ以外はfalse。</returns>
		bool is_game_over_m(__m256i m);
	};

}