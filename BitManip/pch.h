﻿// pch.h: プリコンパイル済みヘッダー ファイルです。
// 次のファイルは、その後のビルドのビルド パフォーマンスを向上させるため 1 回だけコンパイルされます。
// コード補完や多くのコード参照機能などの IntelliSense パフォーマンスにも影響します。
// ただし、ここに一覧表示されているファイルは、ビルド間でいずれかが更新されると、すべてが再コンパイルされます。
// 頻繁に更新するファイルをここに追加しないでください。追加すると、パフォーマンス上の利点がなくなります。
#pragma once

#ifndef PCH_H
#define PCH_H

#define NOMINMAX
// プリコンパイルするヘッダーをここに追加します
#include <windows.h>
#include <string>
//#include <windef.h>
#include <vector>
#include <debugapi.h>
#include <sstream>
#include "framework.h"
#include < immintrin.h >
#include <cstdint>
#include <emmintrin.h>
#include <iostream>
#include <xmmintrin.h>
#include "../Debug_fnc/debug_fnc.h"
#endif //PCH_H