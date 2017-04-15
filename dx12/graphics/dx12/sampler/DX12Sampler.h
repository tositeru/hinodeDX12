#pragma once

#include <d3d12.h>

namespace hinode
{
	namespace graphics
	{
		struct DX12SamplerDesc : public D3D12_SAMPLER_DESC
		{
			DX12SamplerDesc()noexcept;

			DX12SamplerDesc& setAddress(D3D12_TEXTURE_ADDRESS_MODE u, D3D12_TEXTURE_ADDRESS_MODE v, D3D12_TEXTURE_ADDRESS_MODE w)noexcept;
			DX12SamplerDesc& setFilter(D3D12_FILTER filter, UINT maxAnisotropy=0)noexcept;
			DX12SamplerDesc& setLOD(float min, float max, float bias=0.f)noexcept;
			DX12SamplerDesc& setComparisonFunc(D3D12_COMPARISON_FUNC func)noexcept;
			DX12SamplerDesc& setBorderColor(float r, float g, float b, float a)noexcept;
		};
	}
}