#include "stdafx.h"

#include "DX12DescriptorHeap.h"

#include "../common/Log.h"
#include "../common/Exception.h"

namespace hinode
{
	namespace graphics
	{
		/// @brief DX12DescriptorHeapを内部に持つクラス
		class DX12DescriptorHeap::_impl : private ImplInterfacePointer<DX12DescriptorHeap>
		{
		public:
			UNCOPYABLE_IMPL_MOVE_CONSTRUCTOR(DX12DescriptorHeap);

			ID3D12DescriptorHeap* mpDescriptorHeap;
			UINT mSize;

			_impl(DX12DescriptorHeap* pInterface)
				: ImplInterfacePointer(pInterface)
				, mpDescriptorHeap(nullptr)
			{}

			_impl& operator=(_impl&& right)noexcept
			{
				this->mpDescriptorHeap = right.mpDescriptorHeap;
				this->mSize = right.mSize;
				right.mpDescriptorHeap = nullptr;
				right.mSize = 0;
				return *this;
			}

			~_impl()noexcept
			{
				this->clear();
			}

			void clear()noexcept
			{
				safeRelease(&this->mpDescriptorHeap);
				this->mSize = 0;
			}

			void create(ID3D12Device* pDevice, const D3D12_DESCRIPTOR_HEAP_DESC* pDesc)
			{
				this->clear();
				if (FAILED(pDevice->CreateDescriptorHeap(pDesc, IID_PPV_ARGS(&this->mpDescriptorHeap)))) {
					throw HINODE_GRAPHICS_CREATE_EXCEPTION(InvalidArgumentsException, "DX12DescriptorHeap", "create")
						<< "DX12DescriptorHeapの作成に失敗しました";
				}
				this->mSize = pDevice->GetDescriptorHandleIncrementSize(pDesc->Type);
			}

			DX12CPUDescriptorHandle makeCpuHandle(int startIndex)noexcept
			{
				assert(nullptr != this->mpDescriptorHeap || "まだ初期化されていません");
				return DX12CPUDescriptorHandle(*this->interface_(), startIndex);
			}


			HRESULT setName(const wchar_t* name) noexcept
			{
				assert(nullptr != this->mpDescriptorHeap || "まだ初期化されていません");
				return this->mpDescriptorHeap->SetName(name);
			}

			bool isGood()const noexcept
			{
				return nullptr != this->mpDescriptorHeap;
			}

		accessor_declaration:
			ID3D12DescriptorHeap* operator->()noexcept
			{
				assert(nullptr != this->mpDescriptorHeap || "まだ初期化されていません");
				return this->descriptorHeap();
			}

			ID3D12DescriptorHeap* descriptorHeap()noexcept
			{
				assert(nullptr != this->mpDescriptorHeap || "まだ初期化されていません");
				return this->mpDescriptorHeap;
			}

			UINT descriptorHandleIncrementSize()const noexcept
			{
				assert(nullptr != this->mpDescriptorHeap || "まだ初期化されていません");
				return this->mSize;
			}

			ID3D12Device* getDevice()noexcept
			{
				ID3D12Device* pDevice;
				auto hr = this->mpDescriptorHeap->GetDevice(IID_PPV_ARGS(&pDevice));
				assert(SUCCEEDED(hr));
				pDevice->Release();
				return pDevice;
			}
		};

		//===========================================================================
		//
		//	IMPLイディオムのインターフェイスクラスの関数定義
		//

		UNCOPYABLE_PIMPL_IDIOM_CPP_TEMPLATE(DX12DescriptorHeap);

		void DX12DescriptorHeap::clear()noexcept
		{
			this->impl().clear();
		}

		void DX12DescriptorHeap::create(ID3D12Device* pDevice, const D3D12_DESCRIPTOR_HEAP_DESC* pDesc)
		{
			this->impl().create(pDevice, pDesc);
		}

		DX12CPUDescriptorHandle DX12DescriptorHeap::makeCpuHandle(int startIndex)noexcept
		{
			return this->impl().makeCpuHandle(startIndex);
		}

		HRESULT DX12DescriptorHeap::setName(const wchar_t* name) noexcept
		{
			return this->impl().setName(name);
		}

		bool DX12DescriptorHeap::isGood()const noexcept
		{
			return this->impl().isGood();
		}

		ID3D12DescriptorHeap* DX12DescriptorHeap::operator->()noexcept
		{
			return this->impl().descriptorHeap();
		}

		ID3D12DescriptorHeap* DX12DescriptorHeap::descriptorHeap()noexcept
		{
			return this->impl().descriptorHeap();
		}

		UINT DX12DescriptorHeap::descriptorHandleIncrementSize()const noexcept
		{
			return this->impl().descriptorHandleIncrementSize();
		}

		ID3D12Device* DX12DescriptorHeap::getDevice()noexcept
		{
			return this->impl().getDevice();
		}
	}

	namespace graphics
	{
		DX12CPUDescriptorHandle::DX12CPUDescriptorHandle()
			: handleIncrementSize(-1)
			, d3dx12Handle()
		{ }

		DX12CPUDescriptorHandle::DX12CPUDescriptorHandle(DX12DescriptorHeap& heap, int startIndex)noexcept
			: handleIncrementSize(heap.descriptorHandleIncrementSize())
			, d3dx12Handle(heap->GetCPUDescriptorHandleForHeapStart(), startIndex, heap.descriptorHandleIncrementSize())
		{}

		void DX12CPUDescriptorHandle::Offset(int count)noexcept
		{
			this->d3dx12Handle.Offset(count, this->handleIncrementSize);
		}

		DX12CPUDescriptorHandle DX12CPUDescriptorHandle::makeHandle(int offset)const noexcept
		{
			auto h = *this;
			h.Offset(offset);
			return h;
		}

		void DX12CPUDescriptorHandle::createRenderTargetView(ID3D12Device* pDevice, ID3D12Resource* pResource, const D3D12_RENDER_TARGET_VIEW_DESC *pDesc)noexcept
		{
			pDevice->CreateRenderTargetView(pResource, pDesc, this->d3dx12Handle);
		}

		void DX12CPUDescriptorHandle::createDepthStencilView(ID3D12Device* pDevice, ID3D12Resource* pResource, const D3D12_DEPTH_STENCIL_VIEW_DESC *pDesc)noexcept
		{
			pDevice->CreateDepthStencilView(pResource, pDesc, this->d3dx12Handle);
		}

		void DX12CPUDescriptorHandle::createConstantBufferView(ID3D12Device* pDevice, D3D12_CONSTANT_BUFFER_VIEW_DESC* pDesc)noexcept
		{
			pDesc->SizeInBytes = (pDesc->SizeInBytes+255) & ~255;
			pDevice->CreateConstantBufferView(
				pDesc,
				this->d3dx12Handle);
		}

		void DX12CPUDescriptorHandle::createShaderResourceView(ID3D12Device* pDevice, ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc)noexcept
		{
			pDevice->CreateShaderResourceView(pResource, pDesc, this->d3dx12Handle);
		}

		void DX12CPUDescriptorHandle::createUnorderedAccessView(ID3D12Device* pDevice, ID3D12Resource* pResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* pDesc, ID3D12Resource* pCounter)noexcept
		{
			pDevice->CreateUnorderedAccessView(pResource, pCounter, pDesc, this->d3dx12Handle);
		}

		void DX12CPUDescriptorHandle::createSampler(ID3D12Device* pDevice, const D3D12_SAMPLER_DESC* pDesc)noexcept
		{
			pDevice->CreateSampler(pDesc, this->d3dx12Handle);
		}
	}
}