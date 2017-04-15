#pragma once

#include "../../../common/common.h"
#include "../../winapi/KeyObserver.h"
#include "../../math/SimpleMath.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			class PreviewCamera
			{
			public:
				struct InitParam
				{
					math::float3 pos = math::float3{ 0.f, 0.f, 0.f };
					math::float3 front = math::float3{ 0.f, 0.f, 1.f };
					math::float3 up = math::float3{ 0.f, 1.f, 0.f };
					math::float2 planeZ = math::float2{ 0.01f, 100.f };
					float fov = math::toRadian(45.f);
					float aspect = 1.f;
					float speed = 10.f;
					float rotaSpeed = 0.01f;
				};
			public:
				PreviewCamera();
				~PreviewCamera();

				void init(const InitParam& initParam);

				void update();

			accessor_declaration:
				const math::float3& pos()const noexcept;
				const math::float4x4& viewMatrix()const noexcept;
				const math::float4x4& projectionMatrix()const noexcept;
				const math::float4x4& viewProjMatrix()const noexcept;

				const math::float2& planeZ()const noexcept;

			private:
				InitParam mParam;
				math::float3 mRight;
				math::float3 mVelocity;
				math::float2 mRotaVelocity;
				bool mIsResetCameraUp;

				math::float4x4 mViewMatrix;
				math::float4x4 mProjectionMatrix;
				math::float4x4 mViewProjMatrix;

				winapi::KeyDownObserverManager mKeyObserverMng;
			};
		}
	}
}
