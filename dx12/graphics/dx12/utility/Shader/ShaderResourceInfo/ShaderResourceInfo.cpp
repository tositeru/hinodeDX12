#include "stdafx.h"

#include "ShaderResourceInfo.h"

#include <d3dcompiler.h>
#include <wrl/client.h>
#include <assert.h>

#include "../../../common/Common.h"
#include "../../../rootSignature/DX12RootSignature.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			ShaderResourceInfo::ShaderResourceInfo()
			{ }

			ShaderResourceInfo::~ShaderResourceInfo()
			{
				this->clear();
			}

			void ShaderResourceInfo::clear()
			{
				this->mBindLocations.clear();
			}

			void ShaderResourceInfo::append(ID3DBlob* pBlob)
			{
				assert(nullptr != pBlob);
				D3D12_SHADER_BYTECODE bytecode;
				bytecode.pShaderBytecode = pBlob->GetBufferPointer();
				bytecode.BytecodeLength = pBlob->GetBufferSize();
				this->append(bytecode);
			}

			void ShaderResourceInfo::append(D3D12_SHADER_BYTECODE& bytecode)
			{
				Microsoft::WRL::ComPtr<ID3D12ShaderReflection> pReflect;
				auto hr = D3DReflect(bytecode.pShaderBytecode, bytecode.BytecodeLength, IID_PPV_ARGS(&pReflect));
				if (FAILED(hr)) {
					throw HINODE_GRAPHICS_CREATE_EXCEPTION(InvalidArgumentsException, "ShaderResourceInfo", "append")
						<< "シェーダの解析に失敗";
				}

				D3D12_SHADER_DESC shaderDesc;
				pReflect->GetDesc(&shaderDesc);
				shaderDesc.ConstantBuffers;
				shaderDesc.BoundResources;
				//リソース
				for (UINT i = 0; i < shaderDesc.BoundResources; ++i) {
					D3D12_SHADER_INPUT_BIND_DESC bindDesc;
					pReflect->GetResourceBindingDesc(i, &bindDesc);
					auto it = this->mBindLocations.insert({ bindDesc.Name, bindDesc });
					// D3D12_SHADER_INPUT_BIND_DESC::Nameが指している先の寿命が切れてしまうので、キー値を指すようにしている
					it.first->second.Name = it.first->first.c_str();
					//Log(Log::eINFO) << "resource name=" << bindDesc.Name << ", type=" << bindDesc.Type;
					//Log(Log::eINFO) << "  bind point=" << bindDesc.BindPoint << ", count=" << bindDesc.BindCount << ", space=" << bindDesc.Space;
					//Log(Log::eINFO) << "  uID=" << bindDesc.uID << ", uFlags=" << bindDesc.uFlags;
				}

				//定数バッファ内容
				//for (UINT i = 0; i < shaderDesc.ConstantBuffers; ++i) {
				//	auto pCB = pReflect->GetConstantBufferByIndex(i);
				//	D3D12_SHADER_BUFFER_DESC bufDesc;
				//	pCB->GetDesc(&bufDesc);
				//	Log(Log::eINFO) << "cb name=" << bufDesc.Name;
				//	Log(Log::eINFO) << "  size=" << bufDesc.Size << ", variableCount=" << bufDesc.Variables;
				//	bufDesc.Type;
				//	for (UINT j = 0; j < bufDesc.Variables; ++j) {
				//		auto pVar = pCB->GetVariableByIndex(j);
				//		D3D12_SHADER_VARIABLE_DESC varDesc;
				//		pVar->GetDesc(&varDesc);
				//		Log(Log::eINFO) << "  var name=" << varDesc.Name;
				//		Log(Log::eINFO) << "    offset=" << varDesc.StartOffset << ", size=" << varDesc.Size;
				//		Log(Log::eINFO) << "    flags=" << varDesc.uFlags;
				//	}
				//}
			}

			const D3D12_SHADER_INPUT_BIND_DESC& ShaderResourceInfo::find(const std::string& name)const
			{
				auto it = this->mBindLocations.find(name);
				if (this->mBindLocations.end() == it) {
					throw HINODE_GRAPHICS_CREATE_EXCEPTION(InvalidArgumentsException, "ShaderResourceInfo", "find")
						<< "指定した名前のシェーダリソースはありません。name=" << name;
				}
				return it->second;
			}

			const D3D12_SHADER_INPUT_BIND_DESC& ShaderResourceInfo::find(D3D12_DESCRIPTOR_RANGE_TYPE type, UINT slot, UINT space)const
			{
				for (auto& it : this->mBindLocations) {
					if (sIsSame(it.second.Type, type) && it.second.BindPoint == slot && it.second.Space == space) {
						return it.second;
					}
				}

				throw HINODE_GRAPHICS_CREATE_EXCEPTION(InvalidArgumentsException, "ShaderResourceInfo", "find")
					<< "指定したタイプ、スロット、スペースを持つシェーダリソースはありません。シェーダ内で使われていない可能性があります。"
					<< " type=" << DX12RootSignature::sToStr(type) << " slot=" << slot << " space=" << space;
			}

			bool ShaderResourceInfo::isExist(const std::string& name)
			{
				return this->mBindLocations.end() != this->mBindLocations.find(name);
			}

			bool ShaderResourceInfo::sIsSame(D3D_SHADER_INPUT_TYPE shaderInputType, D3D12_DESCRIPTOR_RANGE_TYPE rangeType)noexcept
			{
				switch (shaderInputType) {
				case D3D_SIT_CBUFFER:  return D3D12_DESCRIPTOR_RANGE_TYPE_CBV == rangeType;
				case D3D_SIT_TBUFFER:  return D3D12_DESCRIPTOR_RANGE_TYPE_SRV == rangeType;
				case D3D_SIT_TEXTURE:  return D3D12_DESCRIPTOR_RANGE_TYPE_SRV == rangeType;
				case D3D_SIT_SAMPLER:  return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER == rangeType;
				case D3D_SIT_UAV_RWTYPED:  return D3D12_DESCRIPTOR_RANGE_TYPE_UAV == rangeType;
				case D3D_SIT_STRUCTURED:  return D3D12_DESCRIPTOR_RANGE_TYPE_SRV == rangeType;
				case D3D_SIT_UAV_RWSTRUCTURED:  return D3D12_DESCRIPTOR_RANGE_TYPE_UAV == rangeType;
				case D3D_SIT_BYTEADDRESS:  return D3D12_DESCRIPTOR_RANGE_TYPE_SRV == rangeType;
				case D3D_SIT_UAV_RWBYTEADDRESS:  return D3D12_DESCRIPTOR_RANGE_TYPE_UAV == rangeType;
				case D3D_SIT_UAV_APPEND_STRUCTURED:  return D3D12_DESCRIPTOR_RANGE_TYPE_UAV == rangeType;
				case D3D_SIT_UAV_CONSUME_STRUCTURED:  return D3D12_DESCRIPTOR_RANGE_TYPE_UAV == rangeType;
				case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:  return D3D12_DESCRIPTOR_RANGE_TYPE_UAV == rangeType;
				default:
					assert(false && "未対応");
					return false;
				}
			}

			void ShaderResourceInfo::sWriteLog(const ShaderResourceInfo& info)
			{
				const auto logType = Log::eINFO;
				std::string typeName[] = {
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
				for (auto it : info.mBindLocations) {
					Log(logType) << "name=" << it.first << "; type=" << typeName[it.second.Type];
					Log(logType) << "  slot=" << it.second.BindPoint << ", space=" << it.second.Space << ", count=" << it.second.BindCount;
				}
			}
		}
	}
}