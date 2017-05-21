#include "stdafx.h"

#include "DrafterCamera.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{

			DrafterCamera::DrafterCamera()
			{
				const float RATE = 1 / 60.f;
				this->mKeyObserverMng.add({ 'A', false, [&, RATE](const winapi::KeyDownObserver& This) {
					this->mVelocity -= this->mParam.right * this->mParam.speed * RATE;
				} });
				this->mKeyObserverMng.add({ 'D', false, [&, RATE](const winapi::KeyDownObserver& This) {
					this->mVelocity += this->mParam.right * this->mParam.speed * RATE;
				} });
				this->mKeyObserverMng.add({ 'S', false, [&, RATE](const winapi::KeyDownObserver& This) {
					this->mVelocity -= this->mParam.up * this->mParam.speed * RATE;
				} });
				this->mKeyObserverMng.add({ 'W', false, [&, RATE](const winapi::KeyDownObserver& This) {
					this->mVelocity += this->mParam.up * this->mParam.speed * RATE;
				} });
				this->mKeyObserverMng.add({ 'Q', false, [&, RATE](const winapi::KeyDownObserver& This) {
					this->mVelocity -= this->mFront * this->mParam.speed * RATE;
				} });
				this->mKeyObserverMng.add({ 'E', false, [&, RATE](const winapi::KeyDownObserver& This) {
					this->mVelocity += this->mFront * this->mParam.speed * RATE;
				} });

				//this->mKeyObserverMng.addMouse({ VK_RBUTTON, false, [&](const winapi::MouseObserver& This) {
				//	auto pos = winapi::MouseObserver::sGetCursor();
				//	auto prevCursorPos = This.prevCursorPos();

				//	math::float2* pTarget = &this->mRotaVelocity;

				//	pTarget->y -= (pos.x - prevCursorPos.x) * this->mParam.rotaSpeed / 60.f;
				//	pTarget->x += (pos.y - prevCursorPos.y) * this->mParam.rotaSpeed / 60.f;
				//} });
			}

			DrafterCamera::~DrafterCamera()
			{

			}

			void DrafterCamera::init(const InitParam& initParam)
			{
				this->mParam = initParam;
				this->mParam.right = math::normalize(this->mParam.right);
				this->mParam.up = math::normalize(this->mParam.up);
				this->mFront = math::normalize(math::cross(this->mParam.up, this->mParam.right));
			}

			void DrafterCamera::update()
			{
				this->mKeyObserverMng.update();
				{//カメラの位置更新
					auto targetPos = this->mParam.pos + this->mVelocity;
					this->mParam.pos = math::lerp(this->mParam.pos, targetPos, 0.5f);
					this->mVelocity *= 0.9f;
				}

				//外で使われるパラメータの更新
				auto target = this->mParam.pos + this->mFront;
				this->mViewMatrix = math::makeView(this->mParam.pos, target, this->mParam.up);
				this->mProjectionMatrix = math::makePerspective(this->mParam.planeZ.x, this->mParam.planeZ.y, this->mParam.fov, this->mParam.aspect);
				this->mViewProjMatrix = math::mul(this->mViewMatrix, this->mProjectionMatrix);
			}

			const math::float3& DrafterCamera::pos()const noexcept
			{
				return this->mParam.pos;
			}

			const math::float4x4& DrafterCamera::viewMatrix()const noexcept
			{
				return this->mViewMatrix;
			}

			const math::float4x4& DrafterCamera::projectionMatrix()const noexcept
			{
				return this->mProjectionMatrix;
			}

			const math::float4x4& DrafterCamera::viewProjMatrix()const noexcept
			{
				return this->mViewProjMatrix;
			}

			const math::float2& DrafterCamera::planeZ()const noexcept
			{
				return this->mParam.planeZ;
			}
		}
	}
}
