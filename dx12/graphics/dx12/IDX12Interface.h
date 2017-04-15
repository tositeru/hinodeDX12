#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include "d3dx12.h"
#include <wrl/client.h>

namespace hinode
{
	namespace graphics
	{
		class IDX12Interface
		{
		public:
			virtual ~IDX12Interface() {}

			/// @brief リソースの名前をつける。
			/// GPUデバッグなどで利用できます。
			virtual HRESULT setName(const wchar_t* name) = 0;

			/// @brief 使用できる状態か?
			/// @retval bool
			virtual bool isGood()const noexcept = 0;
		};
	}
}
