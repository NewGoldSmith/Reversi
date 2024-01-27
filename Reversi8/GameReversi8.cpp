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

Game::Game():
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

	, node_array
	{
		new node_t[NODE_UNIT_SIZE]
		,
		[](node_t p[]){
			delete[]p;
		}
	}
	, mr_Node(node_array.get(), NODE_UNIT_SIZE)

	, pfnwkMiniMax{ [](
		_Inout_     PTP_CALLBACK_INSTANCE Instance,
		_Inout_opt_ PVOID                 Context,
		_Inout_     PTP_WORK              Work) {
		if (!Context) {
		  dout("Context is NULL.");
		  return;
		}
		node_t* const pNode = reinterpret_cast<node_t*>(Context);

		unique_ptr<CRITICAL_SECTION, decltype(LeaveCriticalSection)*> unique_lock = {
		  [pNode]() {EnterCriticalSection(&pNode->cs); return &pNode->cs; }()
		  ,LeaveCriticalSection };

		if (pNode->depth >= FIRST_DEPTH)
		{
#ifdef _DEBUG
			int scoreb = pNode->pGame->
				minimax_bb(pNode->bb_X, pNode->bb_C, pNode->player, 0);
#endif // _DEBUG

			pNode->score= pNode->pGame->
				alphabeta_bb(pNode->bb_X, pNode->bb_C, pNode->player, 0, -INF, INF);

#ifdef _DEBUG
			if (pNode->score != scoreb) {
				MessageBoxA(NULL, "スコアが違います。", "検証", MB_ICONEXCLAMATION);
				int i = 0;
			}
#endif // _DEBUG

			pNode->pGame->return_minimax(pNode);
			return;
		}

		char next_player = pNode->player == 'X' ? 'C' : 'X';
		vector<std::pair<int, int>> pairs = move(pNode->pGame->
			get_valid_moves_bb(pNode->bb_X, pNode->bb_C, next_player));
		if (pairs.empty()) {
			next_player = next_player == 'X' ? 'C' : 'X';
			pairs = move(pNode->pGame->get_valid_moves_bb(pNode->bb_X, pNode->bb_C, next_player));
			if (pairs.empty()) {
				// 勝敗確定
				_D("勝敗確定 (" + to_string(pNode->row + 1) + "," + to_string(pNode->col + 1) + ")");
				pNode->score = pNode->pGame->
					evaluate_bbS(pNode->bb_X, pNode->bb_C) > 0 ?
					INF - (int)pNode->depth
					: -INF + (int)pNode->depth;
				pNode->pGame->return_minimax(pNode);
				return;
			}
		}

		if (next_player=='C') {

			for (const auto& a_pair : pairs) {
				node_t* pNextNode = pNode->pGame->mr_Node.Lend();
				pNode->pGame->update_board_bb(pNextNode->bb_X, pNextNode->bb_C
					, pNode->bb_X, pNode->bb_C
					, a_pair.first, a_pair.second, next_player);
#ifdef _DEBUG
				pNextNode->pGame->conv_bbToBoard(&pNextNode->board
					, pNextNode->bb_X, pNextNode->bb_C);
#endif // _DEBUG
				pNextNode->player = next_player;
				pNextNode->depth = pNode->depth + 1;
				pNextNode->row = a_pair.first;
				pNextNode->col = a_pair.second;
				pNextNode->score = 0;
				pNode->score = -INF;
				pNextNode->called_branch_cnt = 0;
				pNextNode->p_parent = pNode;
				pNextNode->vp_branch_nodes = {};
				pNextNode->pGame = pNode->pGame;
				pNode->vp_branch_nodes.push_back(pNextNode);

				if (!(pNextNode->pwk = CreateThreadpoolWork(
					pNode->pGame->pfnwkMiniMax
					, pNextNode
					, &*pNode->pGame->pcbe))) {
					EOut;
					MessageBoxA(NULL, "Err.", "CreateThreadpoolWork", MB_ICONEXCLAMATION);
					return;
				}
				SubmitThreadpoolWork(pNextNode->pwk);
			}
		}
		else {

			for (const auto& a_pair : pairs) {
				node_t* pNextNode = pNode->pGame->mr_Node.Lend();
				pNode->pGame->update_board_bb(pNextNode->bb_X, pNextNode->bb_C
					, pNode->bb_X, pNode->bb_C, a_pair.first, a_pair.second
					, next_player);
#ifdef _DEBUG
				pNextNode->pGame->conv_bbToBoard(&pNextNode->board
					, pNextNode->bb_X, pNextNode->bb_C);
#endif // _DEBUG
				pNextNode->player = next_player;
				pNextNode->depth = pNode->depth + 1;
				pNextNode->row = a_pair.first;
				pNextNode->col = a_pair.second;
				pNextNode->score = 0;
				pNode->score = INF;
				pNextNode->called_branch_cnt = 0;
				pNextNode->p_parent = pNode;
				pNextNode->vp_branch_nodes = {};
				pNextNode->pGame = pNode->pGame;
				pNode->vp_branch_nodes.push_back(pNextNode);

if (!(pNextNode->pwk = CreateThreadpoolWork(
	pNode->pGame->pfnwkMiniMax
	, pNextNode
	, &*pNode->pGame->pcbe))) {
	EOut;
	MessageBoxA(NULL, "Err.", "CreateThreadpoolWork", MB_ICONEXCLAMATION);
	return;
}
SubmitThreadpoolWork(pNextNode->pwk);
			}
		}
} }


{
	init_game_bb();
	init_game_m();
	init_shift_table();
	mr_Node.DebugString("mr_Node");
}

Game::~Game()
{
}

void Game::init_game_m()
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

void Game::init_game_bb()
{
	// 盤面の初期化
	bb_cur_X = 0ULL;
	bb_cur_C = 0ULL;

	// 初期配置
	bb_cur_X |= (1ULL << ((N / 2 - 1) * N + (N / 2 - 1))); // 'X'
	bb_cur_C |= (1ULL << ((N / 2 - 1) * N + N / 2));       // 'C'
	bb_cur_C |= (1ULL << (N / 2 * N + (N / 2 - 1)));       // 'C'
	bb_cur_X |= (1ULL << (N / 2 * N + N / 2));             // 'X'
}

