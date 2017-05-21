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
				this->mCmdLists.clear();
				this->mCmdAllocator.clear();
			}

			void Context::create(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type, ID3D12PipelineState* pPipeline, UINT nodeMask)
			{
				this->clear();

				this->mCmdAllocator.create(pDevice, type);

				this->mCmdLists.create(pDevice, 2, [&](UINT frameIndex) {
					auto result = std::make_unique<FrameData>();
					DX12GraphicsCommandListDesc desc(nodeMask, type, this->mCmdAllocator, pPipeline);
					result->mCmdList.create(pDevice, desc);
					return result;
				});
			}

			void Context::record(ID3D12PipelineState* pState, std::function<void(DX12GraphicsCommandList& cmdList)> pred)
			{
				this->resetAllocator();
				auto pFrameData = dynamic_cast<FrameData*>(this->mCmdLists.currentFrame());
				pFrameData->mCmdList.record(this->mCmdAllocator, pState, pred);
			}

			void Context::resetAllocator()
			{
				if (FAILED(this->mCmdAllocator->Reset())) {
					hinode::graphics::Log(hinode::graphics::Log::eINFO) << "コマンドアロケータのリセットに失敗";
					return;
				}
			}

			bool Context::beginRecord(ID3D12PipelineState* pInitialState)noexcept
			{
				this->resetAllocator();
				this->mCmdLists.waitForCurrent();
				auto& cmdList = *this->currentCmdListPointer();
				return cmdList.reset(this->mCmdAllocator, pInitialState);
			}

			bool Context::executeAndSwap(DX12CommandQueue& cmdQueue, ID3D12PipelineState* pInitialState)noexcept
			{
				{//実行する側のコマンドリスト
					auto& cmdList = *this->currentCmdListPointer();
					unless(cmdList.close()) {
						return false;
					}
					cmdQueue.executeCommandLists({ cmdList });
					this->mCmdLists.endAndGoNextFrame(cmdQueue);
				}

				{//次のコマンドリスト
					this->mCmdLists.waitForCurrent();
					auto& cmdList = *this->currentCmdListPointer();
					return cmdList.reset(this->mCmdAllocator, pInitialState);
				}
			}

			bool Context::endRecord(DX12CommandQueue& cmdQueue)noexcept
			{
				auto& cmdList = *this->currentCmdListPointer();
				unless(cmdList.close()) {
					return false;
				}
				cmdQueue.executeCommandLists({ cmdList });
				this->mCmdLists.endAndGoNextFrame(cmdQueue);
				return true;
			}

			DX12GraphicsCommandList* Context::currentCmdListPointer()noexcept
			{
				auto pFrameData = dynamic_cast<FrameData*>(this->mCmdLists.currentFrame());
				return &pFrameData->mCmdList;
			}

			DX12CommandAllocator& Context::allocator()noexcept
			{
				return this->mCmdAllocator;
			}

			DX12GraphicsCommandList& Context::cmdList()noexcept
			{
				auto pFrameData = dynamic_cast<FrameData*>(this->mCmdLists.currentFrame());
				return pFrameData->mCmdList;
			}

		}
	}
}