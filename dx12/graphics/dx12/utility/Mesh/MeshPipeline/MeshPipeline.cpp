#include "stdafx.h"

#include "MeshPipeline.h"

#include "../../../pipelineState/DX12PipelineState.h"
#include "../../Shader/Shader.h"

//-------------------------------------------------------------------------------------
//	ライブラリ側で用意しているコンパイル済みシェーダバイナリを表す配列のインクルード
//	これらの配列はプロジェクトのコンパイル時に生成されます
//
namespace shader
{
	namespace PVertexPipeline
	{
#ifdef _DEBUG
#include "../Shader/PVertex/bin/PVertexPipeline_RSForward_debug.h"
#include "../Shader/PVertex/bin/PVertexPipeline_VSMain_debug.h"
#include "../Shader/PVertex/bin/PVertexPipeline_PSForward_debug.h"
#include "../Shader/PVertex/bin/PVertexPipeline_VSDeffered_debug.h"
#include "../Shader/PVertex/bin/PVertexPipeline_PSDeffered_debug.h"
#include "../Shader/PVertex/bin/PVertexPipeline_RSZPass_debug.h"
#include "../Shader/PVertex/bin/PVertexPipeline_VSZPass_debug.h"
#else
#include "../Shader/PVertex/bin/PVertexPipeline_RSForward.h"
#include "../Shader/PVertex/bin/PVertexPipeline_VSMain.h"
#include "../Shader/PVertex/bin/PVertexPipeline_PSForward.h"
#include "../Shader/PVertex/bin/PVertexPipeline_VSDeffered.h"
#include "../Shader/PVertex/bin/PVertexPipeline_PSDeffered.h"
#include "../Shader/PVertex/bin/PVertexPipeline_RSZPass.h"
#include "../Shader/PVertex/bin/PVertexPipeline_VSZPass.h"
#endif
	}

	namespace PCVertexPipeline
	{
#ifdef _DEBUG
#include "../Shader/PCVertex/bin/PCVertexPipeline_RSForward_debug.h"
#include "../Shader/PCVertex/bin/PCVertexPipeline_VSMain_debug.h"
#include "../Shader/PCVertex/bin/PCVertexPipeline_PSForward_debug.h"
#include "../Shader/PCVertex/bin/PCVertexPipeline_VSDeffered_debug.h"
#include "../Shader/PCVertex/bin/PCVertexPipeline_PSDeffered_debug.h"
#include "../Shader/PCVertex/bin/PCVertexPipeline_RSZPass_debug.h"
#include "../Shader/PCVertex/bin/PCVertexPipeline_VSZPass_debug.h"
#else
#include "../Shader/PCVertex/bin/PCVertexPipeline_RSForward.h"
#include "../Shader/PCVertex/bin/PCVertexPipeline_VSMain.h"
#include "../Shader/PCVertex/bin/PCVertexPipeline_PSForward.h"
#include "../Shader/PCVertex/bin/PCVertexPipeline_VSDeffered.h"
#include "../Shader/PCVertex/bin/PCVertexPipeline_PSDeffered.h"
#include "../Shader/PCVertex/bin/PCVertexPipeline_RSZPass.h"
#include "../Shader/PCVertex/bin/PCVertexPipeline_VSZPass.h"
#endif
	}

	namespace PTVertexPipeline
	{
#ifdef _DEBUG
#include "../Shader/PTVertex/bin/PTVertexPipeline_RSForward_debug.h"
#include "../Shader/PTVertex/bin/PTVertexPipeline_VSMain_debug.h"
#include "../Shader/PTVertex/bin/PTVertexPipeline_PSForward_debug.h"
#include "../Shader/PTVertex/bin/PTVertexPipeline_VSDeffered_debug.h"
#include "../Shader/PTVertex/bin/PTVertexPipeline_PSDeffered_debug.h"
#include "../Shader/PTVertex/bin/PTVertexPipeline_RSZPass_debug.h"
#include "../Shader/PTVertex/bin/PTVertexPipeline_VSZPass_debug.h"
#else
#include "../Shader/PTVertex/bin/PTVertexPipeline_RSForward.h"
#include "../Shader/PTVertex/bin/PTVertexPipeline_VSMain.h"
#include "../Shader/PTVertex/bin/PTVertexPipeline_PSForward.h"
#include "../Shader/PTVertex/bin/PTVertexPipeline_VSDeffered.h"
#include "../Shader/PTVertex/bin/PTVertexPipeline_PSDeffered.h"
#include "../Shader/PTVertex/bin/PTVertexPipeline_RSZPass.h"
#include "../Shader/PTVertex/bin/PTVertexPipeline_VSZPass.h"
#endif
	}

