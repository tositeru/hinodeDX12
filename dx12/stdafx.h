// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーから使用されていない部分を除外します。

#define NOMINMAX //WINAPIのmin/maxマクロを定義しないためのもの

// TODO: プログラムに必要な追加ヘッダーをここで参照してください
#if defined(_DEBUG)
#define NEW  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define NEW ::new
#endif
