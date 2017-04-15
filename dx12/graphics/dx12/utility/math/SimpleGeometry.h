#pragma once

#include <cassert>
#include <algorithm>
#include <stdint.h>
#include <string>
#include <type_traits>
#include <functional>

#include "SimpleMath.h"

namespace hinode
{
	///  @brief 3次元の図形をまとめたもの
	namespace geometory3D
	{
		/// @brief 中心と各辺の長さの半分を持つAABB
		/// @tparam T
		template<typename  T>
		struct AABBBase
		{
			using type = T;
			using VectorType = math::Vector3<T>;

			VectorType center;
			VectorType range; ///< 各辺の長さの半分

			AABBBase() = default;

			AABBBase(const type& cx, const type& cy, const type& cz, const type& rangeX, const type& rangeY, const type& rangeZ)noexcept
				: center(cs, cy, cz)
				, range(abs(rangeX), abs(rangeY), abs(rangeZ))
			{}

			template<typename Vector1, typename Vector2>
			AABBBase(const Vector1& center, const Vector2& range)noexcept
				: center(center)
				, range(abs(range))
			{}

			/// @brief 最小位置を返す
			/// @retval VectorType
			VectorType minPoint()const noexcept {
				return this->center - this->range;
			}

			/// @brief 最大位置を返す
			/// @retval VectorType
			VectorType maxPoint()const noexcept {
				return this->center + this->range;
			}
		};
		using AABB = AABBBase<float>;
		using IAABB = AABBBase<int>;
		using UAABB = AABBBase<unsigned int>;


		/// @brief 球体
		/// @tparam T
		template<typename T>
		struct SphereBase
		{
			using type = T;
			using VectorType = math::Vector3<T>;

			VectorType center;
			type radius;

			SphereBase() noexcept
				: center(), radius(0)
			{}

			SphereBase(const type& x, const type& y, const type& z, const type& radius) noexcept
				: center(x, y, z), radius(abs(radius))
			{}

			template<typename VectorType1>
			SphereBase(const VectorType1& center, const type& radius) noexcept
				: center(center), radius(abs(radius))
			{}
		};
		using Sphere = SphereBase<float>;
		using ISphere = SphereBase<int>;
		using USphere = SphereBase<unsigned int>;

		/// @brief カプセル
		/// @tparam T
		template<typename T>
		struct CapsuleBase
		{
			using type = T;
			using VectorType = math::Vector3<T>;

			VectorType start;
			VectorType end;
			type radius;

			CapsuleBase()noexcept
				: start(), end(), radius(0.f)
			{}

			CapsuleBase(const type& sx, const type& sy, const type& sz, const type& ex, const type& ey, const type& ez, const type& radius)noexcept
				: start(sx, sy, sz), end(ex, ey, ez), radius(abs(radius))
			{}

			template<typename Vector1, typename Vector2>
			CapsuleBase(const Vector1& start, const Vector2& end, const type& radius)noexcept
				: start(start), end(end), range(abs(radius))
			{}

			void calDistanceVec(VectorType* pOutDistanceVec, type* pOutRadius, const VectorType& point)const noexcept
			{
				auto vec = this->end - this->start;
				auto vecNormalized = normalize(vec);
				auto PC = point - this->start;
				auto proj = dot(PC, vecNormalized);

				float disRate = 0.f;
				if (proj < 0) {
					if (pOutDistanceVec) *pOutDistanceVec = PC;
					if (pOutRadius) *pOutRadius = this->radius;
				} else if (length(vec) <= proj) {
					if (pOutDistanceVec) *pOutDistanceVec = point - this->end;
					if (pOutRadius) *pOutRadius = this->radius;
				} else {
					auto onLine = vecNormalized * proj;
					if (pOutDistanceVec) *pOutDistanceVec = PC - onLine;
					if (pOutRadius) *pOutRadius = this->startRadius + (this->endRadius - this->startRadius) * ((proj*proj) / lengthSq(vec));
				}
			}
		};
		using Capsule = CapsuleBase<float>;
		using ICapsule = CapsuleBase<int>;
		using UCapsule = CapsuleBase<unsigned int>;

		/// @brief 始点と終点の円の半径が異なるカプセル
		/// @tparam T
		template<typename T>
		struct CapsuleExBase
		{
			using type = T;
			using VectorType = math::Vector3<T>;

			VectorType start;
			VectorType end;
			type startRadius;
			type endRadius;

