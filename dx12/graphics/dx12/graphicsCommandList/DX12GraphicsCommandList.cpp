#include "stdafx.h"

#include "DX12GraphicsCommandList.h"

#include "../common/Log.h"
#include "../common/Exception.h"

namespace hinode
{
	namespace graphics
	{
		/// @brief DX12GraphicsCommandListを内部に持つクラス
		class DX12GraphicsCommandList::_impl : private ImplInterfacePointer<DX12GraphicsCommandList>
		{
		public:
			UNCOPYABLE_IMPL_MOVE_CONSTRUCTOR(DX12GraphicsCommandList);

			ID3D12GraphicsCommandList* mpList;

			_impl(DX12GraphicsCommandList* pInterface)
				: ImplInterfacePointer(pInterface)
				, mpList(nullptr)
			{}

			_impl& operator=(_impl&& right)_noexcept
			{
				this->mpList = right.mpList;
				right.mpList = nullptr;
				return *this;
			}

			~_impl()_noexcept
			{
				this->clear();
			}

			void clear()_noexcept
			{
				safeRelease(&this->mpList);
			}

			void create(ID3D12Device* pDevice, DX12GraphicsCommandListDesc& desc)
			{
				if (FAILED(pDevice->CreateCommandList(desc.node, desc.type, desc.pAllocator, desc.pPipelineState, IID_PPV_ARGS(&this->mpList)))) {
					throw HINODE_GRAPHICS_CREATE_EXCEPTION(InvalidArgumentsException, "DX12GraphicsCommandList", "create")
						<< "DX12GraphicsCommandListの作成に失敗しました";
				}
				this->mpList->Close();
			}

			bool reset(ID3D12CommandAllocator* pAllocator, ID3D12PipelineState* pState)noexcept
			{
				assert(nullptr != this->mpList || "まだ初期化されていません");

				auto hr = this->mpList->Reset(pAllocator, pState);
				if (FAILED(hr)) {
					hinode::graphics::Log(hinode::graphics::Log::eERROR) << "コマンドリストのリセットに失敗";
					return false;
				}
				return true;
			}

			bool close()noexcept
			{
				assert(nullptr != this->mpList || "まだ初期化されていません");

				auto hr = this->mpList->Close();
				if (FAILED(hr)) {
					hinode::graphics::Log(hinode::graphics::Log::eERROR) << "コマンドリストの記録終了失敗";
					return false;
				}
				return true;
			}

			bool record(ID3D12CommandAllocator* pAllocator, ID3D12PipelineState* pState, std::function<void(DX12GraphicsCommandList& cmdList)> pred)
			{
				unless(this->reset(pAllocator, pState)) {
					return false;
				}

				pred(*this->interface_());

				unless(this->close()) {
					return false;
				}
				return true;
			}

			void setDescriptorHeaps(std::initializer_list<ID3D12DescriptorHeap*> heaps)
			{
				assert(nullptr != this->mpList || "まだ初期化されていません");
				this->mpList->SetDescriptorHeaps(static_cast<UINT>(heaps.size()), heaps.begin());
			}

			HRESULT setName(const wchar_t* name) _noexcept
			{
				assert(nullptr != this->mpList || "まだ初期化されていません");
				return this->mpList->SetName(name);
			}

			bool isGood()const noexcept
			{
				return nullptr != this->mpList;
			}

		accessor_declaration:
			ID3D12GraphicsCommandList* operator->()_noexcept
			{
				assert(nullptr != this->mpList || "まだ初期化されていません");
				return this->cmdList();
			}

			ID3D12GraphicsCommandList* cmdList()_noexcept
			{
				assert(nullptr != this->mpList || "まだ初期化されていません");
				return this->mpList;
			}
		};

		//===========================================================================
		//
		//	IMPLイディオムのインターフェイスクラスの関数定義
		//

		UNCOPYABLE_PIMPL_IDIOM_CPP_TEMPLATE(DX12GraphicsCommandList);

		void DX12GraphicsCommandList::clear()_noexcept
		{
			this->impl().clear();
		}

		void DX12GraphicsCommandList::create(ID3D12Device* pDevice, DX12GraphicsCommandListDesc& desc)
		{
			this->impl().create(pDevice, desc);
		}
		
		bool DX12GraphicsCommandList::reset(ID3D12CommandAllocator* pAllocator, ID3D12PipelineState* pState)noexcept
		{
			return this->impl().reset(pAllocator, pState);
		}

		bool DX12GraphicsCommandList::close()noexcept
		{
			return this->impl().close();
		}

		bool DX12GraphicsCommandList::record(ID3D12CommandAllocator* pAllocator, ID3D12PipelineState* pState, std::function<void(DX12GraphicsCommandList& cmdList)> pred)
		{
			return this->impl().record(pAllocator, pState, pred);
		}

		void DX12GraphicsCommandList::setDescriptorHeaps(std::initializer_list<ID3D12DescriptorHeap*> heaps)
		{
			this->impl().setDescriptorHeaps(heaps);
		}

		HRESULT DX12GraphicsCommandList::setName(const wchar_t* name) _noexcept
		{
			return this->impl().setName(name);
		}

		bool DX12GraphicsCommandList::isGood()const noexcept
		{
			return this->impl().isGood();
		}

		ID3D12GraphicsCommandList* DX12GraphicsCommandList::operator->()_noexcept
		{
			return this->impl().cmdList();
		}

		ID3D12GraphicsCommandList* DX12GraphicsCommandList::cmdList()_noexcept
		{
			return this->impl().cmdList();
		}
	}

	namespace graphics
	{
		DX12GraphicsCommandListDesc::DX12GraphicsCommandListDesc(UINT node, D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator* pAllocator, ID3D12PipelineState* pPipelineState)noexcept
			: node(node)
			, type(type)
			, pAllocator(pAllocator)
			, pPipelineState(pPipelineState)
		{}
	}
}