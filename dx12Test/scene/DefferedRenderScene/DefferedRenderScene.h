#pragma once

#include "../IScene.h"

#include <graphics/dx12/utility/GBuffer/GBufferManager.h>
#include <graphics/dx12/utility/GBuffer/DefferedLightingPipeline/DefferedLightingPipeline.h>
#include <graphics/dx12/utility/GBuffer/DefferedLightingManager/DefferedLightingManager.h>
#include <graphics/dx12/utility/ShadowMap/ShadowMapManager.h>

class DefferedRenderScene : public IScene
{
public:
	DefferedRenderScene();
	~DefferedRenderScene();

	void clear()override;
	void init(utility::Graphics& graphics, scene::SceneInfo& sceneInfo)override;
	bool paint(utility::Graphics& graphics, scene::SceneInfo& sceneInfo)override;

private:
	utility::ConstantBufferPool mCBPool;
	utility::DescriptorHeapPoolSet mDHPool;
	utility::PreviewCamera mCamera;

	enum TYPE {
		//eVERTEX_P,
		//eVERTEX_PT,
		//eVERTEX_PC,
		eVERTEX_PN,
		eVERTEX_PTN,
		eVERTEX_PTNC,
		eTYPE_COUNT,
	};
	std::array<utility::PipelineSet, eTYPE_COUNT> mPipelines;
	std::array<utility::PipelineSet, eTYPE_COUNT> mPipelinesZPass;
	std::array<utility::Mesh, eTYPE_COUNT> mMeshes;
	utility::Mesh mGroundMesh;
	DX12Resource mTexture;

	utility::GBufferManager mGBuffer;
	utility::DefferedLightingManager mDefferedLighting;

	utility::ShadowMapManager mShadowMap;

};
