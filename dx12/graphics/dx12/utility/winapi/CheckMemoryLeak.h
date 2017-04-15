#pragma once

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

namespace hinode
{
	namespace winapi
	{
		/// @brief メモリリークを調べ始める
		///
		///	リークした際、リークを起こしたコードのファイルと行番号を表示したい際は
		/// 以下のコードをプリコンパイル済みヘッダーなど
		///	全ファイルから見える場所に書いてください
		///	ex)	#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
		///
		/// @param[in] num default=0 num回目のメモリ確保のときに実行を中断します
		void StartMemoryLeakCheck(int num = 0);
	}
}
