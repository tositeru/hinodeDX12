#pragma once

#include <vector>
#include <memory>

#include "../common/Common.h"
#include "../IDX12Interface.h"

namespace hinode
{
	namespace graphics
	{
		/// @brief ID3D12CommandQueueを内部に持つクラス
		class DX12CommandQueue : public IDX12Interface
		{
			UNCOPYABLE_PIMPL_IDIOM_TEMPLATE(DX12CommandQueue)
		public:
			/// @brief メモリ開放
			void clear()_noexcept;

			/// @brief コマンドキューを作成する
			/// @param[in] pDevice
			/// @param[in] desc
			/// @exception hinode::graphics::InvalidArgumentsException
			void create(ID3D12Device* pDevice, const D3D12_COMMAND_QUEUE_DESC& desc);

			/// @brief コマンドリストを実行する
			/// @param[in] lists
			void executeCommandLists(std::initializer_list<ID3D12CommandList*> lists);

			/// @brief 内部のDX12CommandQueueに名前をつける
			/// @param[in] const wchar_t* name
			/// @retval HRESULT
			HRESULT setName(const wchar_t* name) _noexcept override;

			/// @brief 使用できる状態か?
			/// @retval bool
			bool isGood()const noexcept override;

			/// @brief タイムスタンプを取る
			///
			/// 戻り値に前回のrecordTimeStamp()からの経過時間を返します
			/// @retval double
			double recordTimeStamp()noexcept;

		accessor_declaration:
			ID3D12CommandQueue* operator->()_noexcept;

			ID3D12CommandQueue* queue()_noexcept;
			operator ID3D12CommandQueue*()_noexcept { return this->queue(); }

			UINT64 GPUTimestampFrequence()const noexcept;
			double prevProcessingTime()const noexcept;
		};
	}
}