#pragma once

#include "Pipeline/Pipeline.h"
#include "Model/Model.h"
#include "Object/Object.h"
#include "RenderPhase/RenderPhase.h"

namespace scene
{
	class SceneInfoImpl
	{
	public:
		using ObjectHash = std::unordered_map<std::string, Object>;
		using ModelHash = std::unordered_map<std::string, Model>;
		using TextureHash = std::unordered_map<std::string, hinode::graphics::DX12Resource>;
		using PipelineHash = std::unordered_map<std::string, Pipeline>;
		using RenderPhaseHash = std::unordered_map<std::string, RenderPhase>;
		using RenderPhaseOrderContainer = std::vector<RenderPhase*>;

		virtual ~SceneInfoImpl() = 0;

	protected:
		/// @brief �I�u�W�F�N�g��ǉ�����
		/// @param[out] pOut
		/// @param[in] param
		/// @param[in] modelHash
		/// @retval bool true�Ȃ�ǉ����ꂽ
		bool addObject(ObjectHash* pOut, const Object::InitParam& param, const ModelHash& modelHash)noexcept;

		/// @brief �`��t�F�[�Y��ǉ�����
		///
		/// ���̊֐��Œǉ��������̂�SceneInfo::initResource�֐����Ăяo���܂ŁA�������\�[�X���쐬���܂���B
		/// @param[in] param
		/// @retval bool�@true�Ȃ�ǉ����ꂽ
		bool addRenderPhase(RenderPhaseHash* pOut, RenderPhaseOrderContainer* pOutOrder, const RenderPhase::InitParam& param)noexcept;

		/// @brief ���f����ǉ�����
		/// 
		/// ���̊֐��Œǉ��������̂�SceneInfo::initResource�֐����Ăяo���܂ŁA�������\�[�X���쐬���܂���B
		/// @param[in] param
		/// @retval bool�@true�Ȃ�ǉ����ꂽ
		bool addModel(ModelHash* pOut, const Model::InitParam& param)noexcept;

		/// @brief �p�C�v���C����ǉ�����
		/// 
		/// ���̊֐��Œǉ��������̂�SceneInfo::initResource�֐����Ăяo���܂ŁA�������\�[�X���쐬���܂���B
		/// @param[in] param
		/// @retval bool�@true�Ȃ�ǉ����ꂽ
		bool addPipeline(PipelineHash* pOut, const Pipeline::InitParam& param)noexcept;

	};
}