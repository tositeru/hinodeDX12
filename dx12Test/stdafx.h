// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

//websocketppとasioを導入したことでインクルード順によってWINAPI絡みの再定義などエラーが出るようになったので、その回避策
//参考サイト:http://kouma.hatenablog.com/entry/2012/08/15/232143
#define _WINSOCKAPI_

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <new>


// TODO: プログラムに必要な追加ヘッダーをここで参照してください
#if defined(_DEBUG)
#define NEW  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define NEW ::new
#endif
