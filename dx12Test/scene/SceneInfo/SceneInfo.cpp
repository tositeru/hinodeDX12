#include "stdafx.h"

#include <graphics/dx12/common/Log.h>
#include <graphics/dx12/utility/ResourceUploader/ResourceUploader.h>

#include "SceneInfo.h"

using namespace hinode;
using namespace hinode::graphics;

namespace scene
{

	SceneInfo::SceneInfo()
	{

	}

	SceneInfo::~SceneInfo()
	{
		this->clear();
	}

	void SceneInfo::clear()
	{
		this->mModelHash.clear();
		this->mPipelineHash.clear();
		this->mTextureHash.clear();
		this->mRenderPhaseHash.clear();
		this->mpRenderPhasePriorityOrder.clear();
	}

	void SceneInfo::init(const json11::Json& json)
	{
		enum SCENE_ELEMENT_KEY {
			eSCENE_OBJECTS,
			eSCENE_MODELS,
			eSCENE_PIPELINES,
			eSCENE_ELEMENT_KEY_COUNT
		};
		static const std::array<std::string, eSCENE_ELEMENT_KEY_COUNT> KEYS = { {
				"objects",
				"models",
				"pipelines",
			} };

		this->initPipeline(json[KEYS[eSCENE_PIPELINES]]);
		this->initModel(json[KEYS[eSCENE_MODELS]]);
		this->initObject(json[KEYS[eSCENE_OBJECTS]]);
	}

	bool SceneInfo::load(const std::string& filepath)
	{
		std::ifstream in(filepath);
		in.seekg(0, in.end);
		auto length = static_cast<size_t>(in.tellg());
		in.seekg(0, in.beg);
		std::string str;
		str.resize(length);
		in.read(&str[0], length);
		std::string error;
		auto json = json11::Json::parse(str.c_str(), error);
		if (!error.empty()) {
			HINODE_GRAPHICS_ERROR_LOG("SceneInfo", "load") << "ファイルからJSONデータを作成するのに失敗しました。 filepath=" << filepath;
			return false;
		}
		this->init(json);
		return true;
	}

	void SceneInfo::initResource(ID3D12Device* pDevice, UINT nodeMask)
	{
		utility::ResourceUploader uploader;
		uploader.init(pDevice, D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, nodeMask);

		for (auto& it : this->mPipelineHash) {
			if (it.second.isCreatedResource()) continue;
			it.second.createPipelineSet(pDevice, nodeMask);
		}
		for (auto& it : this->mModelHash) {
			if (it.second.isCreatedResource()) continue;
			it.second.createResource(pDevice, &this->mTextureHash, uploader);
		}
		for (auto& it : this->mRenderPhaseHash) {
			if (it.second.isCreatedResource()) continue;
			it.second.createResource(pDevice, nodeMask);
		}
		uploader.upload();

		for (auto& it : this->mPipelineHash) {
			it.second.setEnableFlag(true);
		}
		for (auto& it : this->mModelHash) {
			it.second.setEnableFlag(true);
		}
		for (auto& it : this->mRenderPhaseHash) {
			it.second.setEnableFlag(true);
		}

	}

	void SceneInfo::mergeObject(ObjectHash&& hash)
	{
		for (auto& it : hash) {
			if (this->mObjectHash.end() != this->mObjectHash.find(it.first)) {
				continue;
			}

			this->mObjectHash.insert(std::move(it));
		}
		hash.clear();
	}

	void SceneInfo::mergeModel(ModelHash&& hash)
	{
		for (auto& it : hash) {
			if (this->mModelHash.end() != this->mModelHash.find(it.first)) {
				continue;
			}

			this->mModelHash.insert(std::move(it));
		}
		hash.clear();
	}

	void SceneInfo::mergePipeline(PipelineHash&& hash)
	{
		for (auto& it : hash) {
			if (this->mPipelineHash.end() != this->mPipelineHash.find(it.first)) {
				continue;
			}

			this->mPipelineHash.insert(std::move(it));
		}
		hash.clear();
	}

	void SceneInfo::mergeRenderPhase(RenderPhaseHash&& hash)
	{
		for (auto& it : hash) {
			if (this->mRenderPhaseHash.end() != this->mRenderPhaseHash.find(it.first)) {
				continue;
			}

			this->mpRenderPhasePriorityOrder.push_back(&this->mRenderPhaseHash.at(it.first));
		}
		hash.clear();
		std::sort(this->mpRenderPhasePriorityOrder.begin(), this->mpRenderPhasePriorityOrder.end());
	}

	void SceneInfo::initPipeline(const json11::Json& pipelineJson)
	{
		assert(pipelineJson.is_array());
		this->mPipelineHash.clear();

		for (auto& element : pipelineJson.array_items()) {
			auto& id = element["name"].string_value();

			auto it = this->mPipelineHash.find(id);
			if (it != this->mPipelineHash.end()) {
				Log(Log::eINFO) << "SceneInfo : 同じキーを持つパイプラインが見つかりました。初め以外のものは無視します。: id=" << id;
				continue;
			}

			if (this->mPipelineHash[id].create(element)) {
				Log(Log::eINFO) << "SceneInfo : パイプラインの追加: id=" << id;
			} else {
				Log(Log::eINFO) << "SceneInfo : パイプラインの作成に失敗: id=" << id;
			}
		}
	}

	void SceneInfo::initModel(const json11::Json& modelJson)
	{
		assert(modelJson.is_array());
		this->mModelHash.clear();

		for (auto& element : modelJson.array_items()) {
			auto& id = element["name"].string_value();

			auto it = this->mModelHash.find(id);
			if (it != this->mModelHash.end()) {
				Log(Log::eINFO) << "SceneInfo : 同じキーを持つモデルが見つかりました。初め以外のものは無視します。: id=" << id;
				continue;
			}

			if (this->mModelHash[id].create(element)) {
				Log(Log::eINFO) << "SceneInfo : モデルの追加: id=" << id;
			} else {
				Log(Log::eINFO) << "SceneInfo : モデルの作成に失敗: id=" << id;
			}
		}
	}

	void SceneInfo::initObject(const json11::Json& objectJson)
	{
		assert(objectJson.is_array());
		this->mObjectHash.clear();

		for (auto& element : objectJson.array_items()) {
			auto& id = element["name"].string_value();

			auto it = this->mObjectHash.find(id);
			if (it != this->mObjectHash.end()) {
				Log(Log::eINFO) << "SceneInfo : 同じキーを持つモデルが見つかりました。初め以外のものは無視します。: id=" << id;
				continue;
			}

			unless (this->mObjectHash[id].create(element)) {
				Log(Log::eINFO) << "SceneInfo : オブジェクトの作成に失敗: id=" << id;
				continue;
			}

			auto& modelKey = this->mObjectHash[id].modelKey();
			if (this->mModelHash.end() == this->mModelHash.find(modelKey)) {
				HINODE_GRAPHICS_ERROR_LOG("SceneInfo", "addObject") << "SceneInfo : オブジェクトに指定されたモデルキーがモデルリストに存在しません: id=" << id << ", modelKey=" << modelKey;
				continue;
			}

			this->mObjectHash[id].setEnableFlag(true);
			Log(Log::eINFO) << "SceneInfo : オブジェクトの追加: id=" << id;
		}
	}

	bool SceneInfo::addObject(const Object::InitParam& param)noexcept
	{
		unless(SceneInfoImpl::addObject(&this->mObjectHash, param, this->mModelHash)) {
			return false;
		}
		Log(Log::eINFO) << "SceneInfo : オブジェクトの追加: id=" << param.name;
		return true;
	}

	bool SceneInfo::addRenderPhase(const RenderPhase::InitParam& param)noexcept
	{
		unless(SceneInfoImpl::addRenderPhase(&this->mRenderPhaseHash, &this->mpRenderPhasePriorityOrder, param)) {
			return false;
		}

		Log(Log::eINFO) << "SceneInfo : レンダーフェーズの追加: id=" << param.name;
		return true;
	}

	bool SceneInfo::addModel(const Model::InitParam& param)noexcept
	{
		unless(SceneInfoImpl::addModel(&this->mModelHash, param)) {
			return false;
		}
		Log(Log::eINFO) << "SceneInfo : モデルの追加: id=" << param.name;
		return true;
	}

	bool SceneInfo::addPipeline(const Pipeline::InitParam& param)noexcept
	{
		unless(SceneInfoImpl::addPipeline(&this->mPipelineHash, param)) {
			return false;
		}

		Log(Log::eINFO) << "SceneInfo : パイプラインの追加: id=" << param.name;
		return true;
	}

	bool SceneInfo::deleteObject(const std::string& name)noexcept
	{
		return 1 == this->mObjectHash.erase(name);
	}

	bool SceneInfo::deleteModel(const std::string& name)noexcept
	{
		return 1 == this->mModelHash.erase(name);
	}

	bool SceneInfo::deletePipeline(const std::string& name)noexcept
	{
		return 1 == this->mPipelineHash.erase(name);
	}

	bool SceneInfo::deleteRenderPhase(const std::string& name)noexcept
	{
		return 1 == this->mRenderPhaseHash.erase(name);
	}

	bool SceneInfo::updateObject(const std::string& name, const json11::Json& json)noexcept
	{
		auto it = this->mObjectHash.find(name);
		if (this->mObjectHash.end() == it) {
			return false;
		}
		auto& obj = this->mObjectHash[name];
		obj.update(json);
		return true;
	}

	SceneInfo::ObjectHash& SceneInfo::objectHash()noexcept
	{
		return this->mObjectHash;
	}

	SceneInfo::ModelHash& SceneInfo::modelHash()noexcept
	{
		return this->mModelHash;
	}

	SceneInfo::TextureHash& SceneInfo::textureHash()noexcept
	{
		return this->mTextureHash;
	}

	SceneInfo::PipelineHash& SceneInfo::pipelineHash()noexcept
	{
		return this->mPipelineHash;
	}

	SceneInfo::RenderPhaseHash& SceneInfo::renderPhaseHash()noexcept
	{
		return this->mRenderPhaseHash;
	}

	SceneInfo::RenderPhaseOrderContainer& SceneInfo::renderPhasePriorityOrder()noexcept
	{
		return this->mpRenderPhasePriorityOrder;
	}

	const SceneInfo::ObjectHash& SceneInfo::objectHash()const noexcept
	{
		return this->mObjectHash;
	}

	const SceneInfo::ModelHash& SceneInfo::modelHash()const noexcept
	{
		return this->mModelHash;
	}

	const SceneInfo::TextureHash& SceneInfo::textureHash()const noexcept
	{
		return this->mTextureHash;
	}

	const SceneInfo::PipelineHash& SceneInfo::pipelineHash()const noexcept
	{
		return this->mPipelineHash;
	}

	const SceneInfo::RenderPhaseHash& SceneInfo::renderPhaseHash()const noexcept
	{
		return this->mRenderPhaseHash;
	}

	const SceneInfo::RenderPhaseOrderContainer& SceneInfo::renderPhasePriorityOrder()const noexcept
	{
		return this->mpRenderPhasePriorityOrder;
	}

}
