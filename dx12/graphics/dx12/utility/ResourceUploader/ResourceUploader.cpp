#include "stdafx.h"

#include "ResourceUploader.h"

#include "..\..\common\Exception.h"
#include "..\..\DirectXTex\WICTextureLoader\WICTextureLoader12.h"
#include "..\..\DirectXTex\DDSTextureLoader\DDSTextureLoader12.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			ResourceUploader::ResourceUploader()
				: mIsRecording(false)
			{}

			ResourceUploader::~ResourceUploader()
			{
				if(this->mIsRecording){
					this->upload();
				}

				this->clear();
			}

			void ResourceUploader::clear()
			{
				this->mIsRecording = false;
				this->mCopySrc.clear();
				this->mCmdList.clear();
				this->mCmdAllocator.clear();
				this->mCmdQueue.clear();
			}

			void ResourceUploader::init(ID3D12Device* pDevice, D3D12_COMMAND_QUEUE_PRIORITY prority, UINT nodeMask)
			{
				this->clear();

				D3D12_COMMAND_QUEUE_DESC queueDesc = {};
				queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
				queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
				queueDesc.NodeMask = nodeMask;
				queueDesc.Priority = prority;
				this->mCmdQueue.create(pDevice, queueDesc);

				this->mCmdAllocator.create(pDevice, queueDesc.Type);

				DX12GraphicsCommandListDesc cmdListDesc = {};
				cmdListDesc.pAllocator = this->mCmdAllocator;
				cmdListDesc.pPipelineState = nullptr;
				cmdListDesc.node = nodeMask;
				cmdListDesc.type = queueDesc.Type;
				this->mCmdList.create(pDevice, cmdListDesc);

				this->mFence.create(pDevice, 1, D3D12_FENCE_FLAG_NONE);

				this->mIsRecording = false;
			}

			void ResourceUploader::entryBuffer(DX12Resource& target, void* pData, UINT64 byteSize)
			{
				DX12Resource uploadResource;
				uploadResource.createForCopy(target.getDevice(), target, pData, byteSize);

				unless(this->mIsRecording) {
					this->startRecording();
				}

				this->mCmdList.setBarriers(target.makeBarrier(D3D12_RESOURCE_STATE_COPY_DEST));
				this->mCmdList->CopyResource(target, uploadResource);

				this->mCopySrc.emplace_back(std::move(uploadResource));
			}

			void ResourceUploader::entryTexture(DX12Resource& target, const TextureDesc& desc)
			{
				assert(target.isGood() && "作成されていません。");
				auto& targetDesc = target->GetDesc();

				UINT64 requiredSize;
				auto placed_footprint = target.getCopyableFootprint(0, 0, &requiredSize);
				const auto& footprint = placed_footprint.Footprint;
				DX12Resource uploadResource;
				uploadResource.createForCopy(target.getDevice(), target, nullptr, requiredSize);

				auto pSrcData = pointerCast<uint8_t>(desc.pData);
				uploadResource.update(desc.subresource, nullptr, [&](const D3D12_RANGE* pRange, void* pData, D3D12_RANGE* pWriteRange) {
					//メモ: 読み込んだデータと実際の中間バッファのアライメントは異なるケースがあるのを忘れない
					auto pDest = pointerCast<uint8_t>(pData);
					uint8_t* start;
					const uint8_t* destStart;
					for (auto row = 0u; row < footprint.Height * footprint.Depth; ++row) {
						start = &pDest[row*footprint.RowPitch];
						destStart = &pSrcData[row*desc.rowPitch];
						memcpy(start, destStart, desc.rowPitch);
					}
					return true;
				});

				unless(this->mIsRecording) {
					this->startRecording();
				}

				this->mCmdList.setBarriers(target.makeBarrier(D3D12_RESOURCE_STATE_COPY_DEST));
				CD3DX12_TEXTURE_COPY_LOCATION dest(target, 0);
				CD3DX12_TEXTURE_COPY_LOCATION src(uploadResource.resource(), placed_footprint);
				this->mCmdList->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);

				this->mCopySrc.emplace_back(std::move(uploadResource));
			}

			void ResourceUploader::entryTexture(DX12Resource& target, const std::vector<TextureDesc>& desces)
			{
				assert(target.isGood() && "作成されていません。");
				auto& targetDesc = target->GetDesc();

				UINT64 requiredSize = 0;
				std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> placed_footprints;
				placed_footprints.reserve(desces.size());
				for (auto i = 0u; i < desces.size(); ++i) {
					auto& d = desces[i];
					UINT64 totalSize;
					auto placed_footprint = target.getCopyableFootprint(d.subresource, 0, &totalSize);
					placed_footprint.Offset = requiredSize;
					//Log(Log::eINFO) << i << ":" << totalSize << ", " << totalSize % D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT << "; requireSize=" << placed_footprint.Offset;
					requiredSize += totalSize + (D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT - totalSize % D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
					placed_footprints.emplace_back(placed_footprint);
				}

				DX12Resource uploadResource;
				uploadResource.createForCopy(target.getDevice(), target, nullptr, requiredSize);

				unless(this->mIsRecording) {
					this->startRecording();
				}
				this->mCmdList.setBarriers(target.makeBarrier(D3D12_RESOURCE_STATE_COPY_DEST));

				for (size_t i = 0u; i < desces.size(); ++i) {
					const auto& pf = placed_footprints.at(i);
					const auto& texDesc = desces.at(i);
					uploadResource.update(0, nullptr, [&](const D3D12_RANGE* pRange, void* pData, D3D12_RANGE* pWriteRange) {
						//メモ: 読み込んだデータと中間バッファのアライメントは異なるケースがあるのを忘れない
						auto pSrcData = pointerCast<uint8_t>(texDesc.pData);
						auto pDest = &pointerCast<uint8_t>(pData)[pf.Offset];
						for (auto row = 0u; row < pf.Footprint.Height * pf.Footprint.Depth; ++row) {

							auto start = &pDest[row*pf.Footprint.RowPitch];
							auto destStart = &pSrcData[row*texDesc.rowPitch];
							memcpy(start, destStart, texDesc.rowPitch);
						}
						return true;
					});

					CD3DX12_TEXTURE_COPY_LOCATION src(uploadResource.resource(), pf);

					CD3DX12_TEXTURE_COPY_LOCATION dest(target, texDesc.subresource);
					this->mCmdList->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);
				}

				this->mCopySrc.emplace_back(std::move(uploadResource));
			}

			void ResourceUploader::loadTextureFromWICFile(ID3D12Device* pDevice, DX12Resource& target, const char* filepath)
			{
				std::wstring wFilepath = toWideString(filepath);

				std::unique_ptr<uint8_t[]> decodedData;
				D3D12_SUBRESOURCE_DATA subresource;
				ID3D12Resource* pTex;
				auto hr = DirectX::LoadWICTextureFromFile(pDevice, wFilepath.c_str(), &pTex, decodedData, subresource);
				if (FAILED(hr)) {
					throw HINODE_GRAPHICS_CREATE_EXCEPTION(FailedToCreateException, "ResourceUploader", "loadTextureFromWICFile")
						<< "読み込み失敗 filepath=" << filepath;
				}
				target.create(pTex, D3D12_RESOURCE_STATE_COPY_DEST);
				auto& targetDesc = target->GetDesc();

				TextureDesc texDesc;
				texDesc.pData = decodedData.get();
				texDesc.subresource = 0;
				texDesc.rowPitch = subresource.RowPitch;
				this->entryTexture(target, texDesc);
			}

			void ResourceUploader::loadTextureFromWICFile(ID3D12Device* pDevice, DX12Resource& target, const std::string& filepath)
			{
				this->loadTextureFromWICFile(pDevice, target, filepath.c_str());
			}

			void ResourceUploader::loadTextureFromDDS(ID3D12Device* pDevice, DX12Resource& target, const char* filepath)
			{
				std::wstring wFilepath = toWideString(filepath);

				std::unique_ptr<uint8_t[]> decodedData;
				std::vector<D3D12_SUBRESOURCE_DATA> subresources;
				ID3D12Resource* pTex;
				auto hr = DirectX::LoadDDSTextureFromFile(pDevice, wFilepath.c_str(), &pTex, decodedData, subresources);
				if (FAILED(hr)) {
					throw HINODE_GRAPHICS_CREATE_EXCEPTION(FailedToCreateException, "ResourceUploader", "loadTextureFromDDS")
						<< "読み込み失敗";
				}
				target.create(pTex, D3D12_RESOURCE_STATE_COPY_DEST);

				auto desc = target->GetDesc();
				UINT64 requiredSize = 0u;
				auto placed_footprints = target.getCopyableFootprints(0, desc.MipLevels * desc.DepthOrArraySize, 0, &requiredSize);

				std::vector<TextureDesc> desces;
				desces.resize(subresources.size());
				for (auto i = 0u; i < desces.size(); ++i) {
					desces[i].pData = subresources[i].pData;
					desces[i].rowPitch = subresources[i].RowPitch;
					desces[i].subresource = i;
				}
				this->entryTexture(target, desces);
			}

			void ResourceUploader::loadTextureFromDDS(ID3D12Device* pDevice, DX12Resource& target, const std::string& filepath)
			{
				this->loadTextureFromDDS(pDevice, target, filepath.c_str());
			}

			void ResourceUploader::upload()noexcept
			{
				unless(this->mIsRecording) return;

				this->mCmdList.close();
				this->mCmdQueue.executeCommandLists({this->mCmdList});
				DX12Fence::sSignalAndWait(this->mFence, this->mCmdQueue);
				this->mCopySrc.clear();

				this->mIsRecording = false;
			}

			void ResourceUploader::startRecording()
			{
				if (this->mIsRecording)
					return;

				if (FAILED(this->mCmdAllocator->Reset())) {
					throw HINODE_GRAPHICS_CREATE_EXCEPTION(RunTimeErrorException, "ResourceUploader", "startRecording")
						<< "コマンドアロケータのリセットに失敗";
				}

				unless(this->mCmdList.reset(this->mCmdAllocator, nullptr)) {
					throw HINODE_GRAPHICS_CREATE_EXCEPTION(RunTimeErrorException, "ResourceUploader", "startRecording")
						<< "コマンドリストのリセットに失敗";
				}

				this->mIsRecording = true;
			}
		}
	}
}