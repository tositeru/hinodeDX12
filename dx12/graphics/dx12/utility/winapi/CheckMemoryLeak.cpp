#include "stdafx.h"

#include "CheckMemoryLeak.h"

namespace hinode
{
	namespace winapi
	{
		void StartMemoryLeakCheck(int num) {
			_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
			_CrtSetBreakAlloc(num);
		}
	}
}
