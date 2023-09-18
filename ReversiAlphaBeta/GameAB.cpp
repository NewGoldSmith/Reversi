/**
 * @file Game.cpp
 * @brief �Q�[���N���X�̎���
 * @author Gold Smith
 * @date 2023
 *
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 */
#include "GameAB.h"
using namespace std;

Game::Game()
	:
	board()
	, Frequency{ []() {
		LARGE_INTEGER frequency{};
		QueryPerformanceFrequency(&frequency);
		return frequency;
	}() }
{
	init_game();
}

Game::~Game()
{
}

void Game::init_game()
{
	// �Ֆʂ̏�����
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			board[i][j] = ' ';
	// �����z�u
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
	int turn=0;
	TotalTime = {};
	for (bool b_end(0); !b_end;) {
		display_board();
		turn++;
		cout << turn << "���" << endl;
		int row, col;
		if (current_player == 'C' && !two_player) {
			std::pair<int, int> p;
			QueryPerformanceCounter(&StartingTime);
			if (make_computer_move(&p))
				cout << "�R���s���[�^��" << to_string(p.first + 1)
				<< " " << to_string(p.second + 1) << "���w���܂����B" << endl;
			else
				cout << "�R���s���[�^�̓p�X���܂����B" << endl;
			QueryPerformanceCounter(&EndingTime);
			ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
			ElapsedMicroseconds.QuadPart *= 1000000;
			ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
			double time_taken = ElapsedMicroseconds.QuadPart / 1000000.0;
			cout << "�v�l����:" << fixed << setprecision(3)
				<< time_taken << " seconds." << endl;
			TotalTime.QuadPart += ElapsedMicroseconds.QuadPart;
		}
		else {
			for (; !b_end;) {
				cin.ignore(numeric_limits<streamsize>::max(), '\n');
				if ((get_valid_moves(&board, 'X')).empty())
				{
					cout << "�v���C���[ " << current_player
						<< "�p�X�ɂȂ�܂��B�����L�[�������Ă��������B";
					string str;
					getline(cin, str);
					display_board();
					cout << flush;
					break;
				}
				cout << "�v���C���[ " << current_player << "�A���͂��ĉ�����(�s ��)�B";
				cout << " (�I������ꍇ�� 0 0 �Ɠ���):" ;
				cin >> row >> col;
				if (cin.fail()) {
					cin.clear();
					cout << "�����ȓ��͂ł��B�ēx���͂��Ă��������B" << endl;
					display_board();
					continue;
				}
				if (row == 0 && col == 0)
				{
					b_end = true;
					continue;
				}
				if (row < 1 || row > N || col < 1 || col > N) {
					cout << "�����Ȏ�ł��B�ēx���͂��Ă��������B" << endl;
					display_board();
					continue;
				}
				if (board[row - 1][col - 1] != ' ') {
					cout << "���̃}�X�͊��ɑI������Ă��܂��B" << endl;
					display_board();
					continue;
				}
				if (!is_valid_move(&board, row - 1, col - 1, current_player))
				{
					cout << "���̃}�X�͑I���ł��܂���B" << endl;
					display_board();
					continue;
				}
				update_board(&board, row - 1, col - 1, current_player);
				break;
			}
		}
		if (is_win(&board, 'X')) {
			display_board();
			auto a_score=get_each_countXC(&board);
			cout << "X:"<<a_score.first<<" C:"<<a_score.second << " X�̏���!" << endl;
			break;
		}
		else if (is_win(&board, 'C')) {
			display_board();
			auto a_score = get_each_countXC(&board);
			cout << "X:" << a_score.first << " C:" << a_score.second << " C�̏���!" << endl;
			break;
		}
		else if (is_draw(&board)) {
			display_board();
			cout << "���������ł�!" << endl;
			break;
		}
		// switch player
		current_player = current_player == 'X' ? 'C' : 'X';
	}
	cout << "�R���s���[�^�[�̃g�[�^���v�l���Ԃ�" << fixed << setprecision(3)
		<< (double)TotalTime.QuadPart / 1000000.0 << " seconds.�ł��B" << endl;
}

bool Game::is_draw(const board_t* p_board)const
{
	if (is_game_over(p_board))
	{
		auto pair = get_each_countXC(p_board);
		return pair.first == pair.second ? true : false;
	}
	return false;
}

bool Game::is_win(const board_t* p_board, char player)const
{
	if (is_game_over(p_board))
	{
		auto pair = get_each_countXC(p_board);
		if (pair.first - pair.second > 0 && player == 'X')
			return true;
		if (pair.first - pair.second < 0 && player == 'C')
			return true;
	}
	return false;
}

