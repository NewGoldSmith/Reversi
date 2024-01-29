/**
 * @file GameReversi8.cpp
 * @brief ゲームクラスの実装
 * @author Gold Smith
 * @date 2023 2024
 * SPDX-License-Identifier: MIT
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 */
#include "pch.h"
#include "GameReversi8.h"
#pragma intrinsic(_BitScanReverse64)

using namespace std;
using namespace debug_fnc;
using namespace bit_manip;
using namespace ReversiEngine;
Reversi8::Game::Game():
	ptpp{	[]() {
		PTP_POOL ptpp = CreateThreadpool(NULL);
		if (!ptpp){
			throw runtime_error(EOut);
		}
		SetThreadpoolThreadMaximum(ptpp, MAX_THREADS);
		return ptpp;
		}()
	,
		CloseThreadpool
	}
	
	, pcbe{[this]() {
			const auto pcbe = new TP_CALLBACK_ENVIRON;
			InitializeThreadpoolEnvironment(pcbe);
			SetThreadpoolCallbackPool(pcbe, ptpp.get());
			SetThreadpoolCallbackRunsLong(pcbe);
			return pcbe;
		}()
		,
		[](PTP_CALLBACK_ENVIRON pcbe) {
			DestroyThreadpoolEnvironment(pcbe);
			delete pcbe;
		}
	}

	, ptpcg{	[this]() {
			const PTP_CLEANUP_GROUP ptpcg = CreateThreadpoolCleanupGroup();
			SetThreadpoolCallbackCleanupGroup(pcbe.get(), ptpcg, pfng);
			return ptpcg;
		}()
		,
		[](PTP_CLEANUP_GROUP ptpcg) {
			CloseThreadpoolCleanupGroupMembers(ptpcg,TRUE,NULL);
			CloseThreadpoolCleanupGroup(ptpcg);
		}
	}

	, pfng{[] (
		 _Inout_opt_ PVOID ObjectContext,
		 _Inout_opt_ PVOID CleanupContext)
	{
		if (!ObjectContext) {
			dout("Context is NULL.");
			return;
		}
	}}

	, pcs{[this](){
		InitializeCriticalSection(&cs);
		return &cs;
		}()
		,
		[](CRITICAL_SECTION* pcs) {
			DeleteCriticalSection(pcs);
		}
	}
	
	, uhWaitEvent{ [this]() {
		if(!(hWaitEvent = CreateEvent(NULL, TRUE, FALSE, NULL)))	{
			throw runtime_error(EOut);
		}
		return hWaitEvent; }()
		,
		CloseHandle
	}

	, Frequency{ []() {
		LARGE_INTEGER frequency{};
		QueryPerformanceFrequency(&frequency);
		return frequency;
	}()}


{
	init_game_m();
}

Reversi8::Game::~Game()
{
}

void Reversi8::Game::init_game_m()
{
	// 盤面の初期化
	m_board = _mm256_setzero_si256();

	// 初期配置
	uint64_t init_X = 1ULL << ((N / 2 - 1) * N + (N / 2 - 1)); // 'X'
	uint64_t init_C = (1ULL << ((N / 2 - 1) * N + N / 2)) |    // 'C'
		(1ULL << (N / 2 * N + (N / 2 - 1)));      // 'C'
	init_X |= 1ULL << (N / 2 * N + N / 2);                     // 'X'

	m_board = _mm256_set_epi64x(0, 0, init_X, init_C);
}

void Reversi8::Game::display_board_m(const __m256i m) const
{
	uint64_t bbC;
	uint64_t bbX;

	if (CHECK_BIT(m.m256i_u8[mIndex::ST1_8], ST1::IS_X)) {
		if (CHECK_BIT(m.m256i_u8[mIndex::ST1_8], ST1::IS_MY_TURN_NOW)){
			bbX = m.m256i_u64[mIndex::BB_P64];
			bbC = m.m256i_u64[mIndex::BB_O64];
		}
		else {
			bbX = m.m256i_u64[mIndex::BB_O64];
			bbC = m.m256i_u64[mIndex::BB_P64];
		}
	}
	else {
		if (CHECK_BIT(m.m256i_u8[mIndex::ST1_8], ST1::IS_MY_TURN_NOW)) {
			bbX = m.m256i_u64[mIndex::BB_O64];
			bbC = m.m256i_u64[mIndex::BB_P64];
		}
		else {
			bbX = m.m256i_u64[mIndex::BB_P64];
			bbC = m.m256i_u64[mIndex::BB_O64];
		}
	}

	std::cout << " ";
	for (int row = 0; row < N; row++)
		std::cout << " " << row + 1;
	std::cout << std::endl;

	for (int row = 0; row < N; row++) {
		std::cout << row + 1;
		for (int col = 0; col < N; col++) {
			if (bbX & (1ULL << ((N - 1 - row) * N + (N - 1 - col)))) {
				std::cout << "|X";
			}
			else if (bbC & (1ULL << ((N - 1 - row) * N + (N - 1 - col)))) {
				std::cout << "|C";
			}
			else {
				std::cout << "| ";
			}
		}
		std::cout << "|" << std::endl;
		std::cout << " ";
		for (int col = 0; col < N; col++)
			std::cout << "--";
		std::cout << "-" << std::endl;
	}
}

