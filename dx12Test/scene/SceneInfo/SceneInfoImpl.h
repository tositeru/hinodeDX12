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
		/// @brief オブジェクトを追加する
		/// @param[out] pOut
		/// @param[in] param
		/// @param[in] modelHash
		/// @retval bool trueなら追加された
		bool addObject(ObjectHash* pOut, const Object::InitParam& param, const ModelHash& modelHash)noexcept;

		/// @brief 描画フェーズを追加する
		///
		/// この関数で追加したものはSceneInfo::initResource関数を呼び出すまで、内部リソースを作成しません。
		/// @param[in] param
		/// @retval bool　trueなら追加された
		bool addRenderPhase(RenderPhaseHash* pOut, RenderPhaseOrderContainer* pOutOrder, const RenderPhase::InitParam& param)noexcept;

		/// @brief モデルを追加する
		/// 
		/// この関数で追加したものはSceneInfo::initResource関数を呼び出すまで、内部リソースを作成しません。
		/// @param[in] param
		/// @retval bool　trueなら追加された
		bool addModel(ModelHash* pOut, const Model::InitParam& param)noexcept;

		/// @brief パイプラインを追加する
		/// 
		/// この関数で追加したものはSceneInfo::initResource関数を呼び出すまで、内部リソースを作成しません。
		/// @param[in] param
		/// @retval bool　trueなら追加された
		bool addPipeline(PipelineHash* pOut, const Pipeline::InitParam& param)noexcept;

	};
}