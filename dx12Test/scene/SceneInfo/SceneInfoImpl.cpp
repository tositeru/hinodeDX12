#include "stdafx.h"

#include "SceneInfoImpl.h"

#include <graphics/dx12/common/Log.h>

using namespace hinode;
using namespace hinode::graphics;

namespace scene
{
	SceneInfoImpl::~SceneInfoImpl()
	{

	}

	bool SceneInfoImpl::addObject(ObjectHash* pOut, const Object::InitParam& param, const ModelHash& modelHash)noexcept
	{
		auto& key = param.name;
		auto it = pOut->find(key);
		if (it != pOut->end()) {
			HINODE_GRAPHICS_ERROR_LOG("SceneInfoImpl", "addObject") << "同名のキーが存在しています。 key=" << key;
			return false;
		}

		unless((*pOut)[key].create(param)) {
			HINODE_GRAPHICS_ERROR_LOG("SceneInfoImpl", "addObject") << "SceneInfoImpl : オブジェクトの作成に失敗: id=" << key;
			return false;
		}

		auto& modelKey = (*pOut)[key].modelKey();
		if (modelHash.end() == modelHash.find(modelKey)) {
			HINODE_GRAPHICS_ERROR_LOG("SceneInfoImpl", "addObject") << "SceneInfo : オブジェクトに指定されたモデルキーがモデルリストに存在しません。実行対象から無視するオブジェクトとして追加します。: id=" << key << ", modelKey=" << modelKey;
			return true;
		}

		(*pOut)[key].setEnableFlag(true);
		return true;
	}

	bool SceneInfoImpl::addRenderPhase(RenderPhaseHash* pOut, RenderPhaseOrderContainer* pOutOrder, const RenderPhase::InitParam& param)noexcept
	{
		auto it = pOut->find(param.name);
		if (it != pOut->end()) {
			HINODE_GRAPHICS_ERROR_LOG("SceneInfoImpl", "addRenderPhase") << "同名のキーが存在しています。 key=" << param.name;
			return false;
		}
		unless((*pOut)[param.name].create(param)) {
			HINODE_GRAPHICS_ERROR_LOG("SceneInfoImpl", "addRenderPhase") << "描画フェーズの追加に失敗。 key=" << param.name;
			return false;
		}

		pOutOrder->push_back(&(*pOut)[param.name]);
		std::sort(pOutOrder->begin(), pOutOrder->end());
		return true;
	}

	bool SceneInfoImpl::addModel(ModelHash* pOut, const Model::InitParam& param)noexcept
	{
		auto it = pOut->find(param.name);
		if (it != pOut->end()) {
			HINODE_GRAPHICS_ERROR_LOG("SceneInfoImpl", "addModel") << "同名のキーが存在しています。 key=" << param.name;
			return false;
		}
		unless((*pOut)[param.name].create(param)) {
			HINODE_GRAPHICS_ERROR_LOG("SceneInfoImpl", "addModel") << "モデルの追加に失敗。 key=" << param.name;
			return false;
		}

		return true;
	}

	bool SceneInfoImpl::addPipeline(PipelineHash* pOut, const Pipeline::InitParam& param)noexcept
	{
		auto it = pOut->find(param.name);
		if (it != pOut->end()) {
			HINODE_GRAPHICS_ERROR_LOG("SceneInfoImpl", "addPipeline") << "同名のキーが存在しています。 key=" << param.name;
			return false;
		}
		unless((*pOut)[param.name].create(param)) {
			HINODE_GRAPHICS_ERROR_LOG("SceneInfoImpl", "addPipeline") << "パイプラインの追加に失敗。 key=" << param.name;
			return false;
		}

		return true;
	}

}