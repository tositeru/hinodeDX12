#include "stdafx.h"

#include "VolumetricLightManager.h"

#include <pix.h>
#include <graphics/dx12/utility/Shader/Shader.h>
#include <graphics/dx12/pipelineState/DX12PipelineState.h>
#include <graphics/dx12/sampler/DX12Sampler.h>

using namespace hinode;
using namespace hinode::graphics;

VolumetricLightingManager::VolumetricLightingManager()
{ }

VolumetricLightingManager::~VolumetricLightingManager()
{
	this->clear();
}

void VolumetricLightingManager::clear()
{
	for (auto& pipeline : this->mPipelines) {
		pipeline.clear();
	}
	for (auto& resource : this->mResources) {
		resource.clear();
	}
}

void VolumetricLightingManager::create(ID3D12Device* pDevice, const InitParam& param, hinode::graphics::utility::ResourceUploader& uploader, UINT nodeMask)
{
	{//パイプラインの初期化
		auto compileCSPipeline = [](utility::PipelineSet* pOut, ID3D12Device* pDevice, const std::wstring& shaderFilepath, UINT nodeMask) {
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

		compileCSPipeline(&this->mPipelines[ePIPELINE_VOLUMETRIC_LIGHTING], pDevice, L"scene/DemoScene/Shader/volumetricLight/CSDirectionalVolumeLighting.hlsl", nodeMask);


		{///スキャッタリング計算で使うシーンの粒子濃度を表すボクセルを構築するためのパイプライン
			utility::Shader vs, gs, ps, rootSignature;
			utility::Shader::CompileParam compileParam;
			vs.load(L"scene/DemoScene/Shader/volumetricLight/BuildScatteringAmountGrid.hlsl", compileParam.setEntryAndModel("VSMain", "vs_5_1"));
			gs.load(L"scene/DemoScene/Shader/volumetricLight/BuildScatteringAmountGrid.hlsl", compileParam.setEntryAndModel("GSMain", "gs_5_1"));
			ps.load(L"scene/DemoScene/Shader/volumetricLight/BuildScatteringAmountGrid.hlsl", compileParam.setEntryAndModel("PSMain", "ps_5_1"));
			rootSignature.load(L"scene/DemoScene/Shader/volumetricLight/BuildScatteringAmountGrid.hlsl", compileParam.setEntryAndModel("RS", "rootsig_1_1"));
			CD3DX12_BLEND_DESC blendDesc(D3D12_DEFAULT);
			blendDesc.RenderTarget[0].BlendEnable = true;
			blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
			blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
			blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
			DX12DepthStencilDesc depthStencilDesc;
			depthStencilDesc
				.setDepth(false)
				.isEnableStencil(false);
			DX12_GRAPHICS_PIPELINE_STATE_DESC graphicsDesc;
			graphicsDesc
				.setVS(vs.blob())
				.setGS(gs.blob())
				.setPS(ps.blob())
				.setBlend(&blendDesc)
				.setDepthStencil(&depthStencilDesc)
				.setRTV({DXGI_FORMAT_R16_FLOAT})
				.setPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT);
			this->mPipelines[ePIPELINE_BUILD_AMOUNT_GRID].create(pDevice, &graphicsDesc, rootSignature, nodeMask);
		}
	}

	{//リソース作成
		auto setView = [](DX12Resource* pOut, const wchar_t* name) {
			pOut->appendView(DX12Resource::View().setSRV(pOut->makeSRV()));
			pOut->appendView(DX12Resource::View().setUAV(pOut->makeUAV()));
			pOut->setName(name);
		};

		{//ボクセル
			DX12ResourceDesc resourceDesc;
			resourceDesc.desc = CD3DX12_RESOURCE_DESC::Tex3D(DXGI_FORMAT_R16_FLOAT, param.gridSize.x, param.gridSize.y, param.gridSize.z, 1);
			resourceDesc.desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			resourceDesc.initialStates = D3D12_RESOURCE_STATE_RENDER_TARGET;
			resourceDesc.setClearValueForColor(resourceDesc.desc.Format, 0, 0, 0, 0);
			DX12ResourceHeapDesc heapDesc;
			this->mResources[eRESOURCE_AMOUNT_GRID].create(pDevice, heapDesc, resourceDesc);
			this->mResources[eRESOURCE_AMOUNT_GRID].appendView(DX12Resource::View().setSRV(this->mResources[eRESOURCE_AMOUNT_GRID].makeSRV()));
			this->mResources[eRESOURCE_AMOUNT_GRID].appendView(DX12Resource::View().setRTV(this->mResources[eRESOURCE_AMOUNT_GRID].makeRTV()));
			this->mResources[eRESOURCE_AMOUNT_GRID].setName(L"VolumetricLightingManager::eRESOURCE_AMOUNT_GRID");

			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtvHeapDesc.NodeMask = nodeMask;
			rtvHeapDesc.NumDescriptors = 1;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			this->mGridRTVHeap.create(pDevice, &rtvHeapDesc);

			auto cpuHandle = this->mGridRTVHeap.makeCpuHandle(0);
			cpuHandle.createRenderTargetView(pDevice, this->mResources[eRESOURCE_AMOUNT_GRID], &this->mResources[eRESOURCE_AMOUNT_GRID].view(eVIEW_RTV).rtv());
		}
		{//粒子パーティクル
			UINT bufferSize = static_cast<UINT>(sizeof(this->mParticles[0]) * param.particleCount);
			DX12ResourceDesc resourceDesc;
			resourceDesc.desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
			resourceDesc.initialStates = D3D12_RESOURCE_STATE_COPY_DEST;
			DX12ResourceHeapDesc heapDesc;
			this->mResources[eRESOURCE_AMOUNT_PARTICLE].create(pDevice, heapDesc, resourceDesc);
			DX12Resource::MakeViewDesc srvViewDesc;
			srvViewDesc.elementCount = static_cast<UINT>(param.particleCount);
			srvViewDesc.structureByteStride = sizeof(this->mParticles[0]);
			this->mResources[eRESOURCE_AMOUNT_PARTICLE].appendView(DX12Resource::View().setSRV(this->mResources[eRESOURCE_AMOUNT_PARTICLE].makeSRV(srvViewDesc)));
			this->mResources[eRESOURCE_AMOUNT_PARTICLE].setName(L"VolumetricLightingManager::eRESOURCE_AMOUNT_PARTICLE");

			size_t particleCount = 10000;
			this->mParticles.reserve(particleCount);
			for (auto i = 0; i < particleCount; ++i) {
				buildScatteringAmount::ScatteringAmountParticle p;
				math::float3 center(0, 0, 10.f);
				p.pos = math::setRandom<math::float3>(-30.f, 30.f);
				p.pos += center;
				p.size = math::simpleRandom(1.f, 5.f);
				p.amount = math::simpleRandom(0.5f, 1.f);

				this->mParticles.push_back(p);
			}

			this->mResources[eRESOURCE_AMOUNT_PARTICLE_FOR_UPLOAD].createForCopy(pDevice, this->mResources[eRESOURCE_AMOUNT_PARTICLE], this->mParticles.data(), sizeof(this->mParticles[0]) * this->mParticles.size());
		}
		{//ティザーパターン
			DX12ResourceDesc resourceDesc;
			resourceDesc.desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8_UINT, 4, 4, 1, 1);
			resourceDesc.initialStates = D3D12_RESOURCE_STATE_COPY_DEST;
			DX12ResourceHeapDesc heapDesc;
			this->mResources[eRESOURCE_DITHERED_PATTERN].create(pDevice, heapDesc, resourceDesc);
			this->mResources[eRESOURCE_DITHERED_PATTERN].appendView(DX12Resource::View().setSRV(this->mResources[eRESOURCE_DITHERED_PATTERN].makeSRV()));

			uint8_t pattern[4][4] = {
				{ 0, 8, 2, 10 },
				{ 12, 4, 14, 6 },
				{ 3, 11, 1, 9 },
				{ 15, 7, 13, 5 },
			};
			utility::ResourceUploader::TextureDesc uploaderDesc;
			uploaderDesc.pData = pattern;
			uploaderDesc.rowPitch = sizeof(uint8_t) * 4;
			uploaderDesc.subresource = 0;
			uploader.entryTexture(this->mResources[eRESOURCE_DITHERED_PATTERN], uploaderDesc);
		}
		{//合成結果
			DX12ResourceDesc resourceDesc;
			resourceDesc.desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, param.screenWidth, param.screenHeight, 1, 1);
			resourceDesc.desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			resourceDesc.initialStates = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			DX12ResourceHeapDesc heapDesc;
			this->mResources[eRESOURCE_FINAL].create(pDevice, heapDesc, resourceDesc);
			setView(&this->mResources[eRESOURCE_FINAL], L"VolumetricLightingManager::eRESOURCE_FINAL");
		}
	}
}

