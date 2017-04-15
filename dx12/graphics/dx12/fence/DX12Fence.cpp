#include "stdafx.h"

#include "DX12Fence.h"

#include "../common/Log.h"
#include "../common/Exception.h"

namespace hinode
{
	namespace graphics
	{
		/// @brief DX12Fenceを内部に持つクラス
		class DX12Fence::_impl : private ImplInterfacePointer<DX12Fence>
		{
		public:
			UNCOPYABLE_IMPL_MOVE_CONSTRUCTOR(DX12Fence);

			ID3D12Fence* mpFence;
			HANDLE mHandle;

			_impl(DX12Fence* pInterface)
				: ImplInterfacePointer(pInterface)
				, mpFence(nullptr)
				, mHandle(nullptr)
			{}

			_impl& operator=(_impl&& right)_noexcept
			{
				this->mpFence = right.mpFence;
				this->mHandle = right.mHandle;
				right.mpFence = nullptr;
				right.mHandle = nullptr;
				return *this;
			}

			~_impl()_noexcept
			{
				this->clear();
			}

			void clear()_noexcept
			{
				safeRelease(&this->mpFence);
				if (this->mHandle) {
					CloseHandle(this->mHandle);
					this->mHandle = nullptr;
				}
			}

			void create(ID3D12Device* pDevice, UINT64 initValue, D3D12_FENCE_FLAGS flag)
			{
				this->clear();
				bool isOK = false;
				Finally fin([&]() {
					unless(isOK) {
						this->clear();
					}
				});

				if (FAILED(pDevice->CreateFence(initValue, flag, IID_PPV_ARGS(&this->mpFence)))) {
					throw HINODE_GRAPHICS_CREATE_EXCEPTION(InvalidArgumentsException, "DX12Fence", "create")
						<< "DX12Fenceの作成に失敗しました";
				}
				this->mHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
				if (nullptr == this->mHandle || FAILED(HRESULT_FROM_WIN32(GetLastError()))) {
					throw HINODE_GRAPHICS_CREATE_EXCEPTION(InvalidArgumentsException, "DX12Fence", "create")
						<< "DX12Fenceのイベントハンドル作成に失敗しました";
				}
				isOK = true;
 			}

			HRESULT wait(UINT64 value, DWORD waitMilliseconds = INFINITE)noexcept
			{
				assert(nullptr != this->mpFence || "まだ初期化されていません");
				auto hr = this->mpFence->SetEventOnCompletion(value, this->mHandle);
				if (FAILED(hr)) {
					HINODE_GRAPHICS_ERROR_LOG("DX12Fence", "waitForCompletion")
						<< "イベント設定に失敗しました。";
					return hr;
				}
				WaitForSingleObject(this->mHandle, waitMilliseconds);
				return S_OK;
			}

			HRESULT setName(const wchar_t* name) _noexcept
			{
				assert(nullptr != this->mpFence || "まだ初期化されていません");
				return this->mpFence->SetName(name);
			}

			bool isGood()const noexcept
			{
				return nullptr != this->mpFence;
			}

		accessor_declaration:
			ID3D12Fence* operator->()_noexcept
			{
				assert(nullptr != this->mpFence || "まだ初期化されていません");
				return this->fence();
			}

			ID3D12Fence* fence()_noexcept
			{
				assert(nullptr != this->mpFence || "まだ初期化されていません");
				return this->mpFence;
			}
		};

		bool DX12Fence::sSignalAndWait(DX12Fence& fence, ID3D12CommandQueue* pCmdQueue)noexcept
		{
			assert(fence.isGood() && nullptr != pCmdQueue);

			const auto value = 1 + fence->GetCompletedValue();
			auto hr = pCmdQueue->Signal(fence.fence(), value);
			if (FAILED(hr)) {
				HINODE_GRAPHICS_ERROR_LOG("DX12Fence", "sSignalAndWait") << "シグナルに失敗。";
				return false;
			}
			if (fence->GetCompletedValue() < value) {
				fence.wait(value);
			}
			return true;
		}

		//===========================================================================
		//
		//	IMPLイディオムのインターフェイスクラスの関数定義
		//

		UNCOPYABLE_PIMPL_IDIOM_CPP_TEMPLATE(DX12Fence);

		void DX12Fence::clear()_noexcept
		{
			this->impl().clear();
		}

		void DX12Fence::create(ID3D12Device* pDevice, UINT64 initValue, D3D12_FENCE_FLAGS flag)
		{
			this->impl().create(pDevice, initValue, flag);
		}

		HRESULT DX12Fence::wait(UINT64 value, DWORD waitMilliseconds)noexcept
		{
			return this->impl().wait(value, waitMilliseconds);
		}

		HRESULT DX12Fence::setName(const wchar_t* name) _noexcept
		{
			return this->impl().setName(name);
		}

		bool DX12Fence::isGood()const noexcept
		{
			return this->impl().isGood();
		}

		ID3D12Fence* DX12Fence::operator->()_noexcept
		{
			return this->impl().fence();
		}

		ID3D12Fence* DX12Fence::fence()_noexcept
		{
			return this->impl().fence();
		}
	}
}