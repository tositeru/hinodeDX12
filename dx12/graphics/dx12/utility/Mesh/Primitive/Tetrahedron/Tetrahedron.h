#pragma once

#include <vector>
#include <array>

#include "../../MeshVertexFormat.h"
#include "../../../ResourceUploader/ResourceUploader.h"
#include "../../Mesh.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			class TetrahedronCreator
			{
			public:
				static void sCreate(std::vector<PVertex>& outVertices, std::vector<uint16_t>& outIndices);
				static void sCreate(std::vector<PCVertex>& outVertices, std::vector<uint16_t>& outIndices);
				static void sCreate(std::vector<PTVertex>& outVertices, std::vector<uint16_t>& outIndices);
				static void sCreate(std::vector<PNVertex>& outVertices, std::vector<uint16_t>& outIndices);
				static void sCreate(std::vector<PTNVertex>& outVertices, std::vector<uint16_t>& outIndices);
				static void sCreate(std::vector<PTNCVertex>& outVertices, std::vector<uint16_t>& outIndices);

				template<typename VERTEX>
				static void sCreateMesh(Mesh* pOut, ID3D12Device* pDevice, ResourceUploader& uploader)
				{
					assert(nullptr != pOut);

					std::vector<VERTEX> vertices;
					std::vector<uint16_t> indices;
					sCreate(vertices, indices);
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