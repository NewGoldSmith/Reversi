/**
 * @file Engine8.cpp
 * @brief 思考エンジンの実装
 * @author Gold Smith
 * @date 2023
 * SPDX-License-Identifier: MIT
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 *
 * このファイル内のすべてのコードは、特に明記されていない限り、MITライセンスに従います。
 */
#include "pch.h"
#include "Engine8.h"
using namespace bit_manip;
using namespace debug_fnc;
using namespace std;
using namespace ReversiEngine;

ReversiEngine::Engine8::Engine8(PTP_POOL const ptpp, PTP_CALLBACK_ENVIRON const pcbe, PTP_CLEANUP_GROUP const ptpcg)
	:
	ptpp(ptpp)
	, pcbe(pcbe)
	, ptpcg(ptpcg)
	, pcs{ [this]() {
			InitializeCriticalSection(&cs);
			return &cs;
		}()
		,
		[](CRITICAL_SECTION* pcs) {
			DeleteCriticalSection(pcs);
		}
	}

	, pfnwkMiniMax{ [](
		_Inout_     PTP_CALLBACK_INSTANCE Instance,
		_Inout_opt_ PVOID                 Context,
		_Inout_     PTP_WORK              Work) {
		if (!Context) {
			_D("Context is NULL.");
		  throw std::invalid_argument("Context is NULL.");
		}
		node_t* const pNode = reinterpret_cast<node_t*>(Context);

		std::unique_ptr<CRITICAL_SECTION, decltype(LeaveCriticalSection)*> unique_lock = {
		  [pNode]() {EnterCriticalSection(&pNode->cs); return &pNode->cs; }()
		  ,LeaveCriticalSection };

		if (pNode->m_board.m256i_u8[mIndex::DEPTH8] >= pNode->pEngine->first_depth)
		{
			pNode->score = pNode->pEngine->alphabeta_m(pNode->m_board);
			pNode->pEngine->return_minimax_m(pNode);
			return;
		}
		
		__m256i next_board = pNode->pEngine->make_next_turn_m(pNode->m_board);
		next_board = okuhara::get_moves256(next_board);
		if (!next_board.m256i_u64[mIndex::BB_M64]) {
			next_board = pNode->pEngine->make_next_turn_m(next_board);
			next_board = okuhara::get_moves256(next_board);
			if (!next_board.m256i_u64[mIndex::BB_M64]) {
				// 勝敗確定
				_D("勝敗確定");
				int score = pNode->pEngine->evaluate_by_turn(pNode->m_board);
				pNode->score = score >= 0 ? INF - (int)pNode->m_board.m256i_u8[mIndex::DEPTH8]
						: -INF + (int)pNode->m_board.m256i_u8[mIndex::DEPTH8];
				pNode->pEngine->return_minimax_m(pNode);
				return;
			}
		}

		pNode->score = CHECK_BIT(pNode->m_board.m256i_u8[mIndex::ST1_8], ST1::IS_MY_TURN_NOW) ? -INF : INF;
		pNode->vp_branch_nodes = {};

		for (; next_board.m256i_u64[mIndex::BB_M64];) {
			++ pNode->called_branch_cnt;
			node_t* pBranchNode = pNode->pEngine->p_mr_Node->Lend();
			pBranchNode->m_board = next_board;
			pBranchNode->called_branch_cnt = 0;
			pBranchNode->return_branch_cnt = 0;
			pBranchNode->pEngine = pNode->pEngine;
			DWORD index;
			_BitScanReverse64(&index, next_board.m256i_u64[mIndex::BB_M64]);
			pBranchNode->m_board.m256i_u64[mIndex::BB_M64] = BIT<uint64_t>(index);
			next_board.m256i_u64[mIndex::BB_M64] = RESET_BIT(next_board.m256i_u64[mIndex::BB_M64], index);
			pBranchNode->m_board = okuhara::flip256(pBranchNode->m_board);
			++ pBranchNode->m_board.m256i_u8[mIndex::DEPTH8];
			pBranchNode->p_parent = pNode;
			if (!(pBranchNode->pwk = CreateThreadpoolWork(
				pNode->pEngine->pfnwkMiniMax
				, pBranchNode
				, &*pNode->pEngine->pcbe))) {
				EOut;
				MessageBoxA(NULL, "Err.", "CreateThreadpoolWork", MB_ICONEXCLAMATION);
				return;
			}
			SubmitThreadpoolWork(pBranchNode->pwk);
			pNode->vp_branch_nodes.push_back(pBranchNode);
		}
		pNode->called_branch_cnt = (int)pNode->vp_branch_nodes.size();
	}
}

{
}


ReversiEngine::Engine8::~Engine8()
{
	if (p_mr_Node) {
		delete p_mr_Node;
		p_mr_Node = NULL;
	}
	if (p_node_array) {
		delete []p_node_array;
		p_node_array = NULL;
	}
}

