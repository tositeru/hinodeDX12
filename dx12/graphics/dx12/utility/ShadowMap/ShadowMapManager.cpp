#include "stdafx.h"

#include "ShadowMapManager.h"

#include "../../graphicsCommandList/DX12GraphicsCommandList.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			ShadowMapManager::ShadowMapManager()
			{ }

			ShadowMapManager::~ShadowMapManager()
			{
				this->clear();
			}

			void ShadowMapManager::clear()
			{
				for (auto& map : this->mShadowMaps) {
					map.clear();
				}
			}

			void ShadowMapManager::create(ID3D12Device* pDevice, UINT width, UINT height, DXGI_FORMAT formatDSV, DXGI_FORMAT formatSRVUAV, UINT nodeMask)
			{
				this->clear();

				DX12ResourceHeapDesc heapDesc;
				DX12ResourceDesc resourceDesc;
				resourceDesc.desc = CD3DX12_RESOURCE_DESC::Tex2D(formatDSV, width, height, 1, 1);
				resourceDesc.desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
				resourceDesc.initialStates = D3D12_RESOURCE_STATE_DEPTH_WRITE;
				resourceDesc.setClearValueForDepth(formatDSV, 1.f, 0);
				this->mShadowMaps[eTYPE_DSV].create(pDevice, heapDesc, resourceDesc);
				this->mShadowMaps[eTYPE_DSV].appendView(DX12Resource::View().setDSV(this->mShadowMaps[eTYPE_DSV].makeDSV()));
				this->mShadowMaps[eTYPE_DSV].createDescriptorHeapForClear(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

				D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc;
				descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
				descHeapDesc.NodeMask = nodeMask;
				descHeapDesc.NumDescriptors = 1;
				descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
				this->mDSVHeap.create(pDevice, &descHeapDesc);
				this->mDSVHeap.makeCpuHandle(0).createDepthStencilView(pDevice, this->mShadowMaps[eTYPE_DSV], &this->mShadowMaps[eTYPE_DSV].view(eVIEW_DSV).dsv());

				resourceDesc.desc = CD3DX12_RESOURCE_DESC::Tex2D(formatSRVUAV, width, height, 1, 1);
				resourceDesc.desc.Flags = D3D12_RESOURCE_FLAG_NONE;
				resourceDesc.initialStates = D3D12_RESOURCE_STATE_COPY_DEST;
				resourceDesc.setClearValueForDepth(formatSRVUAV, 1.f, 0);
				this->mShadowMaps[eTYPE_SRV].create(pDevice, heapDesc, resourceDesc);
				this->mShadowMaps[eTYPE_SRV].appendView(DX12Resource::View().setSRV(this->mShadowMaps[eTYPE_SRV].makeSRV()));
			}

			void ShadowMapManager::begin(DX12GraphicsCommandList& cmdList)
			{
				D3D12_VIEWPORT vp;
				vp.Width = static_cast<float>(this->mShadowMaps[eTYPE_DSV]->GetDesc().Width);
				vp.Height = static_cast<float>(this->mShadowMaps[eTYPE_DSV]->GetDesc().Height);
				vp.TopLeftX = vp.TopLeftY = 0.f;
				vp.MinDepth = 0.f;
				vp.MaxDepth = 1.f;
				cmdList->RSSetViewports(1, &vp);
				D3D12_RECT scissorRect;
				scissorRect.top = 0;
				scissorRect.left = 0;
				scissorRect.right = static_cast<decltype(scissorRect.right)>(vp.Width);
				scissorRect.bottom = static_cast<decltype(scissorRect.right)>(vp.Height);
				cmdList->RSSetScissorRects(1, &scissorRect);

				cmdList.setBarriers(
					this->mShadowMaps[eTYPE_DSV].makeBarrier(D3D12_RESOURCE_STATE_DEPTH_WRITE)
				);
				auto dsvHandle = this->mDSVHeap.makeCpuHandle(0);
				cmdList->OMSetRenderTargets(0, nullptr, FALSE, &dsvHandle.d3dx12Handle);

				auto& depthClearValue = this->mShadowMaps[eTYPE_DSV].clearValue();
				cmdList->ClearDepthStencilView(dsvHandle.d3dx12Handle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, depthClearValue.DepthStencil.Depth, depthClearValue.DepthStencil.Stencil, 0, nullptr);
			}

			void ShadowMapManager::end(DX12GraphicsCommandList& cmdList)
			{
				cmdList.setBarriers(
					this->mShadowMaps[eTYPE_DSV].makeBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE),
					this->mShadowMaps[eTYPE_SRV].makeBarrier(D3D12_RESOURCE_STATE_COPY_DEST)
				);

				cmdList->CopyResource(this->mShadowMaps[eTYPE_SRV], this->mShadowMaps[eTYPE_DSV]);
			}

			DX12Resource& ShadowMapManager::shadowMap()noexcept
			{
				return this->mShadowMaps[eTYPE_SRV];
			}

			DX12Resource& ShadowMapManager::getShadowMap(TYPE type)noexcept
			{
				return this->mShadowMaps[type];
			}
		}
	}
}