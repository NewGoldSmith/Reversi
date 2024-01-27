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
#include "MemoryRental.h"
#pragma comment(lib,  "../BitManip/" _STRINGIZE($CONFIGURATION) "/BitManip-" _STRINGIZE($CONFIGURATION) ".lib")
#pragma comment(lib,  "../Debug_fnc/" _STRINGIZE($CONFIGURATION) "/Debug_fnc-" _STRINGIZE($CONFIGURATION) ".lib")
#pragma comment(lib,  "../Engine/" _STRINGIZE($CONFIGURATION) "/Engine-" _STRINGIZE($CONFIGURATION) ".lib")

#define CONFIRM_AVX2

using namespace std;
constexpr int N = 8;
constexpr int INF = 10000;
constexpr int BREAK_CODE = INF + 5;
#define FIRST_DEPTH  3U// 1以上に設定要。
#define SECOND_DEPTH 1U
constexpr int MAX_DEPTH = FIRST_DEPTH + SECOND_DEPTH;
constexpr unsigned MAX_THREADS = 6;
constexpr unsigned NODE_UNIT_SIZE = 0x20000;
#if (FIRST_DEPTH < 1 ) 
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
	typedef unsigned int depth_t;

	HANDLE hWaitEvent=NULL;
	const unique_ptr<remove_pointer_t<HANDLE>, decltype(CloseHandle)*> uhWaitEvent;

	LARGE_INTEGER TotalTime = {};
	LARGE_INTEGER StartingTime{}, EndingTime{}, ElapsedMicroseconds{};
	const LARGE_INTEGER Frequency;
	/// <summary>
	/// スレッドプール部分のノード
	/// </summary>
	struct node_t {
		__m256i m_board{};
		//bb_t bb_C = {};
		//bb_t bb_X = {};
		//board_t board = {};
		//char player = '\0';
		//depth_t depth = 0;
		//int row = 0;
		//int col = 0;
		int score = 0;
		int called_branch_cnt = 0;
		node_t* p_parent = nullptr;
		vector<node_t*> vp_branch_nodes = {};
		Game* pGame = NULL;
		PTP_WORK pwk=NULL;
		alignas(8) CRITICAL_SECTION cs = {};
		const unique_ptr< CRITICAL_SECTION
			, decltype(DeleteCriticalSection)*> pcs;
		node_t();
		node_t(node_t&) = delete;
		node_t(node_t&&)noexcept = delete;
		node_t& operator () (node_t&) = delete;
		node_t& operator = (node_t&) = delete;
		bool operator == (node_t&)const = delete;
	};
	/// <summary>
	/// 指定されたノードと子ノードを削除する。
	/// </summary>
	/// <param name="p_node"></param>
	void node_cut(node_t* const p_node);

	const unique_ptr<node_t,void(*)(node_t[])>node_array;
	MemoryRental<node_t>mr_Node;

	//board_t board;
	///// <summary>
	///// 現在のXのビットボード。
	///// </summary>
	//bb_t bb_cur_X = 0ULL;
	///// <summary>
	///// 現在のCのビットボード。
	///// </summary>
	//bb_t bb_cur_C = 0ULL;
	/// <summary>
	/// __m256型のビットボード。
	/// __m256i_u64[BB_P64]:コンピュータから見た自身のボード。
	/// __m256i_u64[BB_O64]:相手ボード。
	/// __m256i_u64[BB_M64]:move位置。
	/// __m256i_u64[OP64]:その他のステータス。下記参照。
	/// __m256i_i16[ALPHAi16]:alpha値。
	/// __m256i_i16[BETAi16]:beta値。
	/// __m256i_u8[DEPTH8]:depth値。深くなるほど数値が増える。
	/// __m256i_u8[NUM_TURN8]:turn値。
	/// __m256i_u8[ST1_8]:ステータス１値。
	/// CHECK_BIT(__m256i_u8[ST1_8],IS_MY_TURN):自分のターンかどうか。
	/// CHECK_BIT(__m256i_u8[ST1_8],IS_C_X):コンピュータが黒（先手）かどうか。
	/// CHECK_BIT(__m256i_u8[ST1_8],IS_C2C):コンピュータ同志の対戦かどうか。
	/// </summary>
	__m256i m_board{};
	int best_val = -INF;
	//int best_row = -1;
	//int best_col = -1;
	uint64_t best_move_m = 0ULL;
	node_t* p_root_node = NULL;
	//int turn = 0;
	bb_t shift_table[64][8];
	/// <summary>
	/// 盤面のデータを保持する__m256iの頭出し用。
	/// サフィックスの数字は使用するビット幅。
	/// OP64はオプションとして領域を小分けにする。
	/// ST1_8は更に小分けにしてフラグを収める。
	/// </summary>
	const enum mIndex :unsigned {
		BB_P64, BB_O64, BB_M64, OP64, ALPHAi16 = 12, BETAi16
		, DEPTH8=28, NUM_TURN8,ST1_8};
	/// <summary>
	/// IS_MY_TURN_NOWコンピュータの手番である。
	/// F1_IS_C_Xコンピューターが先手番とする。
	/// F1_IS_C2Cコンピューター同志の対戦。
	/// </summary>
	const enum ST1:uint8_t{IS_MY_TURN_NOW,IS_C_X, IS_C2C};
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
	void display_board_m(const __m256i m_board) const;
	void init_game_m();
	void init_game_bb();
	void init_game();
	__m256i __vectorcall change_turn_m(const __m256i m);
	bool is_draw_m(const __m256i m);
	/// <summary>
	/// 引き分けかどうか調べる。
	/// </summary>
	/// <param name="bb_X">ビットボードX</param>
	/// <param name="bb_C">ビットボードC</param>
	/// <returns>引き分けならtrue。それ以外false。</returns>
	bool is_draw_bb(const bb_t bb_X, const bb_t bb_C)const;
	bool is_win_m(const __m256i m);
	/// <summary>
	/// 対象のプレイヤーが勝ちかどうか調べる。
	/// </summary>
	/// <param name="p_board">調べる盤面のポインタ。</param>
	/// <param name="player">対象のプレイヤー。</param>
	/// <returns>勝ちならtrue、それ以外はfalse。</returns>
	bool is_win_bb(const bb_t bb_X, const bb_t bb_C, char player)const;
	bool is_game_over_m(__m256i m);
	/// <summary>
	/// 盤面がゲームオーバーかどうか調べる。
	/// </summary>
	/// <param name="p_board">対象の盤面のポインタ。</param>
	/// <returns>ゲームオーバーならtrue、それ以外はfalse。</returns>
	bool is_game_over_bb(const bb_t p_board_X, const bb_t p_board_C)const;
	void init_shift_table();
	bool __vectorcall is_valid_move_m(const __m256i m)const noexcept;
	/// <summary>
	/// その指し手が有効かどうか調べる。
	/// </summary>
	/// <param name="p_board">調べる盤面のポインタ。</param>
	/// <param name="row">行</param>
	/// <param name="col">列</param>
	/// <param name="ch">手番</param>
	/// <returns>有効ならtrue、無効ならfalseを返す。</returns>
	bool is_valid_move_bb(const bb_t src_X, const bb_t src_C, const int row, const int col, const char player)const noexcept;
	bool is_valid_move(const board_t* const p_board, int row, int col, char ch)const noexcept;
	__m256i __vectorcall get_valid_moves_m(const __m256i m) const;
	vector<pair<int, int>> get_valid_moves_bb(const bb_t bb_X, const bb_t bb_C, const char player)const;
	/// <summary>
	/// 有効な指し手全てを探し、vectorで返す。firstが行、secondが列。
	/// </summary>
	/// <param name="p_board">探す盤面へのポインタ。</param>
	/// <param name="ch">対象のプレイヤー。</param>
	/// <returns>pairのvector。</returns>
	vector<pair<int, int>> get_valid_moves(const board_t* const p_board, const char ch)const;
	vector<pair<int, int>> extract_coordinates(const bb_t m) const;
	inline bool existing_valid_moves_m(const __m256i m);
	bool existing_valid_moves_bb(const bb_t bb_X, const bb_t bb_C, const char player) const noexcept;
	bb_t transfer(bb_t put, unsigned k);
	/// <summary>
	/// 指定された盤面にプレイヤーchが指した手によって裏返る石を裏返させる。
	/// </summary>
	/// <param name="p_board">指定する盤面へのポインタ。盤面のデータは変更される。</param>
	/// <param name="row">指定する指し手の行。</param>
	/// <param name="col">指定する指し手の列。</param>
	/// <param name="ch">手番のプレイヤー</param>
	void update_board_bb(_Out_ bb_t& dst_X, _Out_ bb_t& dst_C, _In_ const bb_t src_X, _In_ const bb_t src_C, _In_ const int row, _In_ const int col, _In_ const char player) const noexcept;
	void update_board(board_t* const p_board, const int row, const int col, const char ch)const;
	/// <summary>
	/// ボードをコピーする。
	/// </summary>
	/// <param name="pdist">コピー先。</param>
	/// <param name="psource">コピー元。</param>
	/// 
	inline void copy_board_bb(bb_t& dist, const bb_t src)const noexcept;
	void copy_board(board_t* const pdist, const board_t* const psrc)const;
	/// <summary>
	/// 石の数を数える。
	/// </summary>
	/// <param name="p_board">'C'または'X'のビットボード。</param>
	/// <returns></returns>
	inline int get_count_bb(const bb_t bb)const noexcept;
	int get_count(const board_t* const p_board, const char player)const noexcept;
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
	int evaluate_bbS(const bb_t bb_X, const bb_t bb_C) const noexcept;
	int evaluate_by_turn(const __m256i m)const noexcept;
	int evaluateS(const board_t* const p_board) const noexcept;
	/// <summary>
	/// __m256型のビットボード。
	/// bbm.m256i_u64[BB_X]:'X'のビットボード
	/// bbm.m256i_u64[BB_C]:'C'のビットボード。
	/// bbm.m256i_u16[ALPHA]:alpha値。
	/// bbm.m256i_u16[BETA]:beta値。
	/// bbm.m256i_u16[DEPTH]:depth値。
	/// bbm.m256i_u16[PLAYER]:player('X','C')値。
	/// </summary>
	/// <returns>評価値。</returns>
	int alphabeta_m(const __m256i bbm)const;
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
	int minimax_bb_cnf(const bb_t bb_X, const bb_t bb_C, const char player, const depth_t depth)const;
	int minimax(const board_t* const p_board, const char player, int depth)const;
	/// <summary>
	/// 整合性確認用。読みの深さはMAX_DEPTHに設定されている。
	/// </summary>
	/// <param name="p_board">board_t型の盤面。</param>
	/// <param name="player">現在の手番。次の手番は関数の中で判断する。</param>
	/// <param name="depth">次手の読みの深さ。</param>
	/// <returns>評価値。</returns>
	int minimax_cnf(const board_t* const p_board, const char player, int depth)const;
	void return_minimax_m(node_t* const p_node);
	void return_minimax(node_t* const p_child);
	bool make_computer_move_m(pair<int, int>* p_pair);
	/// <summary>
	/// コンピュータが一手指す。成功ならp_pairの指す所にその座標を入れる。
	/// 失敗ならp_pairの指す内容は不定。
	/// </summary>
	/// <returns>成功ならtrue、失敗ならfalseが返る。</returns>
	bool make_computer_move_bb(pair<int, int>* p_pair);
	const PTP_WORK_CALLBACK pfnwkMiniMax;
	void conv_bbToBoard(board_t* const p_board, const bb_t bb_X, const bb_t bb_C)const;
	void convBoardTo_bb(bb_t& bbX, bb_t& bbC, const board_t*const board)const;
	bool is_equal(const board_t* const p_board, const bb_t boardX, const bb_t boardC)const;
	uint64_t MAKEQWORD(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f, uint8_t g, uint8_t h) {
		return ((uint64_t)a & 0xff) |
			((uint64_t)b & 0xff) << 8 |
			((uint64_t)c & 0xff) << 16 |
			((uint64_t)d & 0xff) << 24 |
			((uint64_t)e & 0xff) << 32 |
			((uint64_t)f & 0xff) << 40 |
			((uint64_t)g & 0xff) << 48 |
			((uint64_t)h & 0xff) << 56;
	}
};

