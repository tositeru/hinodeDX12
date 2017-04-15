#pragma once

#include "../IScene.h"

class SceneEditorScene : public IScene
{
public:
	SceneEditorScene();
	~SceneEditorScene();

	void clear()override;
	void init(utility::Graphics& graphics, scene::SceneInfo& sceneInfo)override;
	bool paint(utility::Graphics& graphics, scene::SceneInfo& sceneInfo)override;

private:
	utility::ConstantBufferPool mCBPool;
	utility::DescriptorHeapPoolSet mDHPool;
	utility::PreviewCamera mCamera;

};
