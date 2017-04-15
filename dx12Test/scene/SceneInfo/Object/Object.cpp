#include "stdafx.h"

#include "Object.h"

#include "../JsonUtility.h"

using namespace hinode;
using namespace hinode::graphics;

namespace scene
{
	//-----------------------------------------------------------------------------------------------------------
	//
	//	scene::Object::impl
	//
	//-----------------------------------------------------------------------------------------------------------
	class Object::impl
	{
		static const std::string KEY_ORIENTATION;
		static const std::string KEY_MODEL;
		static const std::string KEY_RENDER_PHASE;

		static const std::string KEY_ORIENTATION_POS;
		static const std::string KEY_ORIENTATION_ROTATION;
		static const std::string KEY_ORIENTATION_SCALE;

		std::string mName;
		math::float3 mPos;
		math::quaternion mRot;
		math::float3 mScale;
		math::float4x4 mOrientation;

		std::string mModelKey;
		std::unordered_map<std::string, std::string> mRenderPhaseHash;

	public:
		bool create(const json11::Json& json)noexcept
		{
			unless(this->validate(json)) {
				Log(Log::eERROR) << "Pipeline::impl::create : 渡されたjsonデータの内容に対応していません。 json=" << json.dump();
				return false;
			}

			this->mName = json["name"].string_value();

			{//orientation
				auto& orientation = json[KEY_ORIENTATION];

				this->mPos = toFloat3(orientation[KEY_ORIENTATION_POS], {0, 0, 0});
				this->mRot = toFloat4(orientation[KEY_ORIENTATION_ROTATION], { 0, 0, 0, 1 });
				this->mScale = toFloat3(orientation[KEY_ORIENTATION_SCALE], { 1, 1, 1 });

				this->mOrientation = this->makeOrientation();
			}
			{//model
				this->mModelKey = json[KEY_MODEL].string_value();
			}
			{//render_phase
				for (auto it : json[KEY_RENDER_PHASE].object_items()) {
					this->mRenderPhaseHash.insert({it.first, it.second.string_value()});
				}
			}
			return true;
		}

		bool validate(const json11::Json& json)
		{
			unless(json[KEY_ORIENTATION].is_object()) {
				Log(Log::eERROR) << KEY_ORIENTATION << "の値がオブジェクトではありません";
				return false;
			}
			unless(json[KEY_MODEL].is_string()) {
				Log(Log::eERROR) << KEY_MODEL << "の値が文字列ではありません";
				return false;
			}
			unless(json[KEY_RENDER_PHASE].is_object()) {
				Log(Log::eERROR) << KEY_RENDER_PHASE << "の値がオブジェクトではありません";
				return false;
			}

			return true;
		}

		bool create(const InitParam& param)noexcept
		{
			this->mName = param.name;
			this->mPos = param.pos;
			this->mRot = param.rota;
			this->mScale = param.scale;
			this->mOrientation = this->makeOrientation();

			this->mModelKey = param.modelKey;
			this->mRenderPhaseHash = param.renderPhase;
			return true;
		}

		json11::Json makeJson()const
		{
			return json11::Json::object({
				{"name", this->mName },
				{ KEY_ORIENTATION, json11::Json::object({
						{ KEY_ORIENTATION_POS, json11::Json::array({this->mPos.value[0], this->mPos.value[1], this->mPos.value[2]}) },
						{ KEY_ORIENTATION_ROTATION, json11::Json::array({ this->mRot.value[0], this->mRot.value[1], this->mRot.value[2], this->mRot.value[3] }) },
						{ KEY_ORIENTATION_SCALE, json11::Json::array({ this->mScale.value[0], this->mScale.value[1], this->mScale.value[2] }) },
					})
				},
				{KEY_MODEL, this->mModelKey},
				{KEY_RENDER_PHASE, this->mRenderPhaseHash},
			});
		}

