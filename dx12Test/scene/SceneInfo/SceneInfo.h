#pragma once

#include <unordered_map>
#include <json11/json11.hpp>
#include <graphics/dx12/resource/DX12Resource.h>
#include <graphics/dx12/utility/math/SimpleMath.h>
#include <graphics/dx12/utility/Mesh/Mesh.h>
#include <graphics/dx12/utility/Shader/PipelineSet/PipelineSet.h>
#include <graphics/dx12/utility/Mesh/MeshPipeline/MeshPipeline.h>

#include "SceneInfoImpl.h"

#include "Pipeline/Pipeline.h"
#include "Model/Model.h"
#include "Object/Object.h"
#include "RenderPhase/RenderPhase.h"

namespace scene
{
	class SceneInfo : private SceneInfoImpl
	{
	public:
		SceneInfo();
		~SceneInfo();

		void clear();
		void init(const json11::Json& json);
		bool load(const std::string& filepath);

		void mergeObject(ObjectHash&& hash);
		void mergeModel(ModelHash&& hash);
		void mergePipeline(PipelineHash&& hash);
		void mergeRenderPhase(RenderPhaseHash&& hash);

		/// @brief �܂����\�[�X���쐬���Ă��Ȃ��v�f��T���āA���\�[�X���쐬����
		/// @param[in] pDevice
		/// @param[in] nodeMask
		void initResource(ID3D12Device* pDevice, UINT nodeMask = 0u);

		/// @brief �I�u�W�F�N�g��ǉ�����
		/// @param[in] param
		/// @retval bool
		bool addObject(const Object::InitParam& param)noexcept;

		/// @brief �`��t�F�[�Y��ǉ�����
		///
		/// ���̊֐��Œǉ��������̂�SceneInfo::initResource�֐����Ăяo���܂ŁA�������\�[�X���쐬���܂���B
		/// @param[in] param
		/// @retval bool
		bool addRenderPhase(const RenderPhase::InitParam& param)noexcept;

		/// @brief ���f����ǉ�����
		/// 
		/// ���̊֐��Œǉ��������̂�SceneInfo::initResource�֐����Ăяo���܂ŁA�������\�[�X���쐬���܂���B
		/// @param[in] param
		/// @retval bool
		bool addModel(const Model::InitParam& param)noexcept;

		/// @brief �p�C�v���C����ǉ�����
		/// 
		/// ���̊֐��Œǉ��������̂�SceneInfo::initResource�֐����Ăяo���܂ŁA�������\�[�X���쐬���܂���B
		/// @param[in] param
		/// @retval bool
		bool addPipeline(const Pipeline::InitParam& param)noexcept;

		/// @brief �w�肵�����O�̃I�u�W�F�N�g���폜����
		/// @param[in] name
		/// @retval bool
		bool deleteObject(const std::string& name)noexcept;

		/// @brief �w�肵�����O�̃��f�����폜����
		/// @param[in] name
		/// @retval bool
		bool deleteModel(const std::string& name)noexcept;

		/// @brief �w�肵�����O�̃p�C�v���C�����폜����
		/// @param[in] name
		/// @retval bool
		bool deletePipeline(const std::string& name)noexcept;

		/// @brief �w�肵�����O�̕`��t�F�[�Y���폜����
		/// @param[in] name
		/// @retval bool
		bool deleteRenderPhase(const std::string& name)noexcept;

		/// @brief �w�肵���I�u�W�F�N�g���X�V����
		/// @param[in] json
		/// @retval bool
		bool updateObject(const std::string& name, const json11::Json& json)noexcept;

	accessor_declaration:
		ObjectHash& objectHash()noexcept;
		ModelHash& modelHash()noexcept;
		TextureHash& textureHash()noexcept;
		PipelineHash& pipelineHash()noexcept;
		RenderPhaseHash& renderPhaseHash()noexcept;
		RenderPhaseOrderContainer& renderPhasePriorityOrder()noexcept;

		const ObjectHash& objectHash()const noexcept;
		const ModelHash& modelHash()const noexcept;
		const TextureHash& textureHash()const noexcept;
		const PipelineHash& pipelineHash()const noexcept;
		const RenderPhaseHash& renderPhaseHash()const noexcept;
		const RenderPhaseOrderContainer& renderPhasePriorityOrder()const noexcept;

	private:
		void initPipeline(const json11::Json& pipelineJson);
		void initModel(const json11::Json& modelJson);
		void initObject(const json11::Json& objectJson);

	private:
		ObjectHash mObjectHash;
		ModelHash mModelHash;
		TextureHash mTextureHash;
		PipelineHash mPipelineHash;
		RenderPhaseHash mRenderPhaseHash;
		RenderPhaseOrderContainer mpRenderPhasePriorityOrder;
	};
}
