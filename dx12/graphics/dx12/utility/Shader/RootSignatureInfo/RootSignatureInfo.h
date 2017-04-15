#pragma once

#include <unordered_map>
#include <memory>

#include <d3d12.h>
#include <D3d12shader.h>

#include "../../../common/Common.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			class RootSignatureInfo
			{
			public:
				/// @brief リソースタイプ、スロット、スペース番号から設定先のルート情報へアクセスするためのもの
				struct Key
				{
					enum TYPE
					{
						eCBV,
						eSRV,
						eUAV,
						eSAMPLER,
					};
					TYPE type;
					int slot = -1;
					int space = -1;

					Key() = default;
					Key(const Key&) = default;
					Key& operator=(const Key&) = default;
					Key(TYPE type, int slot, int space);

					bool operator==(const Key& right)const;
					bool operator!=(const Key& right)const;

					static bool sIsSameType(TYPE type, D3D_SHADER_INPUT_TYPE d3dShaderInputType);
					static const std::string& sToStr(TYPE type)noexcept;
					static D3D12_DESCRIPTOR_RANGE_TYPE sToD3D12DescriptorRangeType(TYPE type)noexcept;

					struct Hash
					{
						std::size_t operator()(const Key& key)const;
					};
				};

				/// @brief 実行時にリソースを設定するためのもの
				struct BindingInfo
				{
					D3D12_ROOT_PARAMETER_TYPE type;
					int rootIndex;
					union {
						struct {
							int tableIndex;
						}descriptorTable;
					};
				};

				/// @brief ルート要素の情報
				///
				/// DescriptorHeapを作るのに利用する
				struct RootElement
				{
					D3D12_ROOT_PARAMETER_TYPE type;
					int rootIndex;

					RootElement() = default;
					RootElement(const RootElement&) = default;
					RootElement& operator=(const RootElement& right) = default;
					RootElement(D3D12_ROOT_PARAMETER_TYPE type, int rootIndex);
					virtual ~RootElement() {}
					std::string toTypeStr()const noexcept;

					template<typename T>
					T& cast()noexcept
					{
						return *dynamic_cast<T*>(this);
					}

					template<typename T>
					const T& cast()const noexcept
					{
						return *dynamic_cast<const T*>(this);
					}
				};

				struct RootDescriptorTable : public RootElement
				{
					std::vector<D3D12_DESCRIPTOR_RANGE1> ranges;

					RootDescriptorTable() = default;
					RootDescriptorTable(D3D12_ROOT_PARAMETER_TYPE type, int rootIndex)
						: RootElement(type, rootIndex)
					{}

					UINT calDescriptorCount()const noexcept;
				};

				struct RootConstants : public RootElement
				{
					uint32_t count32BitValue;
					RootConstants() = default;
					RootConstants(D3D12_ROOT_PARAMETER_TYPE type, int rootIndex)
						: RootElement(type, rootIndex)
					{}
				};

			public:
				static void sWriteLog(const RootSignatureInfo& info);

			public:
				RootSignatureInfo();
				RootSignatureInfo(RootSignatureInfo&& right);
				RootSignatureInfo& operator=(RootSignatureInfo&& right);
				~RootSignatureInfo();

				void clear();

				/// @brief Root Signatureを解析してリソースバインディングのデータベースを作る
				/// @param[in] rootSignatureBlob
				/// @param[in] version default=D3D_ROOT_SIGNATURE_VERSION_1_1
				/// @exception hinode::graphics::FailedToCreateException
				void analysis(ID3DBlob* rootSignatureBlob, D3D_ROOT_SIGNATURE_VERSION version = D3D_ROOT_SIGNATURE_VERSION_1_1);

				/// @brief Root Signatureを解析してリソースバインディングのデータベースを作る
				/// @param[in] rootSignatureBytecode
				/// @param[in] version default=D3D_ROOT_SIGNATURE_VERSION_1_1
				/// @exception hinode::graphics::FailedToCreateException
				void analysis(D3D12_SHADER_BYTECODE& rootSignatureBytecode, D3D_ROOT_SIGNATURE_VERSION version = D3D_ROOT_SIGNATURE_VERSION_1_1);

				/// @brief キーと一致しているBindingInfoを探す
				/// @parma[in] key
				const BindingInfo& findBindingInfo(Key& key)const;

				/// @brief 渡されたbindingInfoと一致するもののキーを返す
				///
				///	一致しなかったときは無効なキーを返します
				/// @param[in] bindingInfo
				/// @retval Key 
				Key findKey(const BindingInfo& bindingInfo)const;

				/// @brief 指定されたキーが存在するか調べる
				/// @param[in] key
				/// @retval bool
				bool isExist(const Key& key)const noexcept;

			accessor_declaration:
				const std::unordered_map<Key, BindingInfo, Key::Hash>& bindingHash()const noexcept;
				const std::vector<std::unique_ptr<RootElement>>& rootElements()const noexcept;
				const RootElement& rootElement(int rootIndex)const;
				const RootElement& rootElement(const BindingInfo& info)const;

			private:
				std::unordered_map<Key, BindingInfo, Key::Hash> mBindingHash;
				std::vector<std::unique_ptr<RootElement>> mpRootElements;//Descriptor Tableを作るのに使う
			};
		}
	}
}