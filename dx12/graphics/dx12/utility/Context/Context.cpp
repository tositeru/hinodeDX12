#include "stdafx.h"

#include "Context.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			Context::Context()
			{}

			Context::~Context()
			{
				this->clear();
			}

			void Context::clear()
			{
				this->mCmdList.clear();
				this->mCmdAllocator.clear();
			}

			void Context::create(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type, ID3D12PipelineState* pPipeline, UINT nodeMask)
			{
				this->clear();

				this->mCmdAllocator.create(pDevice, type);

				DX12GraphicsCommandListDesc desc(nodeMask, type, this->mCmdAllocator, pPipeline);
				this->mCmdList.create(pDevice, desc);
			}

			void Context::record(ID3D12PipelineState* pState, std::function<void(DX12GraphicsCommandList& cmdList)> pred)
			{
				this->resetAllocator();
				this->mCmdList.record(this->mCmdAllocator, pState, pred);
			}

			void Context::resetAllocator()
			{
				if (FAILED(this->mCmdAllocator->Reset())) {
					hinode::graphics::Log(hinode::graphics::Log::eINFO) << "コマンドアロケータのリセットに失敗";
					return;
				}
			}

			DX12CommandAllocator& Context::allocator()noexcept
			{
				return this->mCmdAllocator;
			}

			DX12GraphicsCommandList& Context::cmdList()noexcept
			{
				return this->mCmdList;
			}

		}
	}
}