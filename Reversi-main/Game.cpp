/**
 * @file Game.cpp
 * @brief ゲームクラスの実装
 * @author Gold Smith
 * @date 2023
 *
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 */
#include "Game.h"
using namespace std;

Game::Game()
	:
	board()
{
	init_game();
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

void Game::play_game(bool human_first, bool two_player)
{
	char current_player = human_first ? 'X' : 'C';
	for (bool b_end(0); !b_end;) {
		display_board();
		int row, col;
		if (current_player == 'C' && !two_player) {
			std::pair<int, int> p;
			if (make_computer_move(&p))
				cout << "コンピュータが" << to_string(p.first + 1)
				<< " " << to_string(p.second + 1) << "を指しました。" << endl;
			else
				cout << "コンピュータはパスしました。" << endl;
		}
		else {
			for (; !b_end;) {
				cin.clear();
				//cin.ignore(numeric_limits<streamsize>::max(), '\n');
				if ((get_valid_moves(&board, 'X')).empty())
				{
					cout << "プレイヤー " << current_player
						<< "パスになります。何かキーを押してください。";
					string str;
					getline(cin, str);
					display_board();
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
			auto a_score=get_score(&board);
			cout << "X:"<<a_score.first<<" C:"<<a_score.second << " Xの勝ち!" << endl;
			break;
		}
		else if (is_win(&board, 'C')) {
			display_board();
			auto a_score = get_score(&board);
			cout << "X:" << a_score.first << " C:" << a_score.second << " Cの勝ち!" << endl;
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
}

bool Game::is_draw(const board_t* p_board)const
{
	if (is_game_over(p_board))
	{
		auto pair = get_score(p_board);
		return pair.first == pair.second ? true : false;
	}
	return false;
}

bool Game::is_win(const board_t* p_board, char player)const
{
	if (is_game_over(p_board))
	{
		auto pair = get_score(p_board);
		if (pair.first - pair.second > 0 && player == 'X')
			return true;
		if (pair.first - pair.second < 0 && player == 'C')
			return true;
	}
	return false;
}

bool Game::is_game_over(const board_t* p_board)const
{
	// 両プレイヤーともに打てる手があるかどうかを調べる
	if (!get_valid_moves(p_board, 'X').empty() || !get_valid_moves(p_board, 'C').empty())
		return false;

	// ゲーム終了
	return true;
}

bool Game::is_valid_move(const board_t* p_board, int row, int col, char ch)const
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
		if (found_opponent && x >= 0 && x < N && y >= 0 && y < N && (*p_board)[x][y] == ch)
			return true;
	}

	return false;
}

vector<pair<int, int>> Game::get_valid_moves(const board_t* p_board, char ch)const
{
	vector<pair<int, int>> valid_moves;

	// リバーシのルールに基づいて、次に打てる手を探索
	for (int row = 0; row < N; row++)
		for (int col = 0; col < N; col++)
			if (is_valid_move(p_board, row, col, ch))
				valid_moves.emplace_back(row, col);

	return valid_moves;
}

void Game::update_board(board_t* p_board, int row, int col, char ch)const
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

pair<int, int> Game::get_score(const board_t* p_board)const
{
	// 石の数をカウント
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

int Game::evaluate(const board_t* p_board, char ch, int depth, bool* pb_is_settled)const
{
	int score(0);
	if (depth == MAX_DEPTH)
		*pb_is_settled = true;
	if (is_win(p_board, ch)) {
		score = INF - depth;
		return score;
	}
	else if (is_win(p_board, ch == 'C' ? 'X' : 'C')) {
		score = -INF + depth;
		return score;
	}
	auto pair = get_score(p_board);
	// 評価値を計算
	score = pair.first - pair.second;
	if (ch == 'C')
		score = -score;
	return score;
}

int Game::minimax(const board_t* p_board, int depth, char ch)const
{
	bool b_settled(false);
	int score = evaluate(p_board, ch, depth, &b_settled);

	// 読み深さが設定に達した場合
	if (b_settled)
		return score;

	int best = -INF;
	for (const auto& pair : get_valid_moves(p_board, ch)) {
		board_t tmp_board;
		std::copy(&(*p_board)[0][0], &(*p_board)[0][0] + N * N, &tmp_board[0][0]);
		update_board(&tmp_board, pair.first, pair.second, ch);
		best = std::max<int>(best, -minimax(&tmp_board, depth + 1, ch == 'C' ? 'X' : 'C'));
	}
	return best;
}

bool Game::make_computer_move(pair<int, int>* p_pair)
{
	int best_val = -INF;
	int best_row = -1;
	int best_col = -1;
	auto v_pairs = std::move(get_valid_moves(&board, 'C'));
	if (v_pairs.empty())
		return false;
	for (const auto& pair : v_pairs) {
		board_t tmp_board{};
		copy_board(tmp_board, board);
		update_board(&tmp_board, pair.first, pair.second,'C');
		int move_val = -minimax(&tmp_board, 0, 'X');
		if (move_val >= best_val) {
			best_row = pair.first;
			best_col = pair.second;
			best_val = move_val;
		}
	}
	update_board(&board, best_row, best_col, 'C');
	*p_pair = { best_row, best_col };
	return true;
}

inline void Game::copy_board(board_t& dist, const board_t& source) const
{
	for (int i(0); i < N; i++)
		for (int j(0); j < N; j++)
		{
			dist[i][j] = source[i][j];
		}
}


void Game::dout_borad()const
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

void Game::dout(const string& str)const
{
	OutputDebugStringA((str + "\r\n").c_str());
}
