#pragma once

#include "../IScene.h"

#include "ParticleManager/ParticleManager.h"

class BetterDrawCallScene : public IScene
{
public:
	BetterDrawCallScene();
	~BetterDrawCallScene();

	void clear();
	void init(utility::Graphics& graphics, scene::SceneInfo& sceneInfo);
	bool paint(utility::Graphics& graphics, scene::SceneInfo& sceneInfo);

private:
	utility::ConstantBufferPool mCBPool;
	utility::DescriptorHeapPoolSet mDHPool;
	utility::DrafterCamera mCamera;

	DX12QueryHeap mQueryHeap;
	DX12Resource mQueryResult;
	ParticleManager mParticleManager;
};
