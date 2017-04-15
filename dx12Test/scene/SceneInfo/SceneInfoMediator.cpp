#include "stdafx.h"

#include "SceneInfoMediator.h"

#include <graphics/dx12/common/Log.h>
#include "SceneInfo.h"

using namespace hinode;
using namespace hinode::graphics;

namespace scene
{
	SceneInfoMediator::SceneInfoMediator()
	{ }

	SceneInfoMediator::~SceneInfoMediator()
	{

	}

	void SceneInfoMediator::init(SceneInfo* pTarget)
	{
		this->mpTarget = pTarget;
	}

	void SceneInfoMediator::deliver()
	{
		assert(nullptr != this->mpTarget);

		auto lock = this->getLock();

		unless (this->mObjectHash.empty()) {
			unless(lock.owns_lock()) {
				lock.lock();
			}
			this->mpTarget->mergeObject(std::move(this->mObjectHash));
		}
		unless(this->mModelHash.empty()) {
			unless(lock.owns_lock()) {
				lock.lock();
			}
			this->mpTarget->mergeModel(std::move(this->mModelHash));
		}
		unless(this->mPipelineHash.empty()) {
			unless(lock.owns_lock()) {
				lock.lock();
			}
			this->mpTarget->mergePipeline(std::move(this->mPipelineHash));
		}
		unless(this->mRenderPhase.empty()) {
			unless(lock.owns_lock()) {
				lock.lock();
			}
			this->mpTarget->mergeRenderPhase(std::move(this->mRenderPhase));
		}

		unless(this->mDeleteList.empty()) {
			//�폜���X�g�ɂ�����̂��폜����
			for (auto& it : this->mDeleteList) {
				for (auto type : it.second) {
					bool isOK = false;
					switch (type) {
					case ELEMENT_TYPE::eOBJECT:			isOK = this->mpTarget->deleteObject(it.first); break;
					case ELEMENT_TYPE::eMODEL:			isOK = this->mpTarget->deleteModel(it.first); break;
					case ELEMENT_TYPE::ePIPELINE:		isOK = this->mpTarget->deletePipeline(it.first); break;
					case ELEMENT_TYPE::eRENDER_PHASE:	isOK = this->mpTarget->deleteRenderPhase(it.first); break;
					}

					unless(isOK) {
						HINODE_GRAPHICS_ERROR_LOG("SceneInfoMediator", "deliver")
							<< "�폜�Ɏ��s���܂����B name=" << it.first << " type=" << ElementType::sToStr(type);
					} else {
						Log(Log::eINFO) << "�폜���܂����B name=" << it.first << " type=" << ElementType::sToStr(type);
					}
				}
			}
			this->mDeleteList.clear();
		}

		unless(this->mUpdateList.empty()) {
			//�X�V���X�g�ɂ�����̂��폜����
			for (auto& it : this->mUpdateList) {
				for (auto& updateData: it.second) {
					bool isOK = false;
					switch (updateData.type) {
					case ELEMENT_TYPE::eOBJECT:			isOK = this->mpTarget->updateObject(it.first, updateData.json); break;
					//case ELEMENT_TYPE::eMODEL:			isOK = this->mpTarget->deleteModel(it.first); break;
					//case ELEMENT_TYPE::ePIPELINE:		isOK = this->mpTarget->deletePipeline(it.first); break;
					//case ELEMENT_TYPE::eRENDER_PHASE:	isOK = this->mpTarget->deleteRenderPhase(it.first); break;
					}
					unless(isOK) {
						HINODE_GRAPHICS_ERROR_LOG("SceneInfoMediator", "deliver")
							<< "�X�V�Ɏ��s���܂����B name=" << it.first << " type=" << ElementType::sToStr(updateData.type);
					}
				}
			}

			this->mUpdateList.clear();
		}
	}

	SceneInfoMediator::Result SceneInfoMediator::addObject(const Object::InitParam& param)noexcept
	{
		Result result;
		auto& hash = this->mpTarget->objectHash();
		auto it = hash.find(param.name);
		if (hash.end() != it) {
			result.message = "error : �ǉ����悤�Ƃ����I�u�W�F�N�g�Ɠ����̂��̂����łɑ��݂��Ă��܂��B name=" + param.name;
			return result;
		}

		this->mObjectHash[param.name].create(param);

		result.isSuccess = true;
		return result;
	}

	SceneInfoMediator::Result SceneInfoMediator::addRenderPhase(const RenderPhase::InitParam& param)noexcept
	{
		Result result;
		auto& hash = this->mpTarget->renderPhaseHash();
		auto it = hash.find(param.name);
		if (hash.end() != it) {
			result.message = "error : �ǉ����悤�Ƃ����`��t�F�[�Y�Ɠ����̂��̂����łɑ��݂��Ă��܂��Bname=" + param.name;
			return result;
		}

		this->mRenderPhase[param.name].create(param);

		result.isSuccess = true;
		return result;
	}

	SceneInfoMediator::Result SceneInfoMediator::addModel(const Model::InitParam& param)noexcept
	{
		Result result;
		auto& hash = this->mpTarget->modelHash();
		auto it = hash.find(param.name);
		if (hash.end() != it) {
			result.message = "error : �ǉ����悤�Ƃ������f���Ɠ����̂��̂����łɑ��݂��Ă��܂��Bname=" + param.name;
			return result;
		}

		this->mModelHash[param.name].create(param);

		result.isSuccess = true;
		return result;
	}