#include "Shader/volumetricLight/BuildScatteringAmountGrid.hlsl"
#include "Shader/volumetricLight/CSDirectionalVolumeLighting.hlsl"

void VolumetricLightingManager::run(hinode::graphics::DX12GraphicsCommandList& cmdList, hinode::graphics::utility::DescriptorHeapPoolSet& DHPool, hinode::graphics::utility::ConstantBufferPool& CBPool, const Param& param)
{
	PIXBeginEvent(cmdList, 0, L"VolumetricLightingManager::run");
	{//パーティクルの更新
		cmdList.setBarriers(
			this->mResources[eRESOURCE_AMOUNT_PARTICLE].makeBarrier(D3D12_RESOURCE_STATE_COPY_DEST)
		);
		cmdList->CopyResource(this->mResources[eRESOURCE_AMOUNT_PARTICLE], this->mResources[eRESOURCE_AMOUNT_PARTICLE_FOR_UPLOAD]);
	}

	//共通パラメータ
	static float timer = 0.f;
	timer += 0.01f;
	volumetricLightingGridCommon::BuildScatteringAmountGridParam gridParam;
	gridParam.viewMatrix = param.gridViewMatrix;
	gridParam.projMatrix = param.gridProjMatrix;
	gridParam.invProjMatrix = math::inverse(gridParam.projMatrix);
	gridParam.curveture = param.gridCurveture;
	gridParam.depthRange = param.gridDepthRange;
	gridParam.depthDivideCount = static_cast<float>(this->mResources[eRESOURCE_AMOUNT_GRID]->GetDesc().DepthOrArraySize);
	gridParam.timer = timer;
	auto gridParamAllocateInfo = CBPool.allocate(&gridParam, sizeof(gridParam));

	{//スキャッタリング量グリッドの構築
		PIXBeginEvent(cmdList, 0, L"build grid");

		auto& gridTex = this->mResources[eRESOURCE_AMOUNT_GRID];
		D3D12_VIEWPORT vp;
		vp.Width = static_cast<float>(gridTex->GetDesc().Width);
		vp.Height = static_cast<float>(gridTex->GetDesc().Height);
		vp.TopLeftX = vp.TopLeftY = 0.f;
		vp.MinDepth = 0.f;
		vp.MaxDepth = 1.f;
		cmdList->RSSetViewports(1, &vp);
		D3D12_RECT scissorRect;
		scissorRect.top = 0;
		scissorRect.left = 0;
		scissorRect.right = static_cast<decltype(scissorRect.right)>(vp.Width);
		scissorRect.bottom = static_cast<decltype(scissorRect.right)>(vp.Height);
		cmdList->RSSetScissorRects(1, &scissorRect);

		cmdList.setBarriers(
			gridTex.makeBarrier(D3D12_RESOURCE_STATE_RENDER_TARGET),
			this->mResources[eRESOURCE_AMOUNT_PARTICLE].makeBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
		);

		auto rtvHandle = this->mGridRTVHeap.makeCpuHandle(0);
		cmdList->OMSetRenderTargets(1, &rtvHandle.d3dx12Handle, true, nullptr);
		cmdList->ClearRenderTargetView(rtvHandle.d3dx12Handle, gridTex.clearValue().Color, 0, nullptr);

		auto& pipeline = this->mPipelines[ePIPELINE_BUILD_AMOUNT_GRID];
		pipeline.updateCBV("CBBuildScatteringAmountGridParam", DHPool, gridParamAllocateInfo);
		pipeline.updateSRV("bufParticles", DHPool, this->mResources[eRESOURCE_AMOUNT_PARTICLE], &this->mResources[eRESOURCE_AMOUNT_PARTICLE].view(eVIEW_SRV).srv());
		pipeline.bind(cmdList);

		cmdList->IASetVertexBuffers(0, 1, nullptr);
		cmdList->IASetIndexBuffer(nullptr);
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		cmdList->DrawInstanced(static_cast<UINT>(this->mParticles.size()), 1, 0, 0);

		PIXEndEvent(cmdList);
	}
	{
		PIXBeginEvent(cmdList, 0, L"cal volumetric lighting");

		cmdList.setBarriers(
			param.pSceneImage->makeBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
			param.pSceneDepth->makeBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
			param.pShadowMap->makeBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
			this->mResources[eRESOURCE_AMOUNT_GRID].makeBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
			this->mResources[eRESOURCE_FINAL].makeBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
		);

		auto width = static_cast<UINT>(this->mResources[eRESOURCE_FINAL]->GetDesc().Width);
		auto height = static_cast<UINT>(this->mResources[eRESOURCE_FINAL]->GetDesc().Height);
		auto& pipeline = this->mPipelines[ePIPELINE_VOLUMETRIC_LIGHTING];

		directionalVolumetricLighting::ScatteringParam scatteringParam;
		scatteringParam.cbRayMarchCount = param.raymarchCount;
		pipeline.updateCBV("ScatteringParam", DHPool, &scatteringParam, sizeof(scatteringParam), CBPool);
		directionalVolumetricLighting::CameraParam cameraParam;
		cameraParam.cbCameraProj = param.gridProjMatrix;
		cameraParam.cbInvProj = math::inverse(cameraParam.cbCameraProj);
		pipeline.updateCBV("CameraParam", DHPool, &cameraParam, sizeof(cameraParam), CBPool);
		directionalVolumetricLighting::CBScatteringLightParam lightParam;
		lightParam.cbScatteringLightParam.lightDir = param.lightDir;
		lightParam.cbScatteringLightParam.lightColor = param.lightColor;
		lightParam.cbScatteringLightParam.scatteringAmount = param.scatteringAmount;
		lightParam.cbScatteringLightParam.G = param.scatteringG;
		lightParam.cbScatteringLightParam.toShadowMapSpaceMat = param.toShadowmapSpaceMat;
		pipeline.updateCBV("CBScatteringLightParam", DHPool, &lightParam, sizeof(lightParam), CBPool);
		pipeline.updateCBV("CBBuildScatteringAmountGridParam", DHPool, gridParamAllocateInfo);
		pipeline.updateSRV("texDepth", DHPool, *param.pSceneDepth, &param.pSceneDepth->view(param.sceneDepthSRVIndex).srv());
		pipeline.updateSRV("texSceneImage", DHPool, *param.pSceneImage, &param.pSceneImage->view(param.sceneImageSRVIndex).srv());
		pipeline.updateSRV("texShadowMap", DHPool, *param.pShadowMap, &param.pShadowMap->view(param.shadowMapSRVIndex).srv());
		pipeline.updateSRV("texDitheredPattern", DHPool, this->mResources[eRESOURCE_DITHERED_PATTERN], &this->mResources[eRESOURCE_DITHERED_PATTERN].view(eVIEW_SRV).srv());
		pipeline.updateSRV("voxelScatteringAmountGrid", DHPool, this->mResources[eRESOURCE_AMOUNT_GRID], &this->mResources[eRESOURCE_AMOUNT_GRID].view(eVIEW_SRV).srv());
		pipeline.updateUAV("outColor", DHPool, this->mResources[eRESOURCE_FINAL], &this->mResources[eRESOURCE_FINAL].view(eVIEW_UAV).uav());
		DX12SamplerDesc linearClampDesc;
		linearClampDesc
			.setFilter(D3D12_FILTER_MIN_MAG_MIP_LINEAR)
			.setAddress(D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER)
			.setBorderColor(1.f, 1.f, 1.f, 1.f);
		pipeline.updateSampler("samLinerClamp", DHPool, &linearClampDesc);
		pipeline.bind(cmdList);

		auto threadCount = directionalVolumetricLighting::getThreadCount();
		auto dx = DX12PipelineState::sCalDispatchCount(width, threadCount.x);
		auto dy = DX12PipelineState::sCalDispatchCount(height, threadCount.y);
		cmdList->Dispatch(dx, dy, 1);

		PIXEndEvent(cmdList);
	}
	PIXEndEvent(cmdList);
}

DX12Resource& VolumetricLightingManager::result()noexcept
{
	return this->mResources[eRESOURCE_FINAL];
}