		bool update(const json11::Json& json)
		{
			if (json[KEY_ORIENTATION].is_object()) {
				auto& orientation = json[KEY_ORIENTATION];
				if (orientation[KEY_ORIENTATION_POS].is_array()) {
					this->mPos = toFloat3(orientation[KEY_ORIENTATION_POS], { 0, 0, 0 });
				}
				if (orientation[KEY_ORIENTATION_ROTATION].is_array()) {
					this->mRot = toFloat4(orientation[KEY_ORIENTATION_ROTATION], { 0, 0, 0, 1 });
				}
				if (orientation[KEY_ORIENTATION_SCALE].is_array()) {
					this->mScale = toFloat3(orientation[KEY_ORIENTATION_SCALE], { 0, 0, 0 });
				}

				this->mOrientation = this->makeOrientation();
			}

			if (json[KEY_MODEL].is_string()) {
				this->mModelKey = json[KEY_MODEL].string_value();
			}

			for (auto it : json[KEY_RENDER_PHASE].object_items()) {
				this->mRenderPhaseHash[it.first] = it.second.string_value();
			}
			return true;
		}

	accessor_declaration:
		const hinode::math::float4x4& orientation()const noexcept
		{
			return this->mOrientation;
		}

		const std::string& modelKey()const noexcept
		{
			return this->mModelKey;
		}

		const std::unordered_map<std::string, std::string>& renderPhaseHash()const noexcept
		{
			return this->mRenderPhaseHash;
		}

		const std::string& name()const noexcept
		{
			return this->mName;
		}

	private:
		math::float4x4 makeOrientation()const noexcept
		{
			auto result = math::makeScale<math::float4x4>(this->mScale);
			result = math::mul(result, math::convertRota<math::float4x4>(this->mRot));
			result[3] = math::float4(this->mPos, 1);
			return result;
		}

	};

	const std::string Object::impl::KEY_ORIENTATION = "orientation";
	const std::string Object::impl::KEY_MODEL = "model";
	const std::string Object::impl::KEY_RENDER_PHASE = "render_phase";
	const std::string Object::impl::KEY_ORIENTATION_POS = "pos";
	const std::string Object::impl::KEY_ORIENTATION_ROTATION = "rot";
	const std::string Object::impl::KEY_ORIENTATION_SCALE = "scl";

	//-----------------------------------------------------------------------------------------------------------
	//
	//	scene::Object
	//
	//-----------------------------------------------------------------------------------------------------------

	Object::Object()
	{}

	Object::Object(Object&& right)
		: Element(std::move(right))
		, mpImpl(std::move(right.mpImpl))
	{ }

	Object::~Object()
	{}

	Object& Object::operator=(Object&& right)
	{
		this->mpImpl = std::move(right.mpImpl);
		return *this;
	}

	bool Object::create(const json11::Json& json)noexcept
	{
		this->mpImpl = std::make_unique<impl>();
		return this->mpImpl->create(json);
	}

	bool Object::create(const InitParam& param)noexcept
	{
		this->mpImpl = std::make_unique<impl>();
		return this->mpImpl->create(param);
	}

	json11::Json Object::makeJson()const
	{
		assert(nullptr != this->mpImpl);
		return this->mpImpl->makeJson();
	}

	bool Object::update(const json11::Json& json)noexcept
	{
		assert(nullptr != this->mpImpl);
		return this->mpImpl->update(json);
	}

	const std::string& Object::name()const noexcept
	{
		assert(nullptr != this->mpImpl);
		return this->mpImpl->name();
	}

	const hinode::math::float4x4& Object::orientation()const noexcept
	{
		assert(nullptr != this->mpImpl);
		return this->mpImpl->orientation();
	}

	const std::string& Object::modelKey()const noexcept
	{
		assert(nullptr != this->mpImpl);
		return this->mpImpl->modelKey();
	}

	const std::unordered_map<std::string, std::string>& Object::renderPhaseHash()const noexcept
	{
		assert(nullptr != this->mpImpl);
		return this->mpImpl->renderPhaseHash();
	}

}