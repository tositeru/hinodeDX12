#include "stdafx.h"

#include "Mesh.h"

#include "../ResourceUploader/ResourceUploader.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			Mesh::Mesh()
			{}

			Mesh::~Mesh()
			{
				this->clear();
			}

			Mesh::Mesh(Mesh&& right)
				: mVertexBuffer(std::move(right.mVertexBuffer))
				, mIndexBuffer(std::move(right.mIndexBuffer))
				, mIndexCount(right.mIndexCount)
			{ }

			Mesh& Mesh::operator=(Mesh&& right)
			{
				this->mVertexBuffer = std::move(right.mVertexBuffer);
				this->mIndexBuffer = std::move(right.mIndexBuffer);
				this->mIndexCount = right.mIndexCount;
				return *this;
			}

			void Mesh::clear()
			{
				this->mVertexBuffer.clear();
				this->mIndexBuffer.clear();
			}

			void Mesh::create(ID3D12Device* pDevice, ResourceUploader& uploader, const InitParam& initParam)
			{
				DX12ResourceHeapDesc heapDesc(D3D12_HEAP_TYPE_DEFAULT);
				DX12ResourceDesc resourceDesc;
				resourceDesc.desc = CD3DX12_RESOURCE_DESC::Buffer(initParam.vertexCount * initParam.vertixStride);
				resourceDesc.initialStates = D3D12_RESOURCE_STATE_COPY_DEST;
				this->mVertexBuffer.create(pDevice, heapDesc, resourceDesc);
				this->mVertexBuffer.appendView(DX12Resource::View().setVertex(initParam.vertixStride, static_cast<const UINT>(resourceDesc.desc.Width)));

				UINT64 indexStride = 0U;
				switch (initParam.indexFormat) {
				case DXGI_FORMAT_R16_UINT: indexStride = 2; break;
				case DXGI_FORMAT_R32_UINT: indexStride = 4; break;
				default:
					assert(false && "対応していないインデックスのフォーマット");
				}

				resourceDesc.desc = CD3DX12_RESOURCE_DESC::Buffer(initParam.indexCount * indexStride);
				this->mIndexBuffer.create(pDevice, heapDesc, resourceDesc);
				this->mIndexBuffer.appendView(DX12Resource::View().setIndex(initParam.indexFormat, static_cast<const UINT>(resourceDesc.desc.Width)));
				this->mIndexCount = static_cast<decltype(this->mIndexCount)>(initParam.indexCount);

				uploader.entryBuffer(this->mVertexBuffer, initParam.pVertices, initParam.vertexCount * initParam.vertixStride);
				uploader.entryBuffer(this->mIndexBuffer, initParam.pIndices, initParam.indexCount * indexStride);
			}

			void Mesh::bind(DX12GraphicsCommandList& commandList, D3D12_PRIMITIVE_TOPOLOGY primitiveTopology)noexcept
			{
				commandList.setBarriers(this->mVertexBuffer.makeBarrier(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
				commandList.setBarriers(this->mIndexBuffer.makeBarrier(D3D12_RESOURCE_STATE_INDEX_BUFFER));
				commandList->IASetVertexBuffers(0, 1, &this->mVertexBuffer.view().vertex());
				commandList->IASetIndexBuffer(&this->mIndexBuffer.view().index());
				commandList->IASetPrimitiveTopology(primitiveTopology);
			}

			void Mesh::draw(DX12GraphicsCommandList& commadList, UINT instanceCount)noexcept
			{
				commadList->DrawIndexedInstanced(this->mIndexCount, instanceCount, 0, 0, 0);
			}
		}
	}
}