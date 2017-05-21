#pragma once

#include <vector>

#include "../../common/Common.h"
#include "../../fence/DX12Fence.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			class FrameDataManager
			{
			public:
				struct IData
				{
					virtual ~IData()
					{
						this->clear();
					};

					virtual void clear() = 0;
				};

			public:
				FrameDataManager();
				virtual ~FrameDataManager();

				virtual void clear();
				void create(ID3D12Device* pDevice, size_t frameCount, std::function<std::unique_ptr<IData>(UINT frameIndex)> make);

				void endAndGoNextFrame(ID3D12CommandQueue* pCmdQueue);
				void waitForCurrent(DWORD waitMilliseconds = INFINITE);
				void waitPrevFrame(DWORD waitMilliseconds = INFINITE);

			accessor_declaration:
				UINT64 currentFrameIndex()const noexcept;
				IData* currentFrame()noexcept;

				template<typename T>
				T* currentFrame()noexcept
				{
					auto index = static_cast<size_t>(this->currentFrameIndex());
					return dynamic_cast<T*>(this->mFrames[index].mpData.get());
				}

			private:
				UINT64 mCurrentFrame;
				DX12Fence mFence;
				struct Frame {
					UINT64 mFrame = 0u;
					std::unique_ptr<IData> mpData;
				};
				std::vector<Frame> mFrames;
			};
		}
	}
}