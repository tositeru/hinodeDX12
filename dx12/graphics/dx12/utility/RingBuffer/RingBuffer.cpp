#include "stdafx.h"

#include "RingBuffer.h"

#include "../../commandQueue/DX12CommandQueue.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			RingBuffer::RingBuffer()
				: mBufferSize(0)
				, mCurrentUsedByteSize(0)
				, mCurrentOffset(0)
				, mFrameCounter(0)
			{ }

			RingBuffer::~RingBuffer()
			{
				this->clear();
			}

			void RingBuffer::clear()
			{
				if (2 <= this->mFrameQueue.size()) {
					auto tailFrame = this->mFrameQueue.back().mFrame - 1;
					this->mFence.wait(tailFrame);
				}

				this->mBufferSize = 0;
				this->mCurrentOffset = 0;
				this->mCurrentUsedByteSize = 0;
				this->mFrameCounter = 0;
				while (!this->mFrameQueue.empty()) {
					this->mFrameQueue.pop();
				}
				this->mFence.clear();
			}

			void RingBuffer::init(ID3D12Device* pDevice, UINT64 bufferSize)
			{
				RingBuffer::clear();

				this->mFence.create(pDevice, 0, D3D12_FENCE_FLAG_NONE);
				this->mBufferSize = bufferSize;

				this->pushFrameInfo();
			}

			UINT64 RingBuffer::updateCurrentOffset(uint32_t allocateByteSize)
			{
				bool isTail = (this->mBufferSize <= this->mCurrentOffset + allocateByteSize);
				if (isTail) {
					this->mCurrentUsedByteSize += this->mBufferSize - this->mCurrentOffset;
					this->mCurrentOffset = 0;
				}

				bool isFill = true;
				while (!this->mFrameQueue.empty()) {
					auto& oldestFrameInfo = this->mFrameQueue.front();
					isFill = this->mBufferSize < this->mCurrentUsedByteSize + allocateByteSize;
					if (isFill) {
						if (1 < this->mFrameQueue.size()) {
							//確保スペースが無かったら、前のフレームが終了するまで待つ
							this->mFence.wait(oldestFrameInfo.mFrame);
							auto oldestFrameHead = oldestFrameInfo.mOffset;
							this->mFrameQueue.pop();
							auto& nextFrame = this->mFrameQueue.front();
							UINT64 freeByteSize = 0;
							if (nextFrame.mOffset < oldestFrameHead) {
								//次のフレームまで、バッファの末尾を経由していたときの処理
								freeByteSize = this->mBufferSize - oldestFrameHead;
								freeByteSize += nextFrame.mOffset;
							} else {
								freeByteSize = nextFrame.mOffset - oldestFrameHead;
							}
							this->mCurrentUsedByteSize -= freeByteSize;
						} else {
							break;//キューに要素が1つしかなかったら、その要素は現在のフレームなので残す
						}
					} else {
						break;
					}
				}

				if (isFill) {
					//TODO もし、許容量を超えてしまったらstd::vectorのようにバッファを確保し直す?
					throw HINODE_GRAPHICS_CREATE_EXCEPTION(RunTimeErrorException, "RingBuffer", "updateCurrentOffset")
						<< "リングバッファの容量を超えてメモリを割当しようとしました。 割り当て許容量 = " << this->mBufferSize;
				}

				auto result = this->mCurrentOffset;
				this->mCurrentOffset += allocateByteSize;
				this->mCurrentUsedByteSize += allocateByteSize;
				return result;
			}

			void RingBuffer::endAndGoNextFrame(DX12CommandQueue& queue)
			{
				if (this->mFrameQueue.back().mOffset != this->mCurrentOffset) {
					queue->Signal(this->mFence, this->mFrameCounter);
					this->pushFrameInfo();
				}
			}

			UINT64 RingBuffer::currentOffset()const noexcept
			{
				return this->mCurrentOffset;
			}

			UINT64 RingBuffer::currentFrame()const noexcept
			{
				return this->mFrameCounter;
			}

			void RingBuffer::pushFrameInfo()noexcept
			{
				++this->mFrameCounter;
				FrameInfo frameInfo;
				frameInfo.mFrame = this->mFrameCounter;
				frameInfo.mOffset = this->mCurrentOffset;
				this->mFrameQueue.push(frameInfo);
			}

		}
	}
}