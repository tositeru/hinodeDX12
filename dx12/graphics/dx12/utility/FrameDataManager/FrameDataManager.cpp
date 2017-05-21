#include "stdafx.h"

#include "FrameDataManager.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			FrameDataManager::FrameDataManager()
				: mCurrentFrame(0)
			{}

			FrameDataManager::~FrameDataManager()
			{
				this->clear();
			}

			void FrameDataManager::clear()
			{
				if (this->mFence.isGood()) {
					this->mFence.wait(this->mCurrentFrame - 1);
					this->mFence.clear();
				}

				this->mFrames.clear();
				this->mCurrentFrame = 0;
			}

			void FrameDataManager::create(ID3D12Device* pDevice, size_t frameCount, std::function<std::unique_ptr<IData>(UINT frameIndex)> make)
			{
				this->mCurrentFrame = 0;
				this->mFence.create(pDevice, 0, D3D12_FENCE_FLAG_NONE);
				this->mFrames.resize(frameCount);
				for (UINT i = 0u; i < this->mFrames.size(); ++i) {
					this->mFrames[i].mpData = make(i);
				}
			}

			void FrameDataManager::endAndGoNextFrame(ID3D12CommandQueue* pCmdQueue)
			{
				auto currentIndex = static_cast<size_t>(this->currentFrameIndex());
				auto& frame = this->mFrames[currentIndex];

				++this->mCurrentFrame;
				frame.mFrame = this->mCurrentFrame;
				pCmdQueue->Signal(this->mFence, this->mCurrentFrame);
			}

			void FrameDataManager::waitForCurrent(DWORD waitMilliseconds)
			{
				auto currentIndex = static_cast<size_t>(this->currentFrameIndex());
				auto& frame = this->mFrames[currentIndex];
				this->mFence.wait(frame.mFrame, waitMilliseconds);
			}

			void FrameDataManager::waitPrevFrame(DWORD waitMilliseconds)
			{
				auto f = static_cast<size_t>((std::max(1uLL, this->mCurrentFrame) - 1) % this->mFrames.size());
				auto& frame = this->mFrames[f];
				this->mFence.wait(frame.mFrame, waitMilliseconds);
			}

			UINT64 FrameDataManager::currentFrameIndex()const noexcept
			{
				return this->mCurrentFrame%this->mFrames.size();
			}

			FrameDataManager::IData* FrameDataManager::currentFrame()noexcept
			{
				auto index = static_cast<size_t>(this->currentFrameIndex());
				return this->mFrames[index].mpData.get();
			}

			//
			//	FrameDataManager::IData
			//

			void FrameDataManager::IData::clear()
			{

			}
		}
	}
}