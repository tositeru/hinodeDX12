#pragma once

#include "../common/Common.h"
#include "../IDX12Interface.h"

namespace hinode
{
	namespace graphics
	{
		/// @brief ID3D12RootSignatureを内部に持つクラス
		class DX12RootSignature : public IDX12Interface
		{
			UNCOPYABLE_PIMPL_IDIOM_TEMPLATE(DX12RootSignature)
		public:
			/// @brief D3D12_ROOT_SIGNATURE_DESC*からDX12RootSignatureを作成する
			/// @param[out] pOut
			/// @param[in] pDevice
			/// @param[in] pDesc
			/// @param[in] version default=D3D_ROOT_SIGNATURE_VERSION_1
			/// @param[in] nodeMask default=0
			/// @exception hinode::graphics::InvalidArgumentsException
			static void sCreate(DX12RootSignature* pOut, ID3D12Device* pDevice, const D3D12_ROOT_SIGNATURE_DESC* pDesc, D3D_ROOT_SIGNATURE_VERSION version = D3D_ROOT_SIGNATURE_VERSION_1, UINT nodeMask=0);

			/// @brief ファイルからルートシグネチャを作成する
			/// @param[out] pOut
			/// @param[in] pDevice
			/// @param[in] filepath
			/// @param[in] version
			/// @param[out] ppOutBlob
			/// @param[in] nodeMask default=0
			/// @param[in] pInclude
			/// @exception hinode::graphics::InvalidArgumentsException
			static void sLoad(DX12RootSignature* pOut, ID3D12Device* pDevice, const std::wstring filepath, const std::string& entryPoint, const std::string& version, ID3DBlob** ppOutBlob=nullptr, UINT nodeMask=0, ID3DInclude* pInclude=nullptr);

			/// @brief 渡されたものの文字列を返す
			/// @param[in] type
			/// @retval const std::string&
			static const std::string& sToStr(D3D12_DESCRIPTOR_RANGE_TYPE type)noexcept;

			/// @brief 渡されたもの文字列を返す
			/// @param[in] type
			/// @retval const std::string&
			static const std::string& sToStr(D3D_SHADER_INPUT_TYPE type)noexcept;
		public:
			/// @brief メモリ開放
			void clear()_noexcept;

			/// @brief ルートシグネチャを作成する
			///
			/// @param[in] pDevice
			/// @param[in] nodeMask
			/// @param[in] pSignature
			/// @exception hinode::graphics::InvalidArgumentsException
			void create(ID3D12Device* pDevice, UINT nodeMask, ID3DBlob* pSignature);

			/// @brief ルートシグネチャを作成する
			///
			/// @param[in] pDevice
			/// @param[in] nodeMask
			/// @param[in] pBytecode
			/// @exception hinode::graphics::InvalidArgumentsException
			void create(ID3D12Device* pDevice, UINT nodeMask, D3D12_SHADER_BYTECODE& bytecode);

			/// @brief 作成済みのシグナチャーを設定する
			/// @param[in] pSignature
			void set(ID3D12RootSignature* pSignature);

			/// @brief 内部のDX12CommandQueueに名前をつける
			/// @param[in] const wchar_t* name
			/// @retval HRESULT
			HRESULT setName(const wchar_t* name) _noexcept override;

			/// @brief 使用できる状態か?
			/// @retval bool
			bool isGood()const noexcept override;

		accessor_declaration:
			ID3D12RootSignature* operator->()_noexcept;

			ID3D12RootSignature* rootSignature()_noexcept;
			operator ID3D12RootSignature* ()_noexcept{ return this->rootSignature(); }
		};
	}
}
