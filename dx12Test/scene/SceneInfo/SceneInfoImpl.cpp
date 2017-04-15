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
			HINODE_GRAPHICS_ERROR_LOG("SceneInfoImpl", "addObject") << "�����̃L�[�����݂��Ă��܂��B key=" << key;
			return false;
		}

		unless((*pOut)[key].create(param)) {
			HINODE_GRAPHICS_ERROR_LOG("SceneInfoImpl", "addObject") << "SceneInfoImpl : �I�u�W�F�N�g�̍쐬�Ɏ��s: id=" << key;
			return false;
		}

		auto& modelKey = (*pOut)[key].modelKey();
		if (modelHash.end() == modelHash.find(modelKey)) {
			HINODE_GRAPHICS_ERROR_LOG("SceneInfoImpl", "addObject") << "SceneInfo : �I�u�W�F�N�g�Ɏw�肳�ꂽ���f���L�[�����f�����X�g�ɑ��݂��܂���B���s�Ώۂ��疳������I�u�W�F�N�g�Ƃ��Ēǉ����܂��B: id=" << key << ", modelKey=" << modelKey;
			return true;
		}

		(*pOut)[key].setEnableFlag(true);
		return true;
	}

	bool SceneInfoImpl::addRenderPhase(RenderPhaseHash* pOut, RenderPhaseOrderContainer* pOutOrder, const RenderPhase::InitParam& param)noexcept
	{
		auto it = pOut->find(param.name);
		if (it != pOut->end()) {
			HINODE_GRAPHICS_ERROR_LOG("SceneInfoImpl", "addRenderPhase") << "�����̃L�[�����݂��Ă��܂��B key=" << param.name;
			return false;
		}
		unless((*pOut)[param.name].create(param)) {
			HINODE_GRAPHICS_ERROR_LOG("SceneInfoImpl", "addRenderPhase") << "�`��t�F�[�Y�̒ǉ��Ɏ��s�B key=" << param.name;
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
			HINODE_GRAPHICS_ERROR_LOG("SceneInfoImpl", "addModel") << "�����̃L�[�����݂��Ă��܂��B key=" << param.name;
			return false;
		}
		unless((*pOut)[param.name].create(param)) {
			HINODE_GRAPHICS_ERROR_LOG("SceneInfoImpl", "addModel") << "���f���̒ǉ��Ɏ��s�B key=" << param.name;
			return false;
		}

		return true;
	}

	bool SceneInfoImpl::addPipeline(PipelineHash* pOut, const Pipeline::InitParam& param)noexcept
	{
		auto it = pOut->find(param.name);
		if (it != pOut->end()) {
			HINODE_GRAPHICS_ERROR_LOG("SceneInfoImpl", "addPipeline") << "�����̃L�[�����݂��Ă��܂��B key=" << param.name;
			return false;
		}
		unless((*pOut)[param.name].create(param)) {
			HINODE_GRAPHICS_ERROR_LOG("SceneInfoImpl", "addPipeline") << "�p�C�v���C���̒ǉ��Ɏ��s�B key=" << param.name;
			return false;
		}

		return true;
	}

}