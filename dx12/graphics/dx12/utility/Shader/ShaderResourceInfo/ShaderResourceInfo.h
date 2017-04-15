#pragma once

#include <unordered_map>
#include <d3d12shader.h>
#include <d3d12.h>

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			class ShaderResourceInfo
			{
				ShaderResourceInfo(const ShaderResourceInfo&) = delete;
				ShaderResourceInfo& operator=(const ShaderResourceInfo&) = delete;
			public:
				static void sWriteLog(const ShaderResourceInfo& info);

				static bool sIsSame(D3D_SHADER_INPUT_TYPE shaderInputType, D3D12_DESCRIPTOR_RANGE_TYPE rangeType)noexcept;
			public:
				ShaderResourceInfo();
				~ShaderResourceInfo();

				/// @brief メモリ解放
				void clear();

				/// @brief リソース情報を追記する
				///
				/// @param[in] pBlob
				void append(ID3DBlob* pBlob);

				/// @brief リソース情報を追記する
				///
				/// @param[in] bytecode
				void append(D3D12_SHADER_BYTECODE& bytecode);

				/// @brief 名前からバインド情報を検索する
				/// @param[in] name
				/// @retval const D3D12_SHADER_INPUT_BIND_DESC&
				const D3D12_SHADER_INPUT_BIND_DESC& find(const std::string& name)const;

				/// @brief タイプ、スロット、スペースからバインド情報を検索する
				/// @param[in] type D3D12_DESCRIPTOR_RANGE_TYPE
				/// @param[in] slot
				/// @param[in] space
				/// @retval const D3D12_SHADER_INPUT_BIND_DESC&
				const D3D12_SHADER_INPUT_BIND_DESC& find(D3D12_DESCRIPTOR_RANGE_TYPE type, UINT slot, UINT space)const;

				/// @brief 指定した名前を持つバインド情報があるか調べる
				/// @param[in] name
				/// @retval bool
				bool isExist(const std::string& name);

			private:
				std::unordered_map<std::string, D3D12_SHADER_INPUT_BIND_DESC> mBindLocations;
			};
		}
	}
}