bool ReversiEngine::Engine8::search(
	__m256i board
	, HANDLE hEvWait
	, uint16_t max_depth
	, uint16_t first_depth
	,uint64_t num_array)
{
	if (!(this->hEvWait = hEvWait)) {
		throw std::invalid_argument("hEvWait is NULL.");
	}
	if ((max_depth - first_depth) < 0) {
		throw std::invalid_argument("max_depth - first_depth is invalid.");
	}
	if (num_array & (num_array - 1)) {
		_D("num_array must be Power of Two.");
		throw std::invalid_argument("num_array must be Power of Two.");
	}
	ResetEvent(this->hEvWait);
	uint64_t moves = okuhara::get_moves256(board).m256i_u64[mIndex::BB_M64];
	if (!moves) {
		SetEvent(hEvWait);
		return 0;
	}

	if (p_node_array) {
		delete []p_node_array;
	}
	p_node_array = new node_t[num_array];
	if (p_mr_Node) {
		delete p_mr_Node;
	}
	p_mr_Node = new MemoryRental<node_t>(p_node_array, num_array);

	m_board = board;
	this->first_depth = first_depth;
	this->second_depth = max_depth - first_depth;
	called_branch_cnt = 0;
	return_branch_cnt = 0;
	if (CHECK_BIT(m_board.m256i_u8[mIndex::ST1_8], ST1::IS_MY_TURN_NOW)) {
		best_score = -INF;
	}
	else {
		best_score = INF;
	}
	best_move = 0;
	vp_branch_nodes.clear();

	m_board.m256i_i16[mIndex::ALPHAi16] = -INF;
	m_board.m256i_i16[mIndex::BETAi16] = INF;

	for (unsigned long index; _BitScanReverse64(&index, moves);) {
		node_t* pBranchNode = p_mr_Node->Lend();
		pBranchNode->m_board = m_board;
		pBranchNode->m_board.m256i_u64[mIndex::BB_M64] = 1ULL << index;
		moves = RESET_BIT(moves, index);
		pBranchNode->m_board = okuhara::flip256(pBranchNode->m_board);
		pBranchNode->m_board.m256i_u8[mIndex::DEPTH8] = 0;
		pBranchNode->called_branch_cnt = 0;
		pBranchNode->return_branch_cnt = 0;
		pBranchNode->p_parent = NULL;
		pBranchNode->vp_branch_nodes.clear();
		pBranchNode->pEngine = this;
		vp_branch_nodes.push_back(pBranchNode);
		++called_branch_cnt;
		if (!(pBranchNode->pwk = CreateThreadpoolWork(pfnwkMiniMax, pBranchNode, &*pcbe))) {
			EOut;
			return false;
		}
		SubmitThreadpoolWork(pBranchNode->pwk);
	}
	return true;
}

uint64_t ReversiEngine::Engine8::await_best_move()
{
	::WaitForSingleObject(hEvWait, INFINITE);
	return best_move;
}

int ReversiEngine::Engine8::await_best_score()
{
	::WaitForSingleObject(hEvWait, INFINITE);
	return best_score;
}

void ReversiEngine::Engine8::node_cut(node_t* const p_node)
{
	for (node_t* p : p_node->vp_branch_nodes) {
		node_cut(p);
	}
	CloseThreadpoolWork(p_node->pwk);
	p_node->vp_branch_nodes.clear();
	p_mr_Node->Return(p_node);
}

__m256i __vectorcall ReversiEngine::Engine8::make_next_turn_m(const __m256i m)const
{
	__m256i tmp = _mm256_permute4x64_epi64(m, _MM_SHUFFLE(3, 2, 0, 1));
	tmp.m256i_u8[mIndex::ST1_8] = bit_manip::TOGGLE_BIT(m.m256i_u8[mIndex::ST1_8], ST1::IS_MY_TURN_NOW);
	return tmp;
}

