#include "stdafx.h"

#include "PreviewCamera.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			PreviewCamera::PreviewCamera()
				: mRight(1, 0, 0)
			{
				this->mKeyObserverMng.add({ 'A', false, [&](const winapi::KeyDownObserver& This) {
					this->mVelocity -= this->mRight * this->mParam.speed / 60.f;
				} });
				this->mKeyObserverMng.add({ 'D', false, [&](const winapi::KeyDownObserver& This) {
					this->mVelocity += this->mRight * this->mParam.speed / 60.f;
				} });
				this->mKeyObserverMng.add({ 'S', false, [&](const winapi::KeyDownObserver& This) {
					this->mVelocity -= this->mParam.front * this->mParam.speed / 60.f;
				} });
				this->mKeyObserverMng.add({ 'W', false, [&](const winapi::KeyDownObserver& This) {
					this->mVelocity += this->mParam.front * this->mParam.speed / 60.f;
				} });
				this->mKeyObserverMng.add({ 'Q', false, [&](const winapi::KeyDownObserver& This) {
					this->mVelocity -= this->mParam.up * this->mParam.speed / 60.f;
				} });
				this->mKeyObserverMng.add({ 'E', false, [&](const winapi::KeyDownObserver& This) {
					this->mVelocity += this->mParam.up * this->mParam.speed / 60.f;
				} });
				this->mKeyObserverMng.add({ 'Z', false, [&](const winapi::KeyDownObserver& This) {
					mIsResetCameraUp = true;
				} });

				this->mKeyObserverMng.addMouse({ VK_RBUTTON, false, [&](const winapi::MouseObserver& This) {
					auto pos = winapi::MouseObserver::sGetCursor();
					auto prevCursorPos = This.prevCursorPos();

					math::float2* pTarget = &this->mRotaVelocity;

					pTarget->y -= (pos.x - prevCursorPos.x) * this->mParam.rotaSpeed / 60.f;
					pTarget->x += (pos.y - prevCursorPos.y) * this->mParam.rotaSpeed / 60.f;
				} });
			}

			PreviewCamera::~PreviewCamera()
			{

			}

			void PreviewCamera::init(const InitParam& initParam)
			{
				this->mParam = initParam;
				this->mRight = normalize(cross(initParam.up, initParam.front));
			}

			void PreviewCamera::update()
			{
				this->mKeyObserverMng.update();
				{//カメラの位置更新
					auto targetPos = this->mParam.pos + this->mVelocity;
					this->mParam.pos = math::lerp(this->mParam.pos, targetPos, 0.5f);
					this->mVelocity *= 0.9f;
				}

				{//カメラの向き更新
					auto right = math::normalize(math::cross(this->mParam.front, this->mParam.up));
					auto rotaPitch = math::makeRotationAxis<math::float3x3>(right, this->mRotaVelocity.x);
					this->mParam.front = mul(this->mParam.front, rotaPitch);
					this->mParam.up = mul(this->mParam.up, rotaPitch);

					auto rotaYaw = math::makeRotationAxis<math::float3x3>(this->mParam.up, this->mRotaVelocity.y);
					this->mParam.front = mul(this->mParam.front, rotaYaw);

					this->mParam.front = math::normalize(this->mParam.front);
					this->mParam.up = math::normalize(this->mParam.up);
					this->mRight = normalize(cross(this->mParam.up, this->mParam.front));
					this->mRotaVelocity *= 0.9f;
				}
				//自動で上方向を修正したほうが操作しやすかった
				if (true || this->mIsResetCameraUp) {
					auto right = math::cross(this->mParam.front, math::float3(0, 1, 0));
					if (0.f < math::lengthSq(right)) {
						right = math::normalize(right);
						auto Up = math::normalize(math::cross(right, this->mParam.front));
						if (math::nearlyEqual(this->mParam.up, Up, 0.01f)) {
							this->mIsResetCameraUp = false;
						} else {
							this->mParam.up = math::normalize(math::lerp(this->mParam.up, Up, 0.05f));
						}
					}
				}
				//外で使われるパラメータの更新
				auto target = this->mParam.pos + this->mParam.front;
				this->mViewMatrix = math::makeView(this->mParam.pos, target, this->mParam.up);
				this->mProjectionMatrix = math::makePerspective(this->mParam.planeZ.x, this->mParam.planeZ.y, this->mParam.fov, this->mParam.aspect);
				this->mViewProjMatrix = math::mul(this->mViewMatrix, this->mProjectionMatrix);
			}

			const math::float3& PreviewCamera::pos()const noexcept
			{
				return this->mParam.pos;
			}

			const math::float4x4& PreviewCamera::viewMatrix()const noexcept
			{
				return this->mViewMatrix;
			}

			const math::float4x4& PreviewCamera::projectionMatrix()const noexcept
			{
				return this->mProjectionMatrix;
			}

			const math::float4x4& PreviewCamera::viewProjMatrix()const noexcept
			{
				return this->mViewProjMatrix;
			}

			const math::float2& PreviewCamera::planeZ()const noexcept
			{
				return this->mParam.planeZ;
			}
		}
	}
}