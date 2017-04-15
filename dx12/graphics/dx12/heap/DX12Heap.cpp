#include "stdafx.h"

#include "DX12Heap.h"

#include "../common/Exception.h"

namespace hinode
{
	namespace graphics
	{
		/// @brief DX12Resourceを内部に持つクラス
		class DX12Heap::_impl : private ImplInterfacePointer<DX12Heap>
		{
		public:
			UNCOPYABLE_IMPL_MOVE_CONSTRUCTOR(DX12Heap);

			ID3D12Heap* mpHeap;

			_impl(DX12Heap* pInterface)
				: ImplInterfacePointer(pInterface)
				, mpHeap(nullptr)
			{}

			_impl& operator=(_impl&& right)noexcept
			{
				this->mpHeap = right.mpHeap;
				right.mpHeap = nullptr;
				return *this;
			}

			~_impl()noexcept
			{
				this->clear();
			}

			void clear()noexcept
			{
				safeRelease(&this->mpHeap);
			}

			void create(ID3D12Device* pDevice, const D3D12_HEAP_DESC* pDesc)
			{
				this->clear();
				if (FAILED(pDevice->CreateHeap(pDesc, IID_PPV_ARGS(&this->mpHeap)))) {
					throw HINODE_GRAPHICS_CREATE_EXCEPTION(FailedToCreateException, "DX12Heap", "create")
						<< "DX12Resourceの作成に失敗しました";
				}
			}

			HRESULT setName(const wchar_t* name) noexcept
			{
				assert(nullptr != this->mpHeap || "まだ初期化されていません");
				return this->mpHeap->SetName(name);
			}

			bool isGood()const noexcept
			{
				return nullptr != this->mpHeap;
			}

		accessor_declaration:
			ID3D12Heap* operator->()noexcept
			{
				return this->heap();
			}

			ID3D12Heap* heap()noexcept
			{
				assert(nullptr != this->mpHeap || "まだ初期化されていません");
				return this->mpHeap;
			}
		};

		//===========================================================================
		//
		//	IMPLイディオムのインターフェイスクラスの関数定義
		//

		UNCOPYABLE_PIMPL_IDIOM_CPP_TEMPLATE(DX12Heap);

		void DX12Heap::clear()
		{
			this->impl().clear();
		}

		void DX12Heap::create(ID3D12Device* pDevice, const D3D12_HEAP_DESC* pDesc)
		{
			this->impl().create(pDevice, pDesc);
		}

		HRESULT DX12Heap::setName(const wchar_t* name)
		{
			return this->impl().setName(name);
		}

		bool DX12Heap::isGood()const noexcept
		{
			return this->impl().isGood();
		}

		ID3D12Heap* DX12Heap::heap()noexcept
		{
			return this->impl().heap();
		}
	}
}