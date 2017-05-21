#include "stdafx.h"

#include "DX12QueryHeap.h"

namespace hinode
{
	namespace graphics
	{
		/// @brief DX12Fenceを内部に持つクラス
		class DX12QueryHeap::_impl : private ImplInterfacePointer<DX12QueryHeap>
		{
		public:
			UNCOPYABLE_IMPL_MOVE_CONSTRUCTOR(DX12QueryHeap);

			ID3D12QueryHeap* mpQueryHeap;

			_impl(DX12QueryHeap* pInterface)
				: ImplInterfacePointer(pInterface)
				, mpQueryHeap(nullptr)
			{}

			_impl& operator=(_impl&& right)_noexcept
			{
				this->mpQueryHeap = right.mpQueryHeap;
				right.mpQueryHeap = nullptr;
				return *this;
			}

			~_impl()_noexcept
			{
				this->clear();
			}

			void clear()noexcept
			{
				safeRelease(&this->mpQueryHeap);
			}

			void create(ID3D12Device* pDevice, D3D12_QUERY_HEAP_DESC* pDesc)
			{
				this->clear();

				auto hr = pDevice->CreateQueryHeap(pDesc, IID_PPV_ARGS(&this->mpQueryHeap));
				if (FAILED(hr)) {
					throw HINODE_GRAPHICS_CREATE_EXCEPTION(InvalidArgumentsException, "DX12QueryHeap", "create")
						<< "DX12QueryHeapの作成に失敗しました";
				}
			}

			HRESULT setName(const wchar_t* name) noexcept
			{
				assert(nullptr != this->mpQueryHeap || "まだ初期化されていません");
				return this->mpQueryHeap->SetName(name);
			}

			bool isGood()const noexcept
			{
				return nullptr != this->mpQueryHeap;
			}

		accessor_declaration:
			ID3D12QueryHeap* operator->()noexcept
			{
				assert(nullptr != this->mpQueryHeap || "まだ初期化されていません");
				return this->mpQueryHeap;
			}

			ID3D12QueryHeap* queryHeap()noexcept
			{
				assert(nullptr != this->mpQueryHeap || "まだ初期化されていません");
				return this->mpQueryHeap;
			}
		};

		//===========================================================================
		//
		//	IMPLイディオムのインターフェイスクラスの関数定義
		//

		UNCOPYABLE_PIMPL_IDIOM_CPP_TEMPLATE(DX12QueryHeap);

		void DX12QueryHeap::clear()noexcept
		{
			this->impl().clear();
		}

		void DX12QueryHeap::create(ID3D12Device* pDevice, D3D12_QUERY_HEAP_DESC* pDesc)
		{
			this->impl().create(pDevice, pDesc);
		}

		HRESULT DX12QueryHeap::setName(const wchar_t* name) noexcept
		{
			return this->impl().setName(name);
		}

		bool DX12QueryHeap::isGood()const noexcept
		{
			return this->impl().isGood();
		}

		ID3D12QueryHeap* DX12QueryHeap::operator->()noexcept
		{
			return this->impl().operator->();
		}

		ID3D12QueryHeap* DX12QueryHeap::queryHeap()noexcept
		{
			return this->impl().queryHeap();
		}
	}
}