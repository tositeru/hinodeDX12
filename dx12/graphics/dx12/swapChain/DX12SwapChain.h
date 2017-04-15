#pragma once

#include <vector>
#include <memory>

#include "../common/Common.h"
#include "../IDX12Interface.h"

namespace hinode
{
	namespace graphics
	{
		class DX12CommandQueue;

		/// @brief IDXGISwapChain3を内部に持つクラス
		class DX12SwapChain : public IDX12Interface
		{
			UNCOPYABLE_PIMPL_IDIOM_TEMPLATE(DX12SwapChain)
		public:
			/// @brief DXGI_SWAP_CHAIN_DESCを生成する
			/// @param[in] frameCount
			/// @param[in] width
			/// @param[in] height
			/// @param[in] hWnd
			/// @param[in] isWindow
			/// @retval DXGI_SWAP_CHAIN_DESC
			static DXGI_SWAP_CHAIN_DESC sMakeDesc(UINT frameCount, UINT width, UINT height, HWND hWnd, bool isWindow);

		public:
			/// @brief メモリ開放
			void clear()noexcept;

			/// @brief スワップチェインを作成する
			///
			/// @param[in] pQueue
			/// @param[in] pDXGIFactory
			/// @param[in] desc
			/// @exception hinode::graphics::InvalidArgumentsException
			void create(ID3D12CommandQueue* pQueue, IDXGIFactory4* pDXGIFactory, DXGI_SWAP_CHAIN_DESC& desc);

			/// @brief 何もしない関数
			/// @param[in] const wchar_t* name
			/// @retval HRESULT S_OKを必ず返します
			HRESULT setName(const wchar_t* name) noexcept override;

			/// @brief 使用できる状態か?
			/// @retval bool
			bool isGood()const noexcept override;

		accessor_declaration:
			IDXGISwapChain3* operator->()noexcept;
			IDXGISwapChain3* swapChain()noexcept;
			operator IDXGISwapChain3* ()noexcept { return this->swapChain(); }
			UINT bufferCount()const noexcept;
			UINT width()const noexcept;
			UINT height()const noexcept;
		};
	}
}