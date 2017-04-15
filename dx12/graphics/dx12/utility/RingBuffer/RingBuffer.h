#pragma once

#include <queue>

#include "../../fence/DX12Fence.h"

namespace hinode
{
	namespace graphics
	{
		class DX12CommandQueue;

		namespace utility
		{
			/// @brief リングバッファに必要な機能を用意したクラス
			/// このクラスは継承されることを想定しています。
			class RingBuffer
			{
			public:
				RingBuffer();
				virtual ~RingBuffer();

				/// @brief クリア
				virtual void clear();

				/// @brief 次のフレーム用のバッファを割り当て始める
				/// @param[in] queue
				void endAndGoNextFrame(DX12CommandQueue& queue);

			protected:
				/// @brief 初期化
				/// @param pDeivce
				/// @param bufferSize
				void init(ID3D12Device* pDevice, UINT64 bufferSize);

				/// @brief 
				/// @param[in] allocateSize
				/// @retval UINT64 更新後のオフセットを返します
				/// @exception hinode::graphics::RunTimeErrorException
				UINT64 updateCurrentOffset(uint32_t allocateSize);

				/// @brief 現在のオフセットを返す
				/// @retval UINT64
				UINT64 currentOffset()const noexcept;

				UINT64 currentFrame()const noexcept;

			private:
				void pushFrameInfo()noexcept;

			private:
				UINT64 mBufferSize;
				UINT64 mCurrentOffset;
				UINT64 mCurrentUsedByteSize;

				struct FrameInfo
				{
					UINT64 mFrame;
					UINT64 mOffset;
				};
				UINT64 mFrameCounter;
				DX12Fence mFence;
				std::queue<FrameInfo> mFrameQueue;
			};

		}
	}
}