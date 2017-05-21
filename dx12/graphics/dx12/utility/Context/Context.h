#pragma once

#include "../../commandQueue\DX12CommandQueue.h"
#include "../../commandAllocator\DX12CommandAllocator.h"
#include "../../graphicsCommandList/DX12GraphicsCommandList.h"
#include "../../pipelineState\DX12PipelineState.h"
#include "../FrameDataManager/FrameDataManager.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			/// @brief �A���P�[�^�ƃR�}���h���X�g���܂Ƃ߂�����
			class Context
			{
			public:
				Context();
				~Context();

				/// @brief�@���������
				void clear();

				/// @brief �쐬
				/// @param[in] pDevice
				/// @param[in] type
				/// @param[in] pPipelineState default=nullptr
				/// @param[in] nodeMask default=0u
				void create(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type, ID3D12PipelineState* pPipeline = nullptr, UINT nodeMask = 0u);

				/// @brief �A���P�[�^�����Z�b�g����
				void resetAllocator();

				/// @brief �����Ɏ��R�}���h�A���P�[�^���g�p���āA�R�}���h���L�^���܂�
				///
				/// ���̊֐��̏��߂ɃA���P�[�^�ƃR�}���h���X�g�̗��������Z�b�g���A
				/// �I���ɂ̓R�}���h���X�g���N���[�Y���܂��B
				/// @param[in] pState
				/// @param[in] pred
				void record(ID3D12PipelineState* pState, std::function<void(DX12GraphicsCommandList& cmdList)> pred);

				/// @brief �R�}���h�̋L�^���J�n����
				///
				///	�����ŃA���P�[�^�̃��Z�b�g���s���Ă��܂����A��������͍s���Ă��Ȃ��̂Ŏ��O�œ����������s���Ă���������
				/// @param[in] pInitalState
				/// @retval bool 
				bool beginRecord(ID3D12PipelineState* pInitalState)noexcept;

				/// @brief ���݂̃R�}���h���X�g�̓��e��GPU�ɔ��s���A������̂̃R�}���h���X�g���^�[�Q�b�g�ɂ���
				///
				/// ������̃R�}���h���X�g�̓��e���܂����s���̏ꍇ�́A���ꂪ��������܂ő҂��܂��B
				/// @param[in] cmdQueue
				/// @param[in] pInitialState
				/// @retval bool
				bool executeAndSwap(DX12CommandQueue& cmdQueue, ID3D12PipelineState* pInitialState)noexcept;

				/// @brief �R�}���h�̋L�^���I������
				///
				/// @param[in] cmdQueue
				/// @retval bool
				bool endRecord(DX12CommandQueue& cmdQueue)noexcept;

				DX12GraphicsCommandList* currentCmdListPointer()noexcept;

			accessor_declaration:
				DX12CommandAllocator& allocator()noexcept;
				DX12GraphicsCommandList& cmdList()noexcept;

			private:
				DX12CommandAllocator mCmdAllocator;
				struct FrameData : public FrameDataManager::IData {
					DX12GraphicsCommandList mCmdList;

					void clear() override{
						this->mCmdList.clear();
					}
				};
				FrameDataManager mCmdLists;
				DX12Fence mFence;
			};
		}
	}
}