	SceneInfoMediator::Result SceneInfoMediator::addPipeline(const Pipeline::InitParam& param)noexcept
	{
		Result result;
		auto& hash = this->mpTarget->pipelineHash();
		auto it = hash.find(param.name);
		if (hash.end() != it) {
			result.message = "error : �ǉ����悤�Ƃ����p�C�v���C���Ɠ����̂��̂����łɑ��݂��Ă��܂��Bname=" + param.name;
			return result;
		}

		this->mPipelineHash[param.name].create(param);

		result.isSuccess = true;
		return result;
	}

	SceneInfoMediator::Result SceneInfoMediator::addObject(const json11::Json& param)noexcept
	{
		Result result;
		unless(param["name"].is_string()) {
			result.message = "error : �I�u�W�F�N�g�쐬�Ɏg�p�����p�����[�^�ɕ�����^������\"id\"�����݂��܂���B";
			return result;
		}
		auto& name = param["name"].string_value();
		if(name.empty()) {
			result.message = "error : �쐬������̂ɖ��O���t�����Ă��܂���B";
			return result;
		}

		auto& hash = this->mpTarget->objectHash();
		auto it = hash.find(name);
		if (hash.end() != it) {
			result.message = "error : �ǉ����悤�Ƃ����I�u�W�F�N�g�Ɠ����̂��̂����łɑ��݂��Ă��܂��Bname=" + name;
			return result;
		}

		unless(this->mObjectHash[name].create(param)) {
			result.message = "error : �I�u�W�F�N�g�̍쐬�Ɏ��s���܂����Bname=" + name;
			return result;
		}
		this->mObjectHash[name].setEnableFlag(true);
		result.isSuccess = true;
		return result;
	}

	SceneInfoMediator::Result SceneInfoMediator::addRenderPhase(const json11::Json& param)noexcept
	{
		Result result;
		unless(param["name"].is_string()) {
			result.message = "error : �`��t�F�[�Y�쐬�Ɏg�p�����p�����[�^�ɕ�����^������\"id\"�����݂��܂���B";
			return result;
		}
		auto& name = param["name"].string_value();
		if(name.empty()) {
			result.message = "error : �쐬������̂ɖ��O���t�����Ă��܂���B";
			return result;
		}

		auto& hash = this->mpTarget->renderPhaseHash();
		auto it = hash.find(name);
		if (hash.end() != it) {
			result.message = "error : �ǉ����悤�Ƃ����`��t�F�[�Y�Ɠ����̂��̂����łɑ��݂��Ă��܂��Bname=" + name;
			return result;
		}

		this->mRenderPhase[name].create(param);

		result.isSuccess = true;
		return result;
	}

	SceneInfoMediator::Result SceneInfoMediator::addModel(const json11::Json& param)noexcept
	{
		Result result;
		unless(param["name"].is_string()) {
			result.message = "error : ���f���쐬�Ɏg�p�����p�����[�^�ɕ�����^������\"id\"�����݂��܂���B";
			return result;
		}
		auto& name = param["name"].string_value();
		if(name.empty()) {
			result.message = "error : �쐬������̂ɖ��O���t�����Ă��܂���B";
			return result;
		}

		auto& hash = this->mpTarget->modelHash();
		auto it = hash.find(name);
		if (hash.end() != it) {
			result.message = "error : �ǉ����悤�Ƃ������f���Ɠ����̂��̂����łɑ��݂��Ă��܂��Bname=" + name;
			return result;
		}

		this->mModelHash[name].create(param);

		result.isSuccess = true;
		return result;
	}

	SceneInfoMediator::Result SceneInfoMediator::addPipeline(const json11::Json& param)noexcept
	{
		Result result;
		unless(param["name"].is_string()) {
			result.message = "error : �p�C�v���C���쐬�Ɏg�p�����p�����[�^�ɕ�����^������\"id\"�����݂��܂���B";
			return result;
		}
		auto& name = param["name"].string_value();
		if(name.empty()) {
			result.message = "error : �쐬������̂ɖ��O���t�����Ă��܂���B";
			return result;
		}

		auto& hash = this->mpTarget->pipelineHash();
		auto it = hash.find(name);
		if (hash.end() != it) {
			result.message = "error : �ǉ����悤�Ƃ����p�C�v���C���Ɠ����̂��̂����łɑ��݂��Ă��܂��Bname=" + name;
			return result;
		}

		this->mPipelineHash[name].create(param);

		result.isSuccess = true;
		return result;
	}

	void SceneInfoMediator::addDeleteElement(const std::string& name, ELEMENT_TYPE type)noexcept
	{
		auto& list = this->mDeleteList[name];
		auto it = std::find(list.begin(), list.end(), type);
		if (list.end() != it) {
			return ;
		}

		list.push_back(type);
	}

	void SceneInfoMediator::addUpdateElement(const std::string& name, ELEMENT_TYPE type, const json11::Json& json)noexcept
	{
		auto& list = this->mUpdateList[name];
		UpdateData updateData;
		updateData.type = type;
		updateData.json = json;
		auto it = std::find(list.begin(), list.end(), updateData);
		if (list.end() != it) {
			return;
		}

		list.emplace_back(std::move(updateData));
	}

	std::unique_lock<std::mutex> SceneInfoMediator::getLock()noexcept
	{
		return std::unique_lock<std::mutex>(this->mMutex, std::defer_lock);
	}

	bool SceneInfoMediator::isExistData()const noexcept
	{
		bool isEmpty = true;
		isEmpty &= this->mObjectHash.empty();
		isEmpty &= this->mModelHash.empty();
		isEmpty &= this->mTextureHash.empty();
		isEmpty &= this->mPipelineHash.empty();
		isEmpty &= this->mRenderPhase.empty();
		isEmpty &= this->mDeleteList.empty();
		isEmpty &= this->mUpdateList.empty();

		return !isEmpty;
	}
}