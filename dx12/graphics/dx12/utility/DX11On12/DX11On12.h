#pragma once

#include <d3d11on12.h>
#include <dwrite_3.h>
#include <d2d1_3.h>

#include "../../common/Common.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			class DX11On12
			{
			public:
				DX11On12();
				~DX11On12();

				void clear()noexcept;

				void create(ID3D12Device* pDevice, std::initializer_list<ID3D12CommandQueue*> pCmdQueues, UINT nodeMask = 0u);

			private:
				ID3D11On12Device* mpDX11Device;
				ID3D11DeviceContext* mpDX11DeviceContext;
				IDWriteFactory* mpWriteFactory;
				ID2D1Factory3* mpD2dFactory;
				ID2D1Device2* mpD2dDevice;
				ID2D1DeviceContext2* mpD2dDeviceContext;

			};
		}
	}
}