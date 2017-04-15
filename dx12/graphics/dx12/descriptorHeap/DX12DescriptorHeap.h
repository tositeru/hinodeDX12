#pragma once

#include "../common/Common.h"
#include "../IDX12Interface.h"

namespace hinode
{
	namespace graphics
	{
		class DX12DescriptorHeap;

		/// @brief D3DX12_CPU_DESCRIPTOR_HANDLEをもつ構造体
		struct DX12CPUDescriptorHandle
		{
			UINT handleIncrementSize;
			CD3DX12_CPU_DESCRIPTOR_HANDLE d3dx12Handle;

			DX12CPUDescriptorHandle();

			/// @brief コンストラクタ
			/// @param[in] heap
			/// @param[in] startIndex Cpuハンドルが最初に指している場所
			DX12CPUDescriptorHandle(DX12DescriptorHeap& heap, int startIndex)noexcept;

			/// @brief ハンドルが指すポインタをcount分進める
			/// @param[in] count
			void Offset(int count)noexcept;

			/// @brief ハンドルを作成する
			/// @param[in] offset 
			DX12CPUDescriptorHandle makeHandle(int offset)const noexcept;

			/// @brief 現在のcpuハンドルが指している場所にレンダーターゲットビューを設定する
			/// @param[in] pDevice
			/// @param[in] pResource
			/// @param[in] pDesc
			void createRenderTargetView(ID3D12Device* pDevice, ID3D12Resource* pResource, const D3D12_RENDER_TARGET_VIEW_DESC *pDesc)noexcept;

			/// @brief 現在のcpuハンドルが指している場所に深度ステンシルビューを設定する
			/// @param[in] pDevice
			/// @param[in] pResource
			/// @param[in] pDesc
			void createDepthStencilView(ID3D12Device* pDevice, ID3D12Resource* pResource, const D3D12_DEPTH_STENCIL_VIEW_DESC *pDesc)noexcept;

			/// @brief 現在のcpuハンドルが指している場所に定数バッファビューを設定する
			/// @param[in] pDevice
			/// @param[in] pDesc SizeInBytesの値は中で256バイトの倍数になるよう変更します
			void createConstantBufferView(ID3D12Device* pDevice, D3D12_CONSTANT_BUFFER_VIEW_DESC* pDesc)noexcept;

			/// @brief 現在のcpuハンドルが指している場所にシェーダリソースビューを設定する
			/// @param[in] pDevice
			/// @param[in] pResource
			/// @param[in] pDesc
			void createShaderResourceView(ID3D12Device* pDevice, ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc)noexcept;

			/// @brief 現在のcpuハンドルが指している場所にアンオーダードアクセスビューを設定する
			/// @param[in] pDevice
			/// @param[in] pResource
			/// @param[in] pDesc
			/// @param[in] pCounter
			void createUnorderedAccessView(ID3D12Device* pDevice, ID3D12Resource* pResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* pDesc, ID3D12Resource* pCounter)noexcept;

			/// @brief 現在のcpuハンドルが指している場所にサンプラを設定する
			/// @param[in] pDevice
			/// @param[in] pDesc
			void createSampler(ID3D12Device* pDevice, const D3D12_SAMPLER_DESC* pDesc)noexcept;

		};
	}

	namespace graphics
	{
		/// @brief ID3D12DescriptorHeapを内部に持つクラス
		class DX12DescriptorHeap : public IDX12Interface
		{
			UNCOPYABLE_PIMPL_IDIOM_TEMPLATE(DX12DescriptorHeap)
		public:
			/// @brief メモリ開放
			void clear()noexcept;

			/// @brief ルートシグネチャを作成する
			///
			/// @param[in] pDevice
			/// @param[in] pDesc
			/// @exception hinode::graphics::InvalidArgumentsException
			void create(ID3D12Device* pDevice, const D3D12_DESCRIPTOR_HEAP_DESC* pDesc);

			/// @brief CPUデスクリプタを生成する
			/// @param[in] startIndex
			/// @retval DX12CPUDescriptorHandle
			DX12CPUDescriptorHandle makeCpuHandle(int startIndex)noexcept;

			/// @brief 内部のDX12CommandQueueに名前をつける
			/// @param[in] const wchar_t* name
			/// @retval HRESULT
			HRESULT setName(const wchar_t* name) noexcept override;

			/// @brief 使用できる状態か?
			/// @retval bool
			bool isGood()const noexcept override;

		accessor_declaration:
			ID3D12DescriptorHeap* operator->()noexcept;

			ID3D12DescriptorHeap* descriptorHeap()noexcept;
			operator ID3D12DescriptorHeap*()noexcept { return this->descriptorHeap(); }
			UINT descriptorHandleIncrementSize()const noexcept;

			/// @brief 関連しているID3D12Deviceを取得する
			/// @retval ID3D12Device*
			ID3D12Device* getDevice()noexcept;
		};
	}
}
