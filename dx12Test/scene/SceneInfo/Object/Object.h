#pragma once

#include <string>
#include <unordered_map>
#include <json11/json11.hpp>

#include <graphics/dx12/common/Common.h>
#include <graphics/dx12/utility/math/SimpleMath.h>

#include "../Element.h"

namespace scene
{
	/// @brief シーン内で使用するパイプラインを表すクラス
	class Object : public Element
	{
	public:
		struct InitParam
		{
			std::string name="";
			hinode::math::float3 pos = hinode::math::float3{0, 0, 0};
			hinode::math::quaternion rota = hinode::math::quaternion{0, 0, 0, 1};
			hinode::math::float3 scale = hinode::math::float3{1, 1, 1};
			std::string modelKey = "";
			std::unordered_map<std::string, std::string> renderPhase;
		};

	public:
		Object();
		Object(Object&& right);
		~Object();
		Object& operator=(Object&& right);

		bool create(const json11::Json& json)noexcept;
		bool create(const InitParam& param)noexcept;
		json11::Json makeJson()const;
		bool update(const json11::Json& json)noexcept;

	accessor_declaration:
		const std::string& name()const noexcept override;
		const hinode::math::float4x4& orientation()const noexcept;
		const std::string& modelKey()const noexcept;
		const std::unordered_map<std::string, std::string>& renderPhaseHash()const noexcept;

	private:

		class impl;///< パイプラインのタイプに応じて処理を切り替えるためのクラス
		std::unique_ptr<impl> mpImpl;
	};

}
