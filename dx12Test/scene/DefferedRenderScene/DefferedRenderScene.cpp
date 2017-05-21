#include "stdafx.h"

#include "DefferedRenderScene.h"

DefferedRenderScene::DefferedRenderScene()
{
}

DefferedRenderScene::~DefferedRenderScene()
{
	this->clear();
}

void DefferedRenderScene::clear()
{
	this->mCBPool.clear();
	this->mDHPool.clear();

	this->mTexture.clear();
	for (auto& pipeline : this->mPipelines) {
		pipeline.clear();
	}
	for (auto& mesh : this->mMeshes) {
		mesh.clear();
	}
	this->mGBuffer.clear();
	this->mDefferedLighting.clear();
	this->mShadowMap.clear();
	//this->mCSDirectionLighting.clear();
	//this->mSceneTexture.clear();
}

void DefferedRenderScene::init(utility::Graphics& graphics, scene::SceneInfo& sceneInfo)
{
	{
		utility::PreviewCamera::InitParam cameraParam;
		cameraParam.pos(0, 0, -5);
		cameraParam.aspect = graphics.calAspect();
		this->mCamera.init(cameraParam);
		this->mCBPool.create(graphics.device(), 1024 * 246);
		this->mDHPool.create(graphics.device(), 256, 128);
	}

	{//各種類の頂点に対応したパイプライン作成
		//this->mPipelines[eVERTEX_P] = utility::DefferedPipeline::sCreatePVertex(graphics.device());
		//this->mPipelines[eVERTEX_PT] = utility::DefferedPipeline::sCreatePTVertex(graphics.device());
		//this->mPipelines[eVERTEX_PC] = utility::DefferedPipeline::sCreatePCVertex(graphics.device());
		this->mPipelines[eVERTEX_PN] = utility::DefferedPipeline::sCreatePNVertex(graphics.device());
		this->mPipelines[eVERTEX_PTN] = utility::DefferedPipeline::sCreatePTNVertex(graphics.device());
		this->mPipelines[eVERTEX_PTNC] = utility::DefferedPipeline::sCreatePTNCVertex(graphics.device());
	}
	{//ZPass用パイプライン
		this->mPipelinesZPass[eVERTEX_PN] = utility::ZPassPipeline::sCreatePNVertex(graphics.device());
		this->mPipelinesZPass[eVERTEX_PTN] = utility::ZPassPipeline::sCreatePTNVertex(graphics.device());
		this->mPipelinesZPass[eVERTEX_PTNC] = utility::ZPassPipeline::sCreatePTNCVertex(graphics.device());
	}

	utility::ResourceUploader uploader;
	uploader.init(graphics.device());
	uploader.loadTextureFromDDS(graphics.device(), this->mTexture, "data/panda.dds");
	this->mTexture.appendView(DX12Resource::View().setSRV(this->mTexture.makeSRV()));

	{//各種類の頂点形式のメッシュ作成
		//utility::TetrahedronCreator::sCreateMesh<utility::PVertex>(&this->mMeshes[eVERTEX_P], graphics.device(), uploader);
		//utility::TetrahedronCreator::sCreateMesh<utility::PTVertex>(&this->mMeshes[eVERTEX_PT], graphics.device(), uploader);
		//utility::TetrahedronCreator::sCreateMesh<utility::PCVertex>(&this->mMeshes[eVERTEX_PC], graphics.device(), uploader);
		utility::TetrahedronCreator::sCreateMesh<utility::PNVertex>(&this->mMeshes[eVERTEX_PN], graphics.device(), uploader);
		utility::TetrahedronCreator::sCreateMesh<utility::PTNVertex>(&this->mMeshes[eVERTEX_PTN], graphics.device(), uploader);
		utility::TetrahedronCreator::sCreateMesh<utility::PTNCVertex>(&this->mMeshes[eVERTEX_PTNC], graphics.device(), uploader);
	}
	utility::PlaneCreator::sCreateMesh<utility::PNVertex>(&this->mGroundMesh, graphics.device(), uploader, utility::PlaneCreator::ePLANE_XZ);

	this->mGBuffer.create(graphics.device(), graphics.swapchain().width(), graphics.swapchain().height());
	this->mDefferedLighting.create(graphics.device(), graphics.swapchain().width(), graphics.swapchain().height());
	this->mShadowMap.create(graphics.device(), 1024, 1024, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT);
}

