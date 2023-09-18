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
	/// �����board_t�Ƃ����^��`�B
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
	/// ���ۂ̃Q�[���X�^�[�g�B
	/// </summary>
	/// <param name="human_first">�l����U���B</param>
	/// <param name="two_player">2�v���[���[�ΐ킩�B</param>
	void play_game(bool human_first, bool two_player);
private:
	/// <summary>
	/// ���݂�board�̔Ֆʂ�\������B
	/// </summary>
	void display_board()const;
	void init_game();
	/// <summary>
	/// ���̔Ֆʂ������������ǂ������ׂ�B
	/// </summary>
	/// <param name="p_board">���ׂ�Ֆʂ̃|�C���^�B</param>
	/// <returns>���������Ȃ�true�A���������łȂ��Ȃ�false��Ԃ��B</returns>
	bool is_draw(const board_t* const p_board)const;
	/// <summary>
	/// �Ώۂ̃v���C���[���������ǂ������ׂ�B
	/// </summary>
	/// <param name="p_board">���ׂ�Ֆʂ̃|�C���^�B</param>
	/// <param name="player">�Ώۂ̃v���C���[�B</param>
	/// <returns>�����Ȃ�true�A����ȊO��false�B</returns>
	bool is_win(const board_t* const p_board, char player)const;
	/// <summary>
	/// �Ֆʂ��Q�[���I�[�o�[���ǂ������ׂ�B
	/// </summary>
	/// <param name="p_board">�Ώۂ̔Ֆʂ̃|�C���^�B</param>
	/// <returns>�Q�[���I�[�o�[�Ȃ�true�A����ȊO��false�B</returns>
	bool is_game_over(const board_t* const p_board)const;
	/// <summary>
	/// ���̎w���肪�L�����ǂ������ׂ�B
	/// </summary>
	/// <param name="p_board">���ׂ�Ֆʂ̃|�C���^�B</param>
	/// <param name="row">�s</param>
	/// <param name="col">��</param>
	/// <param name="ch">���</param>
	/// <returns>�L���Ȃ�true�A�����Ȃ�false��Ԃ��B</returns>
	bool is_valid_move(const board_t* const p_board, int row, int col, char ch)const;
	/// <summary>
	/// �L���Ȏw����S�Ă�T���Avector�ŕԂ��Bfirst���s�Asecond����B
	/// </summary>
	/// <param name="p_board">�T���Ֆʂւ̃|�C���^�B</param>
	/// <param name="ch">�Ώۂ̃v���C���[�B</param>
	/// <returns>pair��vector�B</returns>
	vector<pair<int, int>> get_valid_moves(const board_t* const p_board, char ch)const;
	/// <summary>
	/// �w�肳�ꂽ�ՖʂɃv���C���[ch���w������ɂ���ė��Ԃ�΂𗠕Ԃ�����B
	/// </summary>
	/// <param name="p_board">�w�肷��Ֆʂւ̃|�C���^�B�Ֆʂ̃f�[�^�͕ύX�����B</param>
	/// <param name="row">�w�肷��w����̍s�B</param>
	/// <param name="col">�w�肷��w����̗�B</param>
	/// <param name="ch">��Ԃ̃v���C���[</param>
	void update_board(board_t* const p_board, int row, int col, char ch)const;
	void copy_board(board_t* const pdist, const board_t* const psource)const;
	/// <summary>
	/// ���ꂼ��̐��𓾂�B
	/// </summary>
	/// <param name="p_board">�X�R�A�𓾂�Ֆʂ̃|�C���^�B</param>
	/// <returns>std::pair�^�̃X�R�A�Bfirst X�̐��Bsecond C�̐��B</returns>
	pair<int, int> get_each_countXC(const board_t* const p_board)const;
	/// <summary>
	/// �]���֐�
	/// </summary>
	/// <param name="ch">�Ώۂ̎��</param>
	/// <param name="depth">�ǂݐ[��</param>
	/// <param name="pb_is_seddled">�������������������ĂȂ����̌��ʂ��󂯎��|�C���^�B</param>
	/// <returns>�]���l</returns>
	int evaluate(const board_t* const  p_board, char ch, int depth)const;
	int alphabeta(const board_t* const p_board, const char player, int depth, int alpha, int beta)const;
	int alphabeta2(const board_t* const p_board, const char player, int depth, int alpha, int beta,HANDLE phStopEvent)const;
	int minimax(const board_t* const p_poard, const char player, int depth)const;
	/// <summary>
	/// �R���s���[�^�����w���B�����Ȃ�p_pair�̎w�����ɂ��̍��W������B
	/// ���s�Ȃ�p_pair�̎w�����e�͕s��B
	/// </summary>
	/// <returns>�����Ȃ�true�A���s�Ȃ�false���Ԃ�B</returns>
	bool make_computer_move(pair<int, int>* p_pair);
	const PTP_WORK_CALLBACK pfnwkMiniMax;
	/// <summary>
	/// �f�o�b�N�o�͂Ƀ{�[�h��\���B
	/// </summary>
	void dout_board_()const;
#if defined(_DEBUG)
#define dout_board() dout_board_()
#else
#define dout_board() __noop
#endif
};

