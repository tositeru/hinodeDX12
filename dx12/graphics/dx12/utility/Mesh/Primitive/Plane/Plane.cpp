#include "stdafx.h"

#include "Plane.h"

#include <array>

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			std::array<math::uint3, 3> helper_makePlaneIndexList()
			{
				return{ {
						math::uint3(0, 1, 2),
						math::uint3(2, 1, 0),
						math::uint3(1, 2, 0),
					} };
			}

			std::array<math::float3, 4> helper_makePosSeed()
			{
				return { {
						math::float3(0.5f, 0.5f, 0.f),
						math::float3(-0.5f, 0.5f, 0.f),
						math::float3(0.5f, -0.5f, 0.f),
						math::float3(-0.5f, -0.5f, 0.f),
					} };
			}

			std::array<uint32_t, 4> helper_makeColor()
			{
				return{ {
						0xffff6666,
						0xff66ff66,
						0xff6666ff,
						0xff999999,
					} };
			}

			std::array<math::float2, 4> helper_makeTexcoord()
			{
				return{ {
						math::float2(0.f, 0.f),
						math::float2(1.f, 0.f),
						math::float2(0.f, 1.f),
						math::float2(1.f, 1.f),
				} };
			}

			std::array<math::float3, 4> helper_makeNormalTable()
			{
				return{ {
					math::float3(0, 0, 1), //ePLANE_XY,
					math::float3(1, 0, 0), //ePLANE_ZY,
					math::float3(0, 1, 0), //ePLANE_XZ,
				} };
			}

			template<typename T> std::vector<T> helper_MakeBaseIndices()
			{
				return {
					0, 1, 2,
					1, 3, 2,
				};
			}

			void PlaneCreator::sCreate(std::vector<PVertex>& outVertices, std::vector<uint16_t>& outIndices, PLANE_TYPE type)
			{
				auto posSeed = helper_makePosSeed();
				auto planeIndex = helper_makePlaneIndexList()[type];
				outVertices.resize(posSeed.size());
				for (size_t i = 0; i < outVertices.size(); ++i) {
					outVertices[i].pos(posSeed[i].value[planeIndex.x], posSeed[i].value[planeIndex.y], posSeed[i].value[planeIndex.z]);
				}

				outIndices = helper_MakeBaseIndices < std::remove_reference<decltype(outIndices[0])>::type > ();
			}

			void PlaneCreator::sCreate(std::vector<PCVertex>& outVertices, std::vector<uint16_t>& outIndices, PLANE_TYPE type)
			{
				auto posSeed = helper_makePosSeed();
				auto colors = helper_makeColor();
				auto planeIndex = helper_makePlaneIndexList()[type];
				outVertices.resize(posSeed.size());
				for (size_t i = 0; i < outVertices.size(); ++i) {
					outVertices[i].pos(posSeed[i].value[planeIndex.x], posSeed[i].value[planeIndex.y], posSeed[i].value[planeIndex.z]);
					outVertices[i].color = colors[i];
				}

				outIndices = helper_MakeBaseIndices < std::remove_reference<decltype(outIndices[0])>::type >();
			}

			void PlaneCreator::sCreate(std::vector<PTVertex>& outVertices, std::vector<uint16_t>& outIndices, PLANE_TYPE type)
			{
				auto posSeed = helper_makePosSeed();
				auto texcoords = helper_makeTexcoord();
				auto planeIndex = helper_makePlaneIndexList()[type];
				outVertices.resize(posSeed.size());
				for (size_t i = 0; i < outVertices.size(); ++i) {
					outVertices[i].pos(posSeed[i].value[planeIndex.x], posSeed[i].value[planeIndex.y], posSeed[i].value[planeIndex.z]);
					outVertices[i].texcoord = texcoords[i];
				}

				outIndices = helper_MakeBaseIndices < std::remove_reference<decltype(outIndices[0])>::type >();
			}

			void PlaneCreator::sCreate(std::vector<PNVertex>& outVertices, std::vector<uint16_t>& outIndices, PLANE_TYPE type)
			{
				auto posSeed = helper_makePosSeed();
				auto planeIndex = helper_makePlaneIndexList()[type];
				outVertices.resize(posSeed.size());
				auto normalTable = helper_makeNormalTable();
				for (size_t i = 0; i < outVertices.size(); ++i) {
					outVertices[i].pos(posSeed[i].value[planeIndex.x], posSeed[i].value[planeIndex.y], posSeed[i].value[planeIndex.z]);
					outVertices[i].normal = normalTable[type];
				}

				outIndices = helper_MakeBaseIndices < std::remove_reference<decltype(outIndices[0])>::type >();
			}

			void PlaneCreator::sCreate(std::vector<PTNVertex>& outVertices, std::vector<uint16_t>& outIndices, PLANE_TYPE type)
			{
				auto posSeed = helper_makePosSeed();
				auto planeIndex = helper_makePlaneIndexList()[type];
				outVertices.resize(posSeed.size());
				auto texcoords = helper_makeTexcoord();
				auto normalTable = helper_makeNormalTable();
				for (size_t i = 0; i < outVertices.size(); ++i) {
					outVertices[i].pos(posSeed[i].value[planeIndex.x], posSeed[i].value[planeIndex.y], posSeed[i].value[planeIndex.z]);
					outVertices[i].normal = normalTable[type];
					outVertices[i].texcoord = texcoords[i];
				}

				outIndices = helper_MakeBaseIndices < std::remove_reference<decltype(outIndices[0])>::type >();
			}

			void PlaneCreator::sCreate(std::vector<PTNCVertex>& outVertices, std::vector<uint16_t>& outIndices, PLANE_TYPE type)
			{
				auto posSeed = helper_makePosSeed();
				auto planeIndex = helper_makePlaneIndexList()[type];
				outVertices.resize(posSeed.size());
				auto texcoords = helper_makeTexcoord();
				auto normalTable = helper_makeNormalTable();
				auto colors = helper_makeColor();
				for (size_t i = 0; i < outVertices.size(); ++i) {
					outVertices[i].pos(posSeed[i].value[planeIndex.x], posSeed[i].value[planeIndex.y], posSeed[i].value[planeIndex.z]);
					outVertices[i].normal = normalTable[type];
					outVertices[i].texcoord = texcoords[i];
					outVertices[i].color = colors[i];
				}

				outIndices = helper_MakeBaseIndices < std::remove_reference<decltype(outIndices[0])>::type >();
			}

		}
	}
}