			CapsuleExBase()noexcept
				: start(), end(), startRadius(0.f), endRadius(0.f)
			{}

			CapsuleExBase(const type& sx, const type& sy, const type& sz, const type& startRadius, const type& ex, const type& ey, const type& ez, const type& endRadius)noexcept
				: start(sx, sy, sz), end(ex, ey, ez), startRadius(abs(startRadius)), endRadius(abs(endRadius))
			{}

			template<typename Vector1, typename Vector2>
			CapsuleExBase(const Vector1& start, const type& startRadius, const Vector2& end, const type& endRadius)noexcept
				: start(start), end(end), startRadius(abs(startRadius)), endRadius(abs(endRadius))
			{}

			/// @brief pointとの最短距離をを表すベクトルとそのベクトルがカプセル表面と交差する場所の半径を計算する
			/// @param[out] pOutDistanceVec
			/// @param[out] pOutRadius
			/// @param[in] point
			void calDistanceVector(VectorType* pOutDistanceVec, type* pOutRadius, const VectorType& point)const noexcept
			{
				auto vec = this->end - this->start;
				auto vecNormalized = normalize(vec);
				auto PC = point - this->start;
				auto proj = dot(PC, vecNormalized);

				float disRate = 0.f;
				if (proj < 0) {
					if (pOutDistanceVec) *pOutDistanceVec = PC;
					if (pOutRadius) *pOutRadius = this->startRadius;
				} else if (length(vec) <= proj) {
					if (pOutDistanceVec) *pOutDistanceVec = point - this->end;
					if (pOutRadius) *pOutRadius = this->endRadius;
				} else {
					auto onLine = vecNormalized * proj;
					if(pOutDistanceVec) *pOutDistanceVec = PC - onLine;
					if(pOutRadius) *pOutRadius = this->startRadius + (this->endRadius- this->startRadius) * ((proj*proj) / lengthSq(vec));
				}
			}
		};
		using CapsuleEx = CapsuleExBase<float>;
		using ICapsuleEx = CapsuleExBase<int>;
		using UCapsuleEx = CapsuleExBase<unsigned int>;

		/// @brief 点とAABBのあたり判定
		/// @tparam PointType
		/// @tparam AABBType
		/// @param[in] point
		/// @param[in] aabb
		/// @retval bool true:当たっている
		template<typename PointType, typename AABBType>
		bool isHit(const math::Vector3<PointType>& point, const AABBBase<AABBType>& aabb)noexcept
		{
			return 
				abs(point.x - aabb.center.x) < aabb.range.x &&
				abs(point.y - aabb.center.y) < aabb.range.y &&
				abs(point.z - aabb.center.z) < aabb.range.z;
		}

		/// @brief 点と球体のあたり判定
		/// @tparam PointType
		/// @tparam SphereType
		/// @param[in] point
		/// @param[in] sphere
		/// @retval bool true:当たっている
		template<typename PointType, typename SphereType>
		bool isHit(const math::Vector3<PointType>& point, const SphereBase<SphereType>& sphere)noexcept
		{
			auto lenSq = math::lengthSq(point - sphere.center);
			return lenSq <= (sphere.radius*sphere.radius);
		}

		/// @brief 点とカプセルのあたり判定
		/// @tparam PointType
		/// @tparam CapsuleType
		/// @param[in] point
		/// @param[in] capsule
		/// @retval bool true:当たっている
		template<typename PointType, typename CapsuleType>
		bool isHit(const math::Vector3<PointType>& point, const CapsuleBase<CapsuleType>& capsule)noexcept
		{
			using Vec3 = math::Vector3<PointType>;
			Vec3 distanceVec;
			CapsuleType radius;
			capsule.calDistanceVec(&distanceVec, &radius, point);
			return math::lengthSq(distanceVec) <= radius;
		}

		/// @brief 点とカプセルEXのあたり判定
		/// @tparam PointType
		/// @tparam CapsuleType
		/// @param[in] point
		/// @param[in] capsule
		/// @retval bool true:当たっている
		template<typename PointType, typename CapsuleType>
		bool isHit(const math::Vector3<PointType>& point, const CapsuleExBase<CapsuleType>& capsule)noexcept
		{
			using Vec3 = math::Vector3<PointType>;
			Vec3 distanceVec;
			CapsuleType radius;
			capsule.calDistanceVec(&distanceVec, &radius, point);
			return math::lengthSq(distanceVec) <= radius;
		}
	}
}
