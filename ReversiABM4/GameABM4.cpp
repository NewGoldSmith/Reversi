/**
 * @file GameABMG.cpp
 * @brief ゲームクラスの実装
 * @author Gold Smith
 * @date 2023
 *
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 */
#include "GameABM4.h"
using namespace std;


Game::Game():
	ptpp{	[]() {
		PTP_POOL ptpp = CreateThreadpool(NULL);
		if (!ptpp){
			throw runtime_error(EOut);
		}
		SetThreadpoolThreadMaximum(ptpp, MAX_THREAD);
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

	, pfnwkMiniMax{	[](
	 _Inout_     PTP_CALLBACK_INSTANCE Instance,
	 _Inout_opt_ PVOID                 Context,
	 _Inout_     PTP_WORK              Work	){
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
			pNode->score = pNode->pGame->
				alphabeta(&pNode->board, pNode->player, 0, -INF, INF);
			if (pNode->score == BREAK_CODE) {
				return;
			}
			pNode->pGame->return_minimax(pNode);
			return;
		}

		char next_player = pNode->player == 'X' ? 'C' : 'X';
		vector<pair<int, int>> pairs = pNode->pGame->get_valid_moves(&pNode->board, next_player);
		if (pairs.empty()) {
			next_player = next_player == 'X' ? 'C' : 'X';
			pairs = pNode->pGame->get_valid_moves(&pNode->board, next_player);
			if (pairs.empty()) {
				// 勝敗確定
				_D("勝敗確定 (" + to_string(pNode->row + 1) + "," + to_string(pNode->col + 1) + ")");
				pNode->score = pNode->pGame->evaluate(&pNode->board, 'C', pNode->depth);
				pNode->pGame->return_minimax(pNode);
				return;
			}
		}

		if ('C' == next_player) {

			for (auto a_pair : pairs) {
				node_t* pNextNode = pNode->pGame->mr_Node.Lend();
				pNode->pGame->copy_board(&pNextNode->board, &pNode->board);
				pNode->pGame->update_board(&pNextNode->board, a_pair.first, a_pair.second, next_player);
				pNextNode->player = next_player;
				pNextNode->depth = pNode->depth + 1;
				pNextNode->row=a_pair.first;
				pNextNode->col=a_pair.second;
				pNextNode->score = -INF;
				pNextNode->called_children_cnt = 0;
				pNextNode->p_parent = pNode;
				pNextNode->vp_child_nodes = {};
				pNextNode->pGame = pNode->pGame;
				pNode->vp_child_nodes.push_back(pNextNode);

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

			for (auto a_pair : pairs) {
				node_t* pNextNode = pNode->pGame->mr_Node.Lend();
				pNode->pGame->copy_board(&pNextNode->board, &pNode->board);
				pNode->pGame->update_board(&pNextNode->board, a_pair.first, a_pair.second, next_player);
				pNextNode->player = next_player;
				pNextNode->depth = pNode->depth + 1;
				pNextNode->row=a_pair.first;
				pNextNode->col=a_pair.second;
				pNextNode->score = INF;
				pNextNode->called_children_cnt = 0;
				pNextNode->p_parent = pNode;
				pNextNode->vp_child_nodes = {};
				pNextNode->pGame = pNode->pGame;
				pNode->vp_child_nodes.push_back(pNextNode);

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
	}}

	, board()
{
	init_game();
	mr_Node.DebugString("mr_Node");
}

Game::~Game()
{
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

void Game::display_board()const
{
	cout << " ";
	for (int row = 0; row < N; row++)
		cout << " " << row + 1;
	cout << endl;
	for (int row = 0; row < N; row++) {
		cout << row + 1;
		for (int col = 0; col < N; col++)
			cout << "|" << board[row][col];
		cout << "|" << endl;
		cout << " ";
		for (int col = 0; col < N; col++)
			cout << "--";
		cout << "-" << endl;
	}
}


void Game::return_minimax( node_t* const p_child)
{
	unique_ptr<CRITICAL_SECTION, decltype(LeaveCriticalSection)*> lock_child = {
		[p_child]() {
			EnterCriticalSection(&p_child->cs);
			return &p_child->cs;
		}(),LeaveCriticalSection };

	unique_ptr<CRITICAL_SECTION, decltype(LeaveCriticalSection)*> lock_parent = {
	[p_child]() {
		EnterCriticalSection(&p_child->p_parent->cs);
		return &p_child->p_parent->cs;
		}(), LeaveCriticalSection
	};

	if (!p_child->p_parent->p_parent) {
		// p_child->p_parent->がルートノードの場合

		p_child->p_parent->called_children_cnt++;

		unique_ptr<CRITICAL_SECTION, decltype(LeaveCriticalSection)*> lock_game = {
			[this]() {EnterCriticalSection(&cs); return &cs; }()
			,LeaveCriticalSection };

		if (best_val < p_child->score) {
			_D("update score" + "(" + to_string(p_child->row+1) + "," 
				+ to_string(p_child->col+1) + "):" + to_string(p_child->score));
			best_val = p_child->score;
			best_row = p_child->row;
			best_col = p_child->col;
		}

		if (p_child->p_parent->called_children_cnt >= p_child->p_parent->vp_child_nodes.size()) {
			SetEvent(p_child->pGame->hWaitEvent);
		}
		return;
	}

	p_child->p_parent->called_children_cnt++;

	if ('C' == p_child->p_parent->player) {
		p_child->p_parent->score = max<int>(p_child->p_parent->score, p_child->score);
		if (p_child->p_parent->called_children_cnt >= p_child->p_parent->vp_child_nodes.size()) {
			return_minimax(p_child->p_parent);
		}
		return;
	}

	else {
		p_child->p_parent->score = min<int>(p_child->p_parent->score, p_child->score);
		if (p_child->p_parent->called_children_cnt >= p_child->p_parent->vp_child_nodes.size()) {
			return_minimax(p_child->p_parent);
		}
	}
}

void Game::node_cut(node_t* const p_node)
{
	for (node_t* p : p_node->vp_child_nodes) {
		node_cut(p);
	}
	mr_Node.Return(p_node);

}

void Game::play_game(bool human_first, bool two_player)
{
	char current_player = human_first ? 'X' : 'C';
	turn = 0;
	TotalTime = {};
	for (bool b_end(0); !b_end;) {
		display_board();
		turn++;
		cout << turn << "手目" << endl;
		int row, col;
		if (current_player == 'C' && !two_player) {
			std::pair<int, int> p;
			QueryPerformanceCounter(&StartingTime);
			if (make_computer_move(&p))
				cout << "コンピュータが" << to_string(p.first + 1)
				<< " " << to_string(p.second + 1) << "を指しました。" << endl;
			else
				cout << "コンピュータはパスしました。" << endl;
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
				if ((get_valid_moves(&board, 'X')).empty())
				{
					cout << "プレイヤー " << current_player
						<< "パスになります。何かキーを押してください。";
					string str;
					getline(cin, str);
					display_board();
					cout << endl;
					break;
				}
				cout << "プレイヤー " << current_player << "、入力して下さい(行 列)。";
				cout << " (終了する場合は 0 0 と入力):" ;
				cin >> row >> col;
				if (cin.fail()) {
					cin.clear();
					cout << "無効な入力です。再度入力してください。" << endl;
					display_board();
					continue;
				}
				if (row == 0 && col == 0)
				{
					b_end = true;
					continue;
				}
				if (row < 1 || row > N || col < 1 || col > N) {
					cout << "無効な手です。再度入力してください。" << endl;
					display_board();
					continue;
				}
				if (board[row - 1][col - 1] != ' ') {
					cout << "そのマスは既に選択されています。" << endl;
					display_board();
					continue;
				}
				if (!is_valid_move(&board, row - 1, col - 1, current_player))
				{
					cout << "そのマスは選択できません。" << endl;
					display_board();
					continue;
				}
				update_board(&board, row - 1, col - 1, current_player);
				break;
			}
		}
		if (is_win(&board, 'X')) {
			display_board();
			cout << "X:" << get_count(&board, 'X') 
				<< " C:" << get_count(&board, 'C') << " Xの勝ち!" << endl;
			break;
		}
		else if (is_win(&board, 'C')) {
			display_board();
			cout << "X:" << get_count(&board, 'X') << 
				" C:" << get_count(&board, 'C') << " Cの勝ち!" << endl;
			break;
		}
		else if (is_draw(&board)) {
			display_board();
			cout << "引き分けです!" << endl;
			break;
		}
		// switch player
		current_player = current_player == 'X' ? 'C' : 'X';
	}
	cout << "コンピューターのトータル思考時間は"	<< fixed << setprecision(3) 
		<< (double)TotalTime.QuadPart / 1000000.0 << " seconds.です。" << endl;
}

bool Game::is_draw(const board_t* const p_board)const
{
	if (is_game_over(p_board))
	{
		return get_count(p_board, 'X') == get_count(p_board, 'C');
	}
	return false;
}

bool Game::is_win(const board_t* const p_board, char player)const
{
	if (is_game_over(p_board)) {
		return get_count(p_board, player) >
			get_count(p_board, player == 'C' ? 'X' : 'C');
	}
	return false;
}

bool Game::is_game_over(const board_t* const p_board)const
{
	// 両プレイヤーともに打てる手があるかどうかを調べる
	return !(existing_valid_moves(p_board, 'X') 
		|| existing_valid_moves(p_board, 'C'));
}

bool Game::is_valid_move(const board_t* const p_board, int row, int col, char player)const
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
#pragma loop(hint_parallel(0))
	for (int dir = 0; dir < 8; dir++) {
		int x = row + dx[dir];
		int y = col + dy[dir];
		bool found_opponent = false;
#pragma loop(hint_parallel(0))
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

vector<pair<int, int>> Game::get_valid_moves(const board_t* const p_board, char ch)const
{
	vector<pair<int, int>> valid_moves;

	// リバーシのルールに基づいて、次に打てる手を探索
	for (int row = 0; row < N; row++)
		for (int col = 0; col < N; col++)
			if (is_valid_move(p_board, row, col, ch))
				valid_moves.emplace_back(row, col);

	return valid_moves;
}

bool Game::existing_valid_moves(const board_t* const p_board, char player) const
{
	// リバーシのルールに基づいて、次に打てる手を探索
	for (int row = 0; row < N; row++)
		for (int col = 0; col < N; col++)
			if (is_valid_move(p_board, row, col, player))
				return true;
	return false;
}

void Game::update_board(board_t* const p_board, int row, int col, char ch)const
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

void Game::copy_board(board_t* const pdist, const board_t* const psource)const
{
	for (int i(0); i < N; i++)
		for (int j(0); j < N; j++)
		{
			(*pdist)[i][j] = (*psource)[i][j];
		}
}

pair<int, int> Game::get_each_countXC(const board_t* const p_board)const
{
	// それぞれのディスクの数をカウント
	int x_count = 0;
	int c_count = 0;
	for (int row = 0; row < N; row++)
		for (int col = 0; col < N; col++)
			if ((*p_board)[row][col] == 'X')
				x_count++;
			else if ((*p_board)[row][col] == 'C')
				c_count++;

	// スコアを返す
	return make_pair(x_count, c_count);
}

int Game::get_count(const board_t* const p_board, const char player) const
{
	// playerのディスクの数をカウント
	int count = 0;
	for (int row = 0; row < N; row++)
		for (int col = 0; col < N; col++)
			if ((*p_board)[row][col] == player)
				count++;
	return count;
}

int Game::both_count(const board_t* const p_board) const
{
	int count = 0;
	for (int row = 0; row < N; row++)
		for (int col = 0; col < N; col++)
			if ((*p_board)[row][col] != ' ')
				count++;
	return count;
}

int Game::evaluate(const board_t* const p_board, char player, int depth)const
{
	// 評価値を計算
	return get_count(p_board, player) - get_count(p_board, player == 'C' ? 'X' : 'C');
}

int Game::evaluateG(const board_t* const p_board, char player, int depth) const
{
	int score(0);

	// 評価値を計算
	const int ptC_x[] = {0,1,0,1,6,7,6,7};
	const int ptC_y[] = {1,0,6,7,0,1,7,6};
	const int ptX_x[] = {1,1,6,6};
	const int ptX_y[] = {1,6,1,6};
	const int ptCorner_x[] = {0,0,7,7};
	const int ptCorner_y[] = {0,7,0,7};
	const int ptCtCorner_x[] = {2,2,5,5};
	const int ptCtCorner_y[] = {2,5,2,5};
	
	int AddScore = 0;
	// 序盤は中央付近に置くとポイント＋とした。
	int turned = both_count(p_board)-4;
	if (turned < 30) {
		for (int row(2); row < 6; row++) {
			for (int col(2); col < 6; col++) {
				if ((*p_board)[row][col] == player) {
					AddScore++;
				}
				if ((*p_board)[row][col] == (player == 'C' ? 'X' : 'C')) {
					AddScore--;
				}
			}
		}
		// 中央付近のコーナーは更にポイントアップ
		for (int x(0); x < sizeof(ptCtCorner_x) / sizeof(ptCtCorner_x[0]); x++) {
			for (int y(0); y < sizeof(ptCtCorner_y) / sizeof(ptCtCorner_y[0]); y++) {
				if ((*p_board)[ptCtCorner_y[y]][ptCtCorner_x[x]] == player) {
					AddScore += 1;
				}
				if ((*p_board)[ptCtCorner_y[y]][ptCtCorner_x[x]] ==
					(player == 'C' ? 'X' : 'C')) {
					AddScore -= 1;
				}
			}
		}
		// コーナーに無い時、X、Cの位置に有れば減点。

		AddScore += check_corner(p_board, player, 0, 0, 1, 1);
		AddScore += check_corner(p_board, player, 7, 0, -1, 1);
		AddScore += check_corner(p_board, player, 0, 7, 1, -1);
		AddScore += check_corner(p_board, player, 7, 7, -1, -1);

		// コーナーに打つとポイントアップ
		//for (int x(0); x < sizeof(ptCorner_x) / sizeof(ptCorner_x[0]); x++) {
		//	for (int y(0); y < sizeof(ptCorner_y) / sizeof(ptCorner_y[0]); y++) {
		//		if ((*p_board)[ptCorner_y[y]][ptCorner_x[x]] == player) {
		//			AddScore += 14;
		//		}
		//		if ((*p_board)[ptCorner_y[y]][ptCorner_x[x]] == (player == 'C' ? 'X' : 'C')) {
		//			AddScore -= 14;
		//		}
		//	}
		//}
	}
	return get_count(p_board, player) + AddScore
		- get_count(p_board, player == 'C' ? 'X' : 'C');
}

int Game::check_corner(const board_t* const p_board, char player, int x, int y, int dx, int dy)const
{
	int Add(0);
	if ((*p_board)[x][y] == ' ') {
		if ((*p_board)[x + dx][y] == player) {
			Add -= 8;
		}

		if ((*p_board)[x][y + dy] == player) {
			Add -= 8;
		}

		if ((*p_board)[x + dx][y + dy] == player) {
			Add -= 16;
		}
	}
	else {
		//コーナーに打つとポイントアップ
		if ((*p_board)[x][y] == player) {
			Add += 16;
		}
	}
	return Add;
}

int Game::alphabeta(const board_t* const p_board, const char player, int depth, int alpha, int beta)const
{
	if (depth >= SECOND_DEPTH)
	{
		int score = evaluate(p_board, 'C', depth);
		return score;
	}

	char next_player = player == 'X' ? 'C' : 'X';
	vector<pair<int, int>> pairs = get_valid_moves(&board, next_player);
	if (pairs.empty()) {
		next_player = next_player == 'X' ? 'C' : 'X';
		pairs = get_valid_moves(&board, next_player);
		if (pairs.empty()) {
			_D("勝敗確定");
			return evaluate(p_board, 'C', depth) > 0 ? INF - depth : -INF + depth;
		}
	}

	if ('C' == next_player) {
		for (auto a_pair : pairs) {
			board_t next_board{};
			copy_board(&next_board, p_board);
			update_board(&next_board, a_pair.first, a_pair.second, next_player);
			alpha = max<int>(alpha, alphabeta(&next_board, next_player, depth + 1, alpha, beta));
			if (alpha >= beta) {
				//_D("alpha cut");
				break;
			}
		}
		return alpha;
	}
	else {
		for (auto a_pair : pairs) {
			board_t next_board{};
			copy_board(&next_board, p_board);
			update_board(&next_board, a_pair.first, a_pair.second, next_player);
			beta = min<int>(beta, alphabeta(&next_board, next_player, depth + 1, alpha, beta));
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
	if (depth >= MAX_DEPTH)
	{
		int score = evaluate(p_board, 'C', depth);
		return score;
	}

	char next_player = player == 'X' ? 'C' : 'X';
	vector<pair<int, int>> pairs = get_valid_moves(&board, next_player);
	if (pairs.empty()) {
		next_player = next_player == 'X' ? 'C' : 'X';
		pairs = get_valid_moves(&board, next_player);
		if (pairs.empty()) {
			_D("勝敗確定");
			return evaluate(p_board, 'C', depth) > 0 ? INF - depth : -INF + depth;
		}
	}

	if ('C' == next_player) {
		int score = -INF;
		for (auto a_pair : pairs) {
			board_t next_board{};
			copy_board(&next_board, p_board);
			update_board(&next_board, a_pair.first, a_pair.second, next_player);
			score = max<int>(score, minimax(&next_board, next_player, depth + 1 ));
		}
		return score;
	}
	else {
		int score = INF;
		for (auto a_pair : pairs) {
			board_t next_board{};
			copy_board(&next_board, p_board);
			update_board(&next_board, a_pair.first, a_pair.second, next_player);
			score = min<int>(score, minimax(&next_board, next_player, depth + 1));
		}
		return score;
	}
}

bool Game::make_computer_move(pair<int, int>* p_pair)
{
	best_val = -INF;
	best_row = -1;
	best_col = -1;

	const auto v_pairs = move(get_valid_moves(&board, 'C'));
	if (v_pairs.empty())
		return false;
	cout << "コンピューター思考中・・・" << endl;

	_D("Num of first nodes:" + to_string(v_pairs.size()));

	p_root_node = mr_Node.Lend();
	p_root_node->player = 'X';
	p_root_node->depth = 0;
	p_root_node->row = 0;
	p_root_node->col = 0;
	p_root_node->called_children_cnt = 0;
	p_root_node->p_parent = nullptr;
	p_root_node->vp_child_nodes = {};
	p_root_node->pGame = this;
	p_root_node->pwk = NULL;

	if (!ResetEvent(hWaitEvent)) {
		EOut;
		return false;
	}

	for (int i(0); i < v_pairs.size(); i++) {

		node_t* pNode = mr_Node.Lend();
		copy_board(&pNode->board, &board);
		update_board(&pNode->board, v_pairs[i].first, v_pairs[i].second, 'C');
		pNode->player = 'C';
		pNode->depth = p_root_node->depth + 1;
		pNode->row = v_pairs[i].first;
		pNode->col = v_pairs[i].second;
		pNode->score= -INF;
		pNode->called_children_cnt = 0;
		pNode->p_parent = p_root_node;
		pNode->vp_child_nodes.clear();
		pNode->pGame = p_root_node->pGame;
		p_root_node->vp_child_nodes.push_back(pNode);
		_D("first node (" + to_string(v_pairs[i].first + 1) + ","
			+ to_string(v_pairs[i].second + 1) + ")");
		if (!(pNode->pwk = CreateThreadpoolWork(pfnwkMiniMax, pNode, &*pcbe))) {
			EOut;
			return false;
		}
		SubmitThreadpoolWork(pNode->pwk);
	}
	DWORD result;
	if(!(result=WaitForSingleObject(hWaitEvent,INFINITE)==WAIT_OBJECT_0))
	{ 
		int i = 0;
	}
	else {
		int i=0;
	}

#if defined(_DEBUG)
	// 有効な選択肢が選択されたかチェック
	stringstream ss;
	if (find(v_pairs.begin(), v_pairs.end(), pair<int,int>(best_row,best_col)) != v_pairs.end()) {
		//_D(string("Pair found in vector v_pairs: (")
		//	+ to_string( best_row)+ "," +to_string( best_col)+ ")");
	}
	else {
		_D(string("Pair not found in vector v_pairs: (")
			+to_string( best_row)+ ", " + to_string( best_col)+ ")");
		MessageBoxA(NULL, ss.str().c_str(), "検証", MB_ICONEXCLAMATION);
	}
#endif // _DEBUG

	update_board(&board, best_row, best_col, 'C');
	*p_pair = { best_row, best_col };

	CloseThreadpoolCleanupGroupMembers(&*ptpcg, TRUE, NULL);
	node_cut(p_root_node);

	return true;
}

void Game::dout_board_()const
{
	stringstream ss;
	ss << " ";
	for (int row = 0; row < N; row++)
		ss << " " << row + 1;
	ss << endl;
	for (int row = 0; row < N; row++) {
		ss << row + 1;
		for (int col = 0; col < N; col++)
			ss << "|" << board[row][col];
		ss << "|" << endl;
		ss << " ";
		for (int col = 0; col < N; col++)
			ss << "--";
		ss << "-" << endl;
	}
	OutputDebugStringA(ss.str().c_str());
}

void dout(const string& str)
{
	OutputDebugStringA((str + "\r\n").c_str());
}

const string ErrOut_(DWORD dw, LPCSTR lpcszFile, LPCSTR lpcszFunction, DWORD dwLine, const string &lpszOpMessage)
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

	const string strOpMessage = "User Message:\"" + lpszOpMessage + "\"";
	stringstream ss;
	ss << lpcszFile << "(" << dwLine << "): error C" << dw << ": "\
		<< lpMsgBuf
		<< "function name: " << lpcszFunction
		<< strOpMessage << "\r\n";
	::OutputDebugStringA(ss.str().c_str());
	cerr << ss.str();

	LocalFree(lpMsgBuf);
	return ss.str().c_str();
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
