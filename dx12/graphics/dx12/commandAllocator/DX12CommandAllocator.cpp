#include "stdafx.h"

#include "DX12CommandAllocator.h"

#include "../common/Log.h"
#include "../common/Exception.h"

namespace hinode
{
	namespace graphics
	{
		class DX12CommandAllocator::_impl : private ImplInterfacePointer<DX12CommandAllocator>
		{
		public:
			UNCOPYABLE_IMPL_MOVE_CONSTRUCTOR(DX12CommandAllocator);

			ID3D12CommandAllocator* mpAllocator;

			_impl(DX12CommandAllocator* pInterface)
				: ImplInterfacePointer(pInterface)
				, mpAllocator(nullptr)
			{}

			_impl& operator=(_impl&& right)_noexcept
			{
				this->mpAllocator = right.mpAllocator;
				right.mpAllocator = nullptr;
				return *this;
			}

			~_impl()_noexcept
			{
				this->clear();
			}

			void clear()_noexcept
			{
				safeRelease(&this->mpAllocator);
			}

			void create(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type)
			{
				this->clear();
				auto hr = pDevice->CreateCommandAllocator(type, IID_PPV_ARGS(&this->mpAllocator));
				if (FAILED(hr)) {
					throw HINODE_GRAPHICS_CREATE_EXCEPTION(InvalidArgumentsException, "DX12CommandAllocator", "create")
						<< "DX12CommandAllocatorの作成に失敗しました";
				}
			}

			HRESULT setName(const wchar_t* name) _noexcept
			{
				assert(nullptr != this->mpAllocator || "まだ初期化されていません");
				return this->mpAllocator->SetName(name);

			}

			bool isGood()const noexcept
			{
				return nullptr != this->mpAllocator;
			}

		accessor_declaration:
			ID3D12CommandAllocator* operator->()_noexcept
			{
				return this->allocator();
			}

			ID3D12CommandAllocator* allocator()_noexcept
			{
				return this->mpAllocator;
			}
		};

		//===========================================================================
		//
		//	IMPLイディオムのインターフェイスクラスの関数定義
		//

		UNCOPYABLE_PIMPL_IDIOM_CPP_TEMPLATE(DX12CommandAllocator);

		void DX12CommandAllocator::clear()_noexcept
		{
			this->impl().clear();
		}

		void DX12CommandAllocator::create(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type)
		{
			this->impl().create(pDevice, type);
		}

		HRESULT DX12CommandAllocator::setName(const wchar_t* name) _noexcept
		{
			return this->impl().setName(name);
		}

		bool DX12CommandAllocator::isGood()const noexcept
		{
			return this->impl().isGood();
		}

		ID3D12CommandAllocator* DX12CommandAllocator::operator->()_noexcept
		{
			return this->impl().allocator();
		}

		ID3D12CommandAllocator* DX12CommandAllocator::allocator()_noexcept
		{
			return this->impl().allocator();
		}

	}
}