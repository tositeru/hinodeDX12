#include "stdafx.h"

#include "Pipeline.h"

#include "../Shader.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			Pipeline::Pipeline()
				: mType(DX12PipelineState::eTYPE_UNKNOWN)
			{ }

			Pipeline::~Pipeline()
			{
				this->clear();
			}

			void Pipeline::clear()
			{
				this->mType = DX12PipelineState::eTYPE_UNKNOWN;
				this->mPipelineState.clear();
				this->mRootSignature.clear();
				this->mRootSignatureInfo.clear();
				this->mShaderResourceInfo.clear();
			}

			void Pipeline::create(ID3D12Device* pDevice, D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, Shader& rootSignature, UINT nodeMask)
			{
				this->mRootSignature.create(pDevice, nodeMask, rootSignature.bytecode());
				this->mRootSignatureInfo.analysis(rootSignature.bytecode());

				pDesc->pRootSignature = this->mRootSignature;
				this->mPipelineState.create(pDevice, pDesc);
				if (nullptr != pDesc->VS.pShaderBytecode) this->mShaderResourceInfo.append(pDesc->VS);
				if (nullptr != pDesc->HS.pShaderBytecode) this->mShaderResourceInfo.append(pDesc->HS);
				if (nullptr != pDesc->DS.pShaderBytecode) this->mShaderResourceInfo.append(pDesc->DS);
				if (nullptr != pDesc->GS.pShaderBytecode) this->mShaderResourceInfo.append(pDesc->GS);
				if (nullptr != pDesc->PS.pShaderBytecode) this->mShaderResourceInfo.append(pDesc->PS);

				this->mType = DX12PipelineState::eTYPE_GRAPHICS;
			}

			void Pipeline::create(ID3D12Device* pDevice, D3D12_COMPUTE_PIPELINE_STATE_DESC* pDesc, Shader& rootSignature, UINT nodeMask)
			{
				this->mRootSignature.create(pDevice, nodeMask, rootSignature.bytecode());
				this->mRootSignatureInfo.analysis(rootSignature.bytecode());

				pDesc->pRootSignature = this->mRootSignature;
				pDesc->NodeMask = nodeMask;
				this->mPipelineState.create(pDevice, pDesc);
				this->mShaderResourceInfo.append(pDesc->CS);

				this->mType = DX12PipelineState::eTYPE_COMPUTE;
			}

			void Pipeline::bind(ID3D12GraphicsCommandList* pCommandList)
			{
				pCommandList->SetPipelineState(this->mPipelineState);
				switch (this->type()) {
				case DX12PipelineState::eTYPE_GRAPHICS: pCommandList->SetGraphicsRootSignature(this->mRootSignature); break;
				case DX12PipelineState::eTYPE_COMPUTE: pCommandList->SetComputeRootSignature(this->mRootSignature); break;
				default:
					assert(false);
				}
			}

			DX12PipelineState::TYPE Pipeline::type()const noexcept
			{
				return this->mType;
			}

			DX12RootSignature& Pipeline::rootSignature()noexcept
			{
				return this->mRootSignature;
			}

			DX12PipelineState& Pipeline::pipelineState()noexcept
			{
				return this->mPipelineState;
			}

			RootSignatureInfo& Pipeline::rootSignatureInfo()noexcept
			{
				return this->mRootSignatureInfo;
			}

			ShaderResourceInfo& Pipeline::shaderResourceInfo()noexcept
			{
				return this->mShaderResourceInfo;
			}

			const DX12RootSignature& Pipeline::rootSignature()const noexcept
			{
				return this->mRootSignature;
			}

			const DX12PipelineState& Pipeline::pipelineState()const noexcept
			{
				return this->mPipelineState;
			}

			const RootSignatureInfo& Pipeline::rootSignatureInfo()const noexcept
			{
				return this->mRootSignatureInfo;
			}

			const ShaderResourceInfo& Pipeline::shaderResourceInfo()const noexcept
			{
				return this->mShaderResourceInfo;
			}

		}
	}
}