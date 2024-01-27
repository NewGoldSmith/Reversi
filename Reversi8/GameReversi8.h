/**
 * @file GameReversi8.h
 * @brief �Q�[���N���X�̐錾
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
#define FIRST_DEPTH  3U// 1�ȏ�ɐݒ�v�B
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
	typedef unsigned int depth_t;

	HANDLE hWaitEvent=NULL;
	const unique_ptr<remove_pointer_t<HANDLE>, decltype(CloseHandle)*> uhWaitEvent;

	LARGE_INTEGER TotalTime = {};
	LARGE_INTEGER StartingTime{}, EndingTime{}, ElapsedMicroseconds{};
	const LARGE_INTEGER Frequency;
	/// <summary>
	/// �X���b�h�v�[�������̃m�[�h
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
	/// �w�肳�ꂽ�m�[�h�Ǝq�m�[�h���폜����B
	/// </summary>
	/// <param name="p_node"></param>
	void node_cut(node_t* const p_node);

	const unique_ptr<node_t,void(*)(node_t[])>node_array;
	MemoryRental<node_t>mr_Node;

	//board_t board;
	///// <summary>
	///// ���݂�X�̃r�b�g�{�[�h�B
	///// </summary>
	//bb_t bb_cur_X = 0ULL;
	///// <summary>
	///// ���݂�C�̃r�b�g�{�[�h�B
	///// </summary>
	//bb_t bb_cur_C = 0ULL;
	/// <summary>
	/// __m256�^�̃r�b�g�{�[�h�B
	/// __m256i_u64[BB_P64]:�R���s���[�^���猩�����g�̃{�[�h�B
	/// __m256i_u64[BB_O64]:����{�[�h�B
	/// __m256i_u64[BB_M64]:move�ʒu�B
	/// __m256i_u64[OP64]:���̑��̃X�e�[�^�X�B���L�Q�ƁB
	/// __m256i_i16[ALPHAi16]:alpha�l�B
	/// __m256i_i16[BETAi16]:beta�l�B
	/// __m256i_u8[DEPTH8]:depth�l�B�[���Ȃ�قǐ��l��������B
	/// __m256i_u8[NUM_TURN8]:turn�l�B
	/// __m256i_u8[ST1_8]:�X�e�[�^�X�P�l�B
	/// CHECK_BIT(__m256i_u8[ST1_8],IS_MY_TURN):�����̃^�[�����ǂ����B
	/// CHECK_BIT(__m256i_u8[ST1_8],IS_C_X):�R���s���[�^�����i���j���ǂ����B
	/// CHECK_BIT(__m256i_u8[ST1_8],IS_C2C):�R���s���[�^���u�̑ΐ킩�ǂ����B
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
	/// �Ֆʂ̃f�[�^��ێ�����__m256i�̓��o���p�B
	/// �T�t�B�b�N�X�̐����͎g�p����r�b�g���B
	/// OP64�̓I�v�V�����Ƃ��ė̈���������ɂ���B
	/// ST1_8�͍X�ɏ������ɂ��ăt���O�����߂�B
	/// </summary>
	const enum mIndex :unsigned {
		BB_P64, BB_O64, BB_M64, OP64, ALPHAi16 = 12, BETAi16
		, DEPTH8=28, NUM_TURN8,ST1_8};
	/// <summary>
	/// IS_MY_TURN_NOW�R���s���[�^�̎�Ԃł���B
	/// F1_IS_C_X�R���s���[�^�[�����ԂƂ���B
	/// F1_IS_C2C�R���s���[�^�[���u�̑ΐ�B
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
	void display_board_m(const __m256i m_board) const;
	void init_game_m();
	void init_game_bb();
	void init_game();
	__m256i __vectorcall change_turn_m(const __m256i m);
	bool is_draw_m(const __m256i m);
	/// <summary>
	/// �����������ǂ������ׂ�B
	/// </summary>
	/// <param name="bb_X">�r�b�g�{�[�hX</param>
	/// <param name="bb_C">�r�b�g�{�[�hC</param>
	/// <returns>���������Ȃ�true�B����ȊOfalse�B</returns>
	bool is_draw_bb(const bb_t bb_X, const bb_t bb_C)const;
	bool is_win_m(const __m256i m);
	/// <summary>
	/// �Ώۂ̃v���C���[���������ǂ������ׂ�B
	/// </summary>
	/// <param name="p_board">���ׂ�Ֆʂ̃|�C���^�B</param>
	/// <param name="player">�Ώۂ̃v���C���[�B</param>
	/// <returns>�����Ȃ�true�A����ȊO��false�B</returns>
	bool is_win_bb(const bb_t bb_X, const bb_t bb_C, char player)const;
	bool is_game_over_m(__m256i m);
	/// <summary>
	/// �Ֆʂ��Q�[���I�[�o�[���ǂ������ׂ�B
	/// </summary>
	/// <param name="p_board">�Ώۂ̔Ֆʂ̃|�C���^�B</param>
	/// <returns>�Q�[���I�[�o�[�Ȃ�true�A����ȊO��false�B</returns>
	bool is_game_over_bb(const bb_t p_board_X, const bb_t p_board_C)const;
	void init_shift_table();
	bool __vectorcall is_valid_move_m(const __m256i m)const noexcept;
	/// <summary>
	/// ���̎w���肪�L�����ǂ������ׂ�B
	/// </summary>
	/// <param name="p_board">���ׂ�Ֆʂ̃|�C���^�B</param>
	/// <param name="row">�s</param>
	/// <param name="col">��</param>
	/// <param name="ch">���</param>
	/// <returns>�L���Ȃ�true�A�����Ȃ�false��Ԃ��B</returns>
	bool is_valid_move_bb(const bb_t src_X, const bb_t src_C, const int row, const int col, const char player)const noexcept;
	bool is_valid_move(const board_t* const p_board, int row, int col, char ch)const noexcept;
	__m256i __vectorcall get_valid_moves_m(const __m256i m) const;
	vector<pair<int, int>> get_valid_moves_bb(const bb_t bb_X, const bb_t bb_C, const char player)const;
	/// <summary>
	/// �L���Ȏw����S�Ă�T���Avector�ŕԂ��Bfirst���s�Asecond����B
	/// </summary>
	/// <param name="p_board">�T���Ֆʂւ̃|�C���^�B</param>
	/// <param name="ch">�Ώۂ̃v���C���[�B</param>
	/// <returns>pair��vector�B</returns>
	vector<pair<int, int>> get_valid_moves(const board_t* const p_board, const char ch)const;
	vector<pair<int, int>> extract_coordinates(const bb_t m) const;
	inline bool existing_valid_moves_m(const __m256i m);
	bool existing_valid_moves_bb(const bb_t bb_X, const bb_t bb_C, const char player) const noexcept;
	bb_t transfer(bb_t put, unsigned k);
	/// <summary>
	/// �w�肳�ꂽ�ՖʂɃv���C���[ch���w������ɂ���ė��Ԃ�΂𗠕Ԃ�����B
	/// </summary>
	/// <param name="p_board">�w�肷��Ֆʂւ̃|�C���^�B�Ֆʂ̃f�[�^�͕ύX�����B</param>
	/// <param name="row">�w�肷��w����̍s�B</param>
	/// <param name="col">�w�肷��w����̗�B</param>
	/// <param name="ch">��Ԃ̃v���C���[</param>
	void update_board_bb(_Out_ bb_t& dst_X, _Out_ bb_t& dst_C, _In_ const bb_t src_X, _In_ const bb_t src_C, _In_ const int row, _In_ const int col, _In_ const char player) const noexcept;
	void update_board(board_t* const p_board, const int row, const int col, const char ch)const;
	/// <summary>
	/// �{�[�h���R�s�[����B
	/// </summary>
	/// <param name="pdist">�R�s�[��B</param>
	/// <param name="psource">�R�s�[���B</param>
	/// 
	inline void copy_board_bb(bb_t& dist, const bb_t src)const noexcept;
	void copy_board(board_t* const pdist, const board_t* const psrc)const;
	/// <summary>
	/// �΂̐��𐔂���B
	/// </summary>
	/// <param name="p_board">'C'�܂���'X'�̃r�b�g�{�[�h�B</param>
	/// <returns></returns>
	inline int get_count_bb(const bb_t bb)const noexcept;
	int get_count(const board_t* const p_board, const char player)const noexcept;
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
	int evaluate_bbS(const bb_t bb_X, const bb_t bb_C) const noexcept;
	int evaluate_by_turn(const __m256i m)const noexcept;
	int evaluateS(const board_t* const p_board) const noexcept;
	/// <summary>
	/// __m256�^�̃r�b�g�{�[�h�B
	/// bbm.m256i_u64[BB_X]:'X'�̃r�b�g�{�[�h
	/// bbm.m256i_u64[BB_C]:'C'�̃r�b�g�{�[�h�B
	/// bbm.m256i_u16[ALPHA]:alpha�l�B
	/// bbm.m256i_u16[BETA]:beta�l�B
	/// bbm.m256i_u16[DEPTH]:depth�l�B
	/// bbm.m256i_u16[PLAYER]:player('X','C')�l�B
	/// </summary>
	/// <returns>�]���l�B</returns>
	int alphabeta_m(const __m256i bbm)const;
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
	int minimax_bb_cnf(const bb_t bb_X, const bb_t bb_C, const char player, const depth_t depth)const;
	int minimax(const board_t* const p_board, const char player, int depth)const;
	/// <summary>
	/// �������m�F�p�B�ǂ݂̐[����MAX_DEPTH�ɐݒ肳��Ă���B
	/// </summary>
	/// <param name="p_board">board_t�^�̔ՖʁB</param>
	/// <param name="player">���݂̎�ԁB���̎�Ԃ͊֐��̒��Ŕ��f����B</param>
	/// <param name="depth">����̓ǂ݂̐[���B</param>
	/// <returns>�]���l�B</returns>
	int minimax_cnf(const board_t* const p_board, const char player, int depth)const;
	void return_minimax_m(node_t* const p_node);
	void return_minimax(node_t* const p_child);
	bool make_computer_move_m(pair<int, int>* p_pair);
	/// <summary>
	/// �R���s���[�^�����w���B�����Ȃ�p_pair�̎w�����ɂ��̍��W������B
	/// ���s�Ȃ�p_pair�̎w�����e�͕s��B
	/// </summary>
	/// <returns>�����Ȃ�true�A���s�Ȃ�false���Ԃ�B</returns>
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

