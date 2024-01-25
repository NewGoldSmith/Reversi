/**
 * @file Engine8.h
 * @brief �v�l�G���W���̐錾
 * @author Gold Smith
 * @date 2024
 * SPDX-License-Identifier: MIT
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 *
 * ���̃t�@�C�����̂��ׂẴR�[�h�́A���ɖ��L����Ă��Ȃ�����AMIT���C�Z���X�ɏ]���܂��B
 */
#pragma once
#include "pch.h"
#include "MemoryRental.h"
#pragma comment(lib,  "../BitManip/" _STRINGIZE($CONFIGURATION) "/BitManip-" _STRINGIZE($CONFIGURATION) ".lib")
#pragma comment(lib,  "../Debug_fnc/" _STRINGIZE($CONFIGURATION) "/Debug_fnc-" _STRINGIZE($CONFIGURATION) ".lib")

namespace ReversiEngine
{
	constexpr int N = 8;
	constexpr int INF = 10000;
	constexpr int BREAK_CODE = INF + 5;
	constexpr unsigned NODE_UNIT_SIZE = 0x20000;

	class Engine8
	{
	public:
		Engine8() = delete;
		Engine8(PTP_POOL const ptpp
			, PTP_CALLBACK_ENVIRON const pcbe
			, PTP_CLEANUP_GROUP const ptpcg);
		~Engine8();
		Engine8(const Engine8&) = delete;
		Engine8(Engine8&&)noexcept = delete;
		Engine8& operator()(const Engine8&) = delete;
		Engine8& operator=(const Engine8&) = delete;
		bool operator==(const Engine8&) const = delete;
		bool search(__m256i board
			, HANDLE hEndEvent
			, uint16_t max_depth = 1
			, uint16_t first_depth = 1
			, uint64_t num_array = 0x2000);
		uint64_t await_best_move();
		__m256i __vectorcall change_turn_m(const __m256i m)const;
		/// <summary>
		/// �Ֆʂ̃f�[�^��ێ�����__m256i�̓��o���p�B
		/// �T�t�B�b�N�X�̐����͎g�p����r�b�g���B
		/// OP64�̓I�v�V�����Ƃ��ė̈���������ɂ���B
		/// ST1_8�͍X�ɏ������ɂ��ăt���O�����߂�B
		/// </summary>
		const enum mIndex :unsigned {
			BB_P64, BB_O64, BB_M64, OP64, ALPHAi16 = 12, BETAi16
			, DEPTH8 = 28, NUM_TURN8, ST1_8};
		/// <summary>
		/// IS_MY_TURN_NOW�R���s���[�^�̎�Ԃł���B
		/// F1_IS_C_X�R���s���[�^�[�����ԂƂ���B
		/// F1_IS_C2C�R���s���[�^�[���u�̑ΐ�B
		/// </summary>
		const enum ST1 :uint8_t { IS_MY_TURN_NOW, IS_C_X, IS_C2C };
	private:
		struct node_t;
		CRITICAL_SECTION cs = {};
		PTP_POOL const ptpp{};
		PTP_CALLBACK_ENVIRON const pcbe{};
		PTP_CLEANUP_GROUP const ptpcg{};
		//const PTP_CLEANUP_GROUP_CANCEL_CALLBACK pfng{};
		const std::unique_ptr< CRITICAL_SECTION
			, decltype(DeleteCriticalSection)*> pcs;
		node_t* p_node_array;
		MemoryRental<node_t>* p_mr_Node;
		uint64_t num_array;
		HANDLE hWaitEvent = NULL;
		__m256i m_board{};
		uint16_t first_depth{};
		uint16_t second_depth{};
		int return_branch_cnt{};
		int called_branch_cnt{};
		std::vector<node_t*> vp_branch_nodes = {};
		int best_score{};
		uint64_t best_move{};
		struct node_t {
			__m256i m_board{};
			CRITICAL_SECTION cs = {};
			const std::unique_ptr< CRITICAL_SECTION
				, decltype(DeleteCriticalSection)*> pcs;
			int score = 0;
			int called_branch_cnt = 0;
			int return_branch_cnt = 0;
			node_t* p_parent = nullptr;
			std::vector<node_t*> vp_branch_nodes = {};
			Engine8* pEngine = NULL;
			PTP_WORK pwk = NULL;

			node_t();
			node_t(node_t&) = delete;
			node_t(node_t&&)noexcept = delete;
			node_t& operator () (node_t&) = delete;
			node_t& operator = (node_t&) = delete;
			bool operator == (node_t&)const = delete;
		};

		PTP_WORK_CALLBACK const pfnwkMiniMax;
		/// <summary>
		/// �w�肳�ꂽ�m�[�h�Ǝq�m�[�h���폜����B
		/// </summary>
		/// <param name="p_node"></param>
		void node_cut(node_t* const p_node);
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
		int alphabeta_m(const __m256i m)const;
		void return_minimax_m(node_t* const p_node);
		int evaluate_by_turn(const __m256i m)const noexcept;


	};
}