void Game::init_game()
{
	// 盤面の初期化
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			board[i][j] = ' ';
	// 初期配置
	board[N / 2 - 1][N / 2 - 1] = 'X';
	board[N / 2 - 1][N / 2] = 'C';
	board[N / 2][N / 2 - 1] = 'C';
	board[N / 2][N / 2] = 'X';
}

void Game::display_board_bb(const bb_t bbX, const bb_t bbC) const
{
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

void Game::display_board_m(const __m256i m) const
{
	uint64_t bbC;
	uint64_t bbX;

	if (CHECK_BIT(m.m256i_u8[mIndex::ST1_8], ST1::IS_C_X)) {
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

void Game::return_minimax_m(node_t* const p_node)
{
}

void Game::return_minimax( node_t* const p_node)
{
	//// ToDo:確認要
	//unique_ptr<CRITICAL_SECTION, decltype(LeaveCriticalSection)*> lock_child = {
	//	[p_node]() {
	//		EnterCriticalSection(&p_node->cs);
	//		return &p_node->cs;
	//	}(),LeaveCriticalSection };

	//unique_ptr<CRITICAL_SECTION, decltype(LeaveCriticalSection)*> lock_parent = {
	//[p_node]() {
	//	EnterCriticalSection(&p_node->p_parent->cs);
	//	return &p_node->p_parent->cs;
	//	}(), LeaveCriticalSection
	//};

	//if (!p_node->p_parent->p_parent) {
	//	dout(string() + __FILE__ + "(" + to_string(__LINE__) + "): "
	//		+ "Turn:" + to_string(turn) + " return score to first node"
	//		+ "(" + to_string(p_node->row + 1) + ","
	//		+ to_string(p_node->col + 1) + "):" + to_string(p_node->score));
	//	p_node->p_parent->called_branch_cnt++;

	//	unique_ptr<CRITICAL_SECTION, decltype(LeaveCriticalSection)*> lock_game = {
	//		[this]() {EnterCriticalSection(&cs); return &cs; }()
	//		,LeaveCriticalSection };

	//	if (best_val <= p_node->score) {
	//		dout(string() + __FILE__ + "(" + to_string(__LINE__) + "): "
	//			+ "Turn:" + to_string(turn) + " update score" + "(" + to_string(p_node->row + 1) + ","
	//			+ to_string(p_node->col + 1) + "):" + to_string(p_node->score));
	//		best_val = p_node->score;
	//		best_row = p_node->row;
	//		best_col = p_node->col;
	//	}

	//	if (p_node->p_parent->called_branch_cnt >= p_node->p_parent->vp_branch_nodes.size()) {
	//		SetEvent(p_node->pGame->hWaitEvent);
	//	}
	//	return;
	//}
	//p_node->p_parent->called_branch_cnt++;
	//if (p_node->player == 'C') {
	//	p_node->p_parent->score = max(p_node->p_parent->score, p_node->score);
	//	if (p_node->p_parent->called_branch_cnt >= p_node->p_parent->vp_branch_nodes.size()) {
	//		return_minimax(p_node->p_parent);
	//	}
	//	return;
	//}
	//else {
	//	p_node->p_parent->score = min(p_node->p_parent->score, p_node->score);
	//	if (p_node->p_parent->called_branch_cnt >= p_node->p_parent->vp_branch_nodes.size()) {
	//		return_minimax(p_node->p_parent);
	//	}
	//	return;
	//}
}

void Game::node_cut(node_t* const p_node)
{
	for (node_t* p : p_node->vp_branch_nodes) {
		node_cut(p);
	}
	mr_Node.Return(p_node);
}

void Game::play_game(bool opponent_first, bool is_c_to_c)
{
	CHANGE_BIT(m_board.m256i_u8[ST1_8], ST1::IS_C_X, !opponent_first);
	CHANGE_BIT(m_board.m256i_u8[ST1_8], ST1::IS_MY_TURN_NOW, !opponent_first);
	CHANGE_BIT(m_board.m256i_u8[ST1_8], ST1::IS_C2C, is_c_to_c);
	m_board.m256i_u8[NUM_TURN8] = 0;
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
			if (make_computer_move_m(&p)) {
				cout << "コンピュータが" << to_string(p.first + 1)
					<< " " << to_string(p.second + 1) << "を指しました。" << endl;
			}
			else {
				cout << "コンピュータはパスしました。" << endl;
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
				if (!get_valid_moves_m(m_board).m256i_u64[mIndex::BB_M64])
				{
					cout << "プレイヤー " << (char)(CHECK_BIT(m_board.m256i_u8[ST1_8], ST1::IS_C_X) ? 'C' : 'X')
						<< "パスになります。何かキーを押してください。" << endl;
					(void)_getch();
					break;
				}
				cout << "プレイヤー " << (char)(CHECK_BIT(m_board.m256i_u8[ST1_8], ST1::IS_C_X) ? 'C' : 'X')
					<< "、入力して下さい(列 行)。 (終了する場合は 0 0 と入力):";
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
				if ((m_board.m256i_u64[mIndex::BB_P64] | m_board.m256i_u64[mIndex::BB_O64]) & (0x8000000000000000 >> ((row - 1) * N + col - 1))) {
					cout << "そのマスは既に選択されています。" << endl;
					display_board_m(m_board);
					continue;
				}
				__m256i board_tmp = m_board;
				board_tmp.m256i_u64[mIndex::BB_M64] = 0x8000000000000000 >> ((row - 1) * N + col - 1);
				if (!is_valid_move_m(board_tmp))
				{
					cout << "そのマスは選択できません。" << endl;
					display_board_m(m_board);
					continue;
				}
				m_board = okuhara::flip256(board_tmp);
				break;
			}
		}
		// switch player
		change_turn_m(m_board);
	}
	if (is_draw_m(m_board)) {
		display_board_m(m_board);
		uint64_t cntX;
		uint64_t cntC;

		if (CHECK_BIT(m_board.m256i_u8[mIndex::ST1_8], ST1::IS_C_X)) {
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

		if (CHECK_BIT(m_board.m256i_u8[mIndex::ST1_8], ST1::IS_C_X)) {
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
			<< (cntC < cntX ? "X":"C") << "の勝ちです！\r\n" << endl;
	}

	cout << "コンピューターのトータル思考時間は" << fixed << setprecision(3)
		<< (double)TotalTime.QuadPart / 1000000.0 << " seconds.です。" << endl;
}

bool Game::is_draw_m(const __m256i m)
{
	if (is_game_over_m(m)) {
		return __popcnt64(m.m256i_u64[mIndex::BB_P64]) == __popcnt64(m.m256i_u64[mIndex::BB_O64]);
	}
	return false;
}

__m256i __vectorcall Game::change_turn_m(const __m256i m)
{
	__m256i tmp = _mm256_permute4x64_epi64(m, _MM_SHUFFLE(2, 3, 0, 1));
	tmp.m256i_u8[mIndex::ST1_8] = TOGGLE_BIT(m.m256i_u8[mIndex::ST1_8], ST1::IS_MY_TURN_NOW);
	return tmp;
}

bool Game::is_draw_bb(const bb_t bb_X, const bb_t bb_C) const
{
	if (is_game_over_bb(bb_X, bb_C))
	{
		return get_count_bb(bb_X) == get_count_bb(bb_C);
	}
	return false;
}

bool Game::is_win_m(const __m256i m)
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

bool Game::is_win_bb(const bb_t bb_X, const bb_t bb_C, char player) const
{
	if (is_game_over_bb(bb_X, bb_C)) {
		if (player == 'X') {
			return get_count_bb(bb_X) > get_count_bb(bb_C);
		}
		else {
			return get_count_bb(bb_C) > get_count_bb(bb_X);
		}
	}
	return false;
}

bool Game::is_game_over_m(__m256i m)
{
	return !(existing_valid_moves_m(m) 
		|| existing_valid_moves_m(change_turn_m(m)));
}

bool Game::is_game_over_bb(const bb_t bb_X, const bb_t bb_C) const
{
	// 両プレイヤーともに打てる手があるかどうかを調べる
	return !(existing_valid_moves_bb(bb_X, bb_C, 'X')
		|| existing_valid_moves_bb(bb_X, bb_C, 'C'));
}

bool __vectorcall Game::is_valid_move_m(const __m256i m)const noexcept
{
	// moveが1以外の場合、想定外なのでメッセージボックスを出す。
	if (__popcnt64(m.m256i_u64[mIndex::BB_M64]) != 1) {
		_D("__popcnt64.Error.");
		MessageBoxA(NULL, "moveが1個以外の個数です。", "検証", MB_ICONEXCLAMATION);
		return false;
	}

	// 既に石が置かれている場合はfalseを返す
	if (m.m256i_u64[mIndex::BB_P64] & m.m256i_u64[mIndex::BB_M64]
		|| m.m256i_u64[mIndex::BB_O64] & m.m256i_u64[mIndex::BB_M64]) {
		return false;
	}

	// placeを中心に各方向探索。posが判定対象ビット位置。
	for (int dir = 0; dir < 8; dir++) {
		unsigned long index;
		_BitScanReverse64(&index, m.m256i_u64[mIndex::BB_M64]);
		uint64_t pos = shift_table[index][dir];

		bool found_opponent = false;
		while (pos & m.m256i_u64[mIndex::BB_O64]) {
			found_opponent = true;
			unsigned long index;
			_BitScanReverse64(&index, pos);
			pos = shift_table[index][dir];
		}
		if (found_opponent && pos & m.m256i_u64[mIndex::BB_P64]) {
			return true;
		}
	}
	return false;
}

bool Game::is_valid_move_bb(
	const bb_t src_X
	, const bb_t src_C
	, const int row
	, const int col
	, const char player)const noexcept
{
	// (row, col)が盤面の範囲外の場合はfalseを返す
	if (row < 0 || row >= N || col < 0 || col >= N)
		return false;

	// 駒を置く
	bb_t place = 0x8000000000000000 >> (row * N + col);

	// (row, col)に既に石が置かれている場合はfalseを返す
	if (place & src_X || place & src_C) {
		return false;
	}

	bb_t player_board = (player == 'X') ? src_X : src_C;
	bb_t opponent_board = (player == 'X') ? src_C : src_X;

	// placeを中心に各方向探索。posが判定対象ビット位置。
	for (int dir = 0; dir < 8; dir++) {
		unsigned long index;
		_BitScanReverse64(&index, place);
		bb_t pos = shift_table[index][dir];

		bool found_opponent = false;
		while (pos & opponent_board) {
			found_opponent = true;
			unsigned long index;
			_BitScanReverse64(&index, pos);
			 pos = shift_table[index][dir];
		}
		if (found_opponent && pos & player_board) {
			return true;
		}
	}
	return false;
}

bool Game::is_valid_move(const board_t* const p_board, int row, int col, char player)const noexcept
{
	// (row, col)が盤面の範囲外の場合はfalseを返す
	if (row < 0 || row >= N || col < 0 || col >= N)
		return false;

	// (row, col)に既に石が置かれている場合はfalseを返す
	if ((*p_board)[row][col] != ' ')
		return false;

	// リバーシのルールに基づいて、(row, col)がchにとって有効な手であるか判断
	const int dx[] = { -1, -1, -1, 0, 1, 1, 1, 0 };
	const int dy[] = { -1, 0, 1, 1, 1, 0, -1, -1 };
	char opponent = player == 'X' ? 'C' : 'X';

	for (int dir = 0; dir < 8; dir++) {
		int x = row + dx[dir];
		int y = col + dy[dir];
		bool found_opponent = false;

		while (x >= 0 && x < N && y >= 0 && y < N && (*p_board)[x][y] == opponent) {
			found_opponent = true;
			x += dx[dir];
			y += dy[dir];
		}
		if (found_opponent && x >= 0 && x < N && y >= 0 && y < N && (*p_board)[x][y] == player)
			return true;
	}

	return false;
}

__m256i __vectorcall Game::get_valid_moves_m(const __m256i m) const
{
	return okuhara::get_moves256(m);
}

vector<pair<int, int>> Game::
get_valid_moves_bb(const bb_t bb_X, const bb_t bb_C, const char player)const
{
	std::vector<std::pair<int, int>> valid_moves;

	// リバーシのルールに基づいて、次に打てる手を探索
	for (int row = 0; row < N; row++) {
		for (int col = 0; col < N; col++) {
			if (is_valid_move_bb(bb_X, bb_C, row, col, player)) {
				valid_moves.emplace_back(row, col);
			}
		}
	}
	return valid_moves;
}

vector<pair<int, int>> Game::
get_valid_moves(const board_t* const p_board, const char ch)const
{
	vector<pair<int, int>> valid_moves;

	// リバーシのルールに基づいて、次に打てる手を探索
	for (int row = 0; row < N; row++)
		for (int col = 0; col < N; col++)
			if (is_valid_move(p_board, row, col, ch))
				valid_moves.emplace_back(row, col);

	return valid_moves;
}

inline bool Game::existing_valid_moves_m(const __m256i m) {
	return okuhara::get_moves256(m).m256i_u64[mIndex::BB_M64];
}

bool Game::existing_valid_moves_bb(const bb_t bb_X, const bb_t bb_C, const char player) const noexcept
{
	// リバーシのルールに基づいて、次に打てる手を探索
	for (int row = 0; row < N; row++)
		for (int col = 0; col < N; col++)
			if (is_valid_move_bb(bb_X, bb_C, row, col, player))
				return true;
	return false;
}

Game::bb_t Game::transfer(bb_t put, unsigned k)
{
	switch (k){
	case 0: //上
		return (put << 8) & 0xffffffffffffff00;
	case 1: //右上
		return (put << 7) & 0x7f7f7f7f7f7f7f00;
	case 2: //右
		return (put >> 1) & 0x7f7f7f7f7f7f7f7f;
	case 3: //右下
		return (put >> 9) & 0x007f7f7f7f7f7f7f;
	case 4: //下
		return (put >> 8) & 0x00ffffffffffffff;
	case 5: //左下
		return (put >> 7) & 0x00fefefefefefefe;
	case 6: //左
		return (put << 1) & 0xfefefefefefefefe;
	case 7: //左上
		return (put << 9) & 0xfefefefefefefe00;
	default:
		return 0;
	}
	MAKEWORD(0, 0);
}

vector<pair<int, int>> Game::extract_coordinates(const bb_t m) const
{
	vector<pair<int, int>> coordinates;
	uint64_t bits = m;
	while (bits) {
		unsigned long pos;
		_BitScanForward64(&pos, bits);
		coordinates.emplace_back(pos / N, pos % N);
		bits ^= 1ULL << pos;  // clear the bit we just processed
	}
	return coordinates;
}

void Game::init_shift_table()
{
	for (int row = 0; row < N; ++row) {
		for (int col = 0; col < N; ++col) {
			unsigned long index = row * N + col;
			bb_t bb = 0x0000000000000001ULL << (row * N + col);
			shift_table[index][0] = (bb << 8); // UP
			shift_table[index][1] = (bb & 0xfefefefefefefefe) << 7; // RIGHT_UP
			shift_table[index][2] = (bb & 0xfefefefefefefefe) >> 1; // RIGHT
			shift_table[index][3] = (bb & 0xfefefefefefefefe) >> 9; // RIGHT_DOWN
			shift_table[index][4] = (bb >> 8); // DOWN
			shift_table[index][5] = (bb & 0x7f7f7f7f7f7f7f7f) >> 7; // LEFT_DOWN
			shift_table[index][6] = (bb & 0x7f7f7f7f7f7f7f7f) << 1; // LEFT
			shift_table[index][7] = (bb & 0x7f7f7f7f7f7f7f7f) << 9; // LEFT_UP
		}
	}
}

void Game::update_board_bb(
	_Out_ bb_t& dst_X
	, _Out_ bb_t& dst_C
	, _In_ const bb_t src_X
	, _In_ const bb_t src_C
	, _In_ const int row
	, _In_ const int col
	, _In_ const char player) const noexcept
{
	// 駒を置く
	const bb_t place = 0x8000000000000000ULL >> (row * N + col);
	if (player == 'X') {
		dst_X = src_X | place;
		dst_C = src_C & ~place;
	}
	else {
		dst_C = src_C | place;
		dst_X = src_X & ~place;
	}

	// リバーシのルールに基づいて、盤面を更新
	bb_t player_board = (player == 'X') ? dst_X : dst_C;
	bb_t opponent_board = (player == 'X') ? dst_C : dst_X;

	bb_t flippset(0ULL);
	for (int dir = 0; dir < 8; dir++) {
		bb_t flipped(0ULL);
		bb_t save_point(0ULL);
		bb_t pos = place;
		for (int i = 0; i < N; i++) { // 最大Nマス動かすことができる
			// shiftに関してはヘッダーの説明を参照。
			unsigned long index;
			_BitScanReverse64(&index, pos);
			pos = shift_table[index][dir] & opponent_board;

			if (pos == 0) {
				break;
			}
			else {
				save_point = pos;
				flipped |= pos;
			}
		}
		unsigned long index;
		_BitScanReverse64(&index, save_point);

		if (save_point && shift_table[index][dir] & player_board) {
			flippset |= flipped;
		}
	}

	if (flippset) {
		if (player == 'X') {
			dst_X |= flippset;
			dst_C &= ~flippset;
		}
		else {
			dst_C |= flippset;
			dst_X &= ~flippset;
		}
	}
}

void Game::update_board(board_t* const p_board, const int row, const int col, const char ch)const
{
	// 駒を置く
	(*p_board)[row][col] = ch;

	// リバーシのルールに基づいて、盤面を更新
	const int dx[] = { -1, -1, -1, 0, 1, 1, 1, 0 };
	const int dy[] = { -1, 0, 1, 1, 1, 0, -1, -1 };
	char opponent = ch == 'X' ? 'C' : 'X';
	for (int dir = 0; dir < 8; dir++) {
		int x = row + dx[dir];
		int y = col + dy[dir];
		bool found_opponent = false;
		while (x >= 0 && x < N && y >= 0 && y < N && (*p_board)[x][y] == opponent) {
			found_opponent = true;
			x += dx[dir];
			y += dy[dir];
		}
		if (found_opponent && x >= 0 && x < N && y >= 0 && y < N && (*p_board)[x][y] == ch) {
			x -= dx[dir];
			y -= dy[dir];
			while (x != row || y != col) {
				(*p_board)[x][y] = ch;
				x -= dx[dir];
				y -= dy[dir];
			}
		}
	}
}

inline void Game::copy_board_bb(bb_t& dist, const bb_t src) const noexcept
{
	dist = src;
}

void Game::copy_board(board_t* const pdist, const board_t* const psrc)const
{
	std::memcpy(pdist, psrc, sizeof(board_t));
}

inline int Game::get_count_bb(const bb_t bb) const noexcept
{
	// ビットカウントを行う
	return (int)__popcnt64(bb);
}

int Game::get_count(const board_t* const p_board, const char player) const noexcept
{
	// playerのディスクの数をカウント
	int count = 0;
	for (int i = 0; i < sizeof(board_t); ++i) {
		if (*((char*)p_board + i) == player) {
			count++;
		}
	}
	return count;
}

int Game::evaluate_bb(const bb_t bb_X, const bb_t bb_C, const char player,const depth_t depth) const
{
	// 評価値を計算
	if (player == 'X') {
		return get_count_bb(bb_X) - get_count_bb(bb_C) - (int)depth;
	}
	else {
		return get_count_bb(bb_C) - get_count_bb(bb_X) - (int)depth;
	}
}

int Game::evaluate(const board_t* const p_board, char player)const
{
	// 評価値を計算
	return get_count(p_board, player) - get_count(p_board, player == 'C' ? 'X' : 'C');
}

int Game::evaluate_bbG(const bb_t bb_X, const bb_t bb_C, const char player,const depth_t depth)  const
{
	// 'C'から見て、石の数が0になっていると、大幅な評価値を返す。
	int countC = (int)__popcnt64(bb_C);
	int countX = (int)__popcnt64(bb_X);
	int AddScore;
	if (countC == 0) {
		AddScore = player == 'C' ? -INF : INF;
	}
	if (countX == 0) {
		AddScore = player == 'X' ? -INF : INF;
	}
	//return countC-countX;
	// 四隅に置かれると減点、置くと加点とする。
	// 四隅を表すビットマスク
	constexpr bb_t mask_corners = 0x8100000000000081;
	// 'X'の石がある四隅
	bb_t corners_X = bb_X & mask_corners;
	// 'C'の石がある四隅
	bb_t corners_C = bb_C & mask_corners;
	// スコア計算
	AddScore = 127 * ((int)__popcnt64(corners_C) -(int)__popcnt64(corners_X));
	if (player == 'X')
		AddScore = -AddScore;
	return AddScore + player == 'C' ? countC - countX:countX-countC;
	//// 'C'から見て、隅に置かれて無い時に、自身が斜め隣に
	//// 置いたら減点、相手が置いたら加点
	//constexpr bb_t mask_0_0 = 0x8000000000000000;
	//constexpr bb_t mask_1_1 = 0x0200000000000000;
	//constexpr bb_t mask_7_0 = 0x0000000000000080;
	//constexpr bb_t mask_6_1 = 0x0000000000001000;
	//constexpr bb_t mask_0_7 = 0x0100000000000000;
	//constexpr bb_t mask_1_6 = 0x0400000000000000;
	//constexpr bb_t mask_7_7 = 0x0000000000000001;
	//constexpr bb_t mask_6_6 = 0x0000000000010000;

	//if (!(bb_X & mask_0_0 || bb_C & mask_0_0)) {
	//	AddScore += (bb_X & mask_1_1) ? -50 : (bb_C & mask_1_1) ? 50 : 0;
	//}
	//if (!(bb_X & mask_7_0 || bb_C & mask_7_0)) {
	//	AddScore += (bb_X & mask_6_1) ? -50 : (bb_C & mask_6_1) ? 50 : 0;
	//}
	//if (!(bb_X & mask_0_7 || bb_C & mask_0_7)) {
	//	AddScore += (bb_X & mask_1_6) ? -50 : (bb_C & mask_1_6) ? 50 : 0;
	//}
	//if (!(bb_X & mask_7_7 || bb_C & mask_7_7)) {
	//	AddScore += (bb_X & mask_6_6) ? -50 : (bb_C & mask_6_6) ? 50 : 0;
	//}

	//// 'C'から見て、隅に置かれて無い時に、自身が縦横の隣に
	//// 置いたら減点、相手が置いたら加点
	//constexpr bb_t mask_1_0 = 0x4000000000000000;
	//constexpr bb_t mask_0_1 = 0x0200000000000000;
	//constexpr bb_t mask_6_0 = 0x0000000000000040;
	//constexpr bb_t mask_7_1 = 0x0000000000000100;
	//constexpr bb_t mask_0_6 = 0x0400000000000000;
	//constexpr bb_t mask_1_7 = 0x0002000000000000;
	//constexpr bb_t mask_6_7 = 0x0000000000010000;
	//constexpr bb_t mask_7_6 = 0x0000000000020000;

	//if (!(bb_X & mask_0_0 || bb_C & mask_0_0)) {
	//	AddScore += (bb_X & mask_1_0) ? -20 : (bb_C & mask_1_0) ? 20 : 0;
	//	AddScore += (bb_X & mask_0_1) ? -20 : (bb_C & mask_0_1) ? 20 : 0;
	//}
	//if (!(bb_X & mask_7_0 || bb_C & mask_7_0)) {
	//	AddScore += (bb_X & mask_6_0) ? -20 : (bb_C & mask_6_0) ? 20 : 0;
	//	AddScore += (bb_X & mask_7_1) ? -20 : (bb_C & mask_7_1) ? 20 : 0;
	//}
	//if (!(bb_X & mask_0_7 || bb_C & mask_0_7)) {
	//	AddScore += (bb_X & mask_1_7) ? -20 : (bb_C & mask_1_7) ? 20 : 0;
	//	AddScore += (bb_X & mask_0_6) ? -20 : (bb_C & mask_0_6) ? 20 : 0;
	//}
	//if (!(bb_X & mask_7_7 || bb_C & mask_7_7)) {
	//	AddScore += (bb_X & mask_6_7) ? -20 : (bb_C & mask_6_7) ? 20 : 0;
	//	AddScore += (bb_X & mask_7_6) ? -20 : (bb_C & mask_7_6) ? 20 : 0;
	//}

	//// 序盤は中央付近に置くとポイント＋とした。
	//if (countC + countX - 4 < 16) {
	//	// 中央領域のビットマスクはどちらか選択。
	//	// 中央4x4領域を表すビットマスク
	//	constexpr bb_t mask_center = 0x003C24243C000000;
	//	
	//	// 中央6x6領域を表すビットマスク
	//	//constexpr bb_t mask_center = 0x007E424242427E00;

	//	AddScore += (int)__popcnt64(bb_C & mask_center) - (int)__popcnt64(bb_X & mask_center);
	//}
	//// 'C'から見たスコアを返す。
	//return countC - countX + AddScore;
}

int Game::evaluate_bbS(const bb_t bb_X, const bb_t bb_C) const noexcept
{
	// 評価値を計算
	return (int)__popcnt64(bb_C) - (int)__popcnt64(bb_X);
}

int Game::evaluateS(const board_t* const p_board) const noexcept
{
	// 評価値を計算
	return get_count(p_board, 'C') - get_count(p_board, 'X');
}

int Game::evaluate_by_turn(const __m256i m) const noexcept
{
	// 評価値を計算
	return (int)__popcnt64(m.m256i_i64[0]) - (int)__popcnt64(m.m256i_i64[1]);
}

int Game::alphabeta_m(const __m256i bbm) const
{
	return 0;
}

int Game::alphabeta_bb(
	const bb_t bb_X
	, const bb_t bb_C
	, const char player
	, const depth_t depth
	, int alpha
	, int beta) const
{
	if (depth >= SECOND_DEPTH)
	{
		int score = evaluate_bbS(bb_X, bb_C);
		return score;
	}

	char next_player = player == 'X' ? 'C' : 'X';
	std::vector<std::pair<int, int>> pairs = move(get_valid_moves_bb(bb_X, bb_C, next_player));

	if (pairs.empty()) {
		next_player = next_player == 'X' ? 'C' : 'X';
		pairs = move(get_valid_moves_bb(bb_X, bb_C, next_player));
		if (pairs.empty()) {
			return evaluate_bbS(bb_X, bb_C) > 0 ? INF - (int)depth : -INF + (int)depth;
		}
	}

	if ('C' == next_player) {
		for (const auto &a_pair : pairs) {
			bb_t next_bb_X;
			bb_t next_bb_C;
			update_board_bb(next_bb_X, next_bb_C, bb_X, bb_C, a_pair.first, a_pair.second, next_player);
			alpha = std::max(alpha, alphabeta_bb(next_bb_X, next_bb_C, next_player, depth + 1, alpha, beta));
			if (alpha >= beta) {
				break;
			}
		}
		return alpha;
	}
	else {
		for (const auto& a_pair : pairs) {
			bb_t next_bb_X;
			bb_t next_bb_C;
			update_board_bb(next_bb_X, next_bb_C, bb_X, bb_C, a_pair.first, a_pair.second, next_player);
			beta = std::min(beta, alphabeta_bb(next_bb_X, next_bb_C, next_player, depth + 1, alpha, beta));
			if (alpha >= beta) {
				break;
			}
		}
		return beta;
	}
}

int Game::alphabeta(
	const board_t* const p_board
	, const char player
	, int depth
	, int alpha
	, int beta)const
{
	if (depth >= MAX_DEPTH)
	{
		int score = evaluateS(p_board);
		return score;
	}

	char next_player = player == 'X' ? 'C' : 'X';
	vector<pair<int, int>> pairs = move(get_valid_moves(p_board, next_player));
	if (pairs.empty()) {
		next_player = next_player == 'X' ? 'C' : 'X';
		pairs = move(get_valid_moves(p_board, next_player));
		if (pairs.empty()) {
			_D("勝敗確定");
			return evaluateS(p_board) > 0 ? INF - (int)depth : -INF + (int)depth;
		}
	}

	if ('C' == next_player) {
		for (const auto& a_pair : pairs) {
			board_t next_board;
			copy_board(&next_board, p_board);
			update_board(&next_board, a_pair.first, a_pair.second, next_player);
			alpha = std::max(alpha
				, alphabeta(&next_board, next_player, depth + 1, alpha, beta));
			if (alpha >= beta) {
				//_D("alpha cut");
				break;
			}
		}
		return alpha;
	}
	else {
		for (const auto& a_pair : pairs) {
			board_t next_board;
			copy_board(&next_board, p_board);
			update_board(&next_board, a_pair.first, a_pair.second, next_player);
			beta = std::min(beta
				, alphabeta(&next_board, next_player, depth + 1, alpha, beta));
			if (alpha >= beta) {
				//_D("beta cut");
				break;
			}
		}
		return beta;
	}
}

int Game::minimax(const board_t* const p_board, const char player, int depth)const
{
	if (depth >= SECOND_DEPTH)
	{
		int score = evaluateS(p_board);
		return score;
	}

	char next_player = player == 'X' ? 'C' : 'X';
	vector<pair<int, int>> pairs = move(get_valid_moves(p_board, next_player));
	if (pairs.empty()) {
		next_player = next_player == 'X' ? 'C' : 'X';
		pairs = move(get_valid_moves(p_board, next_player));
		if (pairs.empty()) {
			_D("勝敗確定");
			int score = evaluateS(p_board) > 0 ? INF - (int)depth : -INF + (int)depth;
			return score;
		}
	}

	if ('C' == next_player) {
		int score = -INF;
		for (const auto& a_pair : pairs) {
			board_t next_board{};
			copy_board(&next_board, p_board);
			update_board(&next_board, a_pair.first, a_pair.second, next_player);
			score = std::max(score, minimax(&next_board, next_player, depth + 1));
		}
		return score;
	}
	else {
		int score = INF;
		for (const auto& a_pair : pairs) {
			board_t next_board{};
			copy_board(&next_board, p_board);
			update_board(&next_board, a_pair.first, a_pair.second, next_player);
			score = std::min(score, minimax(&next_board, next_player, depth + 1));
		}
		return score;
	}
}

int Game::minimax_cnf(const board_t* const p_board, const char player, int depth) const
{
	if (depth >= MAX_DEPTH)
	{
		int score = evaluate(p_board, 'C');
		return score;
	}

	char next_player = player == 'X' ? 'C' : 'X';
	vector<pair<int, int>> pairs = move(get_valid_moves(p_board, next_player));
	if (pairs.empty()) {
		next_player = next_player == 'X' ? 'C' : 'X';
		pairs = move(get_valid_moves(p_board, next_player));
		if (pairs.empty()) {
			_D("勝敗確定");
			return evaluateS(p_board) > 0 ?
				INF - depth
				: -INF + depth;
		}
	}

	if ('C' == next_player) {
		int score = -INF;
		for (const auto& a_pair : pairs) {
			board_t next_board{};
			copy_board(&next_board, p_board);
			update_board(&next_board, a_pair.first, a_pair.second, next_player);
			score = std::max(score, minimax_cnf(&next_board, next_player, depth + 1));
		}
		return score;
	}
	else {
		int score = INF;
		for (const auto& a_pair : pairs) {
			board_t next_board{};
			copy_board(&next_board, p_board);
			update_board(&next_board, a_pair.first, a_pair.second, next_player);
			score = std::min(score, minimax_cnf(&next_board, next_player, depth + 1));
		}
		return score;
	}
}

int Game::minimax_bb(const bb_t bb_X, const bb_t bb_C, const char player, const depth_t depth) const
{
	if (depth >= SECOND_DEPTH)
	{
		int score = evaluate_bbS(bb_X, bb_C);
		return score;
	}

	char next_player = player == 'X' ? 'C' : 'X';
	vector<pair<int, int>> pairs = move(get_valid_moves_bb(bb_X, bb_C, next_player));
	if (pairs.empty()) {
		next_player = next_player == 'X' ? 'C' : 'X';
		pairs = move(get_valid_moves_bb(bb_X, bb_C, next_player));
		if (pairs.empty()) {
			_D("勝敗確定");
			return evaluate_bbS(bb_X, bb_C) > 0 ? INF - (int)depth : -INF + (int)depth;
		}
	}

	if ('C' == next_player) {
		int score = -INF;
		for (const auto& a_pair : pairs) {
			bb_t next_bb_X;
			bb_t next_bb_C;
			update_board_bb(next_bb_X, next_bb_C, bb_X, bb_C, a_pair.first, a_pair.second, next_player);
			int rval = minimax_bb(next_bb_X, next_bb_C, next_player, depth + 1);
			score = std::max(score, rval);
		}
		return score;
	}
	else {
		int score = INF;
		for (const auto& a_pair : pairs) {
			bb_t next_bb_X;
			bb_t next_bb_C;
			update_board_bb(next_bb_X, next_bb_C, bb_X, bb_C, a_pair.first, a_pair.second, next_player);
			int rval = minimax_bb(next_bb_X, next_bb_C, next_player, depth + 1);
			score = std::min(score,rval );
		}
		return score;
	}
}

int Game::minimax_bb_cnf(const bb_t bb_X, const bb_t bb_C, const char player, const depth_t depth) const
{
	if (depth >= MAX_DEPTH)
	{
		int score = evaluate_bbS(bb_X, bb_C);
		return score;
	}

	char next_player = player == 'X' ? 'C' : 'X';
	vector<pair<int, int>> pairs = move(get_valid_moves_bb(bb_X, bb_C, next_player));
	if (pairs.empty()) {
		next_player = next_player == 'X' ? 'C' : 'X';
		pairs = move(get_valid_moves_bb(bb_X, bb_C, next_player));
		if (pairs.empty()) {
			_D("勝敗確定");
			return evaluate_bbS(bb_X, bb_C) > 0 ?
				INF - (depth - FIRST_DEPTH)
				: -INF + (depth - FIRST_DEPTH);
		}
	}

	if ('C' == next_player) {
		int score = -INF;
		for (const auto& a_pair : pairs) {
			bb_t next_bb_X;
			bb_t next_bb_C;
			update_board_bb(next_bb_X, next_bb_C, bb_X, bb_C, a_pair.first, a_pair.second, next_player);
			int rval = minimax_bb_cnf(next_bb_X, next_bb_C, next_player, depth + 1);
			score = std::max(score, rval);
		}
		return score;
	}
	else {
		int score = INF;
		for (const auto& a_pair : pairs) {
			bb_t next_bb_X;
			bb_t next_bb_C;
			update_board_bb(next_bb_X, next_bb_C, bb_X, bb_C, a_pair.first, a_pair.second, next_player);
			int rval = minimax_bb_cnf(next_bb_X, next_bb_C, next_player, depth + 1);
			score = std::min(score, rval);
		}
		return score;
	}
}

bool Game::make_computer_move_m(pair<int, int>* p_pair)
{
	using namespace debug_fnc;
	best_val = -INF;
	best_move_m = 0ULL;

	__m256i POMovesS = okuhara::get_moves256( m_board);

	if (!POMovesS.m256i_u64[mIndex::BB_M64])
		return false;

	cout << "コンピューター思考中・・・" << endl;

	dout(string() + __FILE__ + "(" + to_string(__LINE__) + "): "
		+ "Turn :" + to_string(m_board.m256i_u8[mIndex::NUM_TURN8])
		+ " Num of first nodes:"
		+ std::to_string(__popcnt64(POMovesS.m256i_u64[BB_M64])));

	p_root_node = mr_Node.Lend();
	p_root_node->m_board = m_board;
	p_root_node->m_board.m256i_u64[OP64]
		= CHANGE_BIT(p_root_node->m_board.m256i_u64[OP64], ST1::IS_MY_TURN_NOW, false);
	p_root_node->m_board.m256i_u8[DEPTH8] = 0;
	p_root_node->m_board.m256i_u64[mIndex::BB_M64] = 0ULL;
	p_root_node->called_branch_cnt = 0;
	p_root_node->p_parent = nullptr;
	p_root_node->vp_branch_nodes = {};
	p_root_node->pGame = this;
	p_root_node->pwk = NULL;

	if (!ResetEvent(hWaitEvent)) {
		EOut;
		return false;
	}

	for (unsigned long index; _BitScanReverse64(&index,POMovesS.m256i_u64[BB_M64]);) {

		node_t* pNode = mr_Node.Lend();
		pNode->m_board = POMovesS;
		pNode->m_board.m256i_u64[mIndex::BB_M64] = 1ULL << index;
		POMovesS.m256i_u64[mIndex::BB_M64]
			= RESET_BIT(POMovesS.m256i_u64[mIndex::BB_M64], index);
		pNode->m_board = okuhara::flip256(pNode->m_board);
		pNode->m_board.m256i_u8[mIndex::ST1_8]
			= SET_BIT(pNode->m_board.m256i_u8[mIndex::ST1_8], ST1::IS_MY_TURN_NOW);
		pNode->m_board.m256i_u8[mIndex::DEPTH8] = p_root_node->m_board.m256i_u8[mIndex::DEPTH8]+1;
		pNode->score = -INF;
		pNode->called_branch_cnt = 0;
		pNode->p_parent = p_root_node;
		pNode->vp_branch_nodes.clear();
		pNode->pGame = p_root_node->pGame;
		p_root_node->vp_branch_nodes.push_back(pNode);

		dout(string() + __FILE__ + "(" + to_string(__LINE__) + "): "
			+ "Turn :" + to_string(pNode->m_board.m256i_u8[mIndex::NUM_TURN8])
			+ " first node(" + to_string((63 - index) % 8 + 1) + ","
			+ to_string((63-index) / 8 + 1) + ")");
		if (!(pNode->pwk = CreateThreadpoolWork(pfnwkMiniMax, pNode, &*pcbe))) {
			EOut;
			return false;
		}
		SubmitThreadpoolWork(pNode->pwk);
	}
	DWORD result;
	if (!(result = WaitForSingleObject(hWaitEvent, INFINITE) == WAIT_OBJECT_0))
	{
		_D("WaitForSingleObject.Error.");
		MessageBoxA(NULL, "WaitForSingleObjectエラー。", "検証", MB_ICONEXCLAMATION);
	}

	EnterCriticalSection(&cs);

#ifdef _DEBUG
	board_t bmn;
	conv_bbToBoard(&bmn, m_board.m256i_u64[mIndex::BB_P64], m_board.m256i_u64[mIndex::BB_O64]);
	int score_minimax = minimax_cnf(&bmn, 'X', 0);

	if (score_minimax != best_val) {
		MessageBoxA(NULL, "スコアが違います。", "検証", MB_ICONEXCLAMATION);
		int i = 0;
	}


	// 有効な選択肢が選択されたかチェック
	if (is_valid_move_m(m_board)) {
		MessageBoxA(NULL, "コンピューターは無効な手を選択しました。", "検証", MB_ICONEXCLAMATION);
		return false;
	}
#endif // _DEBUG
	m_board.m256i_u64[mIndex::BB_M64] = best_move_m;
	m_board = okuhara::flip256(m_board);
	LeaveCriticalSection(&cs);

	CloseThreadpoolCleanupGroupMembers(&*ptpcg, TRUE, NULL);
	node_cut(p_root_node);

	return true;
}

void Game::conv_bbToBoard(board_t* const p_board, const bb_t bb_X, const bb_t bb_C)const
{
	for (int i = 0; i < N; ++i) {
		for (int j = 0; j < N; ++j) {
			// ビット位置を計算
			uint64_t bitPos = 0x8000000000000000ULL >> (i * N + j);

			// 'X'または'C'のビットがセットされているかどうかを確認
			if (bb_X & bitPos) {
				(*p_board)[i][j] = 'X';
			}
			else if (bb_C & bitPos) {
				(*p_board)[i][j] = 'C';
			}
			else {
				(*p_board)[i][j] = ' ';
			}
		}
	}
}

void Game::convBoardTo_bb
(bb_t& bbX, bb_t& bbC, const board_t* const p_board)const
{
	// ビットボードを初期化
	bbX = 0ULL;
	bbC = 0ULL;

	for (int i = 0; i < N; ++i) {
		for (int j = 0; j < N; ++j) {
			// ビット位置を計算
			uint64_t bitPos = 0x8000000000000000ULL >> (i * N + j);

			// 'X'または'C'のセルがあるかどうかを確認
			if ((*p_board)[i][j] == 'X') {
				bbX |= bitPos;
			}
			else if ((*p_board)[i][j] == 'C') {
				bbC |= bitPos;
			}
		}
	}
}

bool Game::is_equal
(const board_t* const p_board, const bb_t bb_X, const bb_t bb_C) const
{
	// ビットボードを初期化
	bb_t temp_bb_X = 0ULL;
	bb_t temp_bb_C = 0ULL;

	for (int i = 0; i < N; ++i) {
		for (int j = 0; j < N; ++j) {
			// ビット位置を計算
			uint64_t bitPos = 0x8000000000000000ULL >> (i * N + j);

			// 'X'または'C'のセルがあるかどうかを確認
			if ((*p_board)[i][j] == 'X') {
				temp_bb_X |= bitPos;
			}
			else if ((*p_board)[i][j] == 'C') {
				temp_bb_C |= bitPos;
			}
		}
	}

	// ビットボードが等しいかどうかを確認
	return temp_bb_X == bb_X && temp_bb_C == bb_C;
}

Game::node_t::node_t() :
  pcs{ [this]() {
	InitializeCriticalSection(&cs);
	return &cs;	}()
	,
	[](CRITICAL_SECTION* pcs) {
		DeleteCriticalSection(pcs);
	}
}

{
}
