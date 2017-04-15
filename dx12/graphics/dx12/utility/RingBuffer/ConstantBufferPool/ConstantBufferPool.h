#pragma once

#include <d3d12.h>
#include <queue>

#include "../../../common/Common.h"
#include "../../../resource/DX12Resource.h"
#include "../../../fence/DX12Fence.h"
#include "../RingBuffer.h"

namespace hinode
{
	namespace graphics
	{
		class DX12CommandQueue;

		namespace utility
		{
			/// @brief 定数バッファ用のメモリプール
			///
			/// 後々、複数フレームにわたってメモリ割り当てが出来るようにする
			class ConstantBufferPool : public RingBuffer
			{
				ConstantBufferPool(ConstantBufferPool&) = delete;
				void operator=(ConstantBufferPool&) = delete;
			public:
				struct AllocateInfo
				{
					D3D12_CONSTANT_BUFFER_VIEW_DESC cbViewDesc;
					void* cpuMemory;
				};

			public:
				ConstantBufferPool();
				virtual ~ConstantBufferPool();

				/// @brief メモリ開放
				///
				/// プール内の内容が何も使われていない時によびだすこと TODO 自由なタイミングで呼び出せるようにする
				virtual void clear()override;

				/// @brief メモリプールを作成する
				///
				/// @param[in] pDevice
				/// @param[in] poolByteSize KB単位のほうがいい？
				/// @exception 
				void create(ID3D12Device* pDevice, uint64_t poolByteSize);

				///	@brief プールからメモリを割り当てる
				///
				/// 割当に失敗するとhinode::graphics::RunTimeErrorExceptionを投げます
				/// @param[in] pData pDataの内容を関数内部でコピーしています。　nullptrだとコピーは行いません
				/// @param[in] allocateByteSize
				/// @retval AllocateInfo
				/// @exception hinode::graphics::RunTimeErrorException
				AllocateInfo allocate(const void* pData, uint32_t allocateByteSize);

			accessor_declaration:
				DX12Resource& memoryPool()noexcept;

			private:
				DX12Resource mPool;
				UINT8* mpPoolHead;
				UINT64 mByteSize;

				static const uint32_t M_POOL_ALIGNMENT;
			};
		}
	}
}