bool Game::is_game_over(const board_t* p_board)const
{
	// ���v���C���[�Ƃ��ɑłĂ�肪���邩�ǂ����𒲂ׂ�
	if (!get_valid_moves(p_board, 'X').empty() || !get_valid_moves(p_board, 'C').empty())
		return false;

	// �Q�[���I��
	return true;
}

bool Game::is_valid_move(const board_t* p_board, int row, int col, char ch)const
{
	// (row, col)���Ֆʂ͈̔͊O�̏ꍇ��false��Ԃ�
	if (row < 0 || row >= N || col < 0 || col >= N)
		return false;

	// (row, col)�Ɋ��ɐ΂��u����Ă���ꍇ��false��Ԃ�
	if ((*p_board)[row][col] != ' ')
		return false;

	// ���o�[�V�̃��[���Ɋ�Â��āA(row, col)��ch�ɂƂ��ėL���Ȏ�ł��邩���f
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

	// ���o�[�V�̃��[���Ɋ�Â��āA���ɑłĂ���T��
	for (int row = 0; row < N; row++)
		for (int col = 0; col < N; col++)
			if (is_valid_move(p_board, row, col, ch))
				valid_moves.emplace_back(row, col);

	return valid_moves;
}

void Game::update_board(board_t* p_board, int row, int col, char ch)const
{
	// ���u��
	(*p_board)[row][col] = ch;

	// ���o�[�V�̃��[���Ɋ�Â��āA�Ֆʂ��X�V
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

void Game::copy_board(board_t* pdist, const board_t* psource)const
{
	for (int i(0); i < N; i++)
		for (int j(0); j < N; j++)
		{
			(*pdist)[i][j] = (*psource)[i][j];
		}
}

pair<int, int> Game::get_each_countXC(const board_t* p_board)const
{
	// �΂̐����J�E���g
	int x_count = 0;
	int c_count = 0;
	for (int row = 0; row < N; row++)
		for (int col = 0; col < N; col++)
			if ((*p_board)[row][col] == 'X')
				x_count++;
			else if ((*p_board)[row][col] == 'C')
				c_count++;

	// �X�R�A��Ԃ�
	return make_pair(x_count, c_count);
}

int Game::evaluate(const board_t* p_board, char ch, int depth)const
{
	int score(0);
	if (is_win(p_board, ch)) {
		score = INF - depth;
		return score;
	}
	else if (is_win(p_board, ch == 'C' ? 'X' : 'C')) {
		score = -INF + depth;
		return score;
	}
	auto pair = get_each_countXC(p_board);
	// �]���l���v�Z
	score = pair.first - pair.second;
	if (ch == 'C')
		score = -score;
	return score;
}

int Game::minimax(const board_t* p_board, int depth, char ch)const
{
	return alphabeta(p_board,ch , depth, -INF, INF);
}

int Game::alphabeta(const board_t* p_board,const char player, int depth, int alpha, int beta)const
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
			_D("���s�m��");
			return evaluate(p_board, 'C', depth);
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

bool Game::make_computer_move(pair<int, int>* p_pair)
{
	int best_val = -INF;
	int best_row = -1;
	int best_col = -1;
	auto v_pairs = std::move(get_valid_moves(&board, 'C'));
	if (v_pairs.empty())
		return false;
	cout << "�R���s���[�^�[�v�l���E�E�E" << endl;
	for (const auto& pair : v_pairs) {
		board_t tmp_board{};
		copy_board(&tmp_board, &board);
		update_board(&tmp_board,pair.first,pair.second, 'C');
		int move_val = minimax(&tmp_board, 1, 'C');
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

const string ErrOut_(DWORD dw, LPCSTR lpszFile, LPCSTR lpszFunction, DWORD dwLine, LPCSTR lpszOpMessage)
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

	string strOpMessage;
	if (strlen(lpszOpMessage))
	{
		strOpMessage = (string)"User Message:\"" + lpszOpMessage + "\"";
	}

	stringstream ss;
	ss << lpszFile << "(" << dwLine << "): error C" << dw << ": "\
		<< lpMsgBuf
		<< "function name: " << lpszFunction
		<< strOpMessage << "\r\n";
	::OutputDebugStringA(ss.str().c_str());
	cerr << ss.str();

	LocalFree(lpMsgBuf);
	return ss.str().c_str();
}
