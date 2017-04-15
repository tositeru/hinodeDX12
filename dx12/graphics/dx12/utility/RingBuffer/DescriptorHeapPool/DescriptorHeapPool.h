#pragma once

#include "../../../descriptorHeap/DX12DescriptorHeap.h"
#include "../RingBuffer.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			/// @brief 巨大なDescriptorHeapを管理するクラス
			/// 
			/// ConstantBufferPoolと処理の共通化をしたい
			class DescriptorHeapPool : public RingBuffer
			{
				DescriptorHeapPool(DescriptorHeapPool&) = delete;
				void operator=(DescriptorHeapPool&) = delete;
			public:
				struct AllocateInfo
				{
					AllocateInfo();
					AllocateInfo(DX12CPUDescriptorHandle cpuHandle);
					void clear();

					UINT64 allocateFrameID;
					DX12CPUDescriptorHandle cpuHandle;
					D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
				};

			public:
				DescriptorHeapPool();
				~DescriptorHeapPool();

				void clear();

				/// @brief 
				/// @param[in] pDevice
				/// @param[in] pDesc
				void create(ID3D12Device* pDevice, const D3D12_DESCRIPTOR_HEAP_DESC* pDesc);

				///	@brief プールからメモリを割り当てる
				///
				/// 割当に失敗するとhinode::graphics::RunTimeErrorExceptionを投げます
				/// @param[in] allocationCount default=1
				/// @retval DescriptorHeapPool::AllocateInfo
				/// @exception hinode::graphics::RunTimeErrorException
				AllocateInfo allocate(int allocationCount=1);

				/// @brief 同じIDを持っているか判定する
				/// @parma[in] allocateInfo
				/// @retval bool
				bool isSameAllocateFrameID(const AllocateInfo& allocateInfo)const noexcept;

			accessor_declaration:
				DX12DescriptorHeap& heap()noexcept;
				D3D12_DESCRIPTOR_HEAP_TYPE type()const noexcept;

				/// @brief 関連しているID3D12Deviceを取得する
				/// @retval ID3D12Device*
				ID3D12Device* getDevice();

			private:
				//int mAllocateFrameID;
				int mHeapSize;
				D3D12_DESCRIPTOR_HEAP_TYPE mType;
				DX12DescriptorHeap mHeap;
			};
		}

		namespace utility
		{
			class DescriptorHeapPoolSet
			{
				DescriptorHeapPoolSet(DescriptorHeapPoolSet&) = delete;
				DescriptorHeapPoolSet& operator=(DescriptorHeapPoolSet&) = delete;
			public:
				DescriptorHeapPoolSet() = default;

				/// @brief メモリ解放
				void clear();

				/// @brief 作成
				/// pResourceDescおよびpSamplerDescのFlagsは作成時自動的にD3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLEが設定されます。
				/// @param[in] pDevice
				/// @param[in] pResourceDesc
				/// @param[in] pSamplerDesc
				void create(ID3D12Device* pDevice, const D3D12_DESCRIPTOR_HEAP_DESC* pResourceDesc, const D3D12_DESCRIPTOR_HEAP_DESC* pSamplerDesc);

				/// @brief 個数指定で作成(簡略版)
				/// @param[in] pDevice
				/// @param[in] resourceLimit
				/// @param[in] samplerLimit
				/// @param[in] nodeMask default=0
				void create(ID3D12Device* pDevice, UINT resourceLimit, UINT samplerLimit, UINT nodeMask = 0u);

				/// @brief コマンドリストに設定する
				/// @param[in] pCommandList
				void bind(ID3D12GraphicsCommandList* pCommandList);

				/// @brief 次のフレーム用のバッファを割り当て始める
				/// @param[in] queue
				void endAndGoNextFrame(DX12CommandQueue& queue);

			accessor_declaration:
				DescriptorHeapPool& resource()noexcept;
				DescriptorHeapPool& sampler()noexcept;

			private:
				DescriptorHeapPool mResource;
				DescriptorHeapPool mSampler;
			};
		}
	}
}
