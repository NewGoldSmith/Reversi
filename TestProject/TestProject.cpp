#include "pch.h"
#include "TestProject.h"


using namespace std;
using namespace bit_manip;
using namespace debug_fnc;
using namespace Engine;

#define p1 0b\
00000001\
10001110\
10001110\
10001110\
10001110\
10001110\
10001110\
01111110ULL
#define o1 0b\
01111110\
01110001\
01110001\
01110001\
01110001\
01110001\
01110001\
10000000ULL
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
			SetThreadpoolThreadMaximum(ptpp, 1);
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

		m256 m = (m256)_mm256_setr_epi64x(o1, p1, m1, 0);
		m.ST.is_my_turn_now = 1;
		m.ST.is_x = 0;
		m.ST.num_turn = (uint8_t)__popcnt64(m.P | m.O) - 4;
		if (!debug_fnc::chk_bb(m)) {
			return S_FALSE;
		}
		_DOB(m);
		_DOS(m);
		Engine::Engine8 engine(ptpp.get(), pcbe.get(), ptpcg.get());

		uint8_t depth =2;
		uint8_t first_depth = 0;
		for (;;) {
			if (!engine.search(m, uhWaitEvent.get(), depth, first_depth)) {
				if (!engine.search((m = make_next_turn_m(m)), uhWaitEvent.get(), depth, first_depth)) {
					break;
				}
			}
			m.M = engine.await_best_move();
			m = okuhara::flip256(m);
			++m.ST.num_turn;
			_DOS(m);
			m = make_next_turn_m(m);
		}
	}
	catch (std::exception& e) {
		_D(e.what());
		MessageBoxA(NULL, (std::string("error.") + e.what()).c_str(), "error", MB_ICONEXCLAMATION);
	}
	_CrtDumpMemoryLeaks();
	return 0;
}
