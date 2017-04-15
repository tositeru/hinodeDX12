#pragma once

#include <array>
#include <vector>
#include <graphics/dx12/utility/math/SimpleMath.h>
#include <graphics/dx12/graphicsCommandList/DX12GraphicsCommandList.h>
#include <graphics/dx12/resource/DX12Resource.h>
#include <graphics/dx12/utility/Shader/PipelineSet/PipelineSet.h>
#include <graphics/dx12/utility/RingBuffer/ConstantBufferPool/ConstantBufferPool.h>
#include <graphics/dx12/utility/RingBuffer/DescriptorHeapPool/DescriptorHeapPool.h>
#include <graphics/dx12/utility/ResourceUploader/ResourceUploader.h>

#include "Shader/volumetricLight/BuildScatteringAmountGrid.hlsl"

/// @brief ボリュームライトを管理するクラス
/// GPU Pro5を参考に実装
///
class VolumetricLightingManager
{
	VolumetricLightingManager(const VolumetricLightingManager&) = delete;
	VolumetricLightingManager& operator=(const VolumetricLightingManager&) = delete;
public:
	struct InitParam
	{
		UINT screenWidth;
		UINT screenHeight;
		hinode::math::uint3 gridSize = {64, 64, 16};
		size_t particleCount;
	};

	struct Param {
		float gridCurveture = 2.f;
		float gridDepthRange = 100.f;
		hinode::math::float4x4 gridViewMatrix;
		hinode::math::float4x4 gridProjMatrix;

		hinode::uint raymarchCount = 40;
		hinode::math::float3 lightDir;
		hinode::math::float3 lightColor;
		float scatteringAmount = 32;
		float scatteringG = 0.2f;
		hinode::math::float4x4 toShadowmapSpaceMat;

		hinode::graphics::DX12Resource* pSceneImage = nullptr;
		int sceneImageSRVIndex = 0;

		hinode::graphics::DX12Resource* pSceneDepth = nullptr;
		int sceneDepthSRVIndex = 0;

		hinode::graphics::DX12Resource* pShadowMap = nullptr;
		int shadowMapSRVIndex = 0;
	};

	enum VIEW_TYPE {
		eVIEW_SRV,
		eVIEW_UAV,
		eVIEW_RTV = eVIEW_UAV,
	};

public:
	VolumetricLightingManager();
	~VolumetricLightingManager();

	void clear();
	void create(ID3D12Device* pDevice, const InitParam& param, hinode::graphics::utility::ResourceUploader& uploader, UINT nodeMask = 0u);

	void run(hinode::graphics::DX12GraphicsCommandList& cmdList, hinode::graphics::utility::DescriptorHeapPoolSet& DHPool, hinode::graphics::utility::ConstantBufferPool& CBPool, const Param& param);

accessor_declaration:
	hinode::graphics::DX12Resource& result()noexcept;

private:
	enum PIPELINE_TYPE {
		ePIPELINE_VOLUMETRIC_LIGHTING,
		ePIPELINE_BUILD_AMOUNT_GRID,
		ePIPELINE_TYPE_COUNT,
	};
	std::array<hinode::graphics::utility::PipelineSet, ePIPELINE_TYPE_COUNT> mPipelines;

	enum RESOURCE_TYPE {
		eRESOURCE_AMOUNT_GRID,
		eRESOURCE_AMOUNT_PARTICLE,
		eRESOURCE_AMOUNT_PARTICLE_FOR_UPLOAD,
		eRESOURCE_DITHERED_PATTERN,
		eRESOURCE_FINAL,
		eRESOURCE_TYPE_COUNT,
	};
	std::array<hinode::graphics::DX12Resource, eRESOURCE_TYPE_COUNT> mResources;
	hinode::graphics::DX12DescriptorHeap mGridRTVHeap;
	std::vector<buildScatteringAmount::ScatteringAmountParticle> mParticles;
};
