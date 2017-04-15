#pragma once

#include "../../common/Common.h"
#include "../../resource/DX12Resource.h"
#include "../../graphicsCommandList/DX12GraphicsCommandList.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			class ResourceUploader;

			class Mesh
			{
			public:
				struct InitParam
				{
					void* pVertices;
					UINT64 vertexCount;
					UINT vertixStride;
					void* pIndices;
					UINT64 indexCount;
					DXGI_FORMAT indexFormat;

					InitParam& setVertex(void* pVertices, UINT64 count, UINT stride)noexcept
					{
						this->pVertices = pVertices;
						this->vertexCount = count;
						this->vertixStride = stride;
						return *this;
					}

					InitParam& setIndex(void* pIndices, UINT64 count, DXGI_FORMAT format)noexcept
					{
						this->pIndices = pIndices;
						this->indexCount = count;
						this->indexFormat = format;
						return *this;
					}
				};

			public:
				Mesh();
				~Mesh();

				Mesh(Mesh&& right);
				Mesh& operator=(Mesh&& right);

				/// @brief 初期状態にする
				void clear();

				/// @brief 作成
				///
				/// @param[in] pDevice
				/// @param[in] uploader
				/// @param[in] initParam
				void create(ID3D12Device* pDevice, ResourceUploader& uploader, const InitParam& initParam);

				/// @brief コマンドリストに登録する
				///
				/// @param[in] commandList
				/// @param[in] primitiveTopology
				void bind(DX12GraphicsCommandList& commadList, D3D12_PRIMITIVE_TOPOLOGY primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)noexcept;

				/// @brief 描画コマンド発行
				///
				/// @param[in] commadList
				/// @param[in] instanceCount default=1
				void draw(DX12GraphicsCommandList& commadList, UINT instanceCount = 1)noexcept;

			private:
				DX12Resource mVertexBuffer;
				DX12Resource mIndexBuffer;
				UINT mIndexCount;
			};
		}
	}
}