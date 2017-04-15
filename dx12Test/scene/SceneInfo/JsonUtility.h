#pragma once

#include <json11/json11.hpp>
#include <graphics/dx12/utility/math/SimpleMath.h>

namespace scene
{
	template<typename T>
	T toNumber(const json11::Json& json, const T& defaultValue)
	{
		if (json.is_number()) {
			return static_cast<T>(json.number_value());
		} else {
			return defaultValue;
		}
	}

	hinode::math::float3 toFloat3(const json11::Json& json, const hinode::math::float3& defaultValue);
	hinode::math::float4 toFloat4(const json11::Json& json, const hinode::math::float4& defaultValue);

}

