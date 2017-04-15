#pragma once

#include "../IScene.h"

class MipmapScene : public IScene
{
public:
	MipmapScene();
	~MipmapScene();

	void clear()override;
	void init(utility::Graphics& graphics, scene::SceneInfo& sceneInfo)override;
	bool paint(utility::Graphics& graphics, scene::SceneInfo& sceneInfo)override;

private:
	utility::PreviewCamera mCamera;

	utility::PipelineSet mPTVertexPipeline;
	utility::PipelineSet mCalMiplevelPipeline;

	utility::Mesh mMesh;
	utility::Mesh mGroundMesh;
	DX12Resource mMeshTexture;
	DX12Resource mVisiblableMipmapTexture;

	utility::ConstantBufferPool mCBPool;
	utility::DescriptorHeapPoolSet mDHPool;
};