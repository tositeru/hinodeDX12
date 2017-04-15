#include "stdafx.h"

#include "DefferedLightingManager.h"

#include "../../../graphicsCommandList/DX12GraphicsCommandList.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			DefferedLightingManager::DefferedLightingManager()
			{ }

			DefferedLightingManager::~DefferedLightingManager()
			{
				this->clear();
			}

			void DefferedLightingManager::clear()
			{
				for (auto& pipeline : this->mLighitngPipelines) {
					pipeline.clear();
				}
				this->mLightingTexture.clear();
			}

			void DefferedLightingManager::create(ID3D12Device* pDevice, UINT width, UINT height)
			{
				this->clear();

				this->mLighitngPipelines[ePIPELINE_DIRECTIONAL_LIGHTING] = utility::DefferedLightingPipeline::sCreateDirectionLighting(pDevice);
				this->mLighitngPipelines[ePIPELINE_DIRECTIONAL_LIGHTING_WITH_SHADOW_MAP] = utility::DefferedLightingPipeline::sCreateDirectionLightingWithShadow(pDevice);

				DX12ResourceHeapDesc heapDesc;
				DX12ResourceDesc resourceDesc;
				resourceDesc.desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, width, height);
				resourceDesc.desc.MipLevels = 1;
				resourceDesc.desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
				resourceDesc.initialStates = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
				resourceDesc.setClearValueForColor(resourceDesc.desc.Format, 0, 0, 0, 1);
				this->mLightingTexture.create(pDevice, heapDesc, resourceDesc);
				this->mLightingTexture.appendView(DX12Resource::View().setSRV(this->mLightingTexture.makeSRV()));
				this->mLightingTexture.appendView(DX12Resource::View().setUAV(this->mLightingTexture.makeUAV()));
				this->mLightingTexture.createDescriptorHeapForClear(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			}

			void DefferedLightingManager::setup(DX12GraphicsCommandList& cmdList, DescriptorHeapPoolSet& DHPool, ConstantBufferPool& CBPool, SetupParam& param)
			{
				cmdList.setBarriers(
					param.texAlbedo.pTex->makeBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
					param.texNormal.pTex->makeBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
					param.texSpecular.pTex->makeBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
					param.texDepth.pTex->makeBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
					this->mLightingTexture.makeBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
				);

				auto& descriptorHeap = this->mLightingTexture.descriptorHeapForClear();
				cmdList->ClearUnorderedAccessViewFloat(descriptorHeap->GetGPUDescriptorHandleForHeapStart(), descriptorHeap->GetCPUDescriptorHandleForHeapStart(), this->mLightingTexture, this->mLightingTexture.clearValue().Color, 0, nullptr);

				CSDefferedDirectionLighting::CBParam cbParam;
				cbParam.cbInvViewProj = param.invViewProj;
				cbParam.cbEyePos = param.eyePos;
				auto allocateInfo = CBPool.allocate(&cbParam, sizeof(cbParam));
				for (auto& pipeline : this->mLighitngPipelines) {
					pipeline.updateCBV("CBParam", DHPool, allocateInfo);

					pipeline.updateSRV("txAlbedo", DHPool, *param.texAlbedo.pTex, &param.texAlbedo.srv());
					pipeline.updateSRV("txNormal", DHPool, *param.texNormal.pTex, &param.texNormal.srv());
					pipeline.updateSRV("txSpecular", DHPool, *param.texSpecular.pTex, &param.texSpecular.srv());
					pipeline.updateSRV("txDepth", DHPool, *param.texDepth.pTex, &param.texDepth.srv());
					pipeline.updateUAV("outScreen", DHPool, this->mLightingTexture, &this->mLightingTexture.view(eVIEW_UAV).uav());
				}
			}

			void DefferedLightingManager::lighting(DX12GraphicsCommandList& cmdList, DescriptorHeapPoolSet& DHPool, ConstantBufferPool& CBPool, const DirectionLightParam& param)
			{
				auto type = param.pShadowMap ? ePIPELINE_DIRECTIONAL_LIGHTING_WITH_SHADOW_MAP : ePIPELINE_DIRECTIONAL_LIGHTING;
				if (ePIPELINE_DIRECTIONAL_LIGHTING_WITH_SHADOW_MAP == type) {
					// シャドウマップテクスチャとサンプラはDescriptorTableとして宣言しているため、１フレームに何回でもシャドウマップを使うためにクローンしている
					auto pipeline = this->mLighitngPipelines[type].clone({});

					cmdList.setBarriers(param.pShadowMap->makeBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

					shader::CSDefferedDirectionLightingWithShadow::CBLight cbLight;
					cbLight.cbLight = param.lightParam;
					cbLight.cbShadowMapParam = param.shadowMapParam;
					pipeline.updateCBV("CBLight", DHPool, &cbLight, sizeof(cbLight), CBPool);
					pipeline.updateSRV("txShadowMap", DHPool, *param.pShadowMap, &param.pShadowMap->view(param.shadowMapSRVIndex).srv());
					DX12SamplerDesc shadowmapSampler;
					shadowmapSampler
						.setAddress(D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER)
						.setBorderColor(1, 1, 1, 1)
						.setFilter(D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR)
						.setComparisonFunc(D3D12_COMPARISON_FUNC_LESS)
					;
					//pipeline.updateSampler("smShadowMapSampler", DHPool, &param.shadowMapSampler);
					pipeline.updateSampler("smShadowMapSampler", DHPool, &shadowmapSampler);
					pipeline.bind(cmdList);
				} else {
					auto& pipeline = this->mLighitngPipelines[type];
					pipeline.updateCBV("CBLight", DHPool, &param.lightParam, sizeof(param.lightParam), CBPool);
					pipeline.bind(cmdList);
				}

				const auto threadCount = CSDefferedDirectionLighting::getThreadCount();
				const auto tx = DX12PipelineState::sCalDispatchCount(static_cast<UINT>(this->mLightingTexture->GetDesc().Width), threadCount.x);
				const auto ty = DX12PipelineState::sCalDispatchCount(static_cast<UINT>(this->mLightingTexture->GetDesc().Height), threadCount.y);
				cmdList->Dispatch(tx, ty, 1);
			}

			DX12Resource& DefferedLightingManager::lightingTexture()noexcept
			{
				return this->mLightingTexture;
			}

		}
	}
}