// pch.h: プリコンパイル済みヘッダー ファイルです。
// 次のファイルは、その後のビルドのビルド パフォーマンスを向上させるため 1 回だけコンパイルされます。
// コード補完や多くのコード参照機能などの IntelliSense パフォーマンスにも影響します。
// ただし、ここに一覧表示されているファイルは、ビルド間でいずれかが更新されると、すべてが再コンパイルされます。
// 頻繁に更新するファイルをここに追加しないでください。追加すると、パフォーマンス上の利点がなくなります。
#pragma once

#ifndef PCH_H
#define PCH_H

#define NOMINMAX
#include <debugapi.h>
#include <string>
#include <windows.h>
#include<conio.h>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <iosfwd>
#include <limits>
#include <sstream>
#include < immintrin.h >
#include <cstdint>
#include <emmintrin.h>
#include <iostream>
#include <xmmintrin.h>
#include "../Engine/Engine8.h"
#include "../Debug_fnc/debug_fnc.h"
#include "../BitManip/bit_manip.h"
#include "../BitManip/bit_manip_okuhara.h"
#endif //PCH_H
