#include "stdafx.h"

#include "IScene.h"

#include <iostream>
#include <algorithm>

#include <graphics/dx12/common/Log.h>

using namespace std;
#include "MipmapScene\MipmapScene.h"
#include "RingBufferScene\RingBufferScene.h"
#include "DefferedRenderScene/DefferedRenderScene.h"
#include "SceneEditorScene/SceneEditorScene.h"
#include "DemoScene/DemoScene.h"
#include "BetterDrawCallScene/BetterDrawCallScene.h"

enum SCENE_TYPE {
	eSCENE_MIPMAP,
	eSCENE_RING_BUFFER,
	eSCENE_DEFFERED_RENDER,
	eSCENE_EDITOR_SCENE,
	eSCENE_SSAO,
	eSCENE_BETTER_DRAW_CALL,
	eSCENE_TYPE_COUNT,
};

size_t IScene::sGetStartSceneNo()noexcept
{
	//return eSCENE_MIPMAP;
	//return eSCENE_RING_BUFFER;
	//return eSCENE_DEFFERED_RENDER;
	//return eSCENE_EDITOR_SCENE;
	return eSCENE_BETTER_DRAW_CALL;
	//return eSCENE_SSAO;
}

size_t IScene::sClampSceneNo(size_t sceneNo)noexcept
{
	return std::min<size_t>(sceneNo, eSCENE_TYPE_COUNT);
}

std::unique_ptr<IScene> IScene::sBuild(size_t sceneNo)
{
	switch (sceneNo) {
	case eSCENE_MIPMAP:				return std::make_unique<MipmapScene>();
	case eSCENE_RING_BUFFER:		return std::make_unique<RingBufferScene>();
	case eSCENE_DEFFERED_RENDER:	return std::make_unique<DefferedRenderScene>();
	case eSCENE_EDITOR_SCENE:		return std::make_unique<SceneEditorScene>();
	case eSCENE_SSAO:				return std::make_unique<DemoScene>();
	case eSCENE_BETTER_DRAW_CALL:	return std::make_unique<BetterDrawCallScene>();
	default:
		using namespace hinode::graphics;
		Log(Log::eERROR) << "対応していないシーンがありますので、作ってね。 sceneNo=" << sceneNo;
		assert(false);
	}
	return nullptr;
}