bool DefferedRenderScene::paint(utility::Graphics& graphics, scene::SceneInfo& sceneInfo)
{
	this->mCamera.update();
	graphics.waitForCurrentFrame();

	graphics.currentContext().record(nullptr, [&](DX12GraphicsCommandList& cmdList) {
		this->mDHPool.bind(cmdList);

		static float t = 0;
		t += 0.01f;
		std::array<math::float4x4, eTYPE_COUNT> worldMatrices = { {
			math::mul(
				math::mul(
					math::makeScale<math::float4x4>({ 1, 1, 1 }),
					math::makeRotationY<math::float4x4>(t)),
				math::makeTranslation(math::float3{-2, 0, 0})
			),
			math::mul(
				math::mul(
					math::makeScale<math::float4x4>({ 1, 1, 1 }),
					math::makeRotationY<math::float4x4>(t)),
				math::makeTranslation(math::float3{ 0, 2, 0 })
			),
			math::mul(
				math::mul(
					math::makeScale<math::float4x4>({ 1, 1, 1 }),
					math::makeRotationY<math::float4x4>(t)),
				math::makeTranslation(math::float3{ 0, -2, 0 })
			)
		} };

		shader::DirectionLightParam mainLight;
		mainLight.direction = math::normalize(math::float3(0, -1, 0));
		mainLight.color(1, 0.7f, 0.7f);
		math::float4x4 lightMatrix;

		{//シャドウマップ
			this->mShadowMap.begin(cmdList);
			shader::CameraParam cbCamera;
			cbCamera.eyePos = -10 * mainLight.direction;
			cbCamera.viewProj = math::mul(
				math::makeView(cbCamera.eyePos, math::float3(0, 0, 0), math::float3(0, 0, 1)),
				math::makeOrthogonal(10, 10, 0.1f, 200.f)
			);
			lightMatrix = cbCamera.viewProj;
			auto lightMatrixAllocateInfo = this->mCBPool.allocate(&cbCamera, sizeof(cbCamera));
			for (auto& pipeline : this->mPipelinesZPass) {
				pipeline.updateCBV("CBCamera", this->mDHPool, lightMatrixAllocateInfo);
			}
			auto drawMesh = [&](DX12GraphicsCommandList& cmdList, utility::Mesh& mesh, utility::PipelineSet& pipeline, const math::float4x4& worldMatrix) {
				pipeline.updateCBV("CBModel", this->mDHPool, &worldMatrix, sizeof(worldMatrix), this->mCBPool);
				pipeline.bind(cmdList);

				mesh.bind(cmdList);
				mesh.draw(cmdList);
			};

			drawMesh(cmdList, this->mMeshes[eVERTEX_PN], this->mPipelinesZPass[eVERTEX_PN], worldMatrices[eVERTEX_PN]);
			drawMesh(cmdList, this->mMeshes[eVERTEX_PTN], this->mPipelinesZPass[eVERTEX_PTN], worldMatrices[eVERTEX_PTN]);
			drawMesh(cmdList, this->mMeshes[eVERTEX_PTNC], this->mPipelinesZPass[eVERTEX_PTNC], worldMatrices[eVERTEX_PTNC]);

			this->mShadowMap.end(cmdList);
		}

		{//GBUffer書き込み
			this->mGBuffer.begin(cmdList);

			//書き込み開始
			shader::CameraParam cameraParam;
			cameraParam.eyePos = this->mCamera.pos();
			cameraParam.viewProj = this->mCamera.viewProjMatrix();
			auto cameraAllocateInfo = this->mCBPool.allocate(&cameraParam, sizeof(cameraParam));
			DX12SamplerDesc samplerDesc;
			for (auto& pipeline : this->mPipelines) {
				pipeline.updateCBV("CBCamera", this->mDHPool, cameraAllocateInfo);
			}

			auto drawMesh = [&](DX12GraphicsCommandList& cmdList, utility::Mesh& mesh, utility::PipelineSet& pipeline, const float4x4& worldMatrix, const shader::MaterialParam& materialParam, DX12Resource* pTex, D3D12_SAMPLER_DESC* pSampler) {
				if (pTex && pSampler) {
					pipeline.updateSRV("txTexture", this->mDHPool, *pTex, &pTex->view().srv());
					pipeline.updateSampler("smSampler", this->mDHPool, pSampler);
				}

				pipeline.updateCBV("CBModel", this->mDHPool, &worldMatrix, sizeof(worldMatrix), this->mCBPool);
				pipeline.updateCBV("CBMaterial", this->mDHPool, &materialParam, sizeof(materialParam), this->mCBPool);

				pipeline.bind(cmdList);
				mesh.bind(cmdList);
				mesh.draw(cmdList);
			};

			shader::MaterialParam cbMaterial;
			cbMaterial.diffuseColor(1, 1, 0.5f);
			cbMaterial.alpha = 1;
			cbMaterial.specularColor(1, 1, 1);
			cbMaterial.specularPower = 10.f;
			drawMesh(cmdList, this->mMeshes[eVERTEX_PN], this->mPipelines[eVERTEX_PN], worldMatrices[eVERTEX_PN], cbMaterial, nullptr, nullptr);

			cbMaterial.diffuseColor(0.5f, 1, 1);
			cbMaterial.alpha = 1;
			cbMaterial.specularColor(1, 1, 1);
			cbMaterial.specularPower = 10.f;
			drawMesh(cmdList, this->mMeshes[eVERTEX_PTN], this->mPipelines[eVERTEX_PTN], worldMatrices[eVERTEX_PTN], cbMaterial, &this->mTexture, &samplerDesc);

			cbMaterial.diffuseColor(1, 1, 0.5f);
			cbMaterial.alpha = 1;
			cbMaterial.specularColor(1, 1, 1);
			cbMaterial.specularPower = 10.f;
			drawMesh(cmdList, this->mMeshes[eVERTEX_PTNC], this->mPipelines[eVERTEX_PTNC], worldMatrices[eVERTEX_PTNC], cbMaterial, &this->mTexture, &samplerDesc);

			auto worldMatrix = math::makeScale<math::float4x4>({ 20, 1, 20 });
			worldMatrix[3](0, -3, 0, 1);
			cbMaterial.diffuseColor(1, 1, 1);
			cbMaterial.alpha = 1;
			cbMaterial.specularColor(1, 1, 1);
			cbMaterial.specularPower = 50.f;
			drawMesh(cmdList, this->mGroundMesh, this->mPipelines[eVERTEX_PN], worldMatrix, cbMaterial, nullptr, nullptr);

			this->mGBuffer.end(cmdList);
		}

		{//ライティング実行
			utility::DefferedLightingManager::SetupParam param;
			param.texAlbedo.set(&this->mGBuffer.get(utility::GBufferManager::eGBUFFER_ALBEDO), utility::GBufferManager::eVIEW_SRV);
			param.texNormal.set(&this->mGBuffer.get(utility::GBufferManager::eGBUFFER_NORMAL), utility::GBufferManager::eVIEW_SRV);
			param.texSpecular.set(&this->mGBuffer.get(utility::GBufferManager::eGBUFFER_SPECULER), utility::GBufferManager::eVIEW_SRV);
			param.texDepth.set(&this->mGBuffer.get(utility::GBufferManager::eGBUFFER_DEPTH_FOR_SRV_UAV), utility::GBufferManager::eVIEW_SRV);
			param.invViewProj = math::inverse(this->mCamera.viewProjMatrix());
			param.eyePos = this->mCamera.pos();
			this->mDefferedLighting.setup(cmdList, this->mDHPool, this->mCBPool, param);

			{
				utility::DefferedLightingManager::DirectionLightParam param;
				param.lightParam = mainLight;
				param.pShadowMap = &this->mShadowMap.shadowMap();
				param.shadowMapSRVIndex = this->mShadowMap.eVIEW_SRV;
				param.shadowMapParam.toShadowMapUVSpace = math::mul(lightMatrix, math::makeToUVFromClipSpace());
				param.shadowMapParam.depthOffset = 0.0001f;
				this->mDefferedLighting.lighting(cmdList, this->mDHPool, this->mCBPool, param);
			}

			if (false) {
				shader::DirectionLightParam cbLights[2];
				cbLights[0].direction = math::normalize(math::float3(-1, 0, 0));
				cbLights[0].color = { 1, 0, 0 };
				cbLights[1].direction = math::normalize(math::float3(0, 0, -1));
				cbLights[1].color = { 0, 0, 1 };
				for (auto i = 0u; i < hinode::graphics::arrayLength(cbLights); ++i) {
					utility::DefferedLightingManager::DirectionLightParam param;
					param.lightParam = cbLights[i];
					this->mDefferedLighting.lighting(cmdList, this->mDHPool, this->mCBPool, param);
				}
			}
		}

		{//バックバッファーに書き込み
			cmdList.setBarriers(
				this->mDefferedLighting.lightingTexture().makeBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE),
				graphics.currentRenderTarget().makeBarrier(D3D12_RESOURCE_STATE_COPY_DEST)
			);
			cmdList->CopyResource(graphics.currentRenderTarget(), this->mDefferedLighting.lightingTexture());
		}
		cmdList.setBarriers(graphics.currentRenderTarget().makeBarrier(D3D12_RESOURCE_STATE_PRESENT));
	});
	graphics.present(1, { graphics.currentContext().cmdList() });
	graphics.endAndGoNextFrame();
	this->mCBPool.endAndGoNextFrame(graphics.cmdQueue());
	this->mDHPool.endAndGoNextFrame(graphics.cmdQueue());

	return true;
}