	namespace PNVertexPipeline
	{
#ifdef _DEBUG
#include "../Shader/PNVertex/bin/PNVertexPipeline_RSForward_debug.h"
#include "../Shader/PNVertex/bin/PNVertexPipeline_VSMain_debug.h"
#include "../Shader/PNVertex/bin/PNVertexPipeline_PSForward_debug.h"
#include "../Shader/PNVertex/bin/PNVertexPipeline_RSDeffered_debug.h"
#include "../Shader/PNVertex/bin/PNVertexPipeline_PSDeffered_debug.h"
#include "../Shader/PNVertex/bin/PNVertexPipeline_RSZPass_debug.h"
#include "../Shader/PNVertex/bin/PNVertexPipeline_VSZPass_debug.h"
#else
#include "../Shader/PNVertex/bin/PNVertexPipeline_RSForward.h"
#include "../Shader/PNVertex/bin/PNVertexPipeline_VSMain.h"
#include "../Shader/PNVertex/bin/PNVertexPipeline_PSForward.h"
#include "../Shader/PNVertex/bin/PNVertexPipeline_RSDeffered.h"
#include "../Shader/PNVertex/bin/PNVertexPipeline_PSDeffered.h"
#include "../Shader/PNVertex/bin/PNVertexPipeline_RSZPass.h"
#include "../Shader/PNVertex/bin/PNVertexPipeline_VSZPass.h"
#endif
	}
	namespace PTNVertexPipeline
	{
#ifdef _DEBUG
#include "../Shader/PTNVertex/bin/PTNVertexPipeline_RSForward_debug.h"
#include "../Shader/PTNVertex/bin/PTNVertexPipeline_VSMain_debug.h"
#include "../Shader/PTNVertex/bin/PTNVertexPipeline_PSForward_debug.h"
#include "../Shader/PTNVertex/bin/PTNVertexPipeline_RSDeffered_debug.h"
#include "../Shader/PTNVertex/bin/PTNVertexPipeline_PSDeffered_debug.h"
#include "../Shader/PTNVertex/bin/PTNVertexPipeline_RSZPass_debug.h"
#include "../Shader/PTNVertex/bin/PTNVertexPipeline_VSZPass_debug.h"
#else
#include "../Shader/PTNVertex/bin/PTNVertexPipeline_RSForward.h"
#include "../Shader/PTNVertex/bin/PTNVertexPipeline_VSMain.h"
#include "../Shader/PTNVertex/bin/PTNVertexPipeline_PSForward.h"
#include "../Shader/PTNVertex/bin/PTNVertexPipeline_RSDeffered.h"
#include "../Shader/PTNVertex/bin/PTNVertexPipeline_PSDeffered.h"
#include "../Shader/PTNVertex/bin/PTNVertexPipeline_RSZPass.h"
#include "../Shader/PTNVertex/bin/PTNVertexPipeline_VSZPass.h"
#endif
	}
	namespace PTNCVertexPipeline
	{
#ifdef _DEBUG
#include "../Shader/PTNCVertex/bin/PTNCVertexPipeline_RSForward_debug.h"
#include "../Shader/PTNCVertex/bin/PTNCVertexPipeline_VSMain_debug.h"
#include "../Shader/PTNCVertex/bin/PTNCVertexPipeline_PSForward_debug.h"
#include "../Shader/PTNCVertex/bin/PTNCVertexPipeline_RSDeffered_debug.h"
#include "../Shader/PTNCVertex/bin/PTNCVertexPipeline_PSDeffered_debug.h"
#include "../Shader/PTNCVertex/bin/PTNCVertexPipeline_RSZPass_debug.h"
#include "../Shader/PTNCVertex/bin/PTNCVertexPipeline_VSZPass_debug.h"
#else
#include "../Shader/PTNCVertex/bin/PTNCVertexPipeline_RSForward.h"
#include "../Shader/PTNCVertex/bin/PTNCVertexPipeline_VSMain.h"
#include "../Shader/PTNCVertex/bin/PTNCVertexPipeline_PSForward.h"
#include "../Shader/PTNCVertex/bin/PTNCVertexPipeline_RSDeffered.h"
#include "../Shader/PTNCVertex/bin/PTNCVertexPipeline_PSDeffered.h"
#include "../Shader/PTNCVertex/bin/PTNCVertexPipeline_RSZPass.h"
#include "../Shader/PTNCVertex/bin/PTNCVertexPipeline_VSZPass.h"
#endif
	}
}
//
//
//-------------------------------------------------------------------------------------

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			struct PipelineInfo
			{
				enum {
					eVERTEX,
					eHULL,
					eDOMAIN,
					eGEOMETORY,
					ePIXEL,
					eROOT_SIGNATURE,
					eSTAGE_COUNT,
				};

				std::wstring hlslFilepath = L"";
				std::array<utility::Shader::CompileParam, eSTAGE_COUNT> compileParams;
				std::array<CD3DX12_SHADER_BYTECODE, eSTAGE_COUNT>			bytecodes;
				std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayouts;

				PipelineInfo()
				{
					for (auto& bytecode : this->bytecodes) {
						bytecode.BytecodeLength = 0;
						bytecode.pShaderBytecode = nullptr;
					}
				}

				PipelineSet createWithCompile(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, UINT nodeMask)
				{
					std::array<utility::Shader, eSTAGE_COUNT> shaders;
					for (auto i = 0u; i < compileParams.size(); ++i) {
						auto& compileParam = this->compileParams[i];
						if ("" == compileParam.shaderModel) {
							continue;
						}
						shaders[i].load(this->hlslFilepath, compileParam);
					}

					DX12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc;
					if (pDesc) pipelineDesc = *pDesc;
					if (nullptr != shaders[eVERTEX].blob()) pipelineDesc.setVS(shaders[eVERTEX].blob());
					if (nullptr != shaders[eHULL].blob()) pipelineDesc.setHS(shaders[eHULL].blob());
					if (nullptr != shaders[eDOMAIN].blob()) pipelineDesc.setDS(shaders[eDOMAIN].blob());
					if (nullptr != shaders[eGEOMETORY].blob()) pipelineDesc.setGS(shaders[eGEOMETORY].blob());
					if (nullptr != shaders[ePIXEL].blob()) pipelineDesc.setPS(shaders[ePIXEL].blob());
					pipelineDesc.setInputLayout(this->inputLayouts);

					assert(nullptr != shaders[eROOT_SIGNATURE].blob());

					PipelineSet pipelineSet;
					pipelineSet.create(pDevice, &pipelineDesc, shaders[eROOT_SIGNATURE], nodeMask);
					return pipelineSet;
				}

				PipelineSet createWithBytecode(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, UINT nodeMask, std::initializer_list<DXGI_FORMAT> RTFormats = { DXGI_FORMAT_R8G8B8A8_UNORM })
				{
					DX12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc;
					if (pDesc) pipelineDesc = *pDesc;
					if (nullptr != this->bytecodes[eVERTEX].pShaderBytecode) pipelineDesc.setVS(&this->bytecodes[eVERTEX]);
					if (nullptr != this->bytecodes[eHULL].pShaderBytecode) pipelineDesc.setHS(&this->bytecodes[eHULL]);
					if (nullptr != this->bytecodes[eDOMAIN].pShaderBytecode) pipelineDesc.setDS(&this->bytecodes[eDOMAIN]);
					if (nullptr != this->bytecodes[eGEOMETORY].pShaderBytecode) pipelineDesc.setGS(&this->bytecodes[eGEOMETORY]);
					if (nullptr != this->bytecodes[ePIXEL].pShaderBytecode) pipelineDesc.setPS(&this->bytecodes[ePIXEL]);
					pipelineDesc.setInputLayout(this->inputLayouts);
					pipelineDesc.setRTV(RTFormats);

					assert(nullptr != this->bytecodes[eROOT_SIGNATURE].pShaderBytecode);

					Shader rootSignature;
					rootSignature.setBytecode(this->bytecodes[eROOT_SIGNATURE]);
					PipelineSet pipelineSet;
					pipelineSet.create(pDevice, &pipelineDesc, rootSignature, nodeMask);
					return pipelineSet;
				}
			};

			PipelineSet ForwardPipeline::sCreatePVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, UINT nodeMask)
			{
				PipelineInfo info;
				info.bytecodes[PipelineInfo::eVERTEX] = CD3DX12_SHADER_BYTECODE((void*)PVertexPipeline::g_VSMain, sizeof(PVertexPipeline::g_VSMain));
				info.bytecodes[PipelineInfo::ePIXEL] = CD3DX12_SHADER_BYTECODE((void*)PVertexPipeline::g_PSForward, sizeof(PVertexPipeline::g_PSForward));
				info.bytecodes[PipelineInfo::eROOT_SIGNATURE] = CD3DX12_SHADER_BYTECODE((void*)PVertexPipeline::g_RSForward, sizeof(PVertexPipeline::g_RSForward));
				info.inputLayouts = {
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				};
				auto result = info.createWithBytecode(pDevice, pDesc, nodeMask);
				result.setAutoReplaceRootParameterIndices({ 2 });
				return result;
			}

			PipelineSet ForwardPipeline::sCreatePCVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, UINT nodeMask)
			{
				PipelineInfo info;
				info.bytecodes[PipelineInfo::eVERTEX] = CD3DX12_SHADER_BYTECODE((void*)PCVertexPipeline::g_VSMain, sizeof(PCVertexPipeline::g_VSMain));
				info.bytecodes[PipelineInfo::ePIXEL] = CD3DX12_SHADER_BYTECODE((void*)PCVertexPipeline::g_PSForward, sizeof(PCVertexPipeline::g_PSForward));
				info.bytecodes[PipelineInfo::eROOT_SIGNATURE] = CD3DX12_SHADER_BYTECODE((void*)PCVertexPipeline::g_RSForward, sizeof(PCVertexPipeline::g_RSForward));
				info.inputLayouts = {
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				};
				auto result = info.createWithBytecode(pDevice, pDesc, nodeMask);
				result.setAutoReplaceRootParameterIndices({ 2 });
				return result;
			}

			PipelineSet ForwardPipeline::sCreatePTVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, UINT nodeMask)
			{
				PipelineInfo info;
				info.bytecodes[PipelineInfo::eVERTEX] = CD3DX12_SHADER_BYTECODE((void*)PTVertexPipeline::g_VSMain, sizeof(PTVertexPipeline::g_VSMain));
				info.bytecodes[PipelineInfo::ePIXEL] = CD3DX12_SHADER_BYTECODE((void*)PTVertexPipeline::g_PSForward, sizeof(PTVertexPipeline::g_PSForward));
				info.bytecodes[PipelineInfo::eROOT_SIGNATURE] = CD3DX12_SHADER_BYTECODE((void*)PTVertexPipeline::g_RSForward, sizeof(PTVertexPipeline::g_RSForward));
				info.inputLayouts = {
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				};
				auto result = info.createWithBytecode(pDevice, pDesc, nodeMask);
				result.setAutoReplaceRootParameterIndices({ 2, 3 });
				return result;
			}

			PipelineSet ForwardPipeline::sCreatePNVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, UINT nodeMask)
			{
				PipelineInfo info;
				info.bytecodes[PipelineInfo::eVERTEX] = CD3DX12_SHADER_BYTECODE((void*)PNVertexPipeline::g_VSMain, sizeof(PNVertexPipeline::g_VSMain));
				info.bytecodes[PipelineInfo::ePIXEL] = CD3DX12_SHADER_BYTECODE((void*)PNVertexPipeline::g_PSForward, sizeof(PNVertexPipeline::g_PSForward));
				info.bytecodes[PipelineInfo::eROOT_SIGNATURE] = CD3DX12_SHADER_BYTECODE((void*)PNVertexPipeline::g_RSForward, sizeof(PNVertexPipeline::g_RSForward));
				info.inputLayouts = {
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				};
				return info.createWithBytecode(pDevice, pDesc, nodeMask);
			}

			PipelineSet ForwardPipeline::sCreatePTNVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, UINT nodeMask)
			{
				PipelineInfo info;
				info.bytecodes[PipelineInfo::eVERTEX] = CD3DX12_SHADER_BYTECODE((void*)PTNVertexPipeline::g_VSMain, sizeof(PTNVertexPipeline::g_VSMain));
				info.bytecodes[PipelineInfo::ePIXEL] = CD3DX12_SHADER_BYTECODE((void*)PTNVertexPipeline::g_PSForward, sizeof(PTNVertexPipeline::g_PSForward));
				info.bytecodes[PipelineInfo::eROOT_SIGNATURE] = CD3DX12_SHADER_BYTECODE((void*)PTNVertexPipeline::g_RSForward, sizeof(PTNVertexPipeline::g_RSForward));
				info.inputLayouts = {
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				};
				auto result = info.createWithBytecode(pDevice, pDesc, nodeMask);
				result.setAutoReplaceRootParameterIndices({ 2, 3 });
				return result;
			}

			PipelineSet ForwardPipeline::sCreatePTNCVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, UINT nodeMask)
			{
				PipelineInfo info;
				info.bytecodes[PipelineInfo::eVERTEX] = CD3DX12_SHADER_BYTECODE((void*)PTNCVertexPipeline::g_VSMain, sizeof(PTNCVertexPipeline::g_VSMain));
				info.bytecodes[PipelineInfo::ePIXEL] = CD3DX12_SHADER_BYTECODE((void*)PTNCVertexPipeline::g_PSForward, sizeof(PTNCVertexPipeline::g_PSForward));
				info.bytecodes[PipelineInfo::eROOT_SIGNATURE] = CD3DX12_SHADER_BYTECODE((void*)PTNCVertexPipeline::g_RSForward, sizeof(PTNCVertexPipeline::g_RSForward));
				info.inputLayouts = {
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				};
				auto result = info.createWithBytecode(pDevice, pDesc, nodeMask);
				result.setAutoReplaceRootParameterIndices({2, 3});
				return result;
			}
		}

		namespace utility
		{
			static std::initializer_list<DXGI_FORMAT> g_defferedRTFormats = {
				DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R11G11B10_FLOAT, DXGI_FORMAT_R8G8B8A8_UNORM
			};

			PipelineSet DefferedPipeline::sCreatePVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, UINT nodeMask)
			{
				assert(false && "未実装");
				PipelineInfo info;
				info.bytecodes[PipelineInfo::eVERTEX] = CD3DX12_SHADER_BYTECODE((void*)PVertexPipeline::g_VSDeffered, sizeof(PVertexPipeline::g_VSDeffered));
				info.bytecodes[PipelineInfo::ePIXEL] = CD3DX12_SHADER_BYTECODE((void*)PVertexPipeline::g_PSDeffered, sizeof(PVertexPipeline::g_PSDeffered));
				info.bytecodes[PipelineInfo::eROOT_SIGNATURE] = CD3DX12_SHADER_BYTECODE((void*)PVertexPipeline::g_RSForward, sizeof(PVertexPipeline::g_RSForward));
				info.inputLayouts = {
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				};
				auto result = info.createWithBytecode(pDevice, pDesc, nodeMask, g_defferedRTFormats);
				result.setAutoReplaceRootParameterIndices({ 2 });
				return result;
			}

			PipelineSet DefferedPipeline::sCreatePCVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, UINT nodeMask)
			{
				assert(false && "未実装");
				PipelineInfo info;
				info.bytecodes[PipelineInfo::eVERTEX] = CD3DX12_SHADER_BYTECODE((void*)PCVertexPipeline::g_VSDeffered, sizeof(PCVertexPipeline::g_VSDeffered));
				info.bytecodes[PipelineInfo::ePIXEL] = CD3DX12_SHADER_BYTECODE((void*)PCVertexPipeline::g_PSDeffered, sizeof(PCVertexPipeline::g_PSDeffered));
				info.bytecodes[PipelineInfo::eROOT_SIGNATURE] = CD3DX12_SHADER_BYTECODE((void*)PCVertexPipeline::g_RSForward, sizeof(PCVertexPipeline::g_RSForward));
				info.inputLayouts = {
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				};
				auto result = info.createWithBytecode(pDevice, pDesc, nodeMask, g_defferedRTFormats);
				result.setAutoReplaceRootParameterIndices({ 2 });
				return result;
			}

			PipelineSet DefferedPipeline::sCreatePTVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, UINT nodeMask)
			{
				assert(false && "未実装");
				PipelineInfo info;
				info.bytecodes[PipelineInfo::eVERTEX] = CD3DX12_SHADER_BYTECODE((void*)PTVertexPipeline::g_VSDeffered, sizeof(PTVertexPipeline::g_VSDeffered));
				info.bytecodes[PipelineInfo::ePIXEL] = CD3DX12_SHADER_BYTECODE((void*)PTVertexPipeline::g_PSDeffered, sizeof(PTVertexPipeline::g_PSDeffered));
				info.bytecodes[PipelineInfo::eROOT_SIGNATURE] = CD3DX12_SHADER_BYTECODE((void*)PTVertexPipeline::g_RSForward, sizeof(PTVertexPipeline::g_RSForward));
				info.inputLayouts = {
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				};
				auto result = info.createWithBytecode(pDevice, pDesc, nodeMask, g_defferedRTFormats);
				result.setAutoReplaceRootParameterIndices({ 2, 3 });
				return result;
			}

			PipelineSet DefferedPipeline::sCreatePNVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, UINT nodeMask)
			{
				PipelineInfo info;
				info.bytecodes[PipelineInfo::eVERTEX] = CD3DX12_SHADER_BYTECODE((void*)PNVertexPipeline::g_VSMain, sizeof(PNVertexPipeline::g_VSMain));
				info.bytecodes[PipelineInfo::ePIXEL] = CD3DX12_SHADER_BYTECODE((void*)PNVertexPipeline::g_PSDeffered, sizeof(PNVertexPipeline::g_PSDeffered));
				info.bytecodes[PipelineInfo::eROOT_SIGNATURE] = CD3DX12_SHADER_BYTECODE((void*)PNVertexPipeline::g_RSDeffered, sizeof(PNVertexPipeline::g_RSDeffered));
				info.inputLayouts = {
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				};
				return info.createWithBytecode(pDevice, pDesc, nodeMask, g_defferedRTFormats);
			}

			PipelineSet DefferedPipeline::sCreatePTNVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, UINT nodeMask)
			{
				PipelineInfo info;
				info.bytecodes[PipelineInfo::eVERTEX] = CD3DX12_SHADER_BYTECODE((void*)PTNVertexPipeline::g_VSMain, sizeof(PTNVertexPipeline::g_VSMain));
				info.bytecodes[PipelineInfo::ePIXEL] = CD3DX12_SHADER_BYTECODE((void*)PTNVertexPipeline::g_PSDeffered, sizeof(PTNVertexPipeline::g_PSDeffered));
				info.bytecodes[PipelineInfo::eROOT_SIGNATURE] = CD3DX12_SHADER_BYTECODE((void*)PTNVertexPipeline::g_RSDeffered, sizeof(PTNVertexPipeline::g_RSDeffered));
				info.inputLayouts = {
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				};
				auto result = info.createWithBytecode(pDevice, pDesc, nodeMask, g_defferedRTFormats);
				result.setAutoReplaceRootParameterIndices({2, 3});
				return result;
			}

			PipelineSet DefferedPipeline::sCreatePTNCVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, UINT nodeMask)
			{
				PipelineInfo info;
				info.bytecodes[PipelineInfo::eVERTEX] = CD3DX12_SHADER_BYTECODE((void*)PTNCVertexPipeline::g_VSMain, sizeof(PTNCVertexPipeline::g_VSMain));
				info.bytecodes[PipelineInfo::ePIXEL] = CD3DX12_SHADER_BYTECODE((void*)PTNCVertexPipeline::g_PSDeffered, sizeof(PTNCVertexPipeline::g_PSDeffered));
				info.bytecodes[PipelineInfo::eROOT_SIGNATURE] = CD3DX12_SHADER_BYTECODE((void*)PTNCVertexPipeline::g_RSDeffered, sizeof(PTNCVertexPipeline::g_RSDeffered));
				info.inputLayouts = {
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				};
				auto result = info.createWithBytecode(pDevice, pDesc, nodeMask, g_defferedRTFormats);
				result.setAutoReplaceRootParameterIndices({ 2, 3 });
				return result;
			}

		}

		namespace utility
		{
			PipelineSet ZPassPipeline::sCreatePVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, UINT nodeMask)
			{
				PipelineInfo info;
				info.bytecodes[PipelineInfo::eVERTEX] = CD3DX12_SHADER_BYTECODE((void*)PVertexPipeline::g_VSZPass, sizeof(PVertexPipeline::g_VSZPass));
				info.bytecodes[PipelineInfo::eROOT_SIGNATURE] = CD3DX12_SHADER_BYTECODE((void*)PVertexPipeline::g_RSZPass, sizeof(PVertexPipeline::g_RSZPass));
				info.inputLayouts = {
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				};

				return info.createWithBytecode(pDevice, pDesc, nodeMask, {});
			}

			PipelineSet ZPassPipeline::sCreatePCVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, UINT nodeMask)
			{
				PipelineInfo info;
				info.bytecodes[PipelineInfo::eVERTEX] = CD3DX12_SHADER_BYTECODE((void*)PCVertexPipeline::g_VSZPass, sizeof(PCVertexPipeline::g_VSZPass));
				info.bytecodes[PipelineInfo::eROOT_SIGNATURE] = CD3DX12_SHADER_BYTECODE((void*)PCVertexPipeline::g_RSZPass, sizeof(PCVertexPipeline::g_RSZPass));
				info.inputLayouts = {
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				};
				return info.createWithBytecode(pDevice, pDesc, nodeMask, {});
			}

			PipelineSet ZPassPipeline::sCreatePTVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, UINT nodeMask)
			{
				PipelineInfo info;
				info.bytecodes[PipelineInfo::eVERTEX] = CD3DX12_SHADER_BYTECODE((void*)PTVertexPipeline::g_VSZPass, sizeof(PTVertexPipeline::g_VSZPass));
				info.bytecodes[PipelineInfo::eROOT_SIGNATURE] = CD3DX12_SHADER_BYTECODE((void*)PTVertexPipeline::g_RSZPass, sizeof(PTVertexPipeline::g_RSZPass));
				info.inputLayouts = {
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				};
				return info.createWithBytecode(pDevice, pDesc, nodeMask, {});
			}

			PipelineSet ZPassPipeline::sCreatePNVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, UINT nodeMask)
			{
				PipelineInfo info;
				info.bytecodes[PipelineInfo::eVERTEX] = CD3DX12_SHADER_BYTECODE((void*)PNVertexPipeline::g_VSZPass, sizeof(PNVertexPipeline::g_VSZPass));
				info.bytecodes[PipelineInfo::eROOT_SIGNATURE] = CD3DX12_SHADER_BYTECODE((void*)PNVertexPipeline::g_RSZPass, sizeof(PNVertexPipeline::g_RSZPass));
				info.inputLayouts = {
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				};
				return info.createWithBytecode(pDevice, pDesc, nodeMask, {});
			}

			PipelineSet ZPassPipeline::sCreatePTNVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, UINT nodeMask)
			{
				PipelineInfo info;
				info.bytecodes[PipelineInfo::eVERTEX] = CD3DX12_SHADER_BYTECODE((void*)PTNVertexPipeline::g_VSZPass, sizeof(PTNVertexPipeline::g_VSZPass));
				info.bytecodes[PipelineInfo::eROOT_SIGNATURE] = CD3DX12_SHADER_BYTECODE((void*)PTNVertexPipeline::g_RSZPass, sizeof(PTNVertexPipeline::g_RSZPass));
				info.inputLayouts = {
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				};
				return info.createWithBytecode(pDevice, pDesc, nodeMask, {});
			}

			PipelineSet ZPassPipeline::sCreatePTNCVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, UINT nodeMask)
			{
				PipelineInfo info;
				info.bytecodes[PipelineInfo::eVERTEX] = CD3DX12_SHADER_BYTECODE((void*)PTNCVertexPipeline::g_VSZPass, sizeof(PTNCVertexPipeline::g_VSZPass));
				info.bytecodes[PipelineInfo::eROOT_SIGNATURE] = CD3DX12_SHADER_BYTECODE((void*)PTNCVertexPipeline::g_RSZPass, sizeof(PTNCVertexPipeline::g_RSZPass));
				info.inputLayouts = {
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				};
				return info.createWithBytecode(pDevice, pDesc, nodeMask, {});
			}
		}
	}
}