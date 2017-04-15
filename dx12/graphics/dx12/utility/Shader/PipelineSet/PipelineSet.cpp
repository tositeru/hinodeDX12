#include "stdafx.h"

#include "PipelineSet.h"

#include "../Pipeline/Pipeline.h"
#include "../PipelineResourceBinder/PipelineResourceBinder.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			PipelineSet::PipelineSet()
			{}

			PipelineSet::PipelineSet(const PipelineSet& right)
				: mpPipeline(right.mpPipeline)
				, mpResourceBinder(right.mpResourceBinder)
			{ }

			PipelineSet::PipelineSet(PipelineSet&& right)
				: mpPipeline(std::move(right.mpPipeline))
				, mpResourceBinder(std::move(right.mpResourceBinder))
			{
				right.mpPipeline.reset();
				right.mpResourceBinder.reset();
			}

			PipelineSet& PipelineSet::operator=(const PipelineSet& right)
			{
				this->mpPipeline = right.mpPipeline;
				this->mpResourceBinder = right.mpResourceBinder;
				return *this;
			}

			PipelineSet& PipelineSet::operator=(PipelineSet&& right)
			{
				this->mpPipeline = std::move(right.mpPipeline);
				this->mpResourceBinder = std::move(right.mpResourceBinder);
				right.mpPipeline.reset();
				right.mpResourceBinder.reset();
				return *this;
			}

			PipelineSet::~PipelineSet()
			{
				this->clear();
			}

			void PipelineSet::clear()
			{
				this->mpPipeline.reset();
				this->mpResourceBinder.reset();
			}

			void PipelineSet::create(ID3D12Device* pDevice, D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, Shader& rootSignature, UINT nodeMask)
			{
				this->mpPipeline = std::make_shared<Pipeline>();
				this->mpPipeline->create(pDevice, pDesc, rootSignature);

				this->mpResourceBinder = std::make_shared<PipelineResourceBinder>();
				this->mpResourceBinder->create(pDevice, DX12PipelineState::eTYPE_GRAPHICS, this->mpPipeline->rootSignatureInfo(), nodeMask);
			}

			void PipelineSet::create(ID3D12Device* pDevice, D3D12_COMPUTE_PIPELINE_STATE_DESC* pDesc, Shader& rootSignature, UINT nodeMask)
			{
				this->mpPipeline = std::make_shared<Pipeline>();
				this->mpPipeline->create(pDevice, pDesc, rootSignature, nodeMask);

				this->mpResourceBinder = std::make_shared<PipelineResourceBinder>();
				this->mpResourceBinder->create(pDevice, DX12PipelineState::eTYPE_COMPUTE, this->mpPipeline->rootSignatureInfo(), nodeMask);
			}

			PipelineSet PipelineSet::clone(std::initializer_list<size_t> replaceRootParamIndices)
			{
				PipelineSet ret;
				ret.mpPipeline = this->mpPipeline;
				ret.mpResourceBinder = this->mpResourceBinder;
				if (0 < replaceRootParamIndices.size()) {
					ret.mpResourceBinder->replace(replaceRootParamIndices);
				} else {
					ret.mpResourceBinder->replaceAuto();
				}
				return ret;
			}

			void PipelineSet::setAutoReplaceRootParameterIndices(std::initializer_list<size_t> replaceRootParamIndices)
			{
				this->mpResourceBinder->setAutoReplaceIndex(replaceRootParamIndices);
			}

			void PipelineSet::bind(ID3D12GraphicsCommandList* pCommandList)
			{
				unless(this->mpResourceBinder->validate(*this->mpPipeline)) {
					throw HINODE_GRAPHICS_CREATE_EXCEPTION(RunTimeErrorException, "PipelineSet", "bind")
						<< "未設定のパラメータがある状態でコマンドリストに設定しようとしました。";
				}
				this->mpPipeline->bind(pCommandList);
				this->mpResourceBinder->bind(pCommandList);
			}

			void PipelineSet::updateCBV(const std::string& name, DescriptorHeapPoolSet& dhPoolSet, const void* pData, uint byteSize, ConstantBufferPool& cbPool)
			{
				this->updateCBV(name.c_str(), dhPoolSet, pData, byteSize, cbPool);
			}

			void PipelineSet::updateCBV(const char* name, DescriptorHeapPoolSet& dhPoolSet, const void* pData, uint byteSize, ConstantBufferPool& cbPool)
			{
				auto bindInfo = this->findBindingInfo(name, RootSignatureInfo::Key::eCBV);
				this->mpResourceBinder->updateCBV(bindInfo.rootIndex, bindInfo.descriptorTable.tableIndex, dhPoolSet, pData, byteSize, cbPool);
			}

			void PipelineSet::updateCBV(const std::string& name, DescriptorHeapPoolSet& dhPoolSet, ConstantBufferPool::AllocateInfo& allocateInfo)
			{
				this->updateCBV(name.c_str(), dhPoolSet, allocateInfo);
			}

			void PipelineSet::updateCBV(const char* name, DescriptorHeapPoolSet& dhPoolSet, ConstantBufferPool::AllocateInfo& allocateInfo)
			{
				auto bindInfo = this->findBindingInfo(name, RootSignatureInfo::Key::eCBV);
				this->mpResourceBinder->updateCBV(bindInfo.rootIndex, bindInfo.descriptorTable.tableIndex, dhPoolSet, allocateInfo);
			}

			void PipelineSet::updateSRV(const std::string& name, DescriptorHeapPoolSet& dhPoolSet, ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pSRVDesc)
			{
				this->updateSRV(name.c_str(), dhPoolSet, pResource, pSRVDesc);
			}

			void PipelineSet::updateSRV(const char* name, DescriptorHeapPoolSet& dhPoolSet, ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pSRVDesc)
			{
				auto bindInfo = this->findBindingInfo(name, RootSignatureInfo::Key::eSRV);
				this->mpResourceBinder->updateSRV(bindInfo.rootIndex, bindInfo.descriptorTable.tableIndex, dhPoolSet, pResource, pSRVDesc);
			}

			void PipelineSet::updateUAV(const std::string& name, DescriptorHeapPoolSet& dhPoolSet, ID3D12Resource* pResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* pUAVDesc, ID3D12Resource* pCounterResource)
			{
				this->updateUAV(name.c_str(), dhPoolSet, pResource, pUAVDesc, pCounterResource);
			}

			void PipelineSet::updateUAV(const char* name, DescriptorHeapPoolSet& dhPoolSet, ID3D12Resource* pResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* pUAVDesc, ID3D12Resource* pCounterResource)
			{
				auto bindInfo = this->findBindingInfo(name, RootSignatureInfo::Key::eUAV);
				this->mpResourceBinder->updateUAV(bindInfo.rootIndex, bindInfo.descriptorTable.tableIndex, dhPoolSet, pResource, pUAVDesc, pCounterResource);
			}

			void PipelineSet::updateSampler(const std::string& name, DescriptorHeapPoolSet& dhPoolSet, const D3D12_SAMPLER_DESC* pSamplerDesc)
			{
				this->updateSampler(name.c_str(), dhPoolSet, pSamplerDesc);
			}

			void PipelineSet::updateSampler(const char* name, DescriptorHeapPoolSet& dhPoolSet, const D3D12_SAMPLER_DESC* pSamplerDesc)
			{
				auto bindInfo = this->findBindingInfo(name, RootSignatureInfo::Key::eSAMPLER);
				this->mpResourceBinder->updateSampler(bindInfo.rootIndex, bindInfo.descriptorTable.tableIndex, dhPoolSet, pSamplerDesc);
			}

			RootSignatureInfo::BindingInfo PipelineSet::findBindingInfo(const std::string& name, RootSignatureInfo::Key::TYPE type)noexcept
			{
				return this->findBindingInfo(name.c_str(), type);
			}

			RootSignatureInfo::BindingInfo PipelineSet::findBindingInfo(const char* name, RootSignatureInfo::Key::TYPE type)noexcept
			{
				const D3D12_SHADER_INPUT_BIND_DESC* pInputInfo;
				const RootSignatureInfo::BindingInfo* pBindInfo;
				this->findShaderInputAndBindingInfo(&pInputInfo, &pBindInfo, name, type);
				return *pBindInfo;
			}

			bool PipelineSet::isExistBindingInfo(const std::string& name, RootSignatureInfo::Key::TYPE type)noexcept
			{
				return this->isExistBindingInfo(name.c_str(), type);
			}

			bool PipelineSet::isExistBindingInfo(const char* name, RootSignatureInfo::Key::TYPE type)noexcept
			{
				unless(this->mpPipeline->shaderResourceInfo().isExist(name)) {
					return false;
				}

				auto& inputInfo = this->mpPipeline->shaderResourceInfo().find(name);
				assert(RootSignatureInfo::Key::sIsSameType(type, inputInfo.Type));
				RootSignatureInfo::Key key(type, inputInfo.BindPoint, inputInfo.Space);
				return this->mpPipeline->rootSignatureInfo().isExist(key);
			}


			void PipelineSet::findShaderInputAndBindingInfo(const D3D12_SHADER_INPUT_BIND_DESC** ppOutInputInfo, const RootSignatureInfo::BindingInfo** ppOutBindInfo, const std::string& name, RootSignatureInfo::Key::TYPE keyType)
			{
				try {
					*ppOutInputInfo = &this->mpPipeline->shaderResourceInfo().find(name);
					assert(RootSignatureInfo::Key::sIsSameType(keyType, (*ppOutInputInfo)->Type));
					RootSignatureInfo::Key key(keyType, (*ppOutInputInfo)->BindPoint, (*ppOutInputInfo)->Space);
					*ppOutBindInfo = &this->mpPipeline->rootSignatureInfo().findBindingInfo(key);
				} catch (Exception& e) {
					throw HINODE_GRAPHICS_CREATE_EXCEPTION_WITH_PREV_EXCEPTION(InvalidArgumentsException, "PipelineSet", "findShaderInputAndBindingInfo", e)
						<< "指定した名前のシェーダ入力情報またはバインド情報が見つかりませんでした。ルートシグネチャで指定していない可能性があります。 name=" << name << ", keyType=" << RootSignatureInfo::Key::sToStr(keyType);
				}
			}

			Pipeline& PipelineSet::pipeline()
			{
				return *this->mpPipeline;
			}

			PipelineResourceBinder& PipelineSet::resourceBinder()
			{
				return *this->mpResourceBinder;
			}

		}
	}
}