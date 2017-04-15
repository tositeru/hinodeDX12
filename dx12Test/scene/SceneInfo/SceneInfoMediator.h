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

		/// @brief 初期化時に指定されたSceneInfoに現在保持しているデータを受け渡す
		/// 受け渡した後、このクラスが持っていたデータは全てクリアーされる
		void deliver();

		/// @brief オブジェクトを追加する
		/// @param[in] param
		/// @retval Result
		Result addObject(const Object::InitParam& param)noexcept;

		/// @brief 描画フェーズを追加する
		///
		/// この関数で追加したものはSceneInfo::initResource関数を呼び出すまで、内部リソースを作成しません。
		/// @param[in] param
		/// @retval Result
		Result addRenderPhase(const RenderPhase::InitParam& param)noexcept;

		/// @brief モデルを追加する
		/// 
		/// この関数で追加したものはSceneInfo::initResource関数を呼び出すまで、内部リソースを作成しません。
		/// @param[in] param
		/// @retval Result
		Result addModel(const Model::InitParam& param)noexcept;

		/// @brief パイプラインを追加する
		/// 
		/// この関数で追加したものはSceneInfo::initResource関数を呼び出すまで、内部リソースを作成しません。
		/// @param[in] param
		/// @retval Result
		Result addPipeline(const Pipeline::InitParam& param)noexcept;

		/// @brief オブジェクトを追加する
		/// @param[in] param
		/// @retval Result
		Result addObject(const json11::Json& param)noexcept;

		/// @brief 描画フェーズを追加する
		///
		/// この関数で追加したものはSceneInfo::initResource関数を呼び出すまで、内部リソースを作成しません。
		/// @param[in] param
		/// @retval Result
		Result addRenderPhase(const json11::Json& param)noexcept;

		/// @brief モデルを追加する
		/// 
		/// この関数で追加したものはSceneInfo::initResource関数を呼び出すまで、内部リソースを作成しません。
		/// @param[in] param
		/// @retval Result
		Result addModel(const json11::Json& param)noexcept;

		/// @brief パイプラインを追加する
		/// 
		/// この関数で追加したものはSceneInfo::initResource関数を呼び出すまで、内部リソースを作成しません。
		/// @param[in] param
		/// @retval Result
		Result addPipeline(const json11::Json& param)noexcept;

		/// @brief 削除するものを登録する
		///
		/// @param[in] name
		/// @param[in] type
		/// @retval Result
		void addDeleteElement(const std::string& name, ELEMENT_TYPE type)noexcept;

		/// @brief 更新するものを登録する
		/// @param[in] name
		/// @param[in] type
		/// @param[in] json
		void addUpdateElement(const std::string& name, ELEMENT_TYPE type, const json11::Json& json)noexcept;

		/// @brief マルチスレッド用にロックを取得する
		///
		///	この関数で取得した後、lock関数を呼び出すまで実際にロックしません。
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