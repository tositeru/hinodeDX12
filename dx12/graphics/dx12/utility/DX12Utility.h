#pragma once

#include <unordered_map>

#include <D3d12shader.h>

#include "..\device\DX12Device.h"
#include "..\swapChain\DX12SwapChain.h"
#include "..\commandQueue\DX12CommandQueue.h"
#include "..\commandAllocator\DX12CommandAllocator.h"
#include "..\fence\DX12Fence.h"
#include "..\resource\DX12Resource.h"
#include "..\descriptorHeap\DX12DescriptorHeap.h"
#include "..\pipelineState\DX12PipelineState.h"
#include "..\rootSignature\DX12RootSignature.h"
#include "..\resource\DX12Resource.h"
#include "..\sampler\DX12Sampler.h"
#include "../queryHeap/DX12QueryHeap.h"

#include "Context/Context.h"

//シェーダ関連
#include "Shader\Shader.h"
#include "Shader\Pipeline\Pipeline.h"
#include "Shader\PipelineSet\PipelineSet.h"
#include "Shader\RootSignatureInfo\RootSignatureInfo.h"
#include "Shader\ShaderResourceInfo\ShaderResourceInfo.h"
#include "Shader\PipelineResourceBinder\PipelineResourceBinder.h"
#include "RingBuffer/ConstantBufferPool\ConstantBufferPool.h"
#include "RingBuffer/DescriptorHeapPool/DescriptorHeapPool.h"

//リソース関連
#include "ResourceUploader\ResourceUploader.h"
#include "FrameDataManager/FrameDataManager.h"

// メッシュ関連
#include "Mesh\Mesh.h"
#include "Mesh\MeshPipeline\MeshPipeline.h"
#include "Mesh\MeshVertexFormat.h"
#include "Mesh\Primitive\Tetrahedron\Tetrahedron.h"
#include "Mesh\Primitive\Plane\Plane.h"

//カメラ関連
#include "Camera\PreviewCamera\PreviewCamera.h"
#include "Camera/DrafterCamera/DrafterCamera.h"

namespace hinode
{
	namespace graphics
	{
		/// @brief 便利関数やクラスを定義
		namespace utility
		{
			/// @brief DX12を使う上で必ず必要となるものをまとめたクラス
			class Graphics
			{
				Graphics(Graphics&)=delete;
				void operator=(Graphics&)=delete;
			public:
				Graphics()noexcept;
				~Graphics()noexcept;
				
				/// @brief リソースの開放を行う
				void clear()noexcept;
				
				/// @brief 初期化
				/// @param[in] pDeviceDesc
				/// @param[in] cmdQueueDesc
				/// @param[in] pSwapChainDesc
				/// @param isSwitchScreenMode defalut=false
				/// @exception
				void init(DX12DeviceDesc* pDeviceDesc, const D3D12_COMMAND_QUEUE_DESC& cmdQueueDesc, DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, bool isSwitchScreenMode=false);
				
				/// @brief 現在のレンダーターゲットとデプスステンシルを設定する
				///
				/// ビューポートとシザー矩形も設定します
				/// @param[in] pCommandList
				void bindCurrentRenderTargetAndDepthStencil(ID3D12GraphicsCommandList* pCommandList);

				/// @brief 現在のレンダーターゲットを指定した色でクリアーする
				/// @param[in] pCommandList
				/// @param[in] pClearColor
				void clearCurrentRenderTarget(ID3D12GraphicsCommandList* pCommandList, const float* pClearColor);

				/// @brief 現在の深度ステンシルを指定した値でクリアーする
				/// @param[in] pCommandList
				/// @param[in] pClearColor
				void clearCurrentDepthStencil(ID3D12GraphicsCommandList* pCommandList, float depth, UINT8 stencil);

				/// @brief 現在のバックバッファーを画面に転送する
				///
				/// pCmdListに一つ以上のID3D12CommandList*を指定するとそのコマンドリストを転送前に実行します
				/// @param[in] syncInterval
				/// @param[in] pCmdList defalut={}
				void present(UINT syncInterval, std::initializer_list<ID3D12CommandList*> pCmdLists = {});

				/// @brief 次のフレームに移行する
				void endAndGoNextFrame();

				/// @brief 現在のフレームが利用できるまで待つ
				void waitForCurrentFrame(DWORD waitMilliseconds = INFINITE);

				/// @brief 1つ前のフレームのコマンドが実行し終えるまで待つ
				void waitPrevFrame(DWORD waitMilliseconds = INFINITE);
				
			accessor_declaration:
				DX12Device& device()noexcept{ return this->mDevice; }
				DX12SwapChain& swapchain()noexcept{ return this->mSwapChain; }
				DX12CommandQueue& cmdQueue()noexcept{ return this->mCmdQueue; }
				UINT currentBackBufferIndex()noexcept{ return this->mSwapChain->GetCurrentBackBufferIndex(); }
				Context& currentContext()noexcept;
				DX12Resource& currentRenderTarget()noexcept;
				DX12CPUDescriptorHandle currentRTView()noexcept;
				DX12Resource& currentDepthStencil()noexcept;
				DX12CPUDescriptorHandle currentDSView()noexcept;
				D3D12_VIEWPORT getViewport()noexcept;
				D3D12_RECT getScissorRect()noexcept;
				float calAspect()noexcept;

			private:
				DX12Device mDevice;
				DX12SwapChain mSwapChain;
				DX12CommandQueue mCmdQueue;
				//DX12Fence mFrameFence;
				//std::vector<DX12Resource> mRTs;
				//DX12DescriptorHeap mRTVDescHeap;
				//std::vector<DX12Resource> mDepthStencils;
				//DX12DescriptorHeap mDSVDescHeap;
				struct FrameData : public FrameDataManager::IData
				{
					Context mContext;
					DX12Resource mRT;
					DX12DescriptorHeap mRTVDescHeap;
					DX12Resource mDepthStencil;
					DX12DescriptorHeap mDSVDescHeap;

					void clear()override;
				};
				FrameDataManager mFrameDatas;

#if defined(_DEBUG)
				Microsoft::WRL::ComPtr<ID3D12Debug> mpDebugController;
#endif
			};

			/// @brief チェッカー模様のテクスチャを作成する
			///
			/// サンプルのものをコピペ
			/// @param[in] TextureWidth
			/// @param[in] TextureHeight
			/// @param[in] TexturePixelSize
			/// @retval std:vector<UTIN>
			std::vector<UINT8> GenerateTextureData(UINT TextureWidth, UINT TextureHeight, UINT TexturePixelSize);
		}
	}
}
