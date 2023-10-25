/**
 * @file GameABMG.h
 * @brief �Q�[���N���X�̐錾
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
#define FIRST_DEPTH  2U// 1�ȏ�ɐݒ�v�B
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
	/// �����board_t�Ƃ����^��`�B
	/// �����\�L���W(1,1)�Ƃ��A�E����(N,N)�Ƃ���B
	/// �C���f�b�N�X�w��ł�(0,0)����(N-1,N-1)�ƂȂ�B
	/// </summary>
	typedef char board_t[N][N];

	/// <summary>
	/// board_t�̃r�b�g�{�[�h�ŁB�����0x8000000000000000�Ƃ��A
	/// �E����0x0000000000000001�Ƃ���B
	/// MSB��(0,0)��LSB��(8,8)
	/// </summary>
	typedef uint64_t bb_t;
	/// <summary>
	/// �ǂ݂̐[���̌^
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
	/// ���ۂ̃Q�[���X�^�[�g�B
	/// </summary>
	/// <param name="human_first">�l����U���B</param>
	/// <param name="two_player">2�v���[���[�ΐ킩�B</param>
	void play_game(bool human_first, bool two_player);
private:
	/// <summary>
	/// ���݂̔Ֆʂ�\������B
	/// </summary>
	void display_board_bb(const bb_t bbX, const bb_t bbC)const;
	void init_game_bb();
	void init_game();
	/// <summary>
	/// ���̔Ֆʂ������������ǂ������ׂ�B
	/// </summary>
	/// <param name="p_board">���ׂ�Ֆʂ̃|�C���^�B</param>
	/// <returns>���������Ȃ�true�A���������łȂ��Ȃ�false��Ԃ��B</returns>
	bool is_draw_bb(const bb_t bb_X, const bb_t bb_C)const;
	/// <summary>
	/// �Ώۂ̃v���C���[���������ǂ������ׂ�B
	/// </summary>
	/// <param name="p_board">���ׂ�Ֆʂ̃|�C���^�B</param>
	/// <param name="player">�Ώۂ̃v���C���[�B</param>
	/// <returns>�����Ȃ�true�A����ȊO��false�B</returns>
	bool is_win_bb(const bb_t bb_X, const bb_t bb_C, char player)const;
	/// <summary>
	/// �Ֆʂ��Q�[���I�[�o�[���ǂ������ׂ�B
	/// </summary>
	/// <param name="p_board">�Ώۂ̔Ֆʂ̃|�C���^�B</param>
	/// <returns>�Q�[���I�[�o�[�Ȃ�true�A����ȊO��false�B</returns>
	bool is_game_over_bb(const bb_t p_board_X, const bb_t p_board_C)const;
	void init_shift_table();
	/// <summary>
	/// �r�b�g�{�[�h���1�}�X���V�t�g�����l��Ԃ��B
	/// 8x8�̃r�b�g�{�[�h�p�B�V�t�g�ɂ��ח�ւ̃r�b�g�̉�荞�݂�h���B
	/// </summary>
	/// <param name="b">�r�b�g�{�[�h�B</param>
	/// <param name="direction">enum Direction�ŁA�V�t�g��������������B
	/// 0:��A1:�E��A2:�E�A3:�E���A4:���A5:�����A6:���A7:����A
	/// ����ȊO:�ω������B</param>
	/// <returns>�V�t�g�����r�b�g�{�[�h�l�B</returns>
	bb_t shift(const bb_t b, const unsigned direction)const;
	/// <summary>
	/// ���̎w���肪�L�����ǂ������ׂ�B
	/// </summary>
	/// <param name="p_board">���ׂ�Ֆʂ̃|�C���^�B</param>
	/// <param name="row">�s</param>
	/// <param name="col">��</param>
	/// <param name="ch">���</param>
	/// <returns>�L���Ȃ�true�A�����Ȃ�false��Ԃ��B</returns>
	bool is_valid_move_bb(const bb_t src_X, const bb_t src_C, const int row, const int col, const char player)const;
	bool is_valid_move(const board_t* const p_board, int row, int col, char ch)const;
	/// <summary>
	/// �L���Ȏw����S�Ă�T���Avector�ŕԂ��Bfirst���s�Asecond����B
	/// </summary>
	/// <param name="p_board">�T���Ֆʂւ̃|�C���^�B</param>
	/// <param name="ch">�Ώۂ̃v���C���[�B</param>
	/// <returns>pair��vector�B</returns>
	vector<pair<int, int>> get_valid_moves_bb(const bb_t bb_X, const bb_t bb_C, const char player)const;
	inline vector<pair<int, int>> get_valid_moves(const board_t* const p_board, const char ch)const;
	bool existing_valid_moves_bb(const bb_t bb_X, const bb_t bb_C, const char player) const;
	
	/// <summary>
	/// �w�肳�ꂽ�ՖʂɃv���C���[ch���w������ɂ���ė��Ԃ�΂𗠕Ԃ�����B
	/// </summary>
	/// <param name="p_board">�w�肷��Ֆʂւ̃|�C���^�B�Ֆʂ̃f�[�^�͕ύX�����B</param>
	/// <param name="row">�w�肷��w����̍s�B</param>
	/// <param name="col">�w�肷��w����̗�B</param>
	/// <param name="ch">��Ԃ̃v���C���[</param>
	void update_board_bb(_Inout_ bb_t& dst_X, _Inout_ bb_t& dst_C, _In_ const bb_t src_X, _In_ const bb_t src_C, _In_ const int row, _In_ const int col, _In_ const char player) const;
	//inline void update_board_bb(bb_t& p_board_X, bb_t& p_board_C, int row, int col, char ch)const;
	void update_board(board_t* const p_board, const int row, const int col, const char ch)const;
	/// <summary>
	/// �{�[�h���R�s�[����B
	/// </summary>
	/// <param name="pdist">�R�s�[��B</param>
	/// <param name="psource">�R�s�[���B</param>
	inline void copy_board_bb(bb_t& dist, const bb_t src)const;
	void copy_board(board_t* const pdist, const board_t* const psrc)const;
	/// <summary>
	/// �΂̐��𐔂���B
	/// </summary>
	/// <param name="p_board">'C'�܂���'X'�̔Ֆʂւ̃|�C���^�B</param>
	/// <returns></returns>
	inline int get_count_bb(const bb_t bb)const;
	int get_count(const board_t* const p_board, const char player)const;
	/// <summary>
	/// �]���֐�
	/// </summary>
	/// <param name="ch">�Ώۂ̎��</param>
	/// <param name="depth">�ǂݐ[��</param>
	/// <returns>�]���l</returns>
	int evaluate_bb(const bb_t bb_X, const bb_t bb_C, const char player, const depth_t depth)const;
	/// <summary>
	/// �]���֐��B'C'���猩���X�R�A��Ԃ��B
	/// </summary>
	/// <param name="board_X">'X'�̃{�[�h</param>
	/// <param name="board_C">'C'�̃{�[�h</param>
	/// <param name="depth">�ǂ݂̐[���B</param>
	/// <returns>�]���l</returns>
	int evaluate_bbG(const bb_t bb_X, const bb_t bb_C, const char player, const depth_t depth)  const;
	int evaluate(const board_t* const  p_board, char ch)const;
	inline int evaluate_bbS(const bb_t bb_X, const bb_t bb_C) const;
	//int evaluateG(board_t board_X, board_t board_C, depth_t depth)const;
	//int evaluateS(board_t board_X, board_t board_C)const;
	/// <summary>
	/// �T���G���W���B
	/// </summary>
	/// <param name="p_board_X">'X'�̔ՖʁB</param>
	/// <param name="p_board_C">'C'�̔ՖʁB</param>
	/// <param name="player">��ԁB</param>
	/// <param name="depth">�ǂ݂̐[���B</param>
	/// <param name="alpha">alpha�l�B</param>
	/// <param name="beta">beta�l�B</param>
	/// <returns>�]���l�B</returns>
	int alphabeta_bb(const bb_t bb_X, const bb_t bb_C,const char player, const depth_t depth, int alpha, int beta)const;
	int alphabeta(const board_t* const p_board, const char player, int depth, int alpha, int beta)const;
	int minimax_bb(const bb_t bb_X, const bb_t bb_C, const char player, const depth_t depth)const;
	/// <summary>
	/// �������m�F�p�B�ǂ݂̐[����MAX_DEPTH�ɐݒ肳��Ă���B
	/// </summary>
	/// <param name="bb_X">X�̃r�b�g�{�[�h�B</param>
	/// <param name="bb_C">C�̃r�b�g�{�[�h�B</param>
	/// <param name="player">���w����̃v���[���[�B����͊֐��̒��Ŕ��f�B</param>
	/// <param name="depth">����̓ǂ݂̐[���B</param>
	/// <returns>�]���l�B</returns>
	int minimax_bb_conf(const bb_t bb_X, const bb_t bb_C, const char player, const depth_t depth)const;
	int minimax(const board_t* const p_board, const char player, int depth)const;
	/// <summary>
	/// �������m�F�p�B�ǂ݂̐[����MAX_DEPTH�ɐݒ肳��Ă���B
	/// </summary>
	/// <param name="p_board">board_t�^�̔ՖʁB</param>
	/// <param name="player">���݂̎�ԁB���̎�Ԃ͊֐��̒��Ŕ��f����B</param>
	/// <param name="depth">����̓ǂ݂̐[���B</param>
	/// <returns>�]���l�B</returns>
	int minimax_conf(const board_t* const p_board, const char player, int depth)const;
	void return_minimax(node_t* const p_child);
	/// <summary>
	/// �R���s���[�^�����w���B�����Ȃ�p_pair�̎w�����ɂ��̍��W������B
	/// ���s�Ȃ�p_pair�̎w�����e�͕s��B
	/// </summary>
	/// <returns>�����Ȃ�true�A���s�Ȃ�false���Ԃ�B</returns>
	bool make_computer_move_bb(pair<int, int>* p_pair);
	const PTP_WORK_CALLBACK pfnwkMiniMax;
	void conv_bbToBoard(board_t* const p_board, const bb_t bb_X, const bb_t bb_C)const;
	void convBoardTo_bb(bb_t& bbX, bb_t& bbC, const board_t*const board)const;
	/// <summary>
	/// �f�o�b�N�o�͂Ƀ{�[�h��\���B
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

