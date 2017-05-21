#include "stdafx.h"

#include "DemoScene.h"

#include <graphics/dx12/utility/winapi/KeyObserver.h>

DemoScene::DemoScene()
{}

DemoScene::~DemoScene()
{
	this->clear();
}

void DemoScene::clear()
{
}

void DemoScene::init(utility::Graphics& graphics, scene::SceneInfo& sceneInfo)
{
	this->clear();
	{
		utility::PreviewCamera::InitParam cameraParam;
		cameraParam.pos(-20.6759758f, 3.68892479f, -25.8944092f);
		cameraParam.front = -math::normalize(cameraParam.pos);
		cameraParam.planeZ(0.1f, 100.f);
		auto R = math::normalize(math::cross(math::float3{0, -1, 0}, cameraParam.front));
		cameraParam.up = math::normalize(math::cross(R, cameraParam.front));
		cameraParam.aspect = graphics.calAspect();
		this->mCamera.init(cameraParam);
		this->mCBPool.create(graphics.device(), 1024 * 246);
		this->mDHPool.create(graphics.device(), 256, 128);
	}

	utility::ResourceUploader uploader;
	uploader.init(graphics.device());

	auto width = graphics.swapchain().width();
	auto height = graphics.swapchain().height();
	{//SSAO�֘A�̏�����
		this->mSSAO.create(graphics.device(), width, height);
	}
	{//�{�����[�����C�e�B���O�֘A�̏�����
		VolumetricLightingManager::InitParam initParam;
		initParam.screenWidth = width;
		initParam.screenHeight = height;
		initParam.particleCount = 10000;
		initParam.gridSize(128, 128, 16);
		this->mVolumetricLight.create(graphics.device(), initParam, uploader);
	}
	{//�f�B�t�@�[�h���C�e�B���O�֘A�̏�����
		this->mGBuffer.create(graphics.device(), width, height);
		this->mDefferedLighting.create(graphics.device(), width, height);
	}
	{//�V���h�E�}�b�v�֌W�̏�����
		UINT size = 2048;
		this->mShadowMap.create(graphics.device(), size, size, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT);
	}

	this->initObjects(graphics.device(), uploader);
}