int ReversiEngine::Engine8::alphabeta_m(const __m256i m) const
{
	if (m.m256i_u8[mIndex::DEPTH8] >= second_depth)
	{
		int score = evaluate_by_turn(m);
		return score;
	}
	
	__m256i next_m = make_next_turn_m(m);
	next_m = okuhara::get_moves256(next_m);

	if (!next_m.m256i_u64[mIndex::BB_M64]) {
		next_m = make_next_turn_m(next_m);
		next_m = okuhara::get_moves256(next_m);
		if (!next_m.m256i_u64[mIndex::BB_M64]) {
			// 勝敗確定
			_D("勝敗確定");
			int score = evaluate_by_turn(next_m);
			score = score >= 0 ? INF - (int)next_m.m256i_u8[mIndex::DEPTH8]
				: -INF + (int)next_m.m256i_u8[mIndex::DEPTH8];
			return score;
		}
	}

	if (CHECK_BIT(m.m256i_u8[mIndex::ST1_8],ST1::IS_MY_TURN_NOW)) {

		for (; next_m.m256i_u64[mIndex::BB_M64];) {
			__m256i m_branch = next_m;
			DWORD index;
			_BitScanReverse64(&index, next_m.m256i_u64[mIndex::BB_M64]);
			m_branch.m256i_u64[mIndex::BB_M64] = BIT<uint64_t>(index);
			next_m.m256i_u64[mIndex::BB_M64] = RESET_BIT(next_m.m256i_u64[mIndex::BB_M64], index);
			m_branch = okuhara::flip256(m_branch);
			++m_branch.m256i_u8[mIndex::DEPTH8];
			next_m.m256i_i16[mIndex::ALPHAi16] = max<short>(next_m.m256i_i16[mIndex::ALPHAi16], alphabeta_m(m_branch));
			if (next_m.m256i_i16[mIndex::ALPHAi16] >= next_m.m256i_i16[mIndex::BETAi16]) {
				break;
			}
		}
		return next_m.m256i_i16[mIndex::ALPHAi16];
	}
	else {
		for (; next_m.m256i_u64[mIndex::BB_M64];) {
			__m256i m_child = next_m;
			DWORD index;
			_BitScanReverse64(&index, next_m.m256i_u64[mIndex::BB_M64]);
			m_child.m256i_u64[mIndex::BB_M64] = BIT<uint64_t>(index);
			next_m.m256i_u64[mIndex::BB_M64] = RESET_BIT(next_m.m256i_u64[mIndex::BB_M64], index);
			m_child = okuhara::flip256(m_child);
			++m_child.m256i_u8[mIndex::DEPTH8];
			next_m.m256i_i16[mIndex::BETAi16] = min<short>(next_m.m256i_i16[mIndex::BETAi16], alphabeta_m(m_child));
			if (next_m.m256i_i16[mIndex::ALPHAi16] >= next_m.m256i_i16[mIndex::BETAi16]) {
				break;
			}
		}
		return next_m.m256i_i16[mIndex::BETAi16];
	}
}

void ReversiEngine::Engine8::return_minimax_m(node_t* const p_node)
{
	using namespace std;
	using namespace debug_fnc;
	unique_ptr<CRITICAL_SECTION, decltype(LeaveCriticalSection)*> lock_node = {
		[p_node]() {
			EnterCriticalSection(&p_node->cs);
			return &p_node->cs;
		}(),LeaveCriticalSection };
	if (p_node->p_parent) {
		unique_ptr<CRITICAL_SECTION, decltype(LeaveCriticalSection)*> lock_parent = {
		[p_node]() {
			EnterCriticalSection(&p_node->p_parent->cs);
			return &p_node->p_parent->cs;
			}(), LeaveCriticalSection
		};
		p_node->p_parent->return_branch_cnt++;
		if (CHECK_BIT(p_node->p_parent->m_board.m256i_u8[mIndex::ST1_8], ST1::IS_MY_TURN_NOW)) {
				p_node->p_parent->score = std::max(p_node->p_parent->score, p_node->score);
		}
		else {
			p_node->p_parent->score = std::min(p_node->p_parent->score, p_node->score);
		}
		if (p_node->p_parent->return_branch_cnt >= p_node->p_parent->called_branch_cnt) {
			return_minimax_m(p_node->p_parent);
		}
		return;
	}
	else {// if(p_node->p_parent)
		unique_ptr<CRITICAL_SECTION, decltype(LeaveCriticalSection)*> lock_engine = {
		[this]() {EnterCriticalSection(&cs); return &cs;	}(), LeaveCriticalSection };
		return_branch_cnt++;
		if (CHECK_BIT(m_board.m256i_u8[mIndex::ST1_8], ST1::IS_MY_TURN_NOW)) {
			if (best_score < p_node->score) {
				best_score = p_node->score;
				best_move = p_node->m_board.m256i_u64[mIndex::BB_M64];
			}
		}
		else {
			if (best_score > p_node->score) {
				best_score = p_node->score;
				best_move = p_node->m_board.m256i_u64[mIndex::BB_M64];
			}
		}
		if (return_branch_cnt >= called_branch_cnt) {
			::SetEvent(hEvWait);
			for (node_t* p_node : vp_branch_nodes) {
				node_cut(p_node);
			}
		}
	}
}

int ReversiEngine::Engine8::evaluate_by_turn(const __m256i m) const noexcept
{
	if (CHECK_BIT(m.m256i_u8[mIndex::ST1_8], ST1::IS_MY_TURN_NOW)) {
		return (int)__popcnt64(m.m256i_u64[mIndex::BB_P64]) - (int)__popcnt64(m.m256i_u64[mIndex::BB_O64]);
	}
	else {
		return (int)__popcnt64(m.m256i_u64[mIndex::BB_O64]) - (int)__popcnt64(m.m256i_u64[mIndex::BB_P64]);
	}
}

ReversiEngine::Engine8::node_t::node_t() :
	pcs{ [this]() {
	 InitializeCriticalSection(&cs);
	 return &cs;	}()
	 ,
	 [](CRITICAL_SECTION* pcs) {
		 DeleteCriticalSection(pcs);
	 }
}

{
}
