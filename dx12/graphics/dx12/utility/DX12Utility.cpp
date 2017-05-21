#include "stdafx.h"

#include "DX12Utility.h"

#include "..\common/Log.h"
#include "..\common\Exception.h"

//#include <d3dcompiler.h>

using namespace Microsoft::WRL;
namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			Graphics::Graphics()noexcept
			{

			}

			Graphics::~Graphics()noexcept
			{
				this->clear();
			}

			void Graphics::clear()noexcept
			{
				this->mFrameDatas.clear();

				this->mCmdQueue.clear();
				this->mSwapChain.clear();

				if (this->mDevice.isGood()) {
#ifdef _DEBUG
					ComPtr<ID3D12DebugDevice> pDebugDeivce;
					if (SUCCEEDED(this->mDevice.getDebugDevice(&pDebugDeivce))) {
						pDebugDeivce->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
					}
#endif
				}
				this->mDevice.clear();
			}

			void Graphics::init(DX12DeviceDesc* pDeviceDesc, const D3D12_COMMAND_QUEUE_DESC& cmdQueueDesc, DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, bool isSwitchScreenMode)
			{
				this->clear();

				try {
					UINT dxgiFactoryFlags = 0;
#if defined(_DEBUG)
					if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&this->mpDebugController)))) {
						this->mpDebugController->EnableDebugLayer();
					}
					dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
					ComPtr<IDXGIFactory4> factory;
					if (FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)))) {
						hinode::graphics::Log(hinode::graphics::Log::eINFO) << "IDXGIFactory4の作成に失敗";
						throw HINODE_GRAPHICS_CREATE_EXCEPTION(FailedToCreateException, "Graphics", "init") << "IDXGIFactory4の作成に失敗";
					}

					DX12Device::sCreateDevice(&this->mDevice, pDeviceDesc, factory.Get(), true);
					this->mDevice.setName(L"Default Device");

					this->mCmdQueue.create(this->mDevice.device(), cmdQueueDesc);
					this->mCmdQueue.setName(L"Default Graphics Command Queue");
					this->mSwapChain.create(this->mCmdQueue.queue(), factory.Get(), *pSwapChainDesc);
					this->mSwapChain.setName(L"Default SwapChain");

					if (isSwitchScreenMode) {
						factory->MakeWindowAssociation(pSwapChainDesc->OutputWindow, DXGI_MWA_NO_ALT_ENTER);
					}

					this->mFrameDatas.create(this->mDevice, pSwapChainDesc->BufferCount, [&](UINT frameIndex) {
						auto pResult = std::make_unique<FrameData>();

						pResult->mContext.create(this->mDevice, D3D12_COMMAND_LIST_TYPE_DIRECT);

						D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
						rtvHeapDesc.NumDescriptors = 1;
						rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
						rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
						pResult->mRTVDescHeap.create(this->mDevice.device(), &rtvHeapDesc);
						pResult->mRTVDescHeap.setName(L"Default render target descriptor heap");

						pResult->mRT.setSwapChianBuffer(this->mSwapChain.swapChain(), frameIndex);
						pResult->mRT.setName((std::wstring(L"Default render target") + std::to_wstring(frameIndex)).c_str());
						auto rtvHandle = pResult->mRTVDescHeap.makeCpuHandle(0);
						rtvHandle.createRenderTargetView(this->mDevice.device(), pResult->mRT.resource(), nullptr);

						//深度ステンシルバッファ作成
						D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
						dsvHeapDesc.NumDescriptors = 1;
						dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
						dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
						pResult->mDSVDescHeap.create(this->mDevice, &dsvHeapDesc);
						pResult->mDSVDescHeap.setName(L"Default depth stencil descriptor heap");
						auto dsvHandle = pResult->mDSVDescHeap.makeCpuHandle(0);

						DX12ResourceHeapDesc heapDesc;
						heapDesc.properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
						heapDesc.flags = D3D12_HEAP_FLAG_NONE;
						DX12ResourceDesc resourceDesc;
						resourceDesc.desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, pSwapChainDesc->BufferDesc.Width, pSwapChainDesc->BufferDesc.Height);
						resourceDesc.desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
						resourceDesc.clearValue.Format = resourceDesc.desc.Format;
						resourceDesc.clearValue.DepthStencil.Depth = 1.f;
						resourceDesc.clearValue.DepthStencil.Stencil = 0;
						resourceDesc.initialStates = D3D12_RESOURCE_STATE_DEPTH_WRITE;
						pResult->mDepthStencil.create(this->mDevice, heapDesc, resourceDesc);
						pResult->mDepthStencil.setName((std::wstring(L"Default depth stencil") + std::to_wstring(frameIndex)).c_str());
						dsvHandle.createDepthStencilView(this->mDevice, pResult->mDepthStencil, nullptr);
						return pResult;
					});

				} catch (...) {
					this->clear();
					throw;
				}
			}

			void Graphics::bindCurrentRenderTargetAndDepthStencil(ID3D12GraphicsCommandList* pCommandList)
			{
				assert(nullptr != pCommandList);

				pCommandList->RSSetViewports(1, &this->getViewport());
				pCommandList->RSSetScissorRects(1, &this->getScissorRect());

				auto frameIndex = this->currentBackBufferIndex();
				unless(D3D12_RESOURCE_STATE_RENDER_TARGET == this->currentRenderTarget().currentState()) {
					pCommandList->ResourceBarrier(1, &this->currentRenderTarget().makeBarrier(D3D12_RESOURCE_STATE_RENDER_TARGET));
				}

				auto rtvHandle = this->currentRTView();
				auto dsvHandle = this->currentDSView();
				pCommandList->OMSetRenderTargets(1, &rtvHandle.d3dx12Handle, FALSE, &dsvHandle.d3dx12Handle);
			}

			void Graphics::clearCurrentRenderTarget(ID3D12GraphicsCommandList* pCommandList, const float* pClearColor)
			{
				assert(nullptr != pCommandList);
				auto rtvHandle = this->currentRTView();
				pCommandList->ClearRenderTargetView(rtvHandle.d3dx12Handle, pClearColor, 0, nullptr);
			}

			void Graphics::clearCurrentDepthStencil(ID3D12GraphicsCommandList* pCommandList, float depth, UINT8 stencil)
			{
				assert(nullptr != pCommandList);
				auto dsvHandle = this->currentDSView();
				pCommandList->ClearDepthStencilView(dsvHandle.d3dx12Handle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, depth, stencil, 0, nullptr);
			}

			void Graphics::present(UINT syncInterval, std::initializer_list<ID3D12CommandList*> pCmdLists)
			{
				assert(this->mCmdQueue.isGood());

				if (1 <= pCmdLists.size()) {
					this->mCmdQueue.executeCommandLists(pCmdLists);
				}
				this->mSwapChain->Present(syncInterval, 0);
			}

			void Graphics::endAndGoNextFrame()
			{
				this->mFrameDatas.endAndGoNextFrame(this->mCmdQueue);
			}

			void Graphics::waitForCurrentFrame(DWORD waitMilliseconds)
			{
				this->mFrameDatas.waitForCurrent(waitMilliseconds);
			}

			void Graphics::waitPrevFrame(DWORD waitMilliseconds)
			{
				this->mFrameDatas.waitPrevFrame();
			}

			Context& Graphics::currentContext()noexcept
			{
				return this->mFrameDatas.currentFrame<FrameData>()->mContext;
			}

			DX12Resource& Graphics::currentRenderTarget()noexcept
			{
				return this->mFrameDatas.currentFrame<FrameData>()->mRT;
			}

			DX12Resource& Graphics::currentDepthStencil()noexcept
			{
				return this->mFrameDatas.currentFrame<FrameData>()->mDepthStencil;
			}

			DX12CPUDescriptorHandle  Graphics::currentRTView()noexcept
			{
				return this->mFrameDatas.currentFrame<FrameData>()->mRTVDescHeap.makeCpuHandle(0);
			}

			DX12CPUDescriptorHandle Graphics::currentDSView()noexcept
			{
				return this->mFrameDatas.currentFrame<FrameData>()->mDSVDescHeap.makeCpuHandle(0);
			}

			D3D12_VIEWPORT Graphics::getViewport()noexcept
			{
				DXGI_SWAP_CHAIN_DESC desc;
				this->mSwapChain->GetDesc(&desc);

				D3D12_VIEWPORT vp;
				vp.Width = static_cast<decltype(vp.Width)>(desc.BufferDesc.Width);
				vp.Height = static_cast<decltype(vp.Height)>(desc.BufferDesc.Height);
				vp.TopLeftX = vp.TopLeftY = 0;
				vp.MinDepth = 0.f;
				vp.MaxDepth = 1.f;
				return vp;
			}

			D3D12_RECT Graphics::getScissorRect()noexcept
			{
				DXGI_SWAP_CHAIN_DESC desc;
				this->mSwapChain->GetDesc(&desc);

				D3D12_RECT rect;
				rect.top = rect.left = 0;
				rect.right = desc.BufferDesc.Width;
				rect.bottom = desc.BufferDesc.Height;
				return rect;
			}

			float Graphics::calAspect()noexcept
			{
				DXGI_SWAP_CHAIN_DESC desc;
				this->mSwapChain->GetDesc(&desc);

				return static_cast<float>(desc.BufferDesc.Width) / static_cast<float>(desc.BufferDesc.Height);
				
			}

			//
			//	Graphics::FrameData
			//

			void Graphics::FrameData::clear()
			{
				this->mContext.clear();
				this->mRT.clear();
				this->mDepthStencil.clear();
				this->mRTVDescHeap.clear();
				this->mDSVDescHeap.clear();
			}
		}

		namespace utility
		{
		}

		namespace utility
		{
			std::vector<UINT8> GenerateTextureData(UINT TextureWidth, UINT TextureHeight, UINT TexturePixelSize)
			{
				const UINT rowPitch = TextureWidth * TexturePixelSize;
				const UINT cellPitch = rowPitch >> 3;		// The width of a cell in the checkboard texture.
				const UINT cellHeight = TextureWidth >> 3;	// The height of a cell in the checkerboard texture.
				const UINT textureSize = rowPitch * TextureHeight;

				std::vector<UINT8> data(textureSize);
				UINT8* pData = &data[0];

				for (UINT n = 0; n < textureSize; n += TexturePixelSize) {
					UINT x = n % rowPitch;
					UINT y = n / rowPitch;
					UINT i = x / cellPitch;
					UINT j = y / cellHeight;

					if (i % 2 == j % 2) {
						pData[n] = 0x00;		// R
						pData[n + 1] = 0x00;	// G
						pData[n + 2] = 0x00;	// B
						pData[n + 3] = 0xff;	// A
					} else {
						pData[n] = 0xff;		// R
						pData[n + 1] = 0xff;	// G
						pData[n + 2] = 0xff;	// B
						pData[n + 3] = 0xff;	// A
					}
				}

				return data;
			}
		}
	}
}