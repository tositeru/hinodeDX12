#include "stdafx.h"

#include "RootSignatureInfo.h"

#include <wrl/client.h>
#include "../../../common/Exception.h"
#include "../../../common/log.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			RootSignatureInfo::RootSignatureInfo()
			{

			}

			RootSignatureInfo::RootSignatureInfo(RootSignatureInfo&& right)
				: mBindingHash(right.mBindingHash)
				, mpRootElements(std::move(right.mpRootElements))
			{}

			RootSignatureInfo& RootSignatureInfo::operator=(RootSignatureInfo&& right)
			{
				this->mBindingHash = std::move(right.mBindingHash);
				this->mpRootElements = std::move(right.mpRootElements);
				return *this;
			}

			RootSignatureInfo::~RootSignatureInfo()
			{
				this->clear();
			}

			void RootSignatureInfo::clear()
			{
				this->mpRootElements.clear();
				this->mBindingHash.clear();
			}

			void RootSignatureInfo::analysis(ID3DBlob* rootSignatureBlob, D3D_ROOT_SIGNATURE_VERSION version)
			{
				D3D12_SHADER_BYTECODE bytecode;
				bytecode.pShaderBytecode = rootSignatureBlob->GetBufferPointer();
				bytecode.BytecodeLength = rootSignatureBlob->GetBufferSize();
				this->analysis(bytecode, version);
			}

			void RootSignatureInfo::analysis(D3D12_SHADER_BYTECODE& rootSignatureBytecode, D3D_ROOT_SIGNATURE_VERSION version)
			{
				this->clear();

				assert(nullptr != rootSignatureBytecode.pShaderBytecode);

				Microsoft::WRL::ComPtr<ID3D12VersionedRootSignatureDeserializer> pDeserializer;
				D3D12CreateVersionedRootSignatureDeserializer(rootSignatureBytecode.pShaderBytecode, rootSignatureBytecode.BytecodeLength, IID_PPV_ARGS(&pDeserializer));

				// メモリはpDeserializer内部で確保されるので、こちらから開放する必要はない
				const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* pDesc;
				auto hr = pDeserializer->GetRootSignatureDescAtVersion(version, &pDesc);
				if (FAILED(hr)) {
					throw HINODE_GRAPHICS_CREATE_EXCEPTION(FailedToCreateException, "RootSignature", "analysis")
						<< "Root Signatureの解析に失敗";
				}

				//auto logType = Log::eINFO;
				//Log(logType) << "version=" << pDesc->Version;
				//Log(logType) << "param count = " << pDesc->Desc_1_1.NumParameters;
				//Log(logType) << "static sampler count = " << pDesc->Desc_1_1.NumStaticSamplers;
				//Log(logType) << "flag = " << pDesc->Desc_1_1.Flags;
				this->mpRootElements.reserve(pDesc->Desc_1_1.NumParameters);
				class OffsetManager
				{
				public:
					struct HashKey
					{
						RootSignatureInfo::Key::TYPE type;
						int space = -1;

						HashKey() = default;
						HashKey(Key::TYPE type, int space)
							: type(type)
							, space(space)
						{}

						bool operator==(const HashKey& right)const
						{
							return this->type == right.type && this->space == right.space;
						}

						bool operator!=(const HashKey& right)const
						{
							return !(*this == right);
						}

						struct Hash
						{
							std::size_t operator()(const HashKey& key)const
							{
								return std::hash<size_t>()(std::hash<int>()(key.type) + std::hash<int>()(key.space ^ (-1)));
							}
						};
					};

				public:
					unsigned int calc(UINT offset, const Key& key)
					{
						Key offsetKey(key.type, -1, key.space);

						if (offset == D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND) {
							auto it = this->mOffsetList.find(offsetKey);
							if (it == this->mOffsetList.end()) {
								this->mOffsetList.insert({ offsetKey, key.slot });
								it = this->mOffsetList.find(offsetKey);
							}
							else {
								it->second++;
							}
							return it->second;
						}
						else {
							auto it = this->mOffsetList.find(offsetKey);
							if (it == this->mOffsetList.end()) {
								this->mOffsetList.insert({ offsetKey, key.slot });
								it = this->mOffsetList.find(offsetKey);
							}
							else {
								it->second = key.slot;
							}
							return it->second;
						}
					}

				private:
					std::unordered_map<Key, unsigned int, Key::Hash> mOffsetList; //DescriptorTableのオフセットを記録するもの

				};
				std::unordered_map < OffsetManager::HashKey, OffsetManager, OffsetManager::HashKey::Hash > offsetMngs;

				for (UINT rootIndex = 0; rootIndex < pDesc->Desc_1_1.NumParameters; ++rootIndex) {
					auto& param = pDesc->Desc_1_1.pParameters[rootIndex];

					Key key;
					key.slot = 0;
					BindingInfo bindingInfo;
					bindingInfo.type = param.ParameterType;
					bindingInfo.rootIndex = rootIndex;
					//Log(logType) << "param type=" << pDesc->Desc_1_1.pParameters[i].ParameterType << ", visibillity=" << pDesc->Desc_1_1.pParameters[i].ShaderVisibility;

					switch (param.ParameterType) {
					case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
					{
						auto pRootElement = std::make_unique<RootConstants>(param.ParameterType, rootIndex);
						pRootElement->count32BitValue = param.Constants.Num32BitValues;
						this->mpRootElements.push_back(std::move(pRootElement));

						key.type = Key::eCBV;
						key.slot = param.Constants.ShaderRegister;
						key.space = param.Constants.RegisterSpace;
						OffsetManager::HashKey offsetMngKey(key.type, key.space);
						offsetMngs[offsetMngKey].calc(key.slot, key);

						this->mBindingHash.insert({ key, bindingInfo });

						//Log(logType) << "    (" << param.Constants.ShaderRegister << ", " << param.Constants.RegisterSpace << ")";
						//Log(logType) << "      Num32BitValue=" << param.Constants.Num32BitValues;
						break;
					}
					case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
					{
						auto pRootElement = std::make_unique<RootDescriptorTable>(param.ParameterType, rootIndex);
						//単純なコピーだとpDeserializerがリリースされたときparam.DescriptorTable.pDescriptorRangesの内容も消えるのでコピーする
						pRootElement->ranges.reserve(param.DescriptorTable.NumDescriptorRanges);
						for (auto i = 0u; i < param.DescriptorTable.NumDescriptorRanges; ++i) {
							pRootElement->ranges.emplace_back(param.DescriptorTable.pDescriptorRanges[i]);
						}
						this->mpRootElements.push_back(std::move(pRootElement));

						UINT tableIndex = 0;
						for (UINT rangeIndex = 0; rangeIndex < param.DescriptorTable.NumDescriptorRanges; ++rangeIndex) {
							auto& descriptor = param.DescriptorTable.pDescriptorRanges[rangeIndex];

							switch (descriptor.RangeType) {
							case D3D12_DESCRIPTOR_RANGE_TYPE_SRV: key.type = Key::eSRV; break;
							case D3D12_DESCRIPTOR_RANGE_TYPE_UAV: key.type = Key::eUAV; break;
							case D3D12_DESCRIPTOR_RANGE_TYPE_CBV: key.type = Key::eCBV; break;
							case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER: key.type = Key::eSAMPLER; break;
							default:
								assert(false);
							}
							key.space = descriptor.RegisterSpace;
							bindingInfo.descriptorTable.tableIndex = tableIndex;
							OffsetManager::HashKey offsetMngKey(key.type, key.space);
							auto offset = offsetMngs[offsetMngKey].calc(descriptor.OffsetInDescriptorsFromTableStart, key);
							for (auto i = 0u; i < descriptor.NumDescriptors; ++i) {
								//key.slot = offset + i;
								key.slot = descriptor.BaseShaderRegister + i;
								this->mBindingHash.insert({ key, bindingInfo });
								bindingInfo.descriptorTable.tableIndex++;
							}
							tableIndex = bindingInfo.descriptorTable.tableIndex;

							//Log(logType) << "    range=" << j << " (" << descriptor.BaseShaderRegister << ", " << descriptor.RegisterSpace << ")";
							//Log(logType) << "      count=" << descriptor.NumDescriptors << ", offset=" << descriptor.OffsetInDescriptorsFromTableStart;
							//Log(logType) << "      rangeType=" << descriptor.RangeType << ", flags=" << descriptor.Flags;
						}
						break;
					}
					case D3D12_ROOT_PARAMETER_TYPE_CBV:
					case D3D12_ROOT_PARAMETER_TYPE_SRV:
					case D3D12_ROOT_PARAMETER_TYPE_UAV:
					{
						this->mpRootElements.emplace_back(std::make_unique<RootSignatureInfo::RootElement>(param.ParameterType, rootIndex));

						switch (param.ParameterType) {
						case D3D12_ROOT_PARAMETER_TYPE_CBV: key.type = Key::eCBV; break;
						case D3D12_ROOT_PARAMETER_TYPE_SRV: key.type = Key::eSRV; break;
						case D3D12_ROOT_PARAMETER_TYPE_UAV: key.type = Key::eUAV; break;
						default: assert(false);
						}
						key.slot = param.Descriptor.ShaderRegister;
						key.space = param.Descriptor.RegisterSpace;
						OffsetManager::HashKey offsetMngKey(key.type, key.space);
						offsetMngs[offsetMngKey].calc(key.slot, key);

						this->mBindingHash.insert({ key, bindingInfo });
						break;
					}
					default:
						assert(false);
					}

				}
			}

			const RootSignatureInfo::BindingInfo& RootSignatureInfo::findBindingInfo(Key& key)const
			{
				auto it = this->mBindingHash.find(key);
				if (this->mBindingHash.end() == it) {
					throw HINODE_GRAPHICS_CREATE_EXCEPTION(InvalidArgumentsException, "RootSignatureInfo", "findBindingInfo")
						<< "指定したキーは見つかりませんでした。 key(type=" << key.sToStr(key.type) << ", slot=" << key.slot << ", space=" << key.space << ")";
				}
				return it->second;
			}

			RootSignatureInfo::Key RootSignatureInfo::findKey(const BindingInfo& bindingInfo)const
			{
				for (auto& it  : this->mBindingHash) {
					auto& info = it.second;
					bool isSame = false;
					isSame = info.type == bindingInfo.type;
					isSame &= info.rootIndex == bindingInfo.rootIndex;
					isSame &= (D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE != info.type)
						|| info.descriptorTable.tableIndex == bindingInfo.descriptorTable.tableIndex;
					if (isSame) {
						return it.first;
					}
				}
				return Key();
			}

			bool RootSignatureInfo::isExist(const Key& key)const noexcept
			{
				return this->mBindingHash.end() != this->mBindingHash.find(key);
			}

			const std::unordered_map<RootSignatureInfo::Key, RootSignatureInfo::BindingInfo, RootSignatureInfo::Key::Hash>& RootSignatureInfo::bindingHash()const noexcept
			{
				return this->mBindingHash;
			}

			const std::vector<std::unique_ptr<RootSignatureInfo::RootElement>>& RootSignatureInfo::rootElements()const noexcept
			{
				return this->mpRootElements;
			}

			const RootSignatureInfo::RootElement& RootSignatureInfo::rootElement(int rootIndex)const
			{
				return *this->mpRootElements.at(rootIndex);
			}

			const RootSignatureInfo::RootElement& RootSignatureInfo::rootElement(const BindingInfo& info)const
			{
				return *this->mpRootElements.at(info.rootIndex);
			}

			void RootSignatureInfo::sWriteLog(const RootSignatureInfo& info)
			{
				for (auto& it : info.bindingHash()) {
					auto& key = it.first;
					auto& bindingInfo = it.second;
					Log(Log::eINFO) << "key(" << Key::sToStr(key.type) << ", slot=" << key.slot << ", space=" << key.space << ") => rootIndex=" << bindingInfo.rootIndex << ", type=" << info.rootElement(bindingInfo).toTypeStr();
				}
			}

			//
			//	RootSignatureInfo::Key
			//

			RootSignatureInfo::Key::Key(TYPE type, int slot, int space)
				: type(type)
				, slot(slot)
				, space(space)
			{}

			bool RootSignatureInfo::Key::operator==(const Key& right)const
			{
				return this->type == right.type && this->slot == right.slot && this->space == right.space;
			}
			bool RootSignatureInfo::Key::operator!=(const Key& right)const
			{
				return !(*this == right);
			}

			bool RootSignatureInfo::Key::sIsSameType(TYPE type, D3D_SHADER_INPUT_TYPE d3dShaderInputType)
			{
				static TYPE table[] = {
					eCBV, //D3D_SIT_CBUFFER
					eCBV, //D3D_SIT_TBUFFER
					eSRV, //D3D_SIT_TEXTURE
					eSAMPLER, //D3D_SIT_SAMPLER
					eUAV, //D3D_SIT_UAV_RWTYPED
					eSRV, //D3D_SIT_STRUCTURED
					eUAV, //D3D_SIT_UAV_RWSTRUCTURED
					eSRV, //D3D_SIT_BYTEADDRESS
					eUAV, //D3D_SIT_UAV_RWBYTEADDRESS
					eUAV, //D3D_SIT_UAV_APPEND_STRUCTURED
					eUAV, //D3D_SIT_UAV_CONSUME_STRUCTURED
					eUAV, //D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER
				};
				return type == table[d3dShaderInputType];
			}

			const std::string& RootSignatureInfo::Key::sToStr(TYPE type)noexcept
			{
				static const std::string tbl[] = {
					"eCBV",
					"eSRV",
					"eUAV",
					"eSAMPLER",
				};
				return tbl[type];
			}

			D3D12_DESCRIPTOR_RANGE_TYPE RootSignatureInfo::Key::sToD3D12DescriptorRangeType(TYPE type)noexcept
			{
				static const D3D12_DESCRIPTOR_RANGE_TYPE tbl[] = {
					D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
					D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
					D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
					D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,
				};
				return tbl[type];
			}

			//
			//	RootSignatureInfo::Key::Hash
			//

			std::size_t RootSignatureInfo::Key::Hash::operator()(const Key& key)const
			{
				return std::hash<size_t>()(std::hash<int>()(key.type) + std::hash<int>()(key.slot) + std::hash<int>()(key.space ^ (-1)));
			}

			//
			//	RootSignatureInfo::RootElement
			//
			RootSignatureInfo::RootElement::RootElement(D3D12_ROOT_PARAMETER_TYPE type, int rootIndex)
				: type(type)
				, rootIndex(rootIndex)
			{ }

			std::string RootSignatureInfo::RootElement::toTypeStr()const noexcept
			{
				switch (this->type) {
				case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE: return "DESCRIPTOR_TABLE";
				case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS: return "32BIT_CONSTANTS";
				case D3D12_ROOT_PARAMETER_TYPE_CBV: return "CBV";
				case D3D12_ROOT_PARAMETER_TYPE_SRV: return "SRV";
				case D3D12_ROOT_PARAMETER_TYPE_UAV: return "UAV";
				}
				assert(false);
				return "--ERROR--";
			}

			UINT RootSignatureInfo::RootDescriptorTable::calDescriptorCount()const noexcept
			{
				UINT numDescriptor = 0u;
				for (auto& range : this->ranges) {
					numDescriptor += range.NumDescriptors;
				}
				return numDescriptor;
			}
		}
	}
}