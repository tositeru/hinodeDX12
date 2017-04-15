#include "stdafx.h"

#include "ConstantBufferPool.h"

#include "..\..\..\common\Exception.h"
#include "../../../commandQueue/DX12CommandQueue.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			const uint32_t ConstantBufferPool::M_POOL_ALIGNMENT = 256u;

			ConstantBufferPool::ConstantBufferPool()
				: mByteSize(0u)
			{}

			ConstantBufferPool::~ConstantBufferPool()
			{
				this->clear();
			}

			void ConstantBufferPool::clear()
			{
				RingBuffer::clear();
				//TODO 自由なタイミングで呼び出せるようにする マルチスレッド前提で作るしか無い？
				if (this->mPool.isGood()) {
					this->mPool->Unmap(0, nullptr);
				}
				this->mPool.clear();
				this->mpPoolHead = nullptr;
				this->mByteSize = 0u;
			}

			void ConstantBufferPool::create(ID3D12Device* pDevice, uint64_t poolByteSize)
			{
				this->clear();
				poolByteSize += ConstantBufferPool::M_POOL_ALIGNMENT - poolByteSize % ConstantBufferPool::M_POOL_ALIGNMENT;
				assert(poolByteSize%ConstantBufferPool::M_POOL_ALIGNMENT == 0);
				this->mPool.create(pDevice, &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(poolByteSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr);

				CD3DX12_RANGE readRange(0, 0);
				if (FAILED(this->mPool->Map(0, &readRange, (void**)&this->mpPoolHead))) {
					this->clear();
					throw HINODE_GRAPHICS_CREATE_EXCEPTION(FailedToCreateException, "ConstantBufferPool", "init")
						<< "定数バッファのマップに失敗";
				}

				this->mByteSize = poolByteSize;

				RingBuffer::init(pDevice, this->mByteSize);
			}

			ConstantBufferPool::AllocateInfo ConstantBufferPool::allocate(const void* pData, uint32_t allocateByteSize)
			{
				assert(this->mPool.isGood());

				allocateByteSize = allocateByteSize + (ConstantBufferPool::M_POOL_ALIGNMENT - allocateByteSize % ConstantBufferPool::M_POOL_ALIGNMENT);
				auto offset = RingBuffer::currentOffset();
				RingBuffer::updateCurrentOffset(allocateByteSize);

				AllocateInfo info;
				info.cbViewDesc.BufferLocation = this->mPool->GetGPUVirtualAddress() + offset;
				info.cbViewDesc.SizeInBytes = allocateByteSize;//CB size is required to be 256-byte aligned.

				info.cpuMemory = this->mpPoolHead + offset;
				if (pData) {
					memcpy(info.cpuMemory, pData, allocateByteSize);
				}
				return info;
			}

			DX12Resource& ConstantBufferPool::memoryPool()noexcept
			{
				return this->mPool;
			}

		}
	}
}