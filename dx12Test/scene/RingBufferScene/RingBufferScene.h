#pragma once

#include "../IScene.h"

class RingBufferScene : public IScene
{
public:
	RingBufferScene();
	~RingBufferScene();

	void clear()override;
	void init(utility::Graphics& graphics, scene::SceneInfo& sceneInfo)override;
	bool paint(utility::Graphics& graphics, scene::SceneInfo& sceneInfo)override;

private:
	utility::PreviewCamera mCamera;

	utility::PipelineSet mPipeline;
	utility::PipelineSet mPipelinePN;
	utility::PipelineSet mPipelinePTN;
	utility::PipelineSet mPipelinePTNC;

	utility::Mesh mMesh;
	utility::Mesh mMeshPN;
	utility::Mesh mMeshPTN;
	utility::Mesh mMeshPTNC;
	utility::Mesh mPlane;

	DX12Resource mTexture;

	utility::ConstantBufferPool mCBPool;
	utility::DescriptorHeapPoolSet mDHPool;
};
