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
			/// @brief �����O�o�b�t�@�ɕK�v�ȋ@�\��p�ӂ����N���X
			/// ���̃N���X�͌p������邱�Ƃ�z�肵�Ă��܂��B
			class RingBuffer
			{
			public:
				RingBuffer();
				virtual ~RingBuffer();

				/// @brief �N���A
				virtual void clear();

				/// @brief ���̃t���[���p�̃o�b�t�@�����蓖�Ďn�߂�
				/// @param[in] queue
				void endAndGoNextFrame(DX12CommandQueue& queue);

			protected:
				/// @brief ������
				/// @param pDeivce
				/// @param bufferSize
				void init(ID3D12Device* pDevice, UINT64 bufferSize);

				/// @brief 
				/// @param[in] allocateSize
				/// @retval UINT64 �X�V��̃I�t�Z�b�g��Ԃ��܂�
				/// @exception hinode::graphics::RunTimeErrorException
				UINT64 updateCurrentOffset(uint32_t allocateSize);

				/// @brief ���݂̃I�t�Z�b�g��Ԃ�
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