/**
 * @file GameABMG.h
 * @brief ゲームクラスの宣言
 * @author Gold Smith
 * @date 2023
 *
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 */
#pragma once
#define NOMINMAX
#define _CRTDBG_MAP_ALLOC
#include<Windows.h>
#include<string>
#include<iostream>
#include<sstream>
#include <limits>
#include <vector>
#include <algorithm>
#include <atomic>
#include <iomanip>
#include "MemoryRental.h"
using namespace std;
constexpr int N = 8;
constexpr int INF = 10000;
constexpr int BREAK_CODE = INF + 5;
constexpr int FIRST_DEPTH = 1;
constexpr int SECOND_DEPTH = 11;
constexpr int MAX_DEPTH = FIRST_DEPTH + SECOND_DEPTH;
constexpr unsigned MAX_THREAD = 5;
constexpr unsigned NODE_UNIT_SIZE = 0x1000;
constexpr unsigned STOP_EVENT_SIZE = 0x1000;
void dout(const string& str);
#if defined(_DEBUG)
#define _D(str) dout(string()+__FILE__+"("+to_string(__LINE__)+"): "+str);
#else
#define _D(str) __noop
#endif
#define EOut ErrOut_(GetLastError(),__FILE__,__FUNCTION__,__LINE__)
const string ErrOut_(
	DWORD dw
	, LPCSTR lpcszFile
	, LPCSTR lpcszFunction
	, DWORD dwLine
	, const string& lpszOpMessage = "");

class Game
{
	CRITICAL_SECTION cs = {};
	const unique_ptr<TP_POOL, decltype(CloseThreadpool)*>ptpp;
	const unique_ptr<TP_CALLBACK_ENVIRON
		, void(*)(TP_CALLBACK_ENVIRON*)>pcbe;
	const unique_ptr<TP_CLEANUP_GROUP
		, void(*)(PTP_CLEANUP_GROUP)>ptpcg;
	const PTP_CLEANUP_GROUP_CANCEL_CALLBACK pfng;
	const unique_ptr< CRITICAL_SECTION
		, decltype(DeleteCriticalSection)*> pcs;

	/// <summary>
	/// これでboard_tという型定義。
	/// </summary>
	typedef char board_t[N][N];
	HANDLE hWaitEvent=NULL;
	const unique_ptr<remove_pointer_t<HANDLE>, decltype(CloseHandle)*> uhWaitEvent;
	const unique_ptr < remove_pointer_t<HANDLE[]>
		, void(*)(HANDLE[]) >	unique_hStopEventArray;
	MemoryRental<HANDLE>mr_StopEvents;

	LARGE_INTEGER TotalTime = {};
	LARGE_INTEGER StartingTime{}, EndingTime{}, ElapsedMicroseconds{};
	const LARGE_INTEGER Frequency;

	struct node_t {
		board_t board = {};
		char player = '\0';
		int depth = -1;
		int row = 0;
		int col = 0;
		int alpha = -INF;
		int beta = INF;
		int called_children_cnt = 0;
		node_t* p_parent = nullptr;
		vector<node_t*> vp_child_nodes = {};
		Game* pGame = NULL;
		HANDLE hWaitEvent=NULL;
		HANDLE *phStopEvent = NULL;
		PTP_WORK pwk=NULL;
		CRITICAL_SECTION cs = {};
		const unique_ptr< CRITICAL_SECTION
			, decltype(DeleteCriticalSection)*> pcs;
		node_t();
		node_t(const node_t& other) = delete;
		node_t(node_t&& other)noexcept = delete;
		node_t& operator () (const node_t&) = delete;
		node_t& operator = (const node_t&) = delete;
		bool operator == (const node_t&) = delete;
	};

	void return_score(node_t* const p_parent, int row, int col, int score);
	void worker_cut(node_t* const p_node);
	void node_cut(node_t* const p_node);

	const unique_ptr<node_t,void(*)(node_t[])>node_array;
	MemoryRental<node_t>mr_Node;

