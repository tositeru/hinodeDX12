#pragma once

#include <vector>
#include <memory>

#include "../common/Common.h"
#include "../IDX12Interface.h"

namespace hinode
{
	namespace graphics
	{
		struct DX12DeviceDesc;

		/// @brief ID3D12Deviceを内部に持つクラス
		class DX12Device : public IDX12Interface
		{
			UNCOPYABLE_PIMPL_IDIOM_TEMPLATE(DX12Device)
		public:
			/// @brief 現在使用できるアダプターからDX12Deviceを作成する
			/// @param[out] pOut
			/// @param[inout] pDesc
			/// @param[in] pDXGIFactory
			/// @param[in] isIgnoreSoftware ソフトウェアアダプターを無視するか？ default=true
			/// @exception hinode::graphics::InvalidArgumentsException 作成に失敗した時に投げます
			static void sCreateDevice(DX12Device* pOut, DX12DeviceDesc* pDesc, IDXGIFactory4* pDXGIFactory, bool isIgnoreSoftware = true);

		public:
			/// @brief メモリ開放
			void clear()_noexcept;

			/// @brief デバイスを作成する
			///
			/// 作成に成功した場合はdesc.pAdapterを内部で保持ちます。
			/// @param[in] desc
			/// @exception std::invalid_argument
			void create(DX12DeviceDesc& desc);

			/// @brief 内部のID3D12Deviceに名前をつける
			/// @param[in] const wchar_t* name
			/// @retval HRESULT
			HRESULT setName(const wchar_t* name) _noexcept override;

			/// @brief 使用できる状態か?
			/// @retval bool
			bool isGood()const noexcept override;

		accessor_declaration:
			ID3D12Device* operator->()_noexcept;

			ID3D12Device* device()_noexcept;
			operator ID3D12Device*()_noexcept { return this->device(); }

			IDXGIAdapter1* adapter()_noexcept;
			HRESULT getDebugDevice(ID3D12DebugDevice** ppOut)_noexcept;
		};
	}

	namespace graphics
	{
		struct DX12DeviceDesc
		{
			Microsoft::WRL::ComPtr<IDXGIAdapter1> pAdapter;
			D3D_FEATURE_LEVEL minimumLevel;
		};
	}
}