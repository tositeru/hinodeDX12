#include "stdafx.h"

#include "ShadowMapScene.h"

ShadowMapScene::ShadowMapScene()
{ }

ShadowMapScene::~ShadowMapScene()
{
	this->clear();
}

void ShadowMapScene::clear()
{
	this->mCBPool.clear();
	this->mDHPool.clear();
	this->mGroundMesh.clear();
	for (auto& mesh : this->mMeshes) {
		mesh.clear();
	}
	for (auto& pipeline : this->mPipelines) {
		pipeline.clear();
	}
	this->mShadowMap.clear();
	this->mTexture.clear();
}

void ShadowMapScene::init(utility::Graphics& graphics, scene::SceneInfo& sceneInfo)
{
	this->clear();

	{
		utility::PreviewCamera::InitParam cameraParam;
		cameraParam.pos(0, 0, -5);
		cameraParam.aspect = graphics.calAspect();
		this->mCamera.init(cameraParam);
		this->mCBPool.create(graphics.device(), 1024 * 246);
		this->mDHPool.create(graphics.device(), 256, 128);
	}

	{//各種類の頂点に対応したパイプライン作成
		this->mPipelines[eVERTEX_P] = utility::DefferedPipeline::sCreatePVertex(graphics.device());
		this->mPipelines[eVERTEX_PT] = utility::DefferedPipeline::sCreatePTVertex(graphics.device());
		this->mPipelines[eVERTEX_PC] = utility::DefferedPipeline::sCreatePCVertex(graphics.device());
		this->mPipelines[eVERTEX_PN] = utility::DefferedPipeline::sCreatePNVertex(graphics.device());
		this->mPipelines[eVERTEX_PTN] = utility::DefferedPipeline::sCreatePTNVertex(graphics.device());
		this->mPipelines[eVERTEX_PTNC] = utility::DefferedPipeline::sCreatePTNCVertex(graphics.device());
	}

	this->mShadowMap.create(graphics.device(), 1024, 1024, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT);
}

bool ShadowMapScene::paint(utility::Graphics& graphics, scene::SceneInfo& sceneInfo)
{


	return true;
}
