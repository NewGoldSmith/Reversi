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
using namespace Engine;

Engine::Engine8::Engine8(PTP_POOL const ptpp, PTP_CALLBACK_ENVIRON const pcbe, PTP_CLEANUP_GROUP const ptpcg)
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

		if (pNode->st_node_break) {
			return;
		}

		if (pNode->m_board.ST.depth >= pNode->pEngine->first_depth)
		{
			pNode->score = pNode->pEngine->alphabeta_m(pNode->m_board);
			pNode->pEngine->return_minimax_m(pNode);
			return;
		}
		
		m256 next_board = make_next_turn_m(pNode->m_board);
		next_board.M = okuhara::get_moves256(next_board);
		if (!next_board.M) {
			next_board = make_next_turn_m(next_board);
			next_board.M = okuhara::get_moves256(next_board);
			if (!next_board.M) {
				// 勝敗確定
				pNode->score = evaluate_by_turn(pNode->m_board);
				pNode->pEngine->return_minimax_m(pNode);
				return;
			}
		}
		++next_board.ST.depth;
		++next_board.ST.num_turn;
		pNode->vp_branch_nodes = {};

		for (DWORD index; _BitScanReverse64(&index, next_board.M);) {
			node_t* pBranchNode = pNode->pEngine->p_mr_Node->Lend();
			pBranchNode->m_board = next_board;
			pBranchNode->called_branch_cnt = 0;
			pBranchNode->return_branch_cnt = 0;
			pBranchNode->pEngine = pNode->pEngine;
			pBranchNode->score = next_board.ST.is_my_turn_now ? -INF : INF;
			pBranchNode->m_board.M = 1ULL << index;
			next_board.M &= ~(pBranchNode->m_board.M);
			pBranchNode->m_board = okuhara::flip256(pBranchNode->m_board);
			pBranchNode->p_parent = pNode;
			if (!(pBranchNode->pwk = CreateThreadpoolWork(
				pNode->pEngine->pfnwkMiniMax
				, pBranchNode
				, &*pNode->pEngine->pcbe))) {
				throw std::exception(EOut.c_str());
			}
			SubmitThreadpoolWork(pBranchNode->pwk);
			pNode->vp_branch_nodes.push_back(pBranchNode);
		}
		pNode->called_branch_cnt = (int)pNode->vp_branch_nodes.size();
	}
}

{
}


Engine::Engine8::~Engine8()
{
	if (p_mr_Node) {
		delete p_mr_Node;
	}
	if (p_node_array) {
		delete []p_node_array;
	}
}

