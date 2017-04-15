#pragma once

#include <mutex>
#include <json11/json11.hpp>
#include "SceneInfoImpl.h"

namespace scene
{
	class SceneInfo;

	/// @brief 
	///
	/// 
	class SceneInfoMediator : public SceneInfoImpl
	{
	public:
		struct Result
		{
			bool isSuccess = false;
			std::string message = "error";
		};

	public:
		SceneInfoMediator();
		~SceneInfoMediator();

		void init(SceneInfo* pTarget);

		/// @brief ���������Ɏw�肳�ꂽSceneInfo�Ɍ��ݕێ����Ă���f�[�^���󂯓n��
		/// �󂯓n������A���̃N���X�������Ă����f�[�^�͑S�ăN���A�[�����
		void deliver();

		/// @brief �I�u�W�F�N�g��ǉ�����
		/// @param[in] param
		/// @retval Result
		Result addObject(const Object::InitParam& param)noexcept;

		/// @brief �`��t�F�[�Y��ǉ�����
		///
		/// ���̊֐��Œǉ��������̂�SceneInfo::initResource�֐����Ăяo���܂ŁA�������\�[�X���쐬���܂���B
		/// @param[in] param
		/// @retval Result
		Result addRenderPhase(const RenderPhase::InitParam& param)noexcept;

		/// @brief ���f����ǉ�����
		/// 
		/// ���̊֐��Œǉ��������̂�SceneInfo::initResource�֐����Ăяo���܂ŁA�������\�[�X���쐬���܂���B
		/// @param[in] param
		/// @retval Result
		Result addModel(const Model::InitParam& param)noexcept;

		/// @brief �p�C�v���C����ǉ�����
		/// 
		/// ���̊֐��Œǉ��������̂�SceneInfo::initResource�֐����Ăяo���܂ŁA�������\�[�X���쐬���܂���B
		/// @param[in] param
		/// @retval Result
		Result addPipeline(const Pipeline::InitParam& param)noexcept;

		/// @brief �I�u�W�F�N�g��ǉ�����
		/// @param[in] param
		/// @retval Result
		Result addObject(const json11::Json& param)noexcept;

		/// @brief �`��t�F�[�Y��ǉ�����
		///
		/// ���̊֐��Œǉ��������̂�SceneInfo::initResource�֐����Ăяo���܂ŁA�������\�[�X���쐬���܂���B
		/// @param[in] param
		/// @retval Result
		Result addRenderPhase(const json11::Json& param)noexcept;

		/// @brief ���f����ǉ�����
		/// 
		/// ���̊֐��Œǉ��������̂�SceneInfo::initResource�֐����Ăяo���܂ŁA�������\�[�X���쐬���܂���B
		/// @param[in] param
		/// @retval Result
		Result addModel(const json11::Json& param)noexcept;

		/// @brief �p�C�v���C����ǉ�����
		/// 
		/// ���̊֐��Œǉ��������̂�SceneInfo::initResource�֐����Ăяo���܂ŁA�������\�[�X���쐬���܂���B
		/// @param[in] param
		/// @retval Result
		Result addPipeline(const json11::Json& param)noexcept;

		/// @brief �폜������̂�o�^����
		///
		/// @param[in] name
		/// @param[in] type
		/// @retval Result
		void addDeleteElement(const std::string& name, ELEMENT_TYPE type)noexcept;

		/// @brief �X�V������̂�o�^����
		/// @param[in] name
		/// @param[in] type
		/// @param[in] json
		void addUpdateElement(const std::string& name, ELEMENT_TYPE type, const json11::Json& json)noexcept;

		/// @brief �}���`�X���b�h�p�Ƀ��b�N���擾����
		///
		///	���̊֐��Ŏ擾������Alock�֐����Ăяo���܂Ŏ��ۂɃ��b�N���܂���B
		std::unique_lock<std::mutex> getLock()noexcept;

	public:
		bool isExistData()const noexcept;

	private:
		SceneInfo* mpTarget;
		ObjectHash mObjectHash;
		ModelHash mModelHash;
		TextureHash mTextureHash;
		PipelineHash mPipelineHash;
		RenderPhaseHash mRenderPhase;

		struct UpdateData {
			ELEMENT_TYPE type;
			json11::Json json;

			bool operator==(const UpdateData& right)const noexcept {
				return this->type == right.type;
			}
			bool operator!=(const UpdateData& right)const noexcept {
				return this->type != right.type;
			}
		};
		std::unordered_map<std::string, std::list<UpdateData>> mUpdateList;
		std::unordered_map<std::string, std::list<ELEMENT_TYPE>> mDeleteList;
		std::mutex mMutex;
	};
}