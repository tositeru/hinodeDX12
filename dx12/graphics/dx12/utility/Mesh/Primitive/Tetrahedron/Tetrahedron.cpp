#include "stdafx.h"

#include "Tetrahedron.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			std::array<math::float3, 4> helper_MakePos()
			{
				std::array<math::float3, 4> ret;
				//底面のXZ成分を求めてから、てっぺんの頂点の高さを計算して、全頂点の平均した位置を中心にしている
				//底面のXZ成分を求める
				float l = sqrt(3.f) * 0.5f;
				ret[1](0, 0, 0.5f * l); //底面の頂点1
				ret[2](0.5f, 0, -0.5f * l); //底面の頂点2
				ret[3](-0.5f, 0, -0.5f * l); //底面の頂点3
														 //てっぺんの頂点の高さを計算
				auto centroid = (ret[1] + ret[2] + ret[3]) / 3.f;
				float m = math::length(centroid - ret[1]);
				float n = sqrt(1 - m*m);
				ret[0](centroid.x, 0.5f * n, centroid.z);	//てっぺん
				ret[1].y = -0.5f * n;//底面の高さも合わせて設定する
				ret[2].y = -0.5f * n;
				ret[3].y = -0.5f * n;
				//全頂点の平均した位置を中心になるよう移動
				centroid = (ret[0] + ret[1] + ret[2] + ret[3]) / 4.f;
				for (auto& p : ret) {
					p -= centroid;
				}
				return ret;
			}

			std::array<uint32_t, 4> helper_MakeColor()
			{
				return { {
					0xffff6666,
					0xff66ff66,
					0xff6666ff,
					0xff999999,
				} };
			}
			std::array<math::float2, 3 * 4> helper_MakeTexcoord()
			{
				return { {
					{ 0.5f, 0.f },{ 1.f, 1.f },{ 0.f, 1.f },
					{ 0.5f, 0.f },{ 1.f, 1.f },{ 0.f, 1.f },
					{ 0.5f, 0.f },{ 1.f, 1.f },{ 0.f, 1.f },
					{ 0.5f, 0.f },{ 1.f, 1.f },{ 0.f, 1.f },
					} };
			}

			template<typename T> T helper_MakeBaseIndices()
			{
				return {
					0, 1, 2,
					0, 2, 3,
					0, 3, 1,
					1, 3, 2,
				};
			}

			void TetrahedronCreator::sCreate(std::vector<PVertex>& outVertices, std::vector<uint16_t>& outIndices)
			{
				auto pos = helper_MakePos();
				outVertices.reserve(4);
				for (auto& p : pos) {
					outVertices.emplace_back(p);
				}

				outIndices = helper_MakeBaseIndices<std::remove_reference<decltype(outIndices)>::type>();
			}

			void TetrahedronCreator::sCreate(std::vector<PCVertex>& outVertices, std::vector<uint16_t>& outIndices)
			{
				auto pos = helper_MakePos();
				auto color = helper_MakeColor();

				outVertices.reserve(4);
				for (size_t i = 0u; i < pos.size(); ++i) {
					outVertices.emplace_back(pos[i], color[i]);
				}

				outIndices = helper_MakeBaseIndices<std::remove_reference<decltype(outIndices)>::type>();
			}

			void TetrahedronCreator::sCreate(std::vector<PTVertex>& outVertices, std::vector<uint16_t>& outIndices)
			{
				auto pos = helper_MakePos();
				auto texcoords = helper_MakeTexcoord();
				auto baseIndices = helper_MakeBaseIndices<std::remove_reference<decltype(outIndices)>::type>();
				outVertices.reserve(texcoords.size());
				outIndices.reserve(texcoords.size());
				for (size_t i = 0u; i < outVertices.capacity(); i+=3) {
					outVertices.emplace_back(pos[baseIndices[i + 0]], texcoords[i + 0]);
					outVertices.emplace_back(pos[baseIndices[i + 1]], texcoords[i + 1]);
					outVertices.emplace_back(pos[baseIndices[i + 2]], texcoords[i + 2]);

					using IndexType = std::remove_reference<decltype(outIndices[0])>::type;
					outIndices.emplace_back(static_cast<IndexType>(i + 0));
					outIndices.emplace_back(static_cast<IndexType>(i + 1));
					outIndices.emplace_back(static_cast<IndexType>(i + 2));
				}
			}

			void TetrahedronCreator::sCreate(std::vector<PNVertex>& outVertices, std::vector<uint16_t>& outIndices)
			{
				auto pos = helper_MakePos();
				auto baseIndices = helper_MakeBaseIndices<std::remove_reference<decltype(outIndices)>::type>();
				outVertices.reserve(3*4);
				outIndices.reserve(outVertices.capacity());
				for (size_t i = 0u; i < outVertices.capacity(); i += 3) {
					auto normal = math::cross(pos[baseIndices[i + 1]] - pos[baseIndices[i + 0]], pos[baseIndices[i + 2]] - pos[baseIndices[i + 0]]);
					normal = normalize(normal);
					outVertices.emplace_back(pos[baseIndices[i + 0]], normal);
					outVertices.emplace_back(pos[baseIndices[i + 1]], normal);
					outVertices.emplace_back(pos[baseIndices[i + 2]], normal);

					using IndexType = std::remove_reference<decltype(outIndices[0])>::type;
					outIndices.emplace_back(static_cast<IndexType>(i + 0));
					outIndices.emplace_back(static_cast<IndexType>(i + 1));
					outIndices.emplace_back(static_cast<IndexType>(i + 2));
				}
			}

			void TetrahedronCreator::sCreate(std::vector<PTNVertex>& outVertices, std::vector<uint16_t>& outIndices)
			{
				auto pos = helper_MakePos();
				auto texcoords = helper_MakeTexcoord();
				auto baseIndices = helper_MakeBaseIndices<std::remove_reference<decltype(outIndices)>::type>();
				outVertices.reserve(3 * 4);
				outIndices.reserve(outVertices.capacity());
				for (size_t i = 0u; i < outVertices.capacity(); i += 3) {
					auto normal = math::cross(pos[baseIndices[i + 1]] - pos[baseIndices[i + 0]], pos[baseIndices[i + 2]] - pos[baseIndices[i + 0]]);
					normal = normalize(normal);
					outVertices.emplace_back(pos[baseIndices[i + 0]], texcoords[i + 0], normal);
					outVertices.emplace_back(pos[baseIndices[i + 1]], texcoords[i + 1], normal);
					outVertices.emplace_back(pos[baseIndices[i + 2]], texcoords[i + 2], normal);

					using IndexType = std::remove_reference<decltype(outIndices[0])>::type;
					outIndices.emplace_back(static_cast<IndexType>(i + 0));
					outIndices.emplace_back(static_cast<IndexType>(i + 1));
					outIndices.emplace_back(static_cast<IndexType>(i + 2));
				}

			}

			void TetrahedronCreator::sCreate(std::vector<PTNCVertex>& outVertices, std::vector<uint16_t>& outIndices)
			{
				auto pos = helper_MakePos();
				auto texcoords = helper_MakeTexcoord();
				auto color = helper_MakeColor();
				auto baseIndices = helper_MakeBaseIndices<std::remove_reference<decltype(outIndices)>::type>();
				outVertices.reserve(3 * 4);
				outIndices.reserve(outVertices.capacity());
				for (size_t i = 0u; i < outVertices.capacity(); i += 3) {
					auto normal = math::cross(pos[baseIndices[i + 1]] - pos[baseIndices[i + 0]], pos[baseIndices[i + 2]] - pos[baseIndices[i + 0]]);
					normal = normalize(normal);
					outVertices.emplace_back(pos[baseIndices[i + 0]], texcoords[i + 0], normal, color[baseIndices[i + 0]]);
					outVertices.emplace_back(pos[baseIndices[i + 1]], texcoords[i + 1], normal, color[baseIndices[i + 1]]);
					outVertices.emplace_back(pos[baseIndices[i + 2]], texcoords[i + 2], normal, color[baseIndices[i + 2]]);

					using IndexType = std::remove_reference<decltype(outIndices[0])>::type;
					outIndices.emplace_back(static_cast<IndexType>(i + 0));
					outIndices.emplace_back(static_cast<IndexType>(i + 1));
					outIndices.emplace_back(static_cast<IndexType>(i + 2));
				}
			}


		}
	}
}