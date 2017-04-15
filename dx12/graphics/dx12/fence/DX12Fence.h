#pragma once

#include "../common/Common.h"
#include "../IDX12Interface.h"

namespace hinode
{
	namespace graphics
	{
		/// @brief DX12GraphicsCommandListを内部に持つクラス
		class DX12Fence : public IDX12Interface
		{
			UNCOPYABLE_PIMPL_IDIOM_TEMPLATE(DX12Fence)
		public:
			static bool sSignalAndWait(DX12Fence& fence, ID3D12CommandQueue* pCmdQueue)noexcept;

		public:
			/// @brief メモリ開放
			void clear()_noexcept;

			/// @brief フェンスを作成する
			///
			/// @param[in] pDevice
			/// @param[in] initValue
			/// @param[in] flag
			/// @exception hinode::graphics::InvalidArgumentsException
			void create(ID3D12Device* pDevice, UINT64 initValue, D3D12_FENCE_FLAGS flag);

			/// @brief フェンスがvalueの値になるまで待つ
			/// @param[in] value
			/// @param[in] waitMilliseconds default=INFINITE
			/// @retval HRESULT
			HRESULT wait(UINT64 value, DWORD waitMilliseconds = INFINITE)noexcept;

			/// @brief 内部のDX12CommandQueueに名前をつける
			/// @param[in] const wchar_t* name
			/// @retval HRESULT
			HRESULT setName(const wchar_t* name) _noexcept override;

			/// @brief 使用できる状態か?
			/// @retval bool
			bool isGood()const noexcept override;

		accessor_declaration:
			ID3D12Fence* operator->()_noexcept;

			ID3D12Fence* fence()_noexcept;
			operator ID3D12Fence*()_noexcept { return this->fence(); }
		};
	}
}
