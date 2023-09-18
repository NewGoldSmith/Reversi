/**
 * @file Game.h
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
using namespace std;
constexpr int N = 8;
constexpr int INF = 1000000;
constexpr int MAX_DEPTH = 4;
class Game
{
public:
	Game();
	~Game();
	bool operator==(const Game& other) const = delete;
	/// <summary>
	/// ���ۂ̃Q�[���X�^�[�g�B
	/// </summary>
	/// <param name="human_first">�l����U���B</param>
	/// <param name="two_player">2�v���[���[�ΐ킩�B</param>
	void play_game(bool human_first, bool two_player);
private:
	/// <summary>
	/// �����board_t�Ƃ����^��`�B
	/// </summary>
	typedef char board_t[N][N];
	board_t board;
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
	bool is_draw(const board_t* p_board)const;
	/// <summary>
	/// �Ώۂ̃v���C���[���������ǂ������ׂ�B
	/// </summary>
	/// <param name="p_board">���ׂ�Ֆʂ̃|�C���^�B</param>
	/// <param name="player">�Ώۂ̃v���C���[�B</param>
	/// <returns>�����Ȃ�true�A����ȊO��false�B</returns>
	bool is_win(const board_t* p_board, char player)const;
	/// <summary>
	/// �Ֆʂ��Q�[���I�[�o�[���ǂ������ׂ�B
	/// </summary>
	/// <param name="p_board">�Ώۂ̔Ֆʂ̃|�C���^�B</param>
	/// <returns>�Q�[���I�[�o�[�Ȃ�true�A����ȊO��false�B</returns>
	bool is_game_over(const board_t* p_board)const;
	/// <summary>
	/// ���̎w���肪�L�����ǂ������ׂ�B
	/// </summary>
	/// <param name="p_board">���ׂ�Ֆʂ̃|�C���^�B</param>
	/// <param name="row">�s</param>
	/// <param name="col">��</param>
	/// <param name="ch">���</param>
	/// <returns>�L���Ȃ�true�A�����Ȃ�false��Ԃ��B</returns>
	bool is_valid_move(const board_t* p_board, int row, int col, char ch)const;
	/// <summary>
	/// �L���Ȏw����S�Ă�T���Avector�ŕԂ��Bfirst���s�Asecond����B
	/// </summary>
	/// <param name="p_board">�T���Ֆʂւ̃|�C���^�B</param>
	/// <param name="ch">�Ώۂ̃v���C���[�B</param>
	/// <returns>pair��vector�B</returns>
	vector<pair<int, int>> get_valid_moves(const board_t* p_board, char ch)const;
	/// <summary>
	/// �w�肳�ꂽ�ՖʂɃv���C���[ch���w������ɂ���ė��Ԃ�΂𗠕Ԃ�����B
	/// </summary>
	/// <param name="p_board">�w�肷��Ֆʂւ̃|�C���^�B�Ֆʂ̃f�[�^�͕ύX�����B</param>
	/// <param name="row">�w�肷��w����̍s�B</param>
	/// <param name="col">�w�肷��w����̗�B</param>
	/// <param name="ch">��Ԃ̃v���C���[</param>
	void update_board(board_t* p_board, int row, int col, char ch)const;
	/// <summary>
	/// ���ꂼ��̐��𓾂�B
	/// </summary>
	/// <param name="p_board">�X�R�A�𓾂�Ֆʂ̃|�C���^�B</param>
	/// <returns>std::pair�^�̃X�R�A�Bfirst X�̐��Bsecond C�̐��B</returns>
	pair<int, int> get_score(const board_t* p_board)const;
	/// <summary>
	/// �]���֐�
	/// </summary>
	/// <param name="ch">�Ώۂ̎��</param>
	/// <param name="depth">�ǂݐ[��</param>
	/// <param name="pb_is_seddled">�������������������ĂȂ����̌��ʂ��󂯎��|�C���^�B</param>
	/// <returns>�]���l</returns>
	int evaluate(const board_t* p_board, char ch, int depth, bool* pb_is_seddled)const;
	/// <summary>
	/// ��ǂ݊֐��B
	/// </summary>
	/// <param name="depth">�ǂ݂̐[���B</param>
	/// <param name="ch">���</param>
	/// <returns>�ł��ǂ��]���l</returns>
	int minimax(const board_t* pboard, int depth, char ch)const;
	/// <summary>
	/// �R���s���[�^�����w���B�����Ȃ�p_pair�̎w�����ɂ��̍��W������B
	/// ���s�Ȃ�p_pair�̎w�����e�͕s��B
	/// </summary>
	/// <returns>�����Ȃ�true�A���s�Ȃ�false���Ԃ�B</returns>
	bool make_computer_move(pair<int, int>* p_pair);
	inline void copy_board(board_t& dist, const board_t& source)const;
	/// <summary>
	/// �f�o�b�N�o�͂Ƀ{�[�h��\���B
	/// </summary>
	void dout_borad()const;
	/// <summary>
	/// �f�o�b�N�o�͂ɕ������o�́B
	/// </summary>
	/// <param name="str">�o�͂��镶���B</param>
	void dout(const std::string& str)const;
};

