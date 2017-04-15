#include "stdafx.h"

#include "GBufferManager.h"

#include "../../graphicsCommandList/DX12GraphicsCommandList.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			std::array<DXGI_FORMAT, GBufferManager::eGBUFFER_TYPE_COUNT> GBufferManager::sDefaultFormats()
			{
				return{ {
					DXGI_FORMAT_R8G8B8A8_UNORM,
					DXGI_FORMAT_R11G11B10_FLOAT,
					DXGI_FORMAT_R8G8B8A8_UNORM,
					DXGI_FORMAT_D32_FLOAT,
					DXGI_FORMAT_R32_FLOAT,
				} };
			}

			GBufferManager::GBufferManager()
			{}

			GBufferManager::~GBufferManager()
			{
				this->clear();
			}

			void GBufferManager::clear()
			{
				this->mGBufferRTVHeap.clear();
				this->mGBufferDSVHeap.clear();
				for (auto& gbuffer : this->mGBuffer) {
					gbuffer.clear();
				}
			}

			void GBufferManager::create(ID3D12Device* pDevice, UINT width, UINT height)
			{
				this->create(pDevice, width, height, GBufferManager::sDefaultFormats());
			}

			void GBufferManager::create(ID3D12Device* pDevice, UINT width, UINT height, const std::array<DXGI_FORMAT, eGBUFFER_TYPE_COUNT>& formats)
			{
				this->clear();

				auto createRT = [&](DX12Resource& out, DX12ResourceHeapDesc& heapDesc, DX12ResourceDesc& resourceDesc) {
					out.create(pDevice, heapDesc, resourceDesc);
					auto srv = out.makeSRV();
					out.appendView(DX12Resource::View().setSRV(srv));
					auto uav = out.makeUAV();
					out.appendView(DX12Resource::View().setUAV(uav));
					auto rtv = out.makeRTV();
					out.appendView(DX12Resource::View().setRTV(rtv));
				};

				DX12ResourceHeapDesc heapDesc(D3D12_HEAP_TYPE_DEFAULT);
				DX12ResourceDesc resourceDesc;
				resourceDesc.desc = CD3DX12_RESOURCE_DESC::Tex2D(formats[eGBUFFER_ALBEDO], width, height, 1, 1);
				resourceDesc.desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
				resourceDesc.initialStates = D3D12_RESOURCE_STATE_RENDER_TARGET;
				resourceDesc.setClearValueForColor(resourceDesc.desc.Format, 0.125f, 0, 0, 0);
				createRT(this->mGBuffer[eGBUFFER_ALBEDO], heapDesc, resourceDesc);

				resourceDesc.desc.Format = formats[eGBUFFER_NORMAL];
				resourceDesc.setClearValueForColor(resourceDesc.desc.Format, 0, 1, 0, 0);
				createRT(this->mGBuffer[eGBUFFER_NORMAL], heapDesc, resourceDesc);

				resourceDesc.desc.Format = formats[eGBUFFER_SPECULER];
				resourceDesc.setClearValueForColor(resourceDesc.desc.Format, 0, 0, 0, 0);
				createRT(this->mGBuffer[eGBUFFER_SPECULER], heapDesc, resourceDesc);

				{//RTVのDescriptor Heap作成
					D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
					heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
					heapDesc.NodeMask = 0;
					heapDesc.NumDescriptors = eGBUFFER_RTV_COUNT;
					heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;;
					this->mGBufferRTVHeap.create(pDevice, &heapDesc);
					auto cpuHandle = this->mGBufferRTVHeap.makeCpuHandle(0);
					cpuHandle.createRenderTargetView(pDevice, this->mGBuffer[eGBUFFER_ALBEDO], &this->mGBuffer[eGBUFFER_ALBEDO].view(eVIEW_RTV).rtv());
					cpuHandle.Offset(1);
					cpuHandle.createRenderTargetView(pDevice, this->mGBuffer[eGBUFFER_NORMAL], &this->mGBuffer[eGBUFFER_NORMAL].view(eVIEW_RTV).rtv());
					cpuHandle.Offset(1);
					cpuHandle.createRenderTargetView(pDevice, this->mGBuffer[eGBUFFER_SPECULER], &this->mGBuffer[eGBUFFER_SPECULER].view(eVIEW_RTV).rtv());
				}

				{//深度バッファ系の作成
					resourceDesc.desc.Format = formats[eGBUFFER_DEPTH_FOR_DSV];
					resourceDesc.desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
					resourceDesc.setClearValueForDepth(resourceDesc.desc.Format, 1, 0);
					resourceDesc.initialStates = D3D12_RESOURCE_STATE_DEPTH_WRITE;
					this->mGBuffer[eGBUFFER_DEPTH_FOR_DSV].create(pDevice, heapDesc, resourceDesc);
					auto dsv = this->mGBuffer[eGBUFFER_DEPTH_FOR_DSV].makeDSV();
					this->mGBuffer[eGBUFFER_DEPTH_FOR_DSV].appendView(DX12Resource::View().setDSV(dsv));

					resourceDesc.desc.Format = formats[eGBUFFER_DEPTH_FOR_SRV_UAV];
					resourceDesc.desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
					resourceDesc.initialStates = D3D12_RESOURCE_STATE_COPY_DEST;
					this->mGBuffer[eGBUFFER_DEPTH_FOR_SRV_UAV].create(pDevice, heapDesc, resourceDesc);
					auto srv = this->mGBuffer[eGBUFFER_DEPTH_FOR_SRV_UAV].makeSRV();
					this->mGBuffer[eGBUFFER_DEPTH_FOR_SRV_UAV].appendView(DX12Resource::View().setSRV(srv));
					auto uav = this->mGBuffer[eGBUFFER_DEPTH_FOR_SRV_UAV].makeUAV();
					this->mGBuffer[eGBUFFER_DEPTH_FOR_SRV_UAV].appendView(DX12Resource::View().setUAV(uav));

					D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
					heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
					heapDesc.NodeMask = 0;
					heapDesc.NumDescriptors = eGBUFFER_DSV_COUNT;
					heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
					this->mGBufferDSVHeap.create(pDevice, &heapDesc);
					auto cpuHandle = this->mGBufferDSVHeap.makeCpuHandle(0);
					cpuHandle.createDepthStencilView(pDevice, this->mGBuffer[eGBUFFER_DEPTH_FOR_DSV], &this->mGBuffer[eGBUFFER_DEPTH_FOR_DSV].view(eVIEW_DSV).dsv());
				}
			}

			void GBufferManager::begin(DX12GraphicsCommandList& cmdList)
			{
				D3D12_VIEWPORT vp;
				vp.Width = static_cast<float>(this->mGBuffer[0]->GetDesc().Width);
				vp.Height = static_cast<float>(this->mGBuffer[0]->GetDesc().Height);
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
					this->mGBuffer[eGBUFFER_ALBEDO].makeBarrier(D3D12_RESOURCE_STATE_RENDER_TARGET),
					this->mGBuffer[eGBUFFER_NORMAL].makeBarrier(D3D12_RESOURCE_STATE_RENDER_TARGET),
					this->mGBuffer[eGBUFFER_SPECULER].makeBarrier(D3D12_RESOURCE_STATE_RENDER_TARGET),
					this->mGBuffer[eGBUFFER_DEPTH_FOR_DSV].makeBarrier(D3D12_RESOURCE_STATE_DEPTH_WRITE)
				);
				auto rtvHandle = this->mGBufferRTVHeap.makeCpuHandle(0);
				auto dsvHandle = this->mGBufferDSVHeap.makeCpuHandle(0);
				cmdList->OMSetRenderTargets(eGBUFFER_RTV_COUNT, &rtvHandle.d3dx12Handle, TRUE, &dsvHandle.d3dx12Handle);

				for (auto i = 0u; i < eGBUFFER_RTV_COUNT; ++i) {
					cmdList->ClearRenderTargetView(rtvHandle.d3dx12Handle, this->mGBuffer[i].clearValue().Color, 0, nullptr);
					rtvHandle.Offset(1);
				}
				auto& depthClearValue = this->mGBuffer[eGBUFFER_DEPTH_FOR_DSV].clearValue();
				cmdList->ClearDepthStencilView(dsvHandle.d3dx12Handle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, depthClearValue.DepthStencil.Depth, depthClearValue.DepthStencil.Stencil, 0, nullptr);
			}

			void GBufferManager::end(DX12GraphicsCommandList& cmdList)
			{
				cmdList.setBarriers(
					this->mGBuffer[eGBUFFER_DEPTH_FOR_DSV].makeBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE),
					this->mGBuffer[eGBUFFER_DEPTH_FOR_SRV_UAV].makeBarrier(D3D12_RESOURCE_STATE_COPY_DEST)
				);
				cmdList->CopyResource(this->mGBuffer[eGBUFFER_DEPTH_FOR_SRV_UAV], this->mGBuffer[eGBUFFER_DEPTH_FOR_DSV]);
			}

			DX12Resource& GBufferManager::get(GBUFFER_TYPE type)noexcept
			{
				return this->mGBuffer[type];
			}

			const DX12Resource::View& GBufferManager::getView(GBUFFER_TYPE type, VIEW_TYPE viewType)noexcept
			{
				return this->mGBuffer[type].view(viewType);
			}
		}
	}
}