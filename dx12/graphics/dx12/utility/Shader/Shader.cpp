#include "stdafx.h"

#include "Shader.h"

#include <wrl\client.h>
#include <d3dcompiler.h>

#include "..\..\common\Exception.h"
#include "Includer\Includer.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			Shader::~Shader()
			{
				this->clear();
			}

			void Shader::clear()
			{
				if (this->mpBlob) {
					this->mpBlob->Release();
					this->mpBlob = nullptr;
				}
				this->mBytecode.pShaderBytecode = nullptr;
				this->mBytecode.BytecodeLength = 0;
			}

			void Shader::load(const std::wstring& filepath, const CompileParam& param)
			{
				this->clear();

				UINT compileFlags = param.flags;
				std::vector<D3D_SHADER_MACRO> macros = param.pMacros;
				macros.push_back(D3D_SHADER_MACRO{ "HLSL", "1" });
				macros.push_back(D3D_SHADER_MACRO{ nullptr, nullptr });

#ifdef _DEBUG
				compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
				Microsoft::WRL::ComPtr<ID3DBlob> pError;
				Includer defaultIncluder(filepath);
				ID3DInclude* pUseInclude = param.pInclude ? param.pInclude : &defaultIncluder; //  D3D_COMPILE_STANDARD_FILE_INCLUDE;
				if (FAILED(D3DCompileFromFile(filepath.c_str(), macros.data(), pUseInclude, param.entryPoint.c_str(), param.shaderModel.c_str(), compileFlags, 0, &this->mpBlob, &pError))) {
					auto e = HINODE_GRAPHICS_CREATE_EXCEPTION(FailedToCreateException, "Shader", "load");
					if (pError) {
						char* msg = (char*)pError->GetBufferPointer();
						e << msg;
					}
					else {
						e << "コンパイルに失敗 filepath=" << filepath.c_str() << ", entryPoint=" << param.entryPoint << ", model=" << param.shaderModel;
					}
					throw e;
				}

				this->mBytecode.pShaderBytecode = this->mpBlob->GetBufferPointer();
				this->mBytecode.BytecodeLength = this->mpBlob->GetBufferSize();
			}

			void Shader::setBytecode(D3D12_SHADER_BYTECODE& bytecode)noexcept
			{
				this->mBytecode = bytecode;
			}

			ID3DBlob* Shader::blob()noexcept
			{
				return this->mpBlob;
			}

			D3D12_SHADER_BYTECODE& Shader::bytecode()noexcept
			{
				return this->mBytecode;
			}

			const D3D12_SHADER_BYTECODE& Shader::bytecode()const noexcept
			{
				return this->mBytecode;
			}
		}
	}
}