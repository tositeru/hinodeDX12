#pragma once

#include "../../commandQueue\DX12CommandQueue.h"
#include "../../commandAllocator\DX12CommandAllocator.h"
#include "../../graphicsCommandList/DX12GraphicsCommandList.h"
#include "../../pipelineState\DX12PipelineState.h"

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

			accessor_declaration:
				DX12CommandAllocator& allocator()noexcept;
				DX12GraphicsCommandList& cmdList()noexcept;

			private:
				DX12CommandAllocator mCmdAllocator;
				DX12GraphicsCommandList mCmdList;
			};
		}
	}
}