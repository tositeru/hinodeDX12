#pragma once

#include "../common/Common.h"
#include "../IDX12Interface.h"

namespace hinode
{
	namespace graphics
	{
		class DX12QueryHeap : IDX12Interface
		{
			UNCOPYABLE_PIMPL_IDIOM_TEMPLATE(DX12QueryHeap)
		public:
			void clear()noexcept;

			/// @brief クエリヒープの作成
			/// @param[in] pDevice
			/// @param[in] pDesc
			/// @exception hinode::graphics::InvalidArgumentsException
			void create(ID3D12Device* pDevice, D3D12_QUERY_HEAP_DESC* pDesc);

			/// @brief 内部のDX12CommandQueueに名前をつける
			/// @param[in] const wchar_t* name
			/// @retval HRESULT
			HRESULT setName(const wchar_t* name) noexcept override;

			/// @brief 使用できる状態か?
			/// @retval bool
			bool isGood()const noexcept override;

		accessor_declaration:
			ID3D12QueryHeap* operator->()noexcept;

			ID3D12QueryHeap* queryHeap()noexcept;
			operator ID3D12QueryHeap*()noexcept { return this->queryHeap(); }

		};
	}
}