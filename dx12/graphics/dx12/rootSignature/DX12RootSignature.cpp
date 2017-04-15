#include "stdafx.h"

#include "DX12RootSignature.h"

#include <d3dcompiler.h>

#include "../common/Log.h"
#include "../common/Exception.h"

#pragma comment(lib, "d3dcompiler.lib")

namespace hinode
{
	namespace graphics
	{
		/// @brief DX12RootSignatureを内部に持つクラス
		class DX12RootSignature::_impl : private ImplInterfacePointer<DX12RootSignature>
		{
		public:
			UNCOPYABLE_IMPL_MOVE_CONSTRUCTOR(DX12RootSignature);

			ID3D12RootSignature* mpRootSignature;

			_impl(DX12RootSignature* pInterface)
				: ImplInterfacePointer(pInterface)
				, mpRootSignature(nullptr)
			{}

			_impl& operator=(_impl&& right)_noexcept
			{
				this->mpRootSignature = right.mpRootSignature;
				right.mpRootSignature = nullptr;
				return *this;
			}

			~_impl()_noexcept
			{
				this->clear();
			}

			void clear()_noexcept
			{
				safeRelease(&this->mpRootSignature);
			}

			void create(ID3D12Device* pDevice, UINT nodeMask, ID3DBlob* pSignature)
			{
				D3D12_SHADER_BYTECODE bytecode;
				bytecode.pShaderBytecode = pSignature->GetBufferPointer();
				bytecode.BytecodeLength = pSignature->GetBufferSize();
				this->create(pDevice, nodeMask, bytecode);
			}

			void create(ID3D12Device* pDevice, UINT nodeMask, D3D12_SHADER_BYTECODE& bytecode)
			{
				this->clear();
				assert(nullptr != bytecode.pShaderBytecode && "pBytecode->pShaderBytecodeがnullptrです。");

				if (FAILED(pDevice->CreateRootSignature(nodeMask, bytecode.pShaderBytecode, bytecode.BytecodeLength, IID_PPV_ARGS(&this->mpRootSignature)))) {
					throw HINODE_GRAPHICS_CREATE_EXCEPTION(InvalidArgumentsException, "DX12RootSignature", "create")
						<< "DX12RootSignatureの作成に失敗しました";
				}
			}

			void set(ID3D12RootSignature* pSignature)
			{
				assert(nullptr != pSignature);
				this->clear();

				this->mpRootSignature = pSignature;
			}

			HRESULT setName(const wchar_t* name) _noexcept
			{
				assert(nullptr != this->mpRootSignature || "まだ初期化されていません");
				return this->mpRootSignature->SetName(name);
			}

			bool isGood()const noexcept
			{
				return nullptr != this->mpRootSignature;
			}

		accessor_declaration:
			ID3D12RootSignature* operator->()_noexcept
			{
				assert(nullptr != this->mpRootSignature || "まだ初期化されていません");
				return this->rootSignature();
			}

			ID3D12RootSignature* rootSignature()_noexcept
			{
				assert(nullptr != this->mpRootSignature || "まだ初期化されていません");
				return this->mpRootSignature;
			}
		};

		//===========================================================================
		//
		//	IMPLイディオムのインターフェイスクラスの関数定義
		//

		void DX12RootSignature::sCreate(DX12RootSignature* pOut, ID3D12Device* pDevice, const D3D12_ROOT_SIGNATURE_DESC* pDesc, D3D_ROOT_SIGNATURE_VERSION version, UINT nodeMask)
		{
			Microsoft::WRL::ComPtr<ID3DBlob> signature, error;
			if (FAILED(D3D12SerializeRootSignature(pDesc, version, &signature, &error))) {
				char* msg = (char*)error->GetBufferPointer();
				throw HINODE_GRAPHICS_CREATE_EXCEPTION(InvalidArgumentsException, "DX12RootSignature", "sCreate")
					<< "D3D12SerializeRootSignatureに失敗しました" << Log::eBR
					<< msg;
			}
			pOut->create(pDevice, nodeMask, signature.Get());
		}

