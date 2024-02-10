/**
 * @file mainReversi8.cpp
 * @brief メインの実装
 * @author Gold Smith
 * @date 2023 2024
 *
 * SPDX-License-Identifier: MIT
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 */

#include "pch.h"
#include "mainReversi8.h"

using namespace std;
using namespace Reversi8;
int main()
{
	cout << R"(/**
 * @file )" << _STRINGIZE(APPNAME)R"(
 * @brief Reversi. AlphaBeta. Multi thread. Bitboard. 8th.
 * @author Gold Smith
 * @date 2023 2024
 *
 * SPDX-License-Identifier: MIT
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 */
)" 
<< "threads:" << to_string(MAX_THREADS) << " depth:" << to_string(MAX_DEPTH)
<< endl << endl;

#ifdef CONFIRM_AVX2
	if (![]() {	return IsProcessorFeaturePresent(PF_XSAVE_ENABLED) &&
		IsProcessorFeaturePresent(PF_AVX2_INSTRUCTIONS_AVAILABLE);}()) {
		cout << "AVX2 is not supported on this processor." << endl;
		cout << "Press any key to exit." << endl;
		(void)_getch();
		return 0;
	}
#endif // CONFIRM_AVX2

	try {
		while (true) {
			// コンピュータ対戦の処理
			int order;
			cout << "先攻か後攻か選択してください。" << endl;
			cout << "1: 先攻" << endl;
			cout << "2: 後攻" << endl;
			cout << "3: 終了" << endl;
			cin >> order;
			if (cin.fail()) {
				cin.clear();
				cin.ignore(numeric_limits<streamsize>::max(), '\n');
				cout << "無効な入力です。" << endl;
				continue;
			}

			if (order == 1) {
				// 人が先攻の処理
				Game game;
				game.play_game(true, false);
			}
			else if (order == 2) {
				// 人が後攻の処理
				Game game;
				game.play_game(false, false);
			}
			else if (order == 3) {
				// 終了処理
				break;
			}
			else {
				cout << "無効な選択です。" << endl;
				cin.ignore(numeric_limits<streamsize>::max(), '\n');
				continue;
			}
		}
	}
	catch (const exception& e) {
		cerr << e.what() << endl;
		MessageBoxA(NULL, e.what(), "Error", MB_ICONEXCLAMATION);
	}
	_CrtDumpMemoryLeaks();
	return 0;
}
