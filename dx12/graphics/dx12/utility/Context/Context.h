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

				/// @brief コマンドの記録を開始する
				///
				///	内部でアロケータのリセットを行っていますが、同期判定は行っていないので自前で同期処理を行ってくください
				/// @param[in] pInitalState
				/// @retval bool 
				bool beginRecord(ID3D12PipelineState* pInitalState)noexcept;

				/// @brief 現在のコマンドリストの内容をGPUに発行し、もう一つののコマンドリストをターゲットにする
				///
				/// もう一つのコマンドリストの内容がまだ実行中の場合は、それが完了するまで待ちます。
				/// @param[in] cmdQueue
				/// @param[in] pInitialState
				/// @retval bool
				bool executeAndSwap(DX12CommandQueue& cmdQueue, ID3D12PipelineState* pInitialState)noexcept;

				/// @brief コマンドの記録を終了する
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