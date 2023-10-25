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
#define TEST_CODE
#define NOMINMAX
#define _CRTDBG_MAP_ALLOC
#if !defined(_M_X64) && !defined(__amd64__)
#error "This code must be compiled in x64 configuration."
#endif
#include<Windows.h>
#include<string>
#include<iostream>
#include<sstream>
#include <limits>
#include <vector>
#include <algorithm>
#include <atomic>
#include <iomanip>
#include <cstdint>
#include <utility>
#include <bit>
#include "MemoryRental.h"
using namespace std;
constexpr int N = 8;
constexpr int INF = 10000;
constexpr int BREAK_CODE = INF + 5;
#define FIRST_DEPTH  2U// 1以上に設定要。
#define SECOND_DEPTH 2U
constexpr int MAX_DEPTH = FIRST_DEPTH + SECOND_DEPTH;
constexpr unsigned MAX_THREADS = 1;
constexpr unsigned NODE_UNIT_SIZE = 0x20000;
#if (FIRST_DEPTH < 1 ) 
#error FIRST_DEPTH must be 1 or greater.
#endif

void dout(const string& str);
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
	/// 左上を表記座標(1,1)とし、右下を(N,N)とする。
	/// インデックス指定では(0,0)から(N-1,N-1)となる。
	/// </summary>
	typedef char board_t[N][N];

	/// <summary>
	/// board_tのビットボード版。左上を0x8000000000000000とし、
	/// 右下を0x0000000000000001とする。
	/// MSBが(0,0)でLSBが(8,8)
	/// </summary>
	typedef uint64_t bb_t;
	/// <summary>
	/// 読みの深さの型
	/// </summary>
	typedef unsigned long depth_t;

	HANDLE hWaitEvent=NULL;
	const unique_ptr<remove_pointer_t<HANDLE>, decltype(CloseHandle)*> uhWaitEvent;

	LARGE_INTEGER TotalTime = {};
	LARGE_INTEGER StartingTime{}, EndingTime{}, ElapsedMicroseconds{};
	const LARGE_INTEGER Frequency;

	struct node_t {
		bb_t bb_C = {};
		bb_t bb_X = {};
		board_t board = {};
		char player = '\0';
		depth_t depth = -1;
		int row = 0;
		int col = 0;
		int score = 0;
		int called_children_cnt = 0;
		node_t* p_parent = nullptr;
		vector<node_t*> vp_child_nodes = {};
		Game* pGame = NULL;
		PTP_WORK pwk=NULL;
		alignas(8) CRITICAL_SECTION cs = {};
		const unique_ptr< CRITICAL_SECTION
			, decltype(DeleteCriticalSection)*> pcs;
		node_t();
		node_t(const node_t&) = delete;
		node_t(node_t&&)noexcept = delete;
		node_t& operator () (const node_t&) = delete;
		node_t& operator = (const node_t&) = delete;
		bool operator == (const node_t&)const = delete;
	};

	void node_cut(node_t* const p_node);

	const unique_ptr<node_t,void(*)(node_t[])>node_array;
	MemoryRental<node_t>mr_Node;

	board_t board;
	bb_t bb_cur_X = 0ULL;
	bb_t bb_cur_C = 0ULL;
	int best_val=-INF;
	int best_row=-1;
	int best_col=-1;
	int score_conf = -INF;
	node_t* p_root_node = NULL;
	int turn = 0;

	const enum Direction :unsigned {
		UP,
		RIGHT_UP,
		RIGHT,
		RIGHT_DOWN,
		DOWN,
		LEFT_DOWN,
		LEFT,
		LEFT_UP
	};
	bb_t shift_table[64][8];

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
	/// <summary>
	/// 現在の盤面を表示する。
	/// </summary>
	void display_board_bb(const bb_t bbX, const bb_t bbC)const;
	void init_game_bb();
	void init_game();
	/// <summary>
	/// その盤面が引き分けかどうか調べる。
	/// </summary>
	/// <param name="p_board">調べる盤面のポインタ。</param>
	/// <returns>引き分けならtrue、引き分けでないならfalseを返す。</returns>
	bool is_draw_bb(const bb_t bb_X, const bb_t bb_C)const;
	/// <summary>
	/// 対象のプレイヤーが勝ちかどうか調べる。
	/// </summary>
	/// <param name="p_board">調べる盤面のポインタ。</param>
	/// <param name="player">対象のプレイヤー。</param>
	/// <returns>勝ちならtrue、それ以外はfalse。</returns>
	bool is_win_bb(const bb_t bb_X, const bb_t bb_C, char player)const;
	/// <summary>
	/// 盤面がゲームオーバーかどうか調べる。
	/// </summary>
	/// <param name="p_board">対象の盤面のポインタ。</param>
	/// <returns>ゲームオーバーならtrue、それ以外はfalse。</returns>
	bool is_game_over_bb(const bb_t p_board_X, const bb_t p_board_C)const;
	void init_shift_table();
	/// <summary>
	/// ビットボード上で1マス分シフトした値を返す。
	/// 8x8のビットボード用。シフトによる隣列へのビットの回り込みを防ぐ。
	/// </summary>
	/// <param name="b">ビットボード。</param>
	/// <param name="direction">enum Directionで、シフトする方向を示す。
	/// 0:上、1:右上、2:右、3:右下、4:下、5:左下、6:左、7:左上、
	/// それ以外:変化せず。</param>
	/// <returns>シフトしたビットボード値。</returns>
	bb_t shift(const bb_t b, const unsigned direction)const;
	/// <summary>
	/// その指し手が有効かどうか調べる。
	/// </summary>
	/// <param name="p_board">調べる盤面のポインタ。</param>
	/// <param name="row">行</param>
	/// <param name="col">列</param>
	/// <param name="ch">手番</param>
	/// <returns>有効ならtrue、無効ならfalseを返す。</returns>
	bool is_valid_move_bb(const bb_t src_X, const bb_t src_C, const int row, const int col, const char player)const;
	bool is_valid_move(const board_t* const p_board, int row, int col, char ch)const;
	/// <summary>
	/// 有効な指し手全てを探し、vectorで返す。firstが行、secondが列。
	/// </summary>
	/// <param name="p_board">探す盤面へのポインタ。</param>
	/// <param name="ch">対象のプレイヤー。</param>
	/// <returns>pairのvector。</returns>
	vector<pair<int, int>> get_valid_moves_bb(const bb_t bb_X, const bb_t bb_C, const char player)const;
	inline vector<pair<int, int>> get_valid_moves(const board_t* const p_board, const char ch)const;
	bool existing_valid_moves_bb(const bb_t bb_X, const bb_t bb_C, const char player) const;
	
	/// <summary>
	/// 指定された盤面にプレイヤーchが指した手によって裏返る石を裏返させる。
	/// </summary>
	/// <param name="p_board">指定する盤面へのポインタ。盤面のデータは変更される。</param>
	/// <param name="row">指定する指し手の行。</param>
	/// <param name="col">指定する指し手の列。</param>
	/// <param name="ch">手番のプレイヤー</param>
	void update_board_bb(_Inout_ bb_t& dst_X, _Inout_ bb_t& dst_C, _In_ const bb_t src_X, _In_ const bb_t src_C, _In_ const int row, _In_ const int col, _In_ const char player) const;
	//inline void update_board_bb(bb_t& p_board_X, bb_t& p_board_C, int row, int col, char ch)const;
	void update_board(board_t* const p_board, const int row, const int col, const char ch)const;
	/// <summary>
	/// ボードをコピーする。
	/// </summary>
	/// <param name="pdist">コピー先。</param>
	/// <param name="psource">コピー元。</param>
	inline void copy_board_bb(bb_t& dist, const bb_t src)const;
	void copy_board(board_t* const pdist, const board_t* const psrc)const;
	/// <summary>
	/// 石の数を数える。
	/// </summary>
	/// <param name="p_board">'C'または'X'の盤面へのポインタ。</param>
	/// <returns></returns>
	inline int get_count_bb(const bb_t bb)const;
	int get_count(const board_t* const p_board, const char player)const;
	/// <summary>
	/// 評価関数
	/// </summary>
	/// <param name="ch">対象の手番</param>
	/// <param name="depth">読み深さ</param>
	/// <returns>評価値</returns>
	int evaluate_bb(const bb_t bb_X, const bb_t bb_C, const char player, const depth_t depth)const;
	/// <summary>
	/// 評価関数。'C'から見たスコアを返す。
	/// </summary>
	/// <param name="board_X">'X'のボード</param>
	/// <param name="board_C">'C'のボード</param>
	/// <param name="depth">読みの深さ。</param>
	/// <returns>評価値</returns>
	int evaluate_bbG(const bb_t bb_X, const bb_t bb_C, const char player, const depth_t depth)  const;
	int evaluate(const board_t* const  p_board, char ch)const;
	inline int evaluate_bbS(const bb_t bb_X, const bb_t bb_C) const;
	//int evaluateG(board_t board_X, board_t board_C, depth_t depth)const;
	//int evaluateS(board_t board_X, board_t board_C)const;
	/// <summary>
	/// 探索エンジン。
	/// </summary>
	/// <param name="p_board_X">'X'の盤面。</param>
	/// <param name="p_board_C">'C'の盤面。</param>
	/// <param name="player">手番。</param>
	/// <param name="depth">読みの深さ。</param>
	/// <param name="alpha">alpha値。</param>
	/// <param name="beta">beta値。</param>
	/// <returns>評価値。</returns>
	int alphabeta_bb(const bb_t bb_X, const bb_t bb_C,const char player, const depth_t depth, int alpha, int beta)const;
	int alphabeta(const board_t* const p_board, const char player, int depth, int alpha, int beta)const;
	int minimax_bb(const bb_t bb_X, const bb_t bb_C, const char player, const depth_t depth)const;
	/// <summary>
	/// 整合性確認用。読みの深さはMAX_DEPTHに設定されている。
	/// </summary>
	/// <param name="bb_X">Xのビットボード。</param>
	/// <param name="bb_C">Cのビットボード。</param>
	/// <param name="player">現指し手のプレーヤー。次手は関数の中で判断。</param>
	/// <param name="depth">次手の読みの深さ。</param>
	/// <returns>評価値。</returns>
	int minimax_bb_conf(const bb_t bb_X, const bb_t bb_C, const char player, const depth_t depth)const;
	int minimax(const board_t* const p_board, const char player, int depth)const;
	/// <summary>
	/// 整合性確認用。読みの深さはMAX_DEPTHに設定されている。
	/// </summary>
	/// <param name="p_board">board_t型の盤面。</param>
	/// <param name="player">現在の手番。次の手番は関数の中で判断する。</param>
	/// <param name="depth">次手の読みの深さ。</param>
	/// <returns>評価値。</returns>
	int minimax_conf(const board_t* const p_board, const char player, int depth)const;
	void return_minimax(node_t* const p_child);
	/// <summary>
	/// コンピュータが一手指す。成功ならp_pairの指す所にその座標を入れる。
	/// 失敗ならp_pairの指す内容は不定。
	/// </summary>
	/// <returns>成功ならtrue、失敗ならfalseが返る。</returns>
	bool make_computer_move_bb(pair<int, int>* p_pair);
	const PTP_WORK_CALLBACK pfnwkMiniMax;
	void conv_bbToBoard(board_t* const p_board, const bb_t bb_X, const bb_t bb_C)const;
	void convBoardTo_bb(bb_t& bbX, bb_t& bbC, const board_t*const board)const;
	/// <summary>
	/// デバック出力にボードを表示。
	/// </summary>
	void dout_board_bb_(const bb_t bx, const bb_t bc)const;
	void dout_board_(const board_t* const p_board)const;
	bool is_equal(const board_t* const p_board, const bb_t boardX, const bb_t boardC)const;

#define EOut ErrOut_(GetLastError(),__FILE__,__FUNCTION__,__LINE__)
#if defined(_DEBUG)
#define _D(str) dout(string()+__FILE__+"("+to_string(__LINE__)+"): "+str);
#define _DBB(bx,bc) dout_board_bb_(bx,bc);
#else
#define _D(str) __noop
#define _DBB(bx,bc) __noop
#endif
};

