#pragma once

#include <string>
#include <vector>

#include <d3d12shader.h>
#include <d3d12.h>

#include "../../common/Common.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			/// @brief シェーダをまとめたクラス
			class Shader
			{
			public:
				struct CompileParam
				{
					std::string entryPoint = "main";
					std::string shaderModel;
					std::vector<D3D_SHADER_MACRO> pMacros;
					ID3DInclude* pInclude = nullptr;
					UINT flags = 0;

					CompileParam& setEntryAndModel(const std::string& entryPoint, const std::string& shaderModel)
					{
						this->entryPoint = entryPoint;
						this->shaderModel = shaderModel;
						return *this;
					}
				};

			public:
				Shader() = default;
				~Shader();

				void clear();

				/// @brief ファイルからシェーダをコンパイルする
				/// @param[in] filepath
				/// @param[in] param
				void load(const std::wstring& filepath, const CompileParam& param);

				/// @brief D3D12_SHADER_BYTECODEの内容をコピーする
				///
				/// ポインタのみのコピーなので、もとのbytecodeに設定していたものの寿命に気をつけてください
				/// @param[in] bytecode
 				void setBytecode(D3D12_SHADER_BYTECODE& bytecode)noexcept;

			accessor_declaration:
				ID3DBlob* blob()noexcept;
				D3D12_SHADER_BYTECODE& bytecode()noexcept;
				const D3D12_SHADER_BYTECODE& bytecode()const noexcept;

			private:
				ID3DBlob* mpBlob = nullptr;
				D3D12_SHADER_BYTECODE mBytecode;
			};
		}
	}
}