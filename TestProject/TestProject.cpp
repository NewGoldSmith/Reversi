#include "pch.h"
#include "TestProject.h"


using namespace std;
using namespace bit_manip;
using namespace debug_fnc;
using namespace ReversiEngine;

#define p1 0b\
00000000\
00000000\
00000000\
00001000\
00010000\
00000000\
00000000\
00000000ULL
#define o1 0b\
00000000\
00000000\
00000000\
00010000\
00001000\
00000000\
00000000\
00000000ULL
#define m1 0b\
00000000\
00000000\
00000000\
00000000\
00000000\
00000000\
00000000\
00000000ULL

int main() {
	try {
		const unique_ptr<TP_POOL, decltype(CloseThreadpool)*>ptpp = { []() {
			PTP_POOL ptpp = CreateThreadpool(NULL);
			if (!ptpp) {
				throw runtime_error(EOut);
			}
			SetThreadpoolThreadMaximum(ptpp, 3);
			return ptpp; }(),CloseThreadpool };

		const unique_ptr<TP_CALLBACK_ENVIRON, void(*)(TP_CALLBACK_ENVIRON*)>pcbe{ [](PTP_POOL pt) {
				const auto pcbe1 = new TP_CALLBACK_ENVIRON;
				InitializeThreadpoolEnvironment(pcbe1);
				SetThreadpoolCallbackPool(pcbe1, pt);
				SetThreadpoolCallbackRunsLong(pcbe1);
				return pcbe1;
			}(ptpp.get())
				, [](PTP_CALLBACK_ENVIRON pcbe) {DestroyThreadpoolEnvironment(pcbe); delete pcbe; } };

		PTP_CLEANUP_GROUP_CANCEL_CALLBACK const pfng{ [](
				 _Inout_opt_ PVOID ObjectContext,
				 _Inout_opt_ PVOID CleanupContext) {
				if (!ObjectContext) {
					dout("Context is NULL.");
					return;
				}
				else {
					void* pnode = ObjectContext;
				}
				if (!CleanupContext) {
				}
				else {
					void* pnode = CleanupContext;
				}
			}
		};

		const unique_ptr<TP_CLEANUP_GROUP, void(*)(PTP_CLEANUP_GROUP)>ptpcg
		{ [](PTP_CALLBACK_ENVIRON pcbe,PTP_CLEANUP_GROUP_CANCEL_CALLBACK pfn) {
			PTP_CLEANUP_GROUP const ptpcg = CreateThreadpoolCleanupGroup();
			SetThreadpoolCallbackCleanupGroup(pcbe, ptpcg, pfn);
			return ptpcg; }(pcbe.get(),pfng)
		,	[](PTP_CLEANUP_GROUP ptpcg) {
			CloseThreadpoolCleanupGroupMembers(ptpcg,FALSE,(void*)-1);
			CloseThreadpoolCleanupGroup(ptpcg);	}
		};
		const unique_ptr<remove_pointer_t<HANDLE>, decltype(CloseHandle)*> uhWaitEvent = { []() {
			HANDLE h;
			if (!(h = CreateEvent(NULL, TRUE, FALSE, NULL))) {
				throw runtime_error(EOut);
			}return h; }()	,CloseHandle };

		__m256i m = _mm256_setr_epi64x(p1, o1, 0, 0);
		m.m256i_u8[Engine8::mIndex::ST1_8] = SET_BIT(m.m256i_u8[Engine8::mIndex::ST1_8], Engine8::ST1::IS_MY_TURN_NOW);
		_DOB(m);
		ReversiEngine::Engine8 engine(ptpp.get(), pcbe.get(), ptpcg.get());
		ResetEvent(uhWaitEvent.get());
		engine.search(m, uhWaitEvent.get(),2);
		uint64_t res = engine.await_best_move();
		m.m256i_u64[ReversiEngine::Engine8::mIndex::BB_M64] = res;
		m = okuhara::flip256(m);
		_DOB(m);
		m = engine.make_next_turn_m(m);
		_DOB(m);
		ResetEvent(uhWaitEvent.get());
		if (!engine.search(m, uhWaitEvent.get())) {
			_D("no moves.");
		};
		res = engine.await_best_move();
		m.m256i_u64[ReversiEngine::Engine8::mIndex::BB_M64] = res;
		m = okuhara::flip256(m);
		_DOB(m);
	}
	catch (std::runtime_error& e) {
		_D(e.what());
		MessageBoxA(NULL, (std::string("error.") + e.what()).c_str(), "error", MB_ICONEXCLAMATION);
	}
	_CrtDumpMemoryLeaks();
	return 0;
}
