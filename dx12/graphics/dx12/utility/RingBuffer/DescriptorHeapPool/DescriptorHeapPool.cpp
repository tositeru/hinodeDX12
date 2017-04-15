#include "stdafx.h"

#include "DescriptorHeapPool.h"

#include <array>
#include "../../../common/Exception.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			DescriptorHeapPool::DescriptorHeapPool()
				: mHeapSize(-1)
				, mType(D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES)
			{ }

			DescriptorHeapPool::~DescriptorHeapPool()
			{
				this->clear();
			}

			void DescriptorHeapPool::clear()
			{
				RingBuffer::clear();

				this->mHeapSize = -1;
				this->mHeap.clear();
				this->mType = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
			}

			void DescriptorHeapPool::create(ID3D12Device* pDevice, const D3D12_DESCRIPTOR_HEAP_DESC* pDesc)
			{
				this->clear();

				this->mHeap.create(pDevice, pDesc);
				this->mHeapSize = pDesc->NumDescriptors;
				this->mType = pDesc->Type;

				RingBuffer::init(pDevice, this->mHeapSize);
			}

			DescriptorHeapPool::AllocateInfo DescriptorHeapPool::allocate(int allocationCount)
			{
				auto offset = RingBuffer::updateCurrentOffset(allocationCount);

				DescriptorHeapPool::AllocateInfo ret(this->mHeap.makeCpuHandle(static_cast<int>(offset)));
				ret.gpuHandle = this->mHeap->GetGPUDescriptorHandleForHeapStart();
				ret.gpuHandle.ptr += offset * this->mHeap.descriptorHandleIncrementSize();
				ret.allocateFrameID = RingBuffer::currentFrame();

				return ret;
			}

			bool DescriptorHeapPool::isSameAllocateFrameID(const AllocateInfo& allocateInfo)const noexcept
			{
				return RingBuffer::currentFrame() == allocateInfo.allocateFrameID;
			}

			DX12DescriptorHeap& DescriptorHeapPool::heap()noexcept
			{
				return this->mHeap;
			}

			D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapPool::type()const noexcept
			{
				return this->mType;
			}

			ID3D12Device* DescriptorHeapPool::getDevice()
			{
				return this->mHeap.getDevice();
			}

			//
			// DescriptorHeapPool::AllocateInfo
			//
			DescriptorHeapPool::AllocateInfo::AllocateInfo()
				: allocateFrameID(-1)
				, cpuHandle(DX12CPUDescriptorHandle())
			{}

			DescriptorHeapPool::AllocateInfo::AllocateInfo(DX12CPUDescriptorHandle cpuHandle)
				: allocateFrameID(-1)
				, cpuHandle(cpuHandle)
			{}

			void DescriptorHeapPool::AllocateInfo::clear()
			{
				this->allocateFrameID = -1;
				this->cpuHandle = DX12CPUDescriptorHandle();
				this->gpuHandle = D3D12_GPU_DESCRIPTOR_HANDLE();
			}
		}

		namespace utility
		{
			void DescriptorHeapPoolSet::clear()
			{
				this->mResource.clear();
				this->mSampler.clear();
			}

			void DescriptorHeapPoolSet::create(ID3D12Device* pDevice, const D3D12_DESCRIPTOR_HEAP_DESC* pResourceDesc, const D3D12_DESCRIPTOR_HEAP_DESC* pSamplerDesc)
			{
				this->clear();
				auto desc = *pResourceDesc;
				desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
				this->mResource.create(pDevice, &desc);
				desc = *pSamplerDesc;
				desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
				this->mSampler.create(pDevice, &desc);
			}

			void DescriptorHeapPoolSet::create(ID3D12Device* pDevice, UINT resourceLimit, UINT samplerLimit, UINT nodeMask)
			{
				D3D12_DESCRIPTOR_HEAP_DESC resourceHeapDesc = {};
				resourceHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
				resourceHeapDesc.NumDescriptors = resourceLimit;
				resourceHeapDesc.NodeMask = nodeMask;
				auto samplerHeapDesc = resourceHeapDesc;
				samplerHeapDesc.NumDescriptors = samplerLimit;
				samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
				this->create(pDevice, &resourceHeapDesc, &samplerHeapDesc);
			}

			void DescriptorHeapPoolSet::bind(ID3D12GraphicsCommandList* pCommandList)
			{
				std::array<ID3D12DescriptorHeap*, 2> pHeaps = { {
					this->mResource.heap(),
					this->mSampler.heap()
				} };

				pCommandList->SetDescriptorHeaps(static_cast<UINT>(pHeaps.size()), pHeaps.data());
			}

			void DescriptorHeapPoolSet::endAndGoNextFrame(DX12CommandQueue& queue)
			{
				this->mResource.endAndGoNextFrame(queue);
				this->mSampler.endAndGoNextFrame(queue);
			}

			DescriptorHeapPool& DescriptorHeapPoolSet::resource()noexcept
			{
				return this->mResource;
			}

			DescriptorHeapPool& DescriptorHeapPoolSet::sampler()noexcept
			{
				return this->mSampler;
			}
		}
	}
}