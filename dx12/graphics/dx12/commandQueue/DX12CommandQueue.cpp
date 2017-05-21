#include "stdafx.h"

#include "DX12CommandQueue.h"

#include "../common/Log.h"
#include "../common/Exception.h"

namespace hinode
{
	namespace graphics
	{
		class DX12CommandQueue::_impl : private ImplInterfacePointer<DX12CommandQueue>
		{
		public:
			UNCOPYABLE_IMPL_MOVE_CONSTRUCTOR(DX12CommandQueue);

			ID3D12CommandQueue* mpQueue;
			UINT64 mGPUTimestampFrequence;
			UINT64 mPrevGPUTimestamp;	///< 単位は[ms]

			_impl(DX12CommandQueue* pInterface)
				: ImplInterfacePointer(pInterface)
				, mpQueue(nullptr)
				, mGPUTimestampFrequence(1)
				, mPrevGPUTimestamp(0)
			{}

			_impl& operator=(_impl&& right)_noexcept
			{
				this->mpQueue = right.mpQueue;
				right.mpQueue = nullptr;
				this->mGPUTimestampFrequence = right.mGPUTimestampFrequence;
				return *this;
			}

			~_impl()_noexcept
			{
				this->clear();
			}

			void clear()_noexcept
			{
				safeRelease(&this->mpQueue);
				this->mGPUTimestampFrequence = 1;
			}

			void create(ID3D12Device* pDevice, const D3D12_COMMAND_QUEUE_DESC& desc)
			{
				this->clear();

				if (FAILED(pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&this->mpQueue)))) {
					throw HINODE_GRAPHICS_CREATE_EXCEPTION(InvalidArgumentsException, "DX12CommandQueue", "create")
						<< "ID3D12CommandQueueの作成に失敗しました";
				}

				if (D3D12_COMMAND_LIST_TYPE_DIRECT == desc.Type || D3D12_COMMAND_LIST_TYPE_COMPUTE == desc.Type) {
					this->mpQueue->GetTimestampFrequency(&this->mGPUTimestampFrequence);
				}
			}

			void executeCommandLists(std::initializer_list<ID3D12CommandList*> lists)
			{
				assert(nullptr != this->mpQueue || "まだ初期化されていません");

				this->mpQueue->ExecuteCommandLists(static_cast<UINT>(lists.size()), lists.begin());
			}

			double recordTimeStamp()
			{
				UINT64 GPUTimestamp, CPUTimestamp;
				this->mpQueue->GetClockCalibration(&GPUTimestamp, &CPUTimestamp);
				auto millisecond = (GPUTimestamp - this->mPrevGPUTimestamp) / (double)this->mGPUTimestampFrequence * 1000.0;
				this->mPrevGPUTimestamp = GPUTimestamp;
				return millisecond;
			}

			HRESULT setName(const wchar_t* name)
			{
				assert(nullptr != this->mpQueue || "まだ初期化されていません");
				return this->mpQueue->SetName(name);
			}

			bool isGood()const noexcept
			{
				return nullptr != this->mpQueue;
			}

		accessor_declaration:
			ID3D12CommandQueue* operator->()_noexcept
			{
				assert(nullptr != this->mpQueue || "まだ初期化されていません");
				return this->queue();
			}

			ID3D12CommandQueue* queue()_noexcept
			{
				assert(nullptr != this->mpQueue || "まだ初期化されていません");
				return this->mpQueue;
			}

			UINT64 GPUTimestampFrequence()const noexcept
			{
				assert(nullptr != this->mpQueue || "まだ初期化されていません");
				return this->mGPUTimestampFrequence;
			}
		};

		//===========================================================================
		//
		//	IMPLイディオムのインターフェイスクラスの関数定義
		//


		UNCOPYABLE_PIMPL_IDIOM_CPP_TEMPLATE(DX12CommandQueue);

		void DX12CommandQueue::clear()_noexcept
		{
			this->impl().clear();
		}

		void DX12CommandQueue::create(ID3D12Device* pDevice, const D3D12_COMMAND_QUEUE_DESC& desc)
		{
			this->impl().create(pDevice, desc);
		}

		void DX12CommandQueue::executeCommandLists(std::initializer_list<ID3D12CommandList*> lists)
		{
			this->impl().executeCommandLists(lists);
		}

		double DX12CommandQueue::recordTimeStamp()noexcept
		{
			return this->impl().recordTimeStamp();
		}

		HRESULT DX12CommandQueue::setName(const wchar_t* name) _noexcept
		{
			return this->impl().setName(name);
		}

		bool DX12CommandQueue::isGood()const noexcept
		{
			return this->impl().isGood();
		}

		ID3D12CommandQueue* DX12CommandQueue::operator->()_noexcept
		{
			return this->impl().queue();
		}

		ID3D12CommandQueue* DX12CommandQueue::queue()_noexcept
		{
			return this->impl().queue();
		}

		UINT64 DX12CommandQueue::GPUTimestampFrequence()const noexcept
		{
			return this->impl().GPUTimestampFrequence();
		}
	}
}