void Reversi8::Game::play_game(bool human_first, bool is_c_to_c)
{
	m_board.m256i_u8[mIndex::ST1_8] = CHANGE_BIT(m_board.m256i_u8[mIndex::ST1_8], ST1::IS_X, !human_first);
	m_board.m256i_u8[mIndex::ST1_8] = CHANGE_BIT(m_board.m256i_u8[mIndex::ST1_8], ST1::IS_MY_TURN_NOW, !human_first);
	m_board.m256i_u8[mIndex::ST1_8] = CHANGE_BIT(m_board.m256i_u8[mIndex::ST1_8], ST1::IS_C2C, is_c_to_c);
	m_board.m256i_u8[NUM_TURN8] = 0;
	ReversiEngine::Engine8 engine(ptpp.get(), pcbe.get(), ptpcg.get());
	TotalTime = {};
	for (bool b_end(0); !b_end;) {
		if (is_game_over_m(m_board)) {
			break;
		}
		display_board_m(m_board);
		m_board.m256i_u8[NUM_TURN8]++;
		cout << (int)m_board.m256i_u8[NUM_TURN8] << "手目" << endl;
		int row, col;
		if (CHECK_BIT(m_board.m256i_u8[ST1_8], IS_MY_TURN_NOW)) {
			pair<int, int> p;
			QueryPerformanceCounter(&StartingTime);
			if (engine.search(m_board, uhWaitEvent.get()
				, MAX_DEPTH
				, FIRST_DEPTH
				, NUMBER_OF_NODE_UNIT)) {
				m_board.m256i_u64[mIndex::BB_M64] = engine.await_best_move();
				m_board = okuhara::flip256(m_board);
				DWORD index;
				_BitScanReverse64(&index, m_board.m256i_u64[mIndex::BB_M64]);
				cout << "コンピュータ("<< (CHECK_BIT(m_board.m256i_u8[ST1_8], ST1::IS_X) ? 'X' : 'C')
					<<")が" << to_string((63 - index) % N + 1)
					<< " " << to_string((63 - index) / N + 1) << "を指しました。" << endl;
			}
			else {
				cout << "コンピュータ(" << (CHECK_BIT(m_board.m256i_u8[ST1_8], ST1::IS_X) ? 'X' : 'C')
					<<")はパスしました。" << endl;
			}
			QueryPerformanceCounter(&EndingTime);
			ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
			ElapsedMicroseconds.QuadPart *= 1000000;
			ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
			double time_taken = ElapsedMicroseconds.QuadPart / 1000000.0;
			cout << "思考時間:" << fixed << setprecision(3)
				<< time_taken << " seconds." << endl;
			TotalTime.QuadPart += ElapsedMicroseconds.QuadPart;
		}
		else {
			for (; !b_end;) {
				cin.ignore(numeric_limits<streamsize>::max(), '\n');
				if (!(m_board = okuhara::get_moves256(m_board))
					.m256i_u64[mIndex::BB_M64])
				{
					cout << "プレイヤー(" << (char)(CHECK_BIT(m_board.m256i_u8[ST1_8], ST1::IS_X) ? 'C' : 'X')
						<< ")パスになります。何かキーを押してください。" << endl;
					(void)_getch();
					break;
				}
				cout << "プレイヤー(" << (char)(CHECK_BIT(m_board.m256i_u8[ST1_8], ST1::IS_X) ? 'C' : 'X')
					<< ")、入力して下さい(列 行)。 (終了する場合は 0 0 と入力):";
				cin >> col >> row;
				if (cin.fail()) {
					cin.clear();
					cout << "無効な入力です。再度入力してください。" << endl;
					display_board_m(m_board);
					continue;
				}
				if (row == 0 && col == 0)
				{
					b_end = true;
					continue;
				}
				if (row < 1 || row > N || col < 1 || col > N) {
					cout << "無効な手です。再度入力してください。" << endl;
					display_board_m(m_board);
					continue;
				}
				if ((m_board.m256i_u64[mIndex::BB_P64] | m_board.m256i_u64[mIndex::BB_O64]) & (0x8000000000000000ULL >> ((row - 1) * N + col - 1))) {
					cout << "そのマスは既に選択されています。" << endl;
					display_board_m(m_board);
					continue;
				}
				if (!(m_board.m256i_u64[mIndex::BB_M64]
					&(0x8000000000000000ULL>>((col-1)+(row-1)*8))))
				{
					cout << "そのマスは選択できません。" << endl;
					display_board_m(m_board);
					continue;
				}
				else {
					m_board.m256i_u64[mIndex::BB_M64]
						= 0x8000000000000000ULL >> ((col - 1) + (row - 1) * 8);
				}
				m_board = okuhara::flip256(m_board);
				break;
			}
		}
		// switch player
		m_board = make_next_turn_m(m_board);
	}
	if (is_draw_m(m_board)) {
		display_board_m(m_board);
		uint64_t cntX;
		uint64_t cntC;

		if (CHECK_BIT(m_board.m256i_u8[mIndex::ST1_8], ST1::IS_X)) {
			if (CHECK_BIT(m_board.m256i_u8[mIndex::ST1_8], ST1::IS_MY_TURN_NOW)) {
				cntX = __popcnt64(m_board.m256i_u64[mIndex::BB_P64]);
				cntC = __popcnt64(m_board.m256i_u64[mIndex::BB_O64]);
			}
			else {
				cntX = __popcnt64(m_board.m256i_u64[mIndex::BB_O64]);
				cntC = __popcnt64(m_board.m256i_u64[mIndex::BB_P64]);
			}
		}
		else {
			if (CHECK_BIT(m_board.m256i_u8[mIndex::ST1_8], ST1::IS_MY_TURN_NOW)) {
				cntC = __popcnt64(m_board.m256i_u64[mIndex::BB_P64]);
				cntX = __popcnt64(m_board.m256i_u64[mIndex::BB_O64]);
			}
			else {
				cntC = __popcnt64(m_board.m256i_u64[mIndex::BB_O64]);
				cntX = __popcnt64(m_board.m256i_u64[mIndex::BB_P64]);
			}
		}
		cout << "X:" << cntX
			<< " C:" << cntC
			<< "　引き分けです。" << endl;
	}
	else if (is_win_m(m_board)) {
		display_board_m(m_board);
		uint64_t cntX;
		uint64_t cntC;

		if (CHECK_BIT(m_board.m256i_u8[mIndex::ST1_8], ST1::IS_X)) {
			if (CHECK_BIT(m_board.m256i_u8[mIndex::ST1_8], ST1::IS_MY_TURN_NOW)) {
				cntX = __popcnt64(m_board.m256i_u64[mIndex::BB_P64]);
				cntC = __popcnt64(m_board.m256i_u64[mIndex::BB_O64]);
			}
			else {
				cntX = __popcnt64(m_board.m256i_u64[mIndex::BB_O64]);
				cntC = __popcnt64(m_board.m256i_u64[mIndex::BB_P64]);
			}
		}
		else {
			if (CHECK_BIT(m_board.m256i_u8[mIndex::ST1_8], ST1::IS_MY_TURN_NOW)) {
				cntC = __popcnt64(m_board.m256i_u64[mIndex::BB_P64]);
				cntX = __popcnt64(m_board.m256i_u64[mIndex::BB_O64]);
			}
			else {
				cntC = __popcnt64(m_board.m256i_u64[mIndex::BB_O64]);
				cntX = __popcnt64(m_board.m256i_u64[mIndex::BB_P64]);
			}
		}
		cout << "X:" << cntX
			<< " C:" << cntC
			<< (cntC < cntX ? " X":" C") << "の勝ちです！\r\n" << endl;
	}

	cout << "コンピューターのトータル思考時間は" << fixed << setprecision(3)
		<< (double)TotalTime.QuadPart / 1000000.0 << " seconds.です。" << endl;
}

bool Reversi8::Game::is_draw_m(const __m256i m)
{
	if (is_game_over_m(m)) {
		return __popcnt64(m.m256i_u64[mIndex::BB_P64]) == __popcnt64(m.m256i_u64[mIndex::BB_O64]);
	}
	return false;
}

bool Reversi8::Game::is_win_m(const __m256i m)
{
	if (is_game_over_m(m)) {
		if (CHECK_BIT(m.m256i_u64[mIndex::OP64], ST1::IS_MY_TURN_NOW)) {
			return (int)__popcnt64(m.m256i_u64[mIndex::BB_P64]) > (int)__popcnt64(m.m256i_u64[mIndex::BB_O64]);
		}
		else {
			return (int)__popcnt64(m.m256i_u64[mIndex::BB_O64]) > (int)__popcnt64(m.m256i_u64[mIndex::BB_P64]);
		}
	}
	return false;
}

bool Reversi8::Game::is_game_over_m(__m256i m)
{
	return !(okuhara::get_moves256(m).m256i_u64[mIndex::BB_M64])
		&& !(okuhara::get_moves256(make_next_turn_m(m)).m256i_u64[mIndex::BB_M64]);
}

