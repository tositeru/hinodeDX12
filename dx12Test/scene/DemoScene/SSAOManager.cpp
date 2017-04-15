#include <stdafx.h>

#include "SSAOManager.h"

#include <pix.h>
#include <graphics/dx12/utility/Shader/Shader.h>
#include <graphics/dx12/pipelineState/DX12PipelineState.h>

using namespace hinode;
using namespace hinode::graphics;

SSAOManager::SSAOManager()
{

}

SSAOManager::~SSAOManager()
{

}

void SSAOManager::clear()
{

}

void SSAOManager::create(ID3D12Device* pDevice, UINT width, UINT height, UINT nodeMask)
{
	{//
		auto compilePipeline = [](utility::PipelineSet* pOut, ID3D12Device* pDevice, const std::wstring& shaderFilepath, UINT nodeMask) {
			utility::Shader shader, rootSignature;
			utility::Shader::CompileParam compileParam;
			shader.load(shaderFilepath, compileParam.setEntryAndModel("main", "cs_5_1"));
			rootSignature.load(shaderFilepath, compileParam.setEntryAndModel("RS", "rootsig_1_1"));
			DX12_COMPUTE_PIPELINE_STATE_DESC pipelineDesc;
			pipelineDesc
				.setCS(shader.blob())
				;
			pOut->create(pDevice, &pipelineDesc, rootSignature, nodeMask);
		};

		compilePipeline(&this->mPipelines[ePIPELINE_CAL_VIEW_Z], pDevice, L"scene/DemoScene/Shader/ssao/CSCalViewZ.hlsl", nodeMask);
		compilePipeline(&this->mPipelines[ePIPELINE_CAL_AO], pDevice, L"scene/DemoScene/Shader/ssao/CSCalAO.hlsl", nodeMask);
		compilePipeline(&this->mPipelines[ePIPELINE_CAL_AO_BY_CROSS], pDevice, L"scene/DemoScene/Shader/ssao/CSCalAOByCross.hlsl", nodeMask);
		compilePipeline(&this->mPipelines[ePIPELINE_BLUR_H], pDevice, L"scene/DemoScene/Shader/ssao/CSBlurH.hlsl", nodeMask);
		compilePipeline(&this->mPipelines[ePIPELINE_BLUR_V], pDevice, L"scene/DemoScene/Shader/ssao/CSBlurV.hlsl", nodeMask);
		compilePipeline(&this->mPipelines[ePIPELINE_COMPOSITION], pDevice, L"scene/DemoScene/Shader/ssao/CSComposite.hlsl", nodeMask);
		compilePipeline(&this->mPipelines[ePIPELINE_DEBUG_SHOW_CROSS_SAMPLING], pDevice, L"scene/DemoScene/Shader/ssao/CSDebugShowCrossSampling.hlsl", nodeMask);
		compilePipeline(&this->mPipelines[ePIPELINE_DEBUG_SHOW_RANDOM_SAMPLING], pDevice, L"scene/DemoScene/Shader/ssao/CSDebugShowRandomSampling.hlsl", nodeMask);
	}

	{//リソースの初期化
		auto createTex = [](DX12Resource* pOut, ID3D12Device* pDevice, DXGI_FORMAT format, UINT width, UINT height, const wchar_t* name) {
			DX12ResourceDesc resourceDesc;
			resourceDesc.desc = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, 1, 1);
			resourceDesc.desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			resourceDesc.initialStates = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			DX12ResourceHeapDesc heapDesc;
			pOut->create(pDevice, heapDesc, resourceDesc);
			pOut->appendView(DX12Resource::View().setSRV(pOut->makeSRV()));
			pOut->appendView(DX12Resource::View().setUAV(pOut->makeUAV()));
			pOut->setName(name);
		};
		createTex(&this->mResource[eRESOURCE_VIEW_Z], pDevice, DXGI_FORMAT_R32_FLOAT, width, height, L"eRESOURCE_VIEW_Z");
		createTex(&this->mResource[eRESOURCE_AO], pDevice, DXGI_FORMAT_R16_FLOAT, width, height, L"eRESOURCE_AO");
		createTex(&this->mResource[eRESOURCE_BLUR_1], pDevice, DXGI_FORMAT_R16_FLOAT, width, height, L"eRESOURCE_BLUR_1");
		createTex(&this->mResource[eRESOURCE_BLUR_2], pDevice, DXGI_FORMAT_R16_FLOAT, width, height, L"eRESOURCE_BLUR_2");
		createTex(&this->mResource[eRESOURCE_FINAL], pDevice, DXGI_FORMAT_R8G8B8A8_UNORM, width, height, L"eRESOURCE_FINAL");
	}
}

