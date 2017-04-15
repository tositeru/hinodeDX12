#pragma once

#include <d3d12.h>
#include "../../../common/Common.h"

#include "../../../pipelineState/DX12PipelineState.h"
#include "../../../rootSignature/DX12RootSignature.h"

#include "../ShaderResourceInfo/ShaderResourceInfo.h"
#include "../RootSignatureInfo/RootSignatureInfo.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			class Shader;

			class Pipeline
			{
				Pipeline(Pipeline& right) = delete;
				Pipeline& operator=(Pipeline& right) = delete;
			public:
				Pipeline();
				~Pipeline();

				/// @brief メモリ開放
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

				/// @brief コマンドリストに登録する
				/// @param[in] pCommandList
				void bind(ID3D12GraphicsCommandList* pCommandList);

			accessor_declaration:
				DX12PipelineState::TYPE type()const noexcept;
				DX12RootSignature& rootSignature()noexcept;
				DX12PipelineState& pipelineState()noexcept;
				RootSignatureInfo& rootSignatureInfo()noexcept;
				ShaderResourceInfo& shaderResourceInfo()noexcept;

				const DX12RootSignature& rootSignature()const noexcept;
				const DX12PipelineState& pipelineState()const noexcept;
				const RootSignatureInfo& rootSignatureInfo()const noexcept;
				const ShaderResourceInfo& shaderResourceInfo()const noexcept;

			private:
				DX12PipelineState::TYPE mType;

				DX12RootSignature mRootSignature;
				DX12PipelineState mPipelineState;
				RootSignatureInfo mRootSignatureInfo;
				ShaderResourceInfo mShaderResourceInfo;
			};
		}
	}
}