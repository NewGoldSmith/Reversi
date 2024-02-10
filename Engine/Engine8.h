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
#include "../BitManip/m256.h"
#include "MemoryRental.h"
#include "../Debug_fnc/debug_fnc.h"
#include "../BitManip/bit_manip_okuhara.hpp"

namespace Engine
{
	using namespace bit_manip;

	constexpr int INF = 1000;
	constexpr unsigned DEF_NODE_UNIT_SIZE = 0x1000;
	class Engine8
	{
	public:
		Engine8(PTP_POOL const ptpp
			, PTP_CALLBACK_ENVIRON const pcbe
			, PTP_CLEANUP_GROUP const ptpcg);
		~Engine8();
		bool search(m256 board
			, HANDLE hEvWait
			, uint16_t max_depth = 2
			, uint16_t first_depth = 2
			, uint64_t num_array = DEF_NODE_UNIT_SIZE);
		uint64_t await_best_move();
		int await_best_score();
	private:
		struct node_t;
		CRITICAL_SECTION cs = {};
		PTP_POOL const ptpp{};
		PTP_CALLBACK_ENVIRON const pcbe{};
		PTP_CLEANUP_GROUP const ptpcg{};
		const std::unique_ptr< CRITICAL_SECTION
			, decltype(DeleteCriticalSection)*> pcs;
		node_t* p_node_array{};
		MemoryRental<node_t>* p_mr_Node{};
		uint64_t num_array{};
		HANDLE hEvWait = NULL;
		m256 m_board{};
		uint16_t first_depth{};
		uint16_t second_depth{};
		int return_branch_cnt{};
		int called_branch_cnt{};
		std::vector<node_t*> vp_branch_nodes = {};
		int best_score{};
		uint64_t best_move{};
		struct node_t {
			m256 m_board{};
			CRITICAL_SECTION cs = {};
			const std::unique_ptr< CRITICAL_SECTION
				, decltype(DeleteCriticalSection)*> pcs;
			int score = 0;
			bool st_node_break = false;
			int called_branch_cnt = 0;
			int return_branch_cnt = 0;
			node_t* p_parent = nullptr;
			std::vector<node_t*> vp_branch_nodes = {};
			Engine8* pEngine = NULL;
			PTP_WORK pwk = NULL;
			node_t();
			node_t(const node_t&) = delete;
			node_t(node_t&&) = delete;
			node_t& operator () (const node_t&) = delete;
			node_t& operator = (const node_t&) = delete;
			bool operator == (node_t&)const = delete;
			bool operator != (node_t&)const = delete;
		};

		PTP_WORK_CALLBACK const pfnwkMiniMax;
		/// <summary>
		/// �w�肳�ꂽ�m�[�h�Ǝq�m�[�h���폜����B
		/// </summary>
		/// <param name="p_node"></param>
		void node_cut(node_t* const p_node);
		void node_break(node_t* const p_node);
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
		int alphabeta_m(m256 m)const noexcept;
		void return_minimax_m(node_t* const p_node);
	public:
		Engine8() = delete;
		Engine8(const Engine8&) = delete;
		Engine8(Engine8&&) = delete;
		Engine8& operator()(const Engine8&) = delete;
		Engine8& operator=(const Engine8&) = delete;
		Engine8& operator()(Engine8&&) = delete;
		Engine8& operator=(Engine8&&) = delete;
		bool operator==(const Engine8&)const = delete;
		bool operator!=(const Engine8&)const = delete;
	};

	inline int evaluate_by_turn(const m256 m) {
		if (m.ST.num_turn <= 32) {
			if (m.ST.is_my_turn_now) {
				return (int)__popcnt64(m.O) - (int)__popcnt64(m.P);
			}
			else {
				return (int)__popcnt64(m.P) - (int)__popcnt64(m.O);
			}
		}
		else {
			if (m.ST.is_my_turn_now) {
				return (int)__popcnt64(m.P) - (int)__popcnt64(m.O);
			}
			else {
				return (int)__popcnt64(m.O) - (int)__popcnt64(m.P);
			}
		}
	};

}