bool DemoScene::paint(utility::Graphics& graphics, scene::SceneInfo& sceneInfo)
{
	static winapi::KeyDownObserverManager keyObserver;
	static auto& enableSSAO = keyObserver.add({ '1', true});
	static auto& enableVolumetricLight = keyObserver.add({ '2', true });
	static auto& enableLightRota = keyObserver.add({ '3', true });
	static auto& showCommand = keyObserver.add('0', true, [](const winapi::KeyDownObserver& This) {
		Log(Log::eMEMO)
			<< "1�L�[ : SSAO�̃I���E�I�t" << Log::eBR
			<< "2�L�[ : �{�����[�����C�g�̃I���E�I�t" << Log::eBR
			<< "3�L�[ : ���s���̉�]�̃I���E�I�t" << Log::eBR
			<< "W, S�L�[ : �J�����O�ړ�" << Log::eBR
			<< "A, D�L�[ : �J�������ړ�" << Log::eBR
			<< "Q, E�L�[ : �J�����c�ړ�" << Log::eBR
			<< "�E�h���b�N : �J�����̎��_�ύX" << Log::eBR
			<< "0�L�[ : ���̕��͂̕\��"
			;
	});
	keyObserver.update();

	this->mCamera.update();

	//�`��I�u�W�F�N�g�̍X�V
	for (auto i = 0u; i < this->mDynamicObjCount; ++i) {
		auto& obj = this->mObjects[i];
		obj.rot += math::float3{0.001f, 0.02f, 0.002f};
	}
	{//�n�ʕt�߂œ����I�u�W�F�N�g
		static float t = 0;
		t += 0.01f;
		auto first = this->mDynamicObjCount;
		for (auto i = 0u; i < this->mMoveObjCount; ++i) {
			auto& obj = this->mObjects[first + i];
			obj.pos.z = sin(t + i) * 3.f;
		}
	}
	{//����
		static float t = 0;
		t += 0.005f;
		auto& obj = this->mObjects[this->mFloatFloorIndex];
		obj.pos.y = abs(cos(t)) * 15.f - 3.f;
		auto& obj2 = this->mObjects[this->mFloatFloorIndex+1];
		obj2.pos.y = obj.pos.y;

	}
	for (auto& obj : this->mObjects) {
		//obj.rot.y += 0.01f;
		obj.updateOrientation();
	}

	graphics.waitForCurrentFrame();

	graphics.currentContext().record(nullptr, [&](DX12GraphicsCommandList& cmdList) {
		this->mDHPool.bind(cmdList);

		//�V�[���S�̂̋��ʃf�[�^
		static float tt = 0.f;
		if (enableLightRota) { tt += 0.01f; }
		shader::DirectionLightParam mainLight;
		mainLight.direction = math::normalize(math::float3{ -sin(tt), -1.f, cos(tt) });
		mainLight.color(1, 1, 1);
		math::float4x4 lightMatrix;

		{//�V���h�E�}�b�v
			this->mShadowMap.begin(cmdList);
			shader::CameraParam cbCamera;
			cbCamera.eyePos = -50 * mainLight.direction;
			cbCamera.viewProj = math::mul(
				math::makeView(cbCamera.eyePos, math::float3(0, 0, 0), math::float3(0, 1, 0)),
				math::makeOrthogonal(150, 150, 1.f, 100.f)
			);
			lightMatrix = cbCamera.viewProj;
			auto lightMatrixAllocateInfo = this->mCBPool.allocate(&cbCamera, sizeof(cbCamera));
			for (auto& pipeline : this->mPipelinesZPass) {
				pipeline.updateCBV("CBCamera", this->mDHPool, lightMatrixAllocateInfo);
			}
			for (auto& obj : this->mObjects) {
				auto& mesh = this->mMeshes[obj.meshType][obj.vertexType];
				auto pipeline = this->mPipelinesZPass[obj.vertexType];

				pipeline.updateCBV("CBModel", this->mDHPool, &obj.orientation, sizeof(obj.orientation), this->mCBPool);
				pipeline.bind(cmdList);

				mesh.bind(cmdList);
				mesh.draw(cmdList);
			}

			this->mShadowMap.end(cmdList);
		}

		{//GBuffer��������
			//�V�[���̃J���������p�C�v���C���ɐݒ�
			shader::CameraParam cameraParam;
			cameraParam.eyePos = this->mCamera.pos();
			cameraParam.viewProj = this->mCamera.viewProjMatrix();
			auto cameraAllocateInfo = this->mCBPool.allocate(&cameraParam, sizeof(cameraParam));
			for (auto& pipeline : this->mPipelinesGBuffer) {
				pipeline.updateCBV("CBCamera", this->mDHPool, cameraAllocateInfo);
			}

			this->mGBuffer.begin(cmdList);

			for(auto& obj : this->mObjects) {
				auto& mesh = this->mMeshes[obj.meshType][obj.vertexType];
				auto pipeline = this->mPipelinesGBuffer[obj.vertexType].clone({});

				if (-1 != obj.textureType ) {
					auto& tex = this->mTextures[obj.textureType];
					pipeline.updateSRV("txTexture", this->mDHPool, tex, &tex.view().srv());
					DX12SamplerDesc samplerDesc;
					pipeline.updateSampler("smSampler", this->mDHPool, &samplerDesc);
				}
				pipeline.updateCBV("CBModel", this->mDHPool, &obj.orientation, sizeof(obj.orientation), this->mCBPool);
				pipeline.updateCBV("CBMaterial", this->mDHPool, &obj.cbMaterial, sizeof(obj.cbMaterial), this->mCBPool);
				pipeline.bind(cmdList);

				mesh.bind(cmdList);
				mesh.draw(cmdList);
			}
			this->mGBuffer.end(cmdList);
		}

		{//���C�e�B���O���s
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
				param.shadowMapParam.depthOffset = 0.0035f;
				param.shadowMapParam.darknessRate = 0.4f;
				this->mDefferedLighting.lighting(cmdList, this->mDHPool, this->mCBPool, param);
			}
		}

		DX12Resource* pFinalImage = &this->mDefferedLighting.lightingTexture();
		int SRVIndex = this->mDefferedLighting.eVIEW_SRV;

		if(enableSSAO)
		{//SSAO���s
			SSAOManager::Param param;
			param.projMatrix = this->mCamera.projectionMatrix();
			param.nearFarZ = this->mCamera.planeZ();
			param.pSceneDepth = &this->mGBuffer.get(this->mGBuffer.eGBUFFER_DEPTH_FOR_SRV_UAV);
			param.sceneDepthSRVIndex = this->mGBuffer.eVIEW_SRV;
			param.pSceneImage = pFinalImage;
			param.sceneImageSRVIndex = SRVIndex;
			this->mSSAO.run(cmdList, this->mDHPool, this->mCBPool, param);

			pFinalImage = &this->mSSAO.result();
			SRVIndex = this->mSSAO.eVIEW_SRV;
		}

		if(enableVolumetricLight)
		{//�{�����[�����C�e�B���O���s
			VolumetricLightingManager::Param param;
			param.gridViewMatrix = this->mCamera.viewMatrix();
			param.gridProjMatrix = this->mCamera.projectionMatrix();
			param.lightDir = mainLight.direction;
			param.lightColor = mainLight.color;
			param.pSceneDepth = &this->mGBuffer.get(this->mGBuffer.eGBUFFER_DEPTH_FOR_SRV_UAV);
			param.sceneDepthSRVIndex = this->mGBuffer.eVIEW_SRV;
			param.pSceneImage = pFinalImage;
			param.sceneImageSRVIndex = SRVIndex;
			param.toShadowmapSpaceMat = math::mul(math::inverse(this->mCamera.viewMatrix()), lightMatrix);
			param.pShadowMap = &this->mShadowMap.getShadowMap(this->mShadowMap.eTYPE_SRV);
			param.shadowMapSRVIndex = this->mShadowMap.eVIEW_SRV;
			this->mVolumetricLight.run(cmdList, this->mDHPool, this->mCBPool, param);

			pFinalImage = &this->mVolumetricLight.result();
			SRVIndex = this->mVolumetricLight.eVIEW_SRV;
		}
		{//�o�b�N�o�b�t�@�[�ɏ�������
			cmdList.setBarriers(
				pFinalImage->makeBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE),
				graphics.currentRenderTarget().makeBarrier(D3D12_RESOURCE_STATE_COPY_DEST)
			);
			cmdList->CopyResource(graphics.currentRenderTarget(), *pFinalImage);
		}

		cmdList.setBarriers(graphics.currentRenderTarget().makeBarrier(D3D12_RESOURCE_STATE_PRESENT));
	});

	graphics.present(1, { graphics.currentContext().cmdList() });
	graphics.endAndGoNextFrame();
	this->mCBPool.endAndGoNextFrame(graphics.cmdQueue());
	this->mDHPool.endAndGoNextFrame(graphics.cmdQueue());

	return true;
}


