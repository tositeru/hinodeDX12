#pragma once

#include "../IDX12Interface.h"
#include "../common/Common.h"

namespace hinode
{
	namespace graphics
	{
		class DX12Heap : public IDX12Interface
		{
			UNCOPYABLE_PIMPL_IDIOM_TEMPLATE(DX12Heap)
		public:
			/// @brief メモリ解放
			void clear();

			/// @brief 作成
			/// @param[in] pDevice
			/// @param[in] pDesc
			/// @exception hinode::graphics::FailedToCreateException
			void create(ID3D12Device* pDevice, const D3D12_HEAP_DESC* pDesc);

			/// @brief リソースの名前をつける。
			/// GPUデバッグなどで利用できます。
			HRESULT setName(const wchar_t* name) override;

			/// @brief 使用できる状態か?
			/// @retval bool
			bool isGood()const noexcept override;

		accessor_declaration:
			ID3D12Heap* heap()noexcept;
			operator ID3D12Heap* ()noexcept { return this->heap(); }
			ID3D12Heap* operator->()noexcept { return this->heap(); }
		};
	}
}