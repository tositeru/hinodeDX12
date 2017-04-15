#pragma once

#include <unordered_map>
#include <bitset>

#include <D3d12shader.h>

#include "../../../descriptorHeap/DX12DescriptorHeap.h"
#include "../../../pipelineState/DX12PipelineState.h"

#include "../../RingBuffer/ConstantBufferPool/ConstantBufferPool.h"
#include "../../RingBuffer/DescriptorHeapPool/DescriptorHeapPool.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			class RootSignatureInfo;
			class DescriptorHeapPoolSet;
			class Pipeline;

			/// @brief グラフィックパイプラインのリソースを設定するクラス
			class PipelineResourceBinder
			{
				PipelineResourceBinder(PipelineResourceBinder&) = delete;
				PipelineResourceBinder& operator=(PipelineResourceBinder&) = delete;

			public:
				class IRootElement
				{
				public:
					enum TYPE
					{
						eDESCRIPTOR_TABLE,
						eROOT_CONSTANTS,
						eROOT_CBV,
						eROOT_SRV,
						eROOT_UAV,
					};

				public:
					static D3D12_ROOT_PARAMETER_TYPE sToDX12RootParameterType(TYPE type)noexcept;

				public:
					IRootElement(TYPE type);
					virtual ~IRootElement() {}

					virtual void clear() = 0;
					virtual void bindForGraphics(ID3D12GraphicsCommandList* pCommandList) = 0;
					virtual void bindForCompute(ID3D12GraphicsCommandList* pCommandList) = 0;
					virtual std::shared_ptr<IRootElement> clone() = 0;

					virtual bool validate()const noexcept = 0;

					template<typename T>
					T& cast()
					{
						auto p = dynamic_cast<T*>(this);
						assert(nullptr != p);
						return *p;
					}

				accessor_declaration:
					int rootIndex()const noexcept;
					TYPE type()const noexcept;

				protected:
					int mRootIndex;
					const TYPE mType;
				};

				class DescriptorTable : public IRootElement 
				{
				public:
					enum DESCRIPTOR_TYPE {
						eDESCRIPTOR_UNKNOWN,
						eDESCRIPTOR_CBV_SRV_UAV,
						eDESCRIPTOR_SAMPLER,
					};

				public:
					DescriptorTable();
					DescriptorTable(DESCRIPTOR_TYPE type);

					void create(int rootIndex, DESCRIPTOR_TYPE descriptorType, int initialDescriptorCount)noexcept;
					void allocate(DescriptorHeapPool& dhPool);

					virtual void clear()override;

					virtual void bindForGraphics(ID3D12GraphicsCommandList* pCommandList)override;
					virtual void bindForCompute(ID3D12GraphicsCommandList* pCommandList)override;
					virtual std::shared_ptr<IRootElement> clone()override;

					virtual bool validate()const noexcept;
					void setValidation(size_t offset, size_t count=1);

				accessor_declaration:
					DESCRIPTOR_TYPE descriptorType()const noexcept;
					DescriptorHeapPool::AllocateInfo& dhAllocateInfo()noexcept;
					const DescriptorHeapPool::AllocateInfo& dhAllocateInfo()const noexcept;
					DX12CPUDescriptorHandle getCPUHandle(int offset);
					int descriptorCount()const noexcept;

					const std::vector<bool>& isSetValidation()const noexcept;
				private:
					DESCRIPTOR_TYPE mDescriptorType;
					int mDescriptorCount;
					DescriptorHeapPool::AllocateInfo mDHAllocateInfo;

					//設定状況の検証用
					std::vector<bool> mIsSetValidations;
				};

				class RootConstants : public IRootElement
				{
				public:
					RootConstants();

					virtual void clear()override;
					void create(int rootIndex, int count32BitValue);

					virtual void bindForGraphics(ID3D12GraphicsCommandList* pCommandList)override;
					virtual void bindForCompute(ID3D12GraphicsCommandList* pCommandList)override;
					virtual std::shared_ptr<IRootElement> clone()override;

					virtual bool validate()const noexcept;

					void update(const void* pData, int count, int offset)noexcept;

				accessor_declaration:
					int count()const noexcept;

				private:
					int mCount;
					std::vector<uint32_t> mData;
				};

				/// @brief Root CBV
				///
				/// TODO D3D12_GPU_VIRTUAL_ADDRESSの直持ちは危ないので別のものを持たせる？
				class RootCBV : public IRootElement
				{
				public:
					RootCBV();

					virtual void clear()override;

					void create(int rootIndex);

					virtual void bindForGraphics(ID3D12GraphicsCommandList* pCommandList)override;
					virtual void bindForCompute(ID3D12GraphicsCommandList* pCommandList)override;
					virtual std::shared_ptr<IRootElement> clone()override;

					virtual bool validate()const noexcept;

					void updateGPUAddress(D3D12_GPU_VIRTUAL_ADDRESS pGPUAddress)noexcept;
				accessor_declaration:

				private:
					D3D12_GPU_VIRTUAL_ADDRESS mpGPUAddress;
				};

				/// @brief Root SRV
				///
				/// TODO D3D12_GPU_VIRTUAL_ADDRESSの直持ちは危ないので別のものを持たせる？
				class RootSRV : public IRootElement
				{
				public:
					RootSRV();

					virtual void clear()override;

					void create(int rootIndex);

					virtual void bindForGraphics(ID3D12GraphicsCommandList* pCommandList)override;
					virtual void bindForCompute(ID3D12GraphicsCommandList* pCommandList)override;
					virtual std::shared_ptr<IRootElement> clone()override;

					virtual bool validate()const noexcept;

					void updateGPUAddress(D3D12_GPU_VIRTUAL_ADDRESS pGPUAddress)noexcept;
				accessor_declaration:

				private:
					D3D12_GPU_VIRTUAL_ADDRESS mpGPUAddress;
				};

				/// @brief Root UAV
				///
				/// TODO D3D12_GPU_VIRTUAL_ADDRESSの直持ちは危ないので別のものを持たせる？
				class RootUAV : public IRootElement
				{
				public:
					RootUAV();

					virtual void clear()override;

					void create(int rootIndex);

					virtual void bindForGraphics(ID3D12GraphicsCommandList* pCommandList)override;
					virtual void bindForCompute(ID3D12GraphicsCommandList* pCommandList)override;
					virtual std::shared_ptr<IRootElement> clone()override;

					virtual bool validate()const noexcept;

					void updateGPUAddress(D3D12_GPU_VIRTUAL_ADDRESS pGPUAddress)noexcept;
				accessor_declaration:

				private:
					D3D12_GPU_VIRTUAL_ADDRESS mpGPUAddress;
				};

			public:
				PipelineResourceBinder();
				~PipelineResourceBinder();

				/// @brief メモリ解放
				void clear();

				/// @brief 作成
				/// @param[in] pDevice
				/// @param[in] type
				/// @param[in] rootSignatureInfo
				/// @param[in] nodeMask defalut=0
				void create(ID3D12Device* pDevice, DX12PipelineState::TYPE type, const RootSignatureInfo& rootSignatureInfo, UINT nodeMask = 0);

				/// @brief 同じルートパラメータを共有するGraphicsResourceBinderを作成する
				///
				/// 部分的に独立したルートパラメータを設定したい場合はGraphicsResourceBinder::replace関数を使用してください
				/// @param[out] pOut
				void clone(PipelineResourceBinder* pOut);

				/// @brief 指定したルートパラメータを付け替える
				///
				/// 他のGraphicsResourceBinderと共有したくないルートパラメータを用意したいときに使用してください
				/// @param[in] index
				void replace(size_t index);

				/// @brief 指定したルートパラメータを付け替える
				///
				/// 他のGraphicsResourceBinderと共有したくないルートパラメータを用意したいときに使用してください
				/// @param[in] index
				void replace(std::initializer_list<size_t> indices);

				/// @brief インスタンスに設定されているルートパラメータを付け替える
				///
				/// この関数で付け替えられるルートパラメータはsetAutoReplaceIndex関数で設定できます
				void replaceAuto();

				/// @brief replaceAuto関数で付け替えられるルートパラメータを設定する
				/// @param indices
				void setAutoReplaceIndex(std::initializer_list<size_t> indices);

				/// @brief コマンドリストに登録する
				///
				/// この関数を使用する前に関連づいているDescriptorHeapをコマンドリストに設定しておく必要があります。
				/// @param[in] pCommandList
				void bind(ID3D12GraphicsCommandList* pCommandList);

				/// @brief 現在の設定状況を検証する
				/// @param[in] pipeline
				/// @retval bool true=問題なし, false=未設定なものがある
				bool validate(const Pipeline& pipeline)noexcept;

				/// @brief 定数バッファの更新
				/// @param[in] rootIndex
				/// @param[in] descriptorTableIndex
				/// @param[in] dhPoolSet
				/// @param[in] pData
				/// @param[in] byteSize
				/// @param[in] cbPool
				void updateCBV(size_t rootIndex, size_t descriptorTableIndex, DescriptorHeapPoolSet& dhPoolSet, const void* pData, uint byteSize, ConstantBufferPool& cbPool);

				/// @brief 定数バッファの更新
				///
				/// 複数のパイプライン間で定数バッファを共有したい場合に使うことを想定しています
				/// @param[in] rootIndex
				/// @param[in] descriptorTableIndex
				/// @param[in] dhPoolSet
				/// @param[in] allocateInfo
				void updateCBV(size_t rootIndex, size_t descriptorTableIndex, DescriptorHeapPoolSet& dhPoolSet, ConstantBufferPool::AllocateInfo& allocateInfo);

				/// @brief SRVの更新
				/// @param[in] rootIndex
				/// @param[in] descriptorTableIndex
				/// @param[in] dhPoolSet
				/// @param[in] pResource
				/// @param[in] pSRVDesc
				void updateSRV(size_t rootIndex, size_t descriptorTableIndex, DescriptorHeapPoolSet& dhPoolSet, ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pSRVDesc);

				/// @brief UAVの更新
				/// @param[in] rootIndex
				/// @param[in] descriptorTableIndex
				/// @param[in] dhPoolSet
				/// @param[in] pResource
				/// @param[in] pUAVDesc
				/// @param[in] pCounterResource default=nullptr
				void updateUAV(size_t rootIndex, size_t descriptorTableIndex, DescriptorHeapPoolSet& dhPoolSet, ID3D12Resource* pResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* pUAVDesc, ID3D12Resource* pCounterResource = nullptr);

				/// @brief サンプラの更新
				/// @param[in] rootIndex
				/// @param[in] descriptorTableIndex
				/// @param[in] dhPoolSet
				/// @param[in] pSamplerDesc
				void updateSampler(size_t rootIndex, size_t descriptorTableIndex, DescriptorHeapPoolSet& dhPoolSet, const D3D12_SAMPLER_DESC* pSamplerDesc);

			accessor_declaration:
				DX12PipelineState::TYPE type()const noexcept;
				std::vector<std::shared_ptr<IRootElement>>& rootElements()noexcept;
				IRootElement& rootElement(size_t index);

			private:
				DX12PipelineState::TYPE mType;
				//Root Signatureのサイズの上限が64DWORDなので、64にしている
				// 参考HP: https://msdn.microsoft.com/en-us/library/windows/desktop/dn899209(v=vs.85).aspx
				std::bitset<64> mReplaceIndices; ///< clone関数になにも指定しないときに
				std::vector<std::shared_ptr<IRootElement>> mpRootElements;
			};
		}
	}
}