void DemoScene::initObjects(ID3D12Device* pDevice, hinode::graphics::utility::ResourceUploader& uploader)
{
	Object obj;

	//���ɕ����Ă����
	obj.pos(0, 3, -3);
	obj.scale(4, 4, 4);
	obj.vertexType = eVERTEX_PTN;
	obj.textureType = eTEX_TILE;
	this->mObjects.push_back(obj);

	obj.pos(12, 6, -5);
	obj.scale(4, 4, 4);
	obj.vertexType = eVERTEX_PTN;
	obj.textureType = eTEX_TILE;
	this->mObjects.push_back(obj);

	obj.pos(10, 6, 7);
	obj.scale(4, 4, 4);
	obj.vertexType = eVERTEX_PTN;
	obj.textureType = eTEX_TILE;
	this->mObjects.push_back(obj);

	obj.pos(-12, 6, 7);
	obj.scale(4, 4, 4);
	obj.vertexType = eVERTEX_PTN;
	obj.textureType = eTEX_TILE;
	this->mObjects.push_back(obj);

	obj.pos(-6, 6, -16);
	obj.scale(4, 4, 4);
	obj.vertexType = eVERTEX_PTN;
	obj.textureType = eTEX_TILE;
	this->mObjects.push_back(obj);
	this->mDynamicObjCount = this->mObjects.size();

	//�n�ʕt�߂𓮂��I�u�W�F�N�g
	obj.pos(4, -1, 0);
	obj.scale(4, 4, 4);
	obj.vertexType = eVERTEX_PTN;
	obj.textureType = eTEX_PANDA;
	this->mObjects.push_back(obj);

	obj.pos(-4, -1, 0);
	obj.scale(4, 4, 4);
	obj.vertexType = eVERTEX_PTN;
	obj.textureType = eTEX_PANDA;
	this->mObjects.push_back(obj);

	this->mMoveObjCount = this->mObjects.size() - this->mDynamicObjCount;

	//��
	obj.scale(9, 24, 9);
	obj.pos(15, 0, 0);
	obj.meshType = eMESH_TETRA;
	obj.vertexType = eVERTEX_PTNC;
	obj.textureType = eTEX_PANDA;
	this->mObjects.push_back(obj);

	obj.pos(-15, 0, 0);
	obj.meshType = eMESH_TETRA;
	obj.vertexType = eVERTEX_PTN;
	obj.textureType = eTEX_TILE;
	this->mObjects.push_back(obj);

	obj.pos(0, 0, 15);
	obj.meshType = eMESH_TETRA;
	obj.vertexType = eVERTEX_PTN;
	obj.textureType = eTEX_PANDA;
	this->mObjects.push_back(obj);

	obj.pos(0, 0, -15);
	obj.meshType = eMESH_TETRA;
	obj.vertexType = eVERTEX_PTNC;
	obj.textureType = eTEX_TILE;
	this->mObjects.push_back(obj);

	//��
	obj.pos(0, -1, 0);
	obj.scale(60, 1, 60);
	obj.meshType = eMESH_PLANE;
	obj.vertexType = eVERTEX_PTN;
	obj.textureType = eTEX_TILE;
	this->mObjects.push_back(obj);

	//�{�����[�����C�g���킩��₷�����邽�߂̕�����
	obj.pos(0, 0, 0);
	obj.scale(20, 1, 20);
	obj.meshType = eMESH_PLANE;
	obj.vertexType = eVERTEX_PTN;
	obj.textureType = eTEX_PANDA;
	this->mObjects.push_back(obj);
	this->mFloatFloorIndex = this->mObjects.size() - 1;

	obj.rot(math::toRadian(180.f), 0, 0);
	this->mObjects.push_back(obj);
	obj.rot(0, 0, 0);

	{//���b�V���p�̃p�C�v���C��������
	 //GBuffer�������ݗp
		this->mPipelinesGBuffer[eVERTEX_PTN] = utility::DefferedPipeline::sCreatePTNVertex(pDevice);
		this->mPipelinesGBuffer[eVERTEX_PTNC] = utility::DefferedPipeline::sCreatePTNCVertex(pDevice);

		//Z�p�X�p
		this->mPipelinesZPass[eVERTEX_PTN] = utility::ZPassPipeline::sCreatePTNVertex(pDevice);
		this->mPipelinesZPass[eVERTEX_PTNC] = utility::ZPassPipeline::sCreatePTNCVertex(pDevice);
	}
	{//���b�V��������
		utility::PlaneCreator::sCreateMesh<utility::PTNVertex>(&this->mMeshes[eMESH_PLANE][eVERTEX_PTN], pDevice, uploader, utility::PlaneCreator::ePLANE_XZ);
		utility::PlaneCreator::sCreateMesh<utility::PTNCVertex>(&this->mMeshes[eMESH_PLANE][eVERTEX_PTNC], pDevice, uploader, utility::PlaneCreator::ePLANE_XZ);

		utility::TetrahedronCreator::sCreateMesh<utility::PTNVertex>(&this->mMeshes[eMESH_TETRA][eVERTEX_PTN], pDevice, uploader);
		utility::TetrahedronCreator::sCreateMesh<utility::PTNCVertex>(&this->mMeshes[eMESH_TETRA][eVERTEX_PTNC], pDevice, uploader);
	}
	{//�e�N�X�`���ǂݍ���
		uploader.loadTextureFromDDS(pDevice, this->mTextures[eTEX_PANDA], "data/panda.dds");
		uploader.loadTextureFromWICFile(pDevice, this->mTextures[eTEX_TILE], "data/tile.png");

		//SRV�̍쐬
		for (auto& tex : this->mTextures) {
			tex.appendView(DX12Resource::View().setSRV(tex.makeSRV()));
		}
	}
}