		void DX12RootSignature::sLoad(DX12RootSignature* pOut, ID3D12Device* pDevice, const std::wstring filepath, const std::string& entryPoint, const std::string& version, ID3DBlob** ppOutBlob, UINT nodeMask, ID3DInclude* pInclude)
		{
			Microsoft::WRL::ComPtr<ID3DBlob> signature, error;
			ID3DInclude* pUseInclude = pInclude ? pInclude : D3D_COMPILE_STANDARD_FILE_INCLUDE;
			auto hr = D3DCompileFromFile(filepath.c_str(), nullptr, pUseInclude, entryPoint.c_str(), version.c_str(), 0, 0, &signature, &error);
			if (FAILED(hr) || nullptr == signature) {
				char* msg = (char*)error->GetBufferPointer();
				throw HINODE_GRAPHICS_CREATE_EXCEPTION(InvalidArgumentsException, "DX12RootSignature", "sLoad")
					<< "RootSignatureの読み込みに失敗しました" << Log::eBR
					<< msg;
			}
			if (ppOutBlob) {
				*ppOutBlob = signature.Get();
			}
			pOut->create(pDevice, nodeMask, signature.Get());
		}

		const std::string& DX12RootSignature::sToStr(D3D12_DESCRIPTOR_RANGE_TYPE type)noexcept
		{
			static const std::string tbl[] = {
				"D3D12_DESCRIPTOR_RANGE_TYPE_SRV",
				"D3D12_DESCRIPTOR_RANGE_TYPE_UAV",
				"D3D12_DESCRIPTOR_RANGE_TYPE_CBV",
				"D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER",
			};
			return tbl[type];
		}

		const std::string& DX12RootSignature::sToStr(D3D_SHADER_INPUT_TYPE type)noexcept
		{
			static const std::string tbl[] = {
				"D3D_SIT_CBUFFER",
				"D3D_SIT_TBUFFER",
				"D3D_SIT_TEXTURE",
				"D3D_SIT_SAMPLER",
				"D3D_SIT_UAV_RWTYPED",
				"D3D_SIT_STRUCTURED",
				"D3D_SIT_UAV_RWSTRUCTURED",
				"D3D_SIT_BYTEADDRESS",
				"D3D_SIT_UAV_RWBYTEADDRESS",
				"D3D_SIT_UAV_APPEND_STRUCTURED",
				"D3D_SIT_UAV_CONSUME_STRUCTURED",
				"D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER",
			};
			return tbl[type];
		}

		UNCOPYABLE_PIMPL_IDIOM_CPP_TEMPLATE(DX12RootSignature);

		void DX12RootSignature::clear()_noexcept
		{
			this->impl().clear();
		}

		void DX12RootSignature::create(ID3D12Device* pDevice, UINT nodeMask, ID3DBlob* pSignature)
		{
			this->impl().create(pDevice, nodeMask, pSignature);
		}

		void DX12RootSignature::create(ID3D12Device* pDevice, UINT nodeMask, D3D12_SHADER_BYTECODE& bytecode)
		{
			this->impl().create(pDevice, nodeMask, bytecode);
		}

		void DX12RootSignature::set(ID3D12RootSignature* pSignature)
		{
			this->impl().set(pSignature);
		}

		HRESULT DX12RootSignature::setName(const wchar_t* name) _noexcept
		{
			return this->impl().setName(name);
		}

		bool DX12RootSignature::isGood()const noexcept
		{
			return this->impl().isGood();
		}

		ID3D12RootSignature* DX12RootSignature::operator->()_noexcept
		{
			return this->impl().rootSignature();
		}

		ID3D12RootSignature* DX12RootSignature::rootSignature()_noexcept
		{
			return this->impl().rootSignature();
		}
	}
}