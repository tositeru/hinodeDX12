#pragma once

#include "../math/SimpleMath.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			struct PVertex
			{
				math::float3 pos;
				PVertex() = default;
				PVertex(const math::float3& pos)noexcept
					: pos(pos)
				{}
			};

			struct PCVertex
			{
				math::float3 pos;
				uint32_t color;
				PCVertex()noexcept
				{}

				PCVertex(const math::float3& pos, const uint32_t& color)noexcept
					: pos(pos)
					, color(color)
				{}
			};

			struct PTVertex
			{
				math::float3 pos;
				math::float2 texcoord;
				PTVertex() = default;
				PTVertex(const math::float3& pos, const math::float2& texcoord)noexcept
					: pos(pos)
					, texcoord(texcoord)
				{}
			};

			struct PNVertex
			{
				math::float3 pos;
				math::float3 normal;

				PNVertex() = default;
				PNVertex(const math::float3& pos, const math::float3& normal)noexcept
					: pos(pos)
					, normal(normal)
				{}
			};

			struct PTNVertex
			{
				math::float3 pos;
				math::float2 texcoord;
				math::float3 normal;

				PTNVertex() = default;
				PTNVertex(const math::float3& pos, const math::float2& texcoord, const math::float3& normal)noexcept
					: pos(pos)
					, texcoord(texcoord)
					, normal(normal)
				{}
			};

			struct PTNCVertex
			{
				math::float3 pos;
				math::float2 texcoord;
				math::float3 normal;
				uint32_t color;

				PTNCVertex() = default;
				PTNCVertex(const math::float3& pos, const math::float2& texcoord, const math::float3& normal, const uint32_t& color)noexcept
					: pos(pos)
					, texcoord(texcoord)
					, normal(normal)
					, color(color)
				{}
			};

		}
	}
}