#pragma once

#include <vector>
#include <memory>

#include "../common/Common.h"
#include "../IDX12Interface.h"

namespace hinode
{
	namespace graphics
	{
		/// @brief ID3D12CommandAllocatorを内部に持つクラス
		class DX12CommandAllocator : public IDX12Interface
		{
			UNCOPYABLE_PIMPL_IDIOM_TEMPLATE(DX12CommandAllocator)
		public:
			/// @brief メモリ開放
			void clear()_noexcept;

			/// @brief コマンドアロケータを作成する
			/// @param[in] pDevice
			/// @param[in] type
			/// @exception hinode::graphics::InvalidArgumentsException
			void create(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type);

			/// @brief 内部のID3D12CommandAllocatorに名前をつける
			/// @param[in] const wchar_t* name
			/// @retval HRESULT
			HRESULT setName(const wchar_t* name) _noexcept override;

			/// @brief 使用できる状態か?
			/// @retval bool
			bool isGood()const noexcept override;

		accessor_declaration:
			ID3D12CommandAllocator* operator->()_noexcept;

			ID3D12CommandAllocator* allocator()_noexcept;
			operator ID3D12CommandAllocator*()_noexcept { return this->allocator(); }
		};
	}
}