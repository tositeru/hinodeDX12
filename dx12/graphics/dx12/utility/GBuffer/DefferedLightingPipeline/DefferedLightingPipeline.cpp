#include "stdafx.h"

#include "DefferedLightingPipeline.h"

#include "../../../pipelineState/DX12PipelineState.h"
#include "../../Shader/Shader.h"

//-------------------------------------------------------------------------------------
//	ライブラリ側で用意しているコンパイル済みシェーダバイナリを表す配列のインクルード
//	これらの配列はプロジェクトのコンパイル時に生成されます
//
namespace shader
{
	namespace CSDefferedDirectionLighting
	{
#ifdef _DEBUG
#include "../Shader/DefferedLighting/bin/CSDefferedDirectionLighting_debug.h"
#include "../Shader/DefferedLighting/bin/CSDefferedDirectionLighting_RS_debug.h"
#else
#include "../Shader/DefferedLighting/bin/CSDefferedDirectionLighting.h"
#include "../Shader/DefferedLighting/bin/CSDefferedDirectionLighting_RS.h"
#endif
	}
	namespace CSDefferedDirectionLightingWithShadow
	{
#ifdef _DEBUG
#include "../Shader/DefferedLighting/bin/CSDefferedDirectionLightingWithShadow_debug.h"
#include "../Shader/DefferedLighting/bin/CSDefferedDirectionLightingWithShadow_RS_debug.h"
#else
#include "../Shader/DefferedLighting/bin/CSDefferedDirectionLightingWithShadow.h"
#include "../Shader/DefferedLighting/bin/CSDefferedDirectionLightingWithShadow_RS.h"
#endif
	}
}

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			PipelineSet DefferedLightingPipeline::sCreateDirectionLighting(ID3D12Device* pDevice, const DX12_COMPUTE_PIPELINE_STATE_DESC* pDesc, UINT nodeMask)
			{
				DX12_COMPUTE_PIPELINE_STATE_DESC pipelineDesc;
				if (pDesc) pipelineDesc = *pDesc;
				pipelineDesc.setCS(&CD3DX12_SHADER_BYTECODE((void*)shader::CSDefferedDirectionLighting::g_main, sizeof(shader::CSDefferedDirectionLighting::g_main)));
				utility::Shader rootSignature;
				rootSignature.setBytecode(CD3DX12_SHADER_BYTECODE((void*)shader::CSDefferedDirectionLighting::g_RS, sizeof(shader::CSDefferedDirectionLighting::g_RS)));
				PipelineSet pipelineSet;
				pipelineSet.create(pDevice, &pipelineDesc, rootSignature, nodeMask);
				return pipelineSet;
			}

			PipelineSet DefferedLightingPipeline::sCreateDirectionLightingWithShadow(ID3D12Device* pDevice, const DX12_COMPUTE_PIPELINE_STATE_DESC* pDesc, UINT nodeMask)
			{
				DX12_COMPUTE_PIPELINE_STATE_DESC pipelineDesc;
				if (pDesc) pipelineDesc = *pDesc;
				pipelineDesc.setCS(&CD3DX12_SHADER_BYTECODE((void*)shader::CSDefferedDirectionLightingWithShadow::g_main, sizeof(shader::CSDefferedDirectionLightingWithShadow::g_main)));
				utility::Shader rootSignature;
				rootSignature.setBytecode(CD3DX12_SHADER_BYTECODE((void*)shader::CSDefferedDirectionLightingWithShadow::g_RS, sizeof(shader::CSDefferedDirectionLightingWithShadow::g_RS)));
				PipelineSet pipelineSet;
				pipelineSet.create(pDevice, &pipelineDesc, rootSignature, nodeMask);
				pipelineSet.setAutoReplaceRootParameterIndices({ 2, 3 });
				return pipelineSet;
			}

		}
	}
}