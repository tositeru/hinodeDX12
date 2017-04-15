#include "stdafx.h"

#include "DX12Sampler.h"

#include <cfloat>

namespace hinode
{
	namespace graphics
	{
		DX12SamplerDesc::DX12SamplerDesc()noexcept
		{
			(*this)
				.setAddress(D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP)
				.setFilter(D3D12_FILTER_MIN_MAG_MIP_LINEAR)
				.setLOD(0.f, FLT_MAX, 0.f)
				.setComparisonFunc(D3D12_COMPARISON_FUNC_ALWAYS)
				.setBorderColor(1, 1, 1, 1)
				;
		}

		DX12SamplerDesc& DX12SamplerDesc::setAddress(D3D12_TEXTURE_ADDRESS_MODE u, D3D12_TEXTURE_ADDRESS_MODE v, D3D12_TEXTURE_ADDRESS_MODE w)noexcept
		{
			this->AddressU = u;
			this->AddressV = v;
			this->AddressW = w;
			return *this;
		}

		DX12SamplerDesc& DX12SamplerDesc::setFilter(D3D12_FILTER filter, UINT maxAnisotropy)noexcept
		{
			this->Filter = filter;
			this->MaxAnisotropy = maxAnisotropy;
			return *this;
		}

		DX12SamplerDesc& DX12SamplerDesc::setLOD(float min, float max, float bias)noexcept
		{
			this->MinLOD = min;
			this->MaxLOD = max;
			this->MipLODBias = bias;
			return *this;
		}

		DX12SamplerDesc& DX12SamplerDesc::setComparisonFunc(D3D12_COMPARISON_FUNC func)noexcept
		{
			this->ComparisonFunc = func;
			return *this;
		}

		DX12SamplerDesc& DX12SamplerDesc::setBorderColor(float r, float g, float b, float a)noexcept
		{
			this->BorderColor[0] = r;
			this->BorderColor[1] = g;
			this->BorderColor[2] = b;
			this->BorderColor[3] = a;
			return *this;
		}

	}
}