#include "Shader/ssao/CSCalViewZ.hlsl"
#include "Shader/ssao/CSCalAO.hlsl"
#include "Shader/ssao/CSBlurH.hlsl"
#include "Shader/ssao/CSBlurV.hlsl"
#include "Shader/ssao/CSComposite.hlsl"

void SSAOManager::run(hinode::graphics::DX12GraphicsCommandList& cmdList, hinode::graphics::utility::DescriptorHeapPoolSet& DHPool, hinode::graphics::utility::ConstantBufferPool& CBPool, const Param& param)
{
	PIXBeginEvent(cmdList, 0, L"SSAOManager::run");

	//パラメータチェック
	auto width = static_cast<UINT>(this->mResource[0]->GetDesc().Width);
	auto height = static_cast<UINT>(this->mResource[0]->GetDesc().Height);


	{//ビュー空間の深度値を求める
		PIXBeginEvent(cmdList, 0, L"cal view z");
		auto& pipeline = this->mPipelines[ePIPELINE_CAL_VIEW_Z];

		cmdList.setBarriers(
			param.pSceneDepth->makeBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
			this->mResource[eRESOURCE_VIEW_Z].makeBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
		);

		calViewZ::Param p;
		p.cbNearFarZ = param.nearFarZ;
		pipeline.updateCBV("Param", DHPool, &p, sizeof(p), CBPool);
		pipeline.updateSRV("tDepth", DHPool, *param.pSceneDepth, &param.pSceneDepth->view(param.sceneDepthSRVIndex).srv());
		pipeline.updateUAV("uOutViewZ", DHPool, this->mResource[eRESOURCE_VIEW_Z], &this->mResource[eRESOURCE_VIEW_Z].view(eVIEW_UAV).uav());
		pipeline.bind(cmdList);

		auto threadCount = calViewZ::getThreadCount();
		auto dx = DX12PipelineState::sCalDispatchCount(width, threadCount.x);
		auto dy = DX12PipelineState::sCalDispatchCount(height, threadCount.y);
		cmdList->Dispatch(dx, dy, 1);
		PIXEndEvent(cmdList);
	}
	{//AO計算
		PIXBeginEvent(cmdList, 0, L"cal ao");
		auto& pipeline = this->mPipelines[ePIPELINE_CAL_AO];

		cmdList.setBarriers(
			this->mResource[eRESOURCE_VIEW_Z].makeBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
			this->mResource[eRESOURCE_AO].makeBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
		);

		ssaoCommon::AOCommonParam sceneParam;
		sceneParam.projXYToView.x = param.projMatrix._11;
		sceneParam.projXYToView.y = param.projMatrix._22;
		sceneParam.samplingRange = param.samplingRadius;
		sceneParam.samplingCount = param.samplingCount;
		sceneParam.samplingStep = param.samplingStep;
		pipeline.updateCBV("CommonParam", DHPool, &sceneParam, sizeof(sceneParam), CBPool);

		ssaoCommon::AlchemyAOParam aoParam;
		aoParam.beta = param.beta;
		aoParam.K = param.K;
		aoParam.radius = param.radiusInWorld;
		aoParam.sigma = param.sigma;
		pipeline.updateCBV("AOParam", DHPool, &aoParam, sizeof(aoParam), CBPool);

		pipeline.updateSRV("tViewZ", DHPool, this->mResource[eRESOURCE_VIEW_Z], &this->mResource[eRESOURCE_VIEW_Z].view(eVIEW_SRV).srv());
		pipeline.updateUAV("uOutAO", DHPool, this->mResource[eRESOURCE_AO], &this->mResource[eRESOURCE_AO].view(eVIEW_UAV).uav());
		pipeline.bind(cmdList);

		auto threadCount = calAO::getThreadCount();
		auto dx = DX12PipelineState::sCalDispatchCount(width, threadCount.x);
		auto dy = DX12PipelineState::sCalDispatchCount(height, threadCount.y);
		cmdList->Dispatch(dx, dy, 1);
		PIXEndEvent(cmdList);
	}
	{//ブラー処理
		blurCommon::BlurCommonParam blurParam;
		blurParam.blurRadius = param.blurRadius;
		blurParam.spaceRange = param.blurRangeInWorld;
		blurParam.gaussianSigma = param.blurGaussianSigma;
		auto blurParamAllocateInfo = CBPool.allocate(&blurParam, sizeof(blurParam));

		{//横ブラー
			PIXBeginEvent(cmdList, 0, L"blur horizontal");
			auto& pipeline = this->mPipelines[ePIPELINE_BLUR_H];

			cmdList.setBarriers(
				this->mResource[eRESOURCE_AO].makeBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
				this->mResource[eRESOURCE_BLUR_1].makeBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
			);

			pipeline.updateCBV("Param", DHPool, blurParamAllocateInfo);
			pipeline.updateSRV("tAO", DHPool, this->mResource[eRESOURCE_AO], &this->mResource[eRESOURCE_AO].view(eVIEW_SRV).srv());
			pipeline.updateSRV("tViewZ", DHPool, this->mResource[eRESOURCE_VIEW_Z], &this->mResource[eRESOURCE_VIEW_Z].view(eVIEW_SRV).srv());
			pipeline.updateUAV("uOut", DHPool, this->mResource[eRESOURCE_BLUR_1], &this->mResource[eRESOURCE_BLUR_1].view(eVIEW_UAV).uav());
			pipeline.bind(cmdList);

			auto threadCount = blurH::getThreadCount();
			threadCount.x -= blurParam.blurRadius * 2;
			auto dx = DX12PipelineState::sCalDispatchCount(width, threadCount.x);
			auto dy = DX12PipelineState::sCalDispatchCount(height, threadCount.y);
			cmdList->Dispatch(dx, dy, 1);
			PIXEndEvent(cmdList);
		}
		{//縦ブラー
			PIXBeginEvent(cmdList, 0, L"blur vertical");
			auto& pipeline = this->mPipelines[ePIPELINE_BLUR_V];

			cmdList.setBarriers(
				this->mResource[eRESOURCE_BLUR_1].makeBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
				this->mResource[eRESOURCE_BLUR_2].makeBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
			);

			pipeline.updateCBV("Param", DHPool, blurParamAllocateInfo);
			pipeline.updateSRV("tAO", DHPool, this->mResource[eRESOURCE_BLUR_1], &this->mResource[eRESOURCE_BLUR_1].view(eVIEW_SRV).srv());
			pipeline.updateSRV("tViewZ", DHPool, this->mResource[eRESOURCE_VIEW_Z], &this->mResource[eRESOURCE_VIEW_Z].view(eVIEW_SRV).srv());
			pipeline.updateUAV("uOut", DHPool, this->mResource[eRESOURCE_BLUR_2], &this->mResource[eRESOURCE_BLUR_2].view(eVIEW_UAV).uav());
			pipeline.bind(cmdList);

			auto threadCount = blurV::getThreadCount();
			threadCount.y -= blurParam.blurRadius * 2;
			auto dx = DX12PipelineState::sCalDispatchCount(width, threadCount.x);
			auto dy = DX12PipelineState::sCalDispatchCount(height, threadCount.y);
			cmdList->Dispatch(dx, dy, 1);
			PIXEndEvent(cmdList);
		}
	}
	{//合成
		PIXBeginEvent(cmdList, 0, L"composite");
		auto& pipeline = this->mPipelines[ePIPELINE_COMPOSITION];

		cmdList.setBarriers(
			param.pSceneImage->makeBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
			this->mResource[eRESOURCE_BLUR_2].makeBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
			this->mResource[eRESOURCE_FINAL].makeBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
		);

		pipeline.updateSRV("tScene", DHPool, *param.pSceneImage, &param.pSceneImage->view(param.sceneImageSRVIndex).srv());
		pipeline.updateSRV("tAO", DHPool, this->mResource[eRESOURCE_BLUR_2], &this->mResource[eRESOURCE_BLUR_2].view(eVIEW_SRV).srv());
		pipeline.updateUAV("uOut", DHPool, this->mResource[eRESOURCE_FINAL], &this->mResource[eRESOURCE_FINAL].view(eVIEW_UAV).uav());
		pipeline.bind(cmdList);

		auto threadCount = composite::getThreadCount();
		auto dx = DX12PipelineState::sCalDispatchCount(width, threadCount.x);
		auto dy = DX12PipelineState::sCalDispatchCount(height, threadCount.y);
		cmdList->Dispatch(dx, dy, 1);
		PIXEndEvent(cmdList);
	}
	PIXEndEvent(cmdList);
}

hinode::graphics::DX12Resource& SSAOManager::result()noexcept
{
	return this->mResource[eRESOURCE_FINAL];
}