	board_t board;
	int best_val=-INF;
	int best_row=-1;
	int best_col=-1;
	node_t* p_root_node = NULL;
public:
	Game();
	~Game();
	Game(const Game&) = delete;
	Game(Game&&)noexcept = delete;
	Game& operator()(const Game&) = delete;
	Game& operator=(const Game&) = delete;
	bool operator==(const Game& other) const = delete;
	/// <summary>
	/// 実際のゲームスタート。
	/// </summary>
	/// <param name="human_first">人が先攻か。</param>
	/// <param name="two_player">2プレーヤー対戦か。</param>
	void play_game(bool human_first, bool two_player);
private:
	/// <summary>
	/// 現在のboardの盤面を表示する。
	/// </summary>
	void display_board()const;
	void init_game();
	/// <summary>
	/// その盤面が引き分けかどうか調べる。
	/// </summary>
	/// <param name="p_board">調べる盤面のポインタ。</param>
	/// <returns>引き分けならtrue、引き分けでないならfalseを返す。</returns>
	bool is_draw(const board_t* const p_board)const;
	/// <summary>
	/// 対象のプレイヤーが勝ちかどうか調べる。
	/// </summary>
	/// <param name="p_board">調べる盤面のポインタ。</param>
	/// <param name="player">対象のプレイヤー。</param>
	/// <returns>勝ちならtrue、それ以外はfalse。</returns>
	bool is_win(const board_t* const p_board, char player)const;
	/// <summary>
	/// 盤面がゲームオーバーかどうか調べる。
	/// </summary>
	/// <param name="p_board">対象の盤面のポインタ。</param>
	/// <returns>ゲームオーバーならtrue、それ以外はfalse。</returns>
	bool is_game_over(const board_t* const p_board)const;
	/// <summary>
	/// その指し手が有効かどうか調べる。
	/// </summary>
	/// <param name="p_board">調べる盤面のポインタ。</param>
	/// <param name="row">行</param>
	/// <param name="col">列</param>
	/// <param name="ch">手番</param>
	/// <returns>有効ならtrue、無効ならfalseを返す。</returns>
	bool is_valid_move(const board_t* const p_board, int row, int col, char ch)const;
	/// <summary>
	/// 有効な指し手全てを探し、vectorで返す。firstが行、secondが列。
	/// </summary>
	/// <param name="p_board">探す盤面へのポインタ。</param>
	/// <param name="ch">対象のプレイヤー。</param>
	/// <returns>pairのvector。</returns>
	vector<pair<int, int>> get_valid_moves(const board_t* const p_board, char ch)const;
	/// <summary>
	/// 指定された盤面にプレイヤーchが指した手によって裏返る石を裏返させる。
	/// </summary>
	/// <param name="p_board">指定する盤面へのポインタ。盤面のデータは変更される。</param>
	/// <param name="row">指定する指し手の行。</param>
	/// <param name="col">指定する指し手の列。</param>
	/// <param name="ch">手番のプレイヤー</param>
	void update_board(board_t* const p_board, int row, int col, char ch)const;
	void copy_board(board_t* const pdist, const board_t* const psource)const;
	/// <summary>
	/// それぞれの数を得る。
	/// </summary>
	/// <param name="p_board">スコアを得る盤面のポインタ。</param>
	/// <returns>std::pair型のスコア。first Xの数。second Cの数。</returns>
	pair<int, int> get_each_countXC(const board_t* const p_board)const;
	/// <summary>
	/// 評価関数
	/// </summary>
	/// <param name="ch">対象の手番</param>
	/// <param name="depth">読み深さ</param>
	/// <param name="pb_is_seddled">決着が着いたか着いてないかの結果を受け取るポインタ。</param>
	/// <returns>評価値</returns>
	int evaluate(const board_t* const  p_board, char ch, int depth)const;
	int alphabeta(const board_t* const p_board, const char player, int depth, int alpha, int beta)const;
	int alphabeta2(const board_t* const p_board, const char player, int depth, int alpha, int beta,HANDLE phStopEvent)const;
	int minimax(const board_t* const p_poard, const char player, int depth)const;
	/// <summary>
	/// コンピュータが一手指す。成功ならp_pairの指す所にその座標を入れる。
	/// 失敗ならp_pairの指す内容は不定。
	/// </summary>
	/// <returns>成功ならtrue、失敗ならfalseが返る。</returns>
	bool make_computer_move(pair<int, int>* p_pair);
	const PTP_WORK_CALLBACK pfnwkMiniMax;
	/// <summary>
	/// デバック出力にボードを表示。
	/// </summary>
	void dout_board_()const;
#if defined(_DEBUG)
#define dout_board() dout_board_()
#else
#define dout_board() __noop
#endif
};

