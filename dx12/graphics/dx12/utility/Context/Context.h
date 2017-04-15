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
			/// @brief アロケータとコマンドリストをまとめたもの
			class Context
			{
			public:
				Context();
				~Context();

				/// @brief　メモリ解放
				void clear();

				/// @brief 作成
				/// @param[in] pDevice
				/// @param[in] type
				/// @param[in] pPipelineState default=nullptr
				/// @param[in] nodeMask default=0u
				void create(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type, ID3D12PipelineState* pPipeline = nullptr, UINT nodeMask = 0u);

				/// @brief アロケータをリセットする
				void resetAllocator();

				/// @brief 内部に持つコマンドアロケータを使用して、コマンドを記録します
				///
				/// この関数の初めにアロケータとコマンドリストの両方をリセットし、
				/// 終わりにはコマンドリストをクローズします。
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