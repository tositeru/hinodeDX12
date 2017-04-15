#include "stdafx.h"

#include "JsonUtility.h"

using namespace hinode;

namespace scene
{
	math::float3 toFloat3(const json11::Json& json, const math::float3& defaultValue)
	{
		if (json.is_array()) {
			math::float3 result;
			auto& e = json.array_items();
			const auto count = std::min<size_t>(result.COL, e.size());
			for (auto i = 0u; i < count; ++i) {
				if (e[i].is_number()) {
					result.value[i] = static_cast<float>(e[i].number_value());
				} else if (e[i].is_string()) {
					result.value[i] = std::stof(e[i].string_value());
				}
			}
			return result;
		} else {
			return defaultValue;
		}
	}

	hinode::math::float4 toFloat4(const json11::Json& json, const hinode::math::float4& defaultValue)
	{
		if (json.is_array()) {
			math::float4 result;
			auto& e = json.array_items();
			const auto count = std::min<size_t>(result.COL, e.size());
			for (auto i = 0u; i < count; ++i) {
				if (e[i].is_number()) {
					result.value[i] = static_cast<float>(e[i].number_value());
				} else if (e[i].is_string()) {
					result.value[i] = std::stof(e[i].string_value());
				}
			}
			return result;
		} else {
			return defaultValue;
		}
	}
}