bool Engine::Engine8::search(
	m256 board
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
	uint64_t moves = okuhara::get_moves256(board);
	if (!moves) {
		SetEvent(hEvWait);
		return 0;
	}

	if (p_node_array) {
		delete[]p_node_array;
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
	best_score = m_board.ST.is_my_turn_now ? -INF : INF;
	best_move = 0;
	vp_branch_nodes.clear();

	m_board.ST.alpha = -INF;
	m_board.ST.beta = INF;
	m_board.ST.depth = 0;
	++m_board.ST.num_turn;
	m_board.ST.ID = 0;

	unique_ptr<CRITICAL_SECTION, decltype(LeaveCriticalSection)*> lock_engine = {
	[this]() {EnterCriticalSection(&cs); return &cs; }()
	, LeaveCriticalSection };

	for (DWORD index; _BitScanReverse64(&index, moves);) {
		node_t* pBranchNode = p_mr_Node->Lend();
		pBranchNode->m_board = m_board;
		pBranchNode->m_board.M = 1ULL << index;
		moves &= ~(pBranchNode->m_board.M);
		pBranchNode->m_board = okuhara::flip256(pBranchNode->m_board);
		pBranchNode->score = best_score;
		pBranchNode->pEngine = this;
		if (!(pBranchNode->pwk = CreateThreadpoolWork(pfnwkMiniMax, pBranchNode, &*pcbe))) {
			throw std::exception(EOut.c_str());
		}
		SubmitThreadpoolWork(pBranchNode->pwk);
		vp_branch_nodes.push_back(pBranchNode);
	}
	called_branch_cnt= (int)vp_branch_nodes.size();
	return true;
}

uint64_t Engine::Engine8::await_best_move()
{
	::WaitForSingleObject(hEvWait, INFINITE);
	return best_move;
}

int Engine::Engine8::await_best_score()
{
	::WaitForSingleObject(hEvWait, INFINITE);
	return best_score;
}

void Engine::Engine8::node_cut(node_t* const p_node)
{
	unique_ptr<CRITICAL_SECTION, decltype(LeaveCriticalSection)*> lock_node = {
		[p_node]() {EnterCriticalSection(&p_node->cs); return &p_node->cs; }()
		,LeaveCriticalSection };
	for (node_t* &p : p_node->vp_branch_nodes) {
		node_cut(p);
	}
	CloseThreadpoolWork(p_node->pwk);
	p_node->vp_branch_nodes.clear();
	p_mr_Node->Return(p_node);
}

void Engine::Engine8::node_break(node_t* const p_node)
{
	unique_ptr<CRITICAL_SECTION, decltype(LeaveCriticalSection)*> lock_node = {
		[p_node]() {EnterCriticalSection(&p_node->cs); return &p_node->cs; }()
		,LeaveCriticalSection };
	for (node_t* &p : p_node->vp_branch_nodes) {
		node_break(p);
	}
	p_node->st_node_break = true;
}

int Engine::Engine8::alphabeta_m(m256 m) const noexcept
{
	if (m.ST.depth >= second_depth)
	{
		return Engine::evaluate_by_turn(m);
	}
	
	m256 next_m = make_next_turn_m(m);
	next_m.M = okuhara::get_moves256(next_m);

	if (!next_m.M) {
		next_m = make_next_turn_m(next_m);
		next_m.M = okuhara::get_moves256(next_m);
		if (!next_m.M) {
			// 勝敗確定
			return evaluate_by_turn(next_m);
		}
	}
	++next_m.ST.depth;
	++next_m.ST.num_turn;
	if (m.ST.is_my_turn_now) {
		for (DWORD index; _BitScanReverse64(&index, next_m.M);) {
			m256 m_branch = next_m;
			m_branch.M = 1ULL << index;
			next_m.M &= ~(m_branch.M);
			m_branch = okuhara::flip256(m_branch);
			next_m.ST.alpha = max<int16_t>(next_m.ST.alpha, alphabeta_m(m_branch));
			if (next_m.ST.alpha >= next_m.ST.beta) {
				break;
			}
		}
		return next_m.ST.alpha;
	}
	else {
		for (DWORD index; _BitScanReverse64(&index, next_m.M);) {
			m256 m_branch = next_m;
			m_branch.M = 1ULL << index;
			next_m.M &= ~(m_branch.M);
			m_branch = okuhara::flip256(m_branch);
			next_m.ST.beta = min<int16_t>(next_m.ST.beta, alphabeta_m(m_branch));
			if (next_m.ST.alpha >= next_m.ST.beta) {
				break;
			}
		}
		return next_m.ST.beta;
	}
}

void Engine::Engine8::return_minimax_m(node_t* const p_node)
{
	using namespace std;
	using namespace debug_fnc;
	unique_ptr<CRITICAL_SECTION, decltype(LeaveCriticalSection)*> lock_node = {
		[p_node]() {EnterCriticalSection(&p_node->cs);return &p_node->cs;}()
		,LeaveCriticalSection };
	if (p_node->p_parent) {
		unique_ptr<CRITICAL_SECTION, decltype(LeaveCriticalSection)*> lock_parent = {
		[p_node]() {
			EnterCriticalSection(&p_node->p_parent->cs);
			return &p_node->p_parent->cs;
			}(), LeaveCriticalSection
		};
		p_node->p_parent->return_branch_cnt++;
		if (p_node->p_parent->m_board.ST.is_my_turn_now) {
			if (p_node->p_parent->score <= p_node->score) {
				p_node->p_parent->score = p_node->score;
			}
		}
		else {
			if (p_node->p_parent->score >= p_node->score) {
				p_node->p_parent->score = p_node->score;
			}
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
		if (m_board.ST.is_my_turn_now) {
			if (best_score < p_node->score) {
				best_score = p_node->score;
				best_move = p_node->m_board.M;
			}
		}
		else {
			if (best_score > p_node->score) {
				best_score = p_node->score;
				best_move = p_node->m_board.M;
			}
		}
		if (return_branch_cnt >= called_branch_cnt) {
			for (node_t* &p_node : vp_branch_nodes) {
				node_cut(p_node);
			}
			vp_branch_nodes.clear();
			::SetEvent(hEvWait);
		}
	}
}

Engine::Engine8::node_t::node_t() :
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
