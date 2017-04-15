#pragma once

#include <array>
#include <graphics/dx12/utility/math/SimpleMath.h>
#include <graphics/dx12/graphicsCommandList/DX12GraphicsCommandList.h>
#include <graphics/dx12/resource/DX12Resource.h>
#include <graphics/dx12/utility/Shader/PipelineSet/PipelineSet.h>
#include <graphics/dx12/utility/RingBuffer/ConstantBufferPool/ConstantBufferPool.h>
#include <graphics/dx12/utility/RingBuffer/DescriptorHeapPool/DescriptorHeapPool.h>

class SSAOManager
{
	SSAOManager(SSAOManager&) = delete;
	SSAOManager& operator=(SSAOManager&) = delete;

public:
	struct Param {
		float radiusInWorld = 1.5f;
		float beta = 0.001f;
		float sigma = 1;
		float K = 0.8f;

		hinode::uint samplingStep = 2;
		hinode::uint samplingCount = 6;
		hinode::uint samplingRadius = 12;	///< サンプリングする半径 (ピクセル単位)

		int blurRadius = 8;
		float blurRangeInWorld = 0.5f;
		float blurGaussianSigma = 3.f;

		hinode::math::float2		nearFarZ;
		hinode::math::float4x4		projMatrix;

		hinode::graphics::DX12Resource* pSceneDepth = nullptr;
		int sceneDepthSRVIndex = 0;

		hinode::graphics::DX12Resource* pSceneImage = nullptr;
		int sceneImageSRVIndex = 0;
	};

	enum RESOURCE_VIEW_TYPE {
		eVIEW_SRV,
		eVIEW_UAV,
	};

public:
	SSAOManager();
	~SSAOManager();

	void clear();
	void create(ID3D12Device* pDevice, UINT width, UINT height, UINT nodeMask = 0u);

	void run(hinode::graphics::DX12GraphicsCommandList& cmdList, hinode::graphics::utility::DescriptorHeapPoolSet& DHPool, hinode::graphics::utility::ConstantBufferPool& CBPool, const Param& param);

accessor_declaration:
	hinode::graphics::DX12Resource& result()noexcept;

private:
	enum PIPELINE_TYPE {
		ePIPELINE_CAL_VIEW_Z,
		ePIPELINE_CAL_AO,
		ePIPELINE_CAL_AO_BY_CROSS,
		ePIPELINE_BLUR_H,
		ePIPELINE_BLUR_V,
		ePIPELINE_COMPOSITION,
		ePIPELINE_DEBUG_SHOW_RANDOM_SAMPLING,
		ePIPELINE_DEBUG_SHOW_CROSS_SAMPLING,
		ePIPELINE_TYPE_COUNT,
	};
	std::array<hinode::graphics::utility::PipelineSet, ePIPELINE_TYPE_COUNT> mPipelines;

	enum RESOURCE_TYPE {
		eRESOURCE_VIEW_Z,
		eRESOURCE_AO,
		eRESOURCE_BLUR_1,
		eRESOURCE_BLUR_2,
		eRESOURCE_FINAL,
		eRESOURCE_TYPE_COUNT,
	};
	std::array<hinode::graphics::DX12Resource, eRESOURCE_TYPE_COUNT> mResource;;
};
