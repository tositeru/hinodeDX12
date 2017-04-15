#pragma once

#include <array>

#include "../../../resource/DX12Resource.h"
#include "../../../descriptorHeap/DX12DescriptorHeap.h"
#include "../../Shader/PipelineSet/PipelineSet.h"
#include "../../../sampler/DX12Sampler.h"
#include "../DefferedLightingPipeline/DefferedLightingPipeline.h"

namespace hinode
{
	namespace graphics
	{
		class DX12GraphicsCommandList;

		namespace utility
		{
			class DefferedLightingManager
			{
			public:
				enum PIPELINE_COUNT
				{
					ePIPELINE_DIRECTIONAL_LIGHTING,
					ePIPELINE_DIRECTIONAL_LIGHTING_WITH_SHADOW_MAP,
					ePIPELINE_COUNT,
				};

				enum VIEW_TYPE
				{
					eVIEW_SRV,
					eVIEW_UAV,
				};

				struct SetupParam
				{
					float4x4 invViewProj;
					float3 eyePos;

					struct Texture {
						DX12Resource* pTex = nullptr;
						int srvIndex = 0;

						void set(DX12Resource* pTex, int srvIndex)
						{
							this->pTex = pTex;
							this->srvIndex = srvIndex;
						}

						const D3D12_SHADER_RESOURCE_VIEW_DESC& srv()
						{
							return this->pTex->view(this->srvIndex).srv();
						}
					};
					Texture texAlbedo;
					Texture texNormal;
					Texture texSpecular;
					Texture texDepth;
				};

				struct DirectionLightParam
				{
					shader::DirectionLightParam lightParam;
					shader::ShadowMapParam shadowMapParam;
					DX12Resource* pShadowMap = nullptr;
					int shadowMapSRVIndex = 0;
					DX12SamplerDesc shadowMapSampler;

					DirectionLightParam()
					{
						shadowMapSampler.setAddress(D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER);
						shadowMapSampler.BorderColor[0] = 1.f;
						shadowMapSampler.BorderColor[1] = 1.f;
						shadowMapSampler.BorderColor[2] = 1.f;
						shadowMapSampler.BorderColor[3] = 1.f;
					}
				};

			public:
				DefferedLightingManager();
				~DefferedLightingManager();

				/// @brief メモリ解放
				void clear();

				/// @brief 作成
				/// @param[in] pDevice
				/// @param[in] width
				/// @param[in] height
				void create(ID3D12Device* pDevice, UINT width, UINT height);

				/// @brief ライティングの準備を行う
				/// @param[in] cmdList
				/// @param[in] DHPool
				/// @param[in] CBPool
				/// @param[in] param
				void setup(DX12GraphicsCommandList& cmdList, DescriptorHeapPoolSet& DHPool, ConstantBufferPool& CBPool, SetupParam& param);

				/// @brief 平行光のライティングを行う
				///
				/// @param[in] cmdList
				/// @param[in] DHPool
				/// @param[in] CBPool
				/// @param[in] param
				void lighting(DX12GraphicsCommandList& cmdList, DescriptorHeapPoolSet& DHPool, ConstantBufferPool& CBPool, const DirectionLightParam& param);

			accessor_declaration:
				DX12Resource& lightingTexture()noexcept;

			private:
				std::array<utility::PipelineSet, ePIPELINE_COUNT> mLighitngPipelines;
				DX12Resource mLightingTexture;
			};
		}
	}
}