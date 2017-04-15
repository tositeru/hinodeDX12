#pragma once

#include <memory>
#include <d3d12.h>
#include <d3d12shader.h>

#include "../../../common/Common.h"
#include "../RootSignatureInfo/RootSignatureInfo.h"
#include "../../RingBuffer/ConstantBufferPool/ConstantBufferPool.h"
#include "../../RingBuffer/DescriptorHeapPool/DescriptorHeapPool.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			class Shader;
			class Pipeline;
			class PipelineResourceBinder;
			class DescriptorHeapPoolSet;

			/// @brief パイプラインに必要なものをまとめたもの
			///
			/// 1つのPipelineSetを通じて、2回以上の描画をする際は、ルートパラメータにあるDescriptorTableの内容を各描画で違う値にすることはできません。(DX12のデバッグモードを有効にしていると警告が出ます。)
			/// そのような場合は PipelineSet::clone関数に異なる内容にしたいルートパラメータの添字を指定して、PipelineSetを複製してください。
			/// なお、DescriptorTable以外のルートパラメータの値を更新する際は上記の回避策は必要ありません。
			class PipelineSet
			{
			public:
				PipelineSet();
				PipelineSet(const PipelineSet& right);
				PipelineSet(PipelineSet&& right);
				PipelineSet& operator=(const PipelineSet& right);
				PipelineSet& operator=(PipelineSet&& right);
				~PipelineSet();

				/// @brief メモリ解放
				void clear();

				/// @brief 作成
				/// @brief pDevice
				/// @brief pDesc
				/// @brief rootSignature
				/// @brief nodeMask default=0
				void create(ID3D12Device* pDevice, D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, Shader& rootSignature, UINT nodeMask = 0);

				/// @brief 作成
				/// @brief pDevice
				/// @brief pDesc
				/// @brief rootSignature
				/// @brief nodeMask default=0
				void create(ID3D12Device* pDevice, D3D12_COMPUTE_PIPELINE_STATE_DESC* pDesc, Shader& rootSignature, UINT nodeMask = 0);

				/// @brief 同じ内容のものを作成する
				///
				/// 1つのPipelineSetを通じて、2回以上の描画をする際は、ルートパラメータにあるDescriptorTableの内容を各描画で違う値にすることはできません。(DX12のデバッグモードを有効にしていると警告が出ます。)
				/// そのような場合は この関数に異なる内容にしたいルートパラメータの添字を指定して、PipelineSetを複製してください。
				/// なお、DescriptorTable以外のルートパラメータの値を更新する際は上記の回避策は必要ありません。
				/// replaceRootParamIndicesに指定しなかったルートパラメータは共有されます。
				/// ただし、空のreplaceRootParamIndicesを指定した際はsetAutoReplaceRootParameterIndices関数で指定したリストが代わりに使用されますので、注意してください。
				/// @param[in] replaceRootParamIndices
				/// @retval GraphicsPipelineSet
				PipelineSet clone(std::initializer_list<size_t> replaceRootParamIndices);

				/// @brief clone関数にreplaceRootParamIndicesを指定したとき場合に代わりに使われるリストを設定する
				///
				/// デフォルトでは何も指定していません。
				/// @param[in] replaceRootParameterIndices
				void setAutoReplaceRootParameterIndices(std::initializer_list<size_t> replaceRootParamIndices);

				/// @brief コマンドリストに登録する
				///
				/// この関数を呼び出した後はupdateXXX関数を呼びだしてリソースの内容を変更しても大丈夫です
				/// @param[in] pCommandList
				void bind(ID3D12GraphicsCommandList* pCommandList);

				/// @brief 定数バッファの更新
				/// @param[in] name
				/// @param[in] dhPoolSet
				/// @param[in] pData
				/// @param[in] byteSize
				/// @param[in] cbPool
				void updateCBV(const std::string& name, DescriptorHeapPoolSet& dhPoolSet, const void* pData, uint byteSize, ConstantBufferPool& cbPool);

				/// @brief 定数バッファの更新
				/// @param[in] name
				/// @param[in] dhPoolSet
				/// @param[in] pData
				/// @param[in] byteSize
				/// @param[in] cbPool
				void updateCBV(const char* name, DescriptorHeapPoolSet& dhPoolSet, const void* pData, uint byteSize, ConstantBufferPool& cbPool);

				/// @brief 定数バッファの更新
				/// @param[in] name
				/// @param[in] dhPoolSet
				/// @param[in] allocateInfo
				void updateCBV(const std::string& name, DescriptorHeapPoolSet& dhPoolSet, ConstantBufferPool::AllocateInfo& allocateInfo);

				/// @brief 定数バッファの更新
				/// @param[in] name
				/// @param[in] dhPoolSet
				/// @param[in] allocateInfo
				void updateCBV(const char* name, DescriptorHeapPoolSet& dhPoolSet, ConstantBufferPool::AllocateInfo& allocateInfo);

				/// @brief SRVの更新
				/// @param[in] name
				/// @param[in] dhPoolSet
				/// @param[in] pResource
				/// @param[in] pSRVDesc
				void updateSRV(const std::string& name, DescriptorHeapPoolSet& dhPoolSet, ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pSRVDesc);

				/// @brief SRVの更新
				/// @param[in] name
				/// @param[in] dhPoolSet
				/// @param[in] pResource
				/// @param[in] pSRVDesc
				void updateSRV(const char* name, DescriptorHeapPoolSet& dhPoolSet, ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pSRVDesc);

				/// @brief UAVの更新
				/// @param[in] name
				/// @param[in] dhPoolSet
				/// @param[in] pResource
				/// @param[in] pUAVDesc
				/// @param[in] pCounterResource default=nullptr
				void updateUAV(const std::string& name, DescriptorHeapPoolSet& dhPoolSet, ID3D12Resource* pResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* pUAVDesc, ID3D12Resource* pCounterResource = nullptr);

				/// @brief UAVの更新
				/// @param[in] name
				/// @param[in] dhPoolSet
				/// @param[in] pResource
				/// @param[in] pUAVDesc
				/// @param[in] pCounterResource default=nullptr
				void updateUAV(const char* name, DescriptorHeapPoolSet& dhPoolSet, ID3D12Resource* pResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* pUAVDesc, ID3D12Resource* pCounterResource = nullptr);

				/// @brief サンプラの更新
				/// @param[in] dhPoolSet
				/// @param[in] name
				/// @param[in] pSamplerDesc
				void updateSampler(const std::string& name, DescriptorHeapPoolSet& dhPoolSet, const D3D12_SAMPLER_DESC* pSamplerDesc);

				/// @brief サンプラの更新
				/// @param[in] dhPoolSet
				/// @param[in] name
				/// @param[in] pSamplerDesc
				void updateSampler(const char* name, DescriptorHeapPoolSet& dhPoolSet, const D3D12_SAMPLER_DESC* pSamplerDesc);

				/// @brief 同名のバインド情報を取得する
				/// @param[in] name
				/// @param[in] type
				/// @retval RootSignatureInfo::BindingInfo
				RootSignatureInfo::BindingInfo findBindingInfo(const std::string& name, RootSignatureInfo::Key::TYPE type)noexcept;

				/// @brief 同名のバインド情報を取得する
				/// @param[in] name
				/// @param[in] type
				/// @retval RootSignatureInfo::BindingInfo
				RootSignatureInfo::BindingInfo findBindingInfo(const char* name, RootSignatureInfo::Key::TYPE type)noexcept;

				/// @brief 同名の定数バッファのバインド情報を取得する
				/// @param[in] name
				/// @param[in] type
				/// @retval RootSignatureInfo::BindingInfo
				bool isExistBindingInfo(const std::string& name, RootSignatureInfo::Key::TYPE type)noexcept;

				/// @brief 同名のバインド情報があるか調べる
				/// @param[in] name
				/// @param[in] type
				/// @retval RootSignatureInfo::BindingInfo
				bool isExistBindingInfo(const char* name, RootSignatureInfo::Key::TYPE type)noexcept;

			accessor_declaration:
				Pipeline& pipeline();
				PipelineResourceBinder& resourceBinder();

			private:
				/// @brief　ルートパラメータに値を設定するために必要な情報を探す
				/// @param[out] ppOutInputInfo
				/// @parma[out] ppOutBindInfo
				/// @param[in] name
				/// @param[in] keyType
				void findShaderInputAndBindingInfo(const D3D12_SHADER_INPUT_BIND_DESC** ppOutInputInfo, const RootSignatureInfo::BindingInfo** ppOutBindInfo, const std::string& name, RootSignatureInfo::Key::TYPE keyType);

			private:
				std::shared_ptr<Pipeline> mpPipeline;
				std::shared_ptr<PipelineResourceBinder> mpResourceBinder;
			};
		}
	}
}