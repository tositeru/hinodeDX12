#pragma once

#include <vector>

#include "../../MeshVertexFormat.h"
#include "../../../ResourceUploader/ResourceUploader.h"
#include "../../Mesh.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			class PlaneCreator
			{
			public:
				enum PLANE_TYPE {
					ePLANE_XY,
					ePLANE_ZY,
					ePLANE_XZ,
				};
			public:
				static void sCreate(std::vector<PVertex>& outVertices, std::vector<uint16_t>& outIndices, PLANE_TYPE type);
				static void sCreate(std::vector<PCVertex>& outVertices, std::vector<uint16_t>& outIndices, PLANE_TYPE type);
				static void sCreate(std::vector<PTVertex>& outVertices, std::vector<uint16_t>& outIndices, PLANE_TYPE type);
				static void sCreate(std::vector<PNVertex>& outVertices, std::vector<uint16_t>& outIndices, PLANE_TYPE type);
				static void sCreate(std::vector<PTNVertex>& outVertices, std::vector<uint16_t>& outIndices, PLANE_TYPE type);
				static void sCreate(std::vector<PTNCVertex>& outVertices, std::vector<uint16_t>& outIndices, PLANE_TYPE type);

				template<typename VERTEX>
				static void sCreateMesh(Mesh* pOut, ID3D12Device* pDevice, ResourceUploader& uploader, PLANE_TYPE type)
				{
					assert(nullptr != pOut);

					std::vector<VERTEX> vertices;
					std::vector<uint16_t> indices;
					sCreate(vertices, indices, type);
					Mesh::InitParam initParam;
					initParam
						.setVertex(vertices.data(), vertices.size(), sizeof(vertices[0]))
						.setIndex(indices.data(), indices.size(), DXGI_FORMAT_R16_UINT);
					pOut->create(pDevice, uploader, initParam);
				}
			};
		}
	}
}