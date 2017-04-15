#include "stdafx.h"

#include "SceneEditorScene.h"

#include "../SceneInfo/SceneInfo.h"

SceneEditorScene::SceneEditorScene()
{ }

SceneEditorScene::~SceneEditorScene()
{
	this->clear();
}

void SceneEditorScene::clear()
{
	this->mCBPool.clear();
	this->mDHPool.clear();
}

void SceneEditorScene::init(utility::Graphics& graphics, scene::SceneInfo& sceneInfo)
{
	this->clear();

	{
		utility::PreviewCamera::InitParam cameraParam;
		cameraParam.pos(0, 0, -5);
		cameraParam.aspect = graphics.calAspect();
		this->mCamera.init(cameraParam);
		this->mCBPool.create(graphics.device(), 1024 * 246);
		this->mDHPool.create(graphics.device(), 256, 128);
	}

	{
		scene::Pipeline::InitParam param;
		param.name = "PTN";
		param.type = scene::Pipeline::InitParam::eTYPE_SYSTEM;
		param.unique.system.type = scene::Pipeline::SYSTEM_PIPELINE_TYPE::eFORWARD;
		param.unique.system.vertex = scene::VERTEX::ePTN;
		sceneInfo.addPipeline(param);
	}
	{
		scene::Model::InitParam param;
		param.name = "tetra_PTN";
		param.type = scene::Model::InitParam::TYPE::eTYPE_SYSTEM;
		param.system.primitive = scene::Model::SYSTEM_MESH_PRIMITIVE::eTETRAHEDRON;
		param.system.vertex = scene::VERTEX::ePTN;
		param.system.material.diffuseColor(1, 1, 1);
		param.system.material.alpha = 1;
		param.system.material.specularColor(1, 1, 1);
		param.system.material.specularPower = 10;
		param.system.textures[(int)scene::Model::SYSTEM_MESH_TEXTURE::eDIFFUSE] = "data/panda.dds";
		sceneInfo.addModel(param);
	}
	{
		scene::Object::InitParam param;
		param.name = "2";
		param.pos(4, 0, 0);
		param.scale(1, 1, 1);
		param.modelKey = "tetra_PTN";
		param.renderPhase["forward"] = "PTN";
		sceneInfo.addObject(param);
	}
	{
		scene::RenderPhase::InitParam param;
		param.name = "forward";
		param.priority = 0;
		param.rendertarget = scene::RenderPhase::RENDERTARGET::eBACK_BUFFER;
		sceneInfo.addRenderPhase(param);
	}
	sceneInfo.initResource(graphics.device());
}

bool SceneEditorScene::paint(utility::Graphics& graphics, scene::SceneInfo& sceneInfo)
{
	this->mCamera.update();
	graphics.waitForCurrentFrame();

	graphics.currentContext().record(nullptr, [&](DX12GraphicsCommandList& cmdList) {
		this->mDHPool.bind(cmdList);

		{//‹¤’Êƒpƒ‰ƒ[ƒ^‚ÌÝ’è
			shader::DirectionLightParam lightParam;
			lightParam.direction = math::normalize(math::float3(0, -1, 0));
			lightParam.color(1, 1, 1);
			auto lightAllocationInfo = this->mCBPool.allocate(&lightParam, sizeof(lightParam));

			CameraParam cameraParam;
			cameraParam.eyePos = this->mCamera.pos();
			cameraParam.viewProj = this->mCamera.viewProjMatrix();
			auto cameraAllocationInfo = this->mCBPool.allocate(&cameraParam, sizeof(cameraParam));
			for (auto& it : sceneInfo.pipelineHash()) {
				auto& pipeline = it.second.pipelineSet();
				if (pipeline.isExistBindingInfo("CBCamera", utility::RootSignatureInfo::Key::eCBV)) {
					pipeline.updateCBV("CBCamera", this->mDHPool, cameraAllocationInfo);
				}
				if (pipeline.isExistBindingInfo("CBLight", utility::RootSignatureInfo::Key::eCBV)) {
					pipeline.updateCBV("CBLight", this->mDHPool, lightAllocationInfo);
				}
			}
		}

		for (auto& pPhase : sceneInfo.renderPhasePriorityOrder()) {
			unless(pPhase->isEnable()) continue;

			pPhase->begin(graphics, cmdList);
			for (auto& it : sceneInfo.objectHash()) {
				auto& obj = it.second;
				unless(obj.isEnable()) continue;

				auto& phaseHash = obj.renderPhaseHash();
				auto it = phaseHash.find(pPhase->name());
				if (it == phaseHash.end()) {
					continue;
				}

				auto& pipeline = sceneInfo.pipelineHash().at(it->second);
				auto& model = sceneInfo.modelHash()[obj.modelKey()];
				unless(pipeline.isEnable() && model.isEnable()) continue;


				auto& orientation = obj.orientation();
				auto p = pipeline.pipelineSet().clone({});
				p.updateCBV("CBModel", this->mDHPool, &orientation, sizeof(orientation), this->mCBPool);
				p.updateCBV("CBMaterial", this->mDHPool, &model.materialParam(), sizeof(model.materialParam()), this->mCBPool);

				if (p.isExistBindingInfo("txTexture", utility::RootSignatureInfo::Key::eSRV)) {
					auto& texture = sceneInfo.textureHash().at(model.diffuseTexKey());
					p.updateSRV("txTexture", this->mDHPool, texture, &texture.view().srv());
				}
				if (p.isExistBindingInfo("smSampler", utility::RootSignatureInfo::Key::eSAMPLER)) {
					DX12SamplerDesc sampler;
					p.updateSampler("smSampler", this->mDHPool, &sampler);
				}

				p.bind(cmdList);
				model.mesh().bind(cmdList);
				model.mesh().draw(cmdList);
			}
			pPhase->end(graphics, cmdList);
		}

		cmdList.setBarriers(graphics.currentRenderTarget().makeBarrier(D3D12_RESOURCE_STATE_PRESENT));
	});

	graphics.present(1, { graphics.currentContext().cmdList() });
	graphics.endAndGoNextFrame();
	this->mCBPool.endAndGoNextFrame(graphics.cmdQueue());
	this->mDHPool.endAndGoNextFrame(graphics.cmdQueue());

	return true;
}

