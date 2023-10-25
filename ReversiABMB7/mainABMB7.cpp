/**
 * @file mainABMB7.cpp
 * @brief メインの実装
 * @author Gold Smith
 * @date 2023
 *
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 */

#include "mainABMB7.h"
#include <iostream>
#include <limits>
#include <exception>
#include <windows.h>
#include <crtdbg.h>

using namespace std;

int main()
{
	cout << R"(/**
 * @file )" << _STRINGIZE(APPNAME)R"(
 * @brief Reversi. AlphaBeta. Multi thread. Bitboard. 7th.
 * @author Gold Smith
 * @date 2023
 *
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 */
)" 
<< "threads:" << to_string(MAX_THREADS) << " depth:" << to_string(MAX_DEPTH)
<< endl << endl;

	try {
		;
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
				// 先攻の処理
				Game game;
				game.play_game(true, false);
			}
			else if (order == 2) {
				// 後攻の処理
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