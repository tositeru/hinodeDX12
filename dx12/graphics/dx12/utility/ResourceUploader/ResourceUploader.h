#pragma once

#include <list>
#include "..\..\resource\DX12Resource.h"
#include "..\..\commandQueue\DX12CommandQueue.h"
#include "..\..\commandAllocator\DX12CommandAllocator.h"
#include "..\..\graphicsCommandList\DX12GraphicsCommandList.h"
#include "..\..\fence\DX12Fence.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			class ResourceUploader
			{
				ResourceUploader(ResourceUploader&) = delete;
				void operator=(ResourceUploader&) = delete;

			public:
				struct TextureDesc
				{
					UINT subresource = 0u;
					const void* pData = nullptr;	///< 転送するデータの先頭アドレス。 entryXXXを呼び出すときには指している先のデータが存在するようにしてください。
					UINT64 rowPitch = 0;	///< pDataが指しているデータの1行あたりのデータ長
				};

			public:
				ResourceUploader();
				~ResourceUploader();

				/// @brief メモリ解放
				void clear();

				/// @brief 初期化
				/// @parma[in] pDevice
				void init(ID3D12Device* pDevice, D3D12_COMMAND_QUEUE_PRIORITY prority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, UINT nodeMask = 0x0);

				/// @brief データをGPUにアップロードするバッファを登録します
				/// @param[in] target
				/// @param[in] pData
				/// @param[in] byteSize
				/// @exception hinode::graphics::RunTimeErrorException
				void entryBuffer(DX12Resource& target, void* pData, UINT64 byteSize);

				/// @brief データをGPUにアップロードするテクスチャを登録します
				/// @param[in] target
				/// @param[in] desc
				void entryTexture(DX12Resource& target, const TextureDesc& desc);

				/// @brief データをGPUにアップロードするテクスチャを登録します
				///
				/// 複数のサブリソースを一度にアップロードします
				/// @param[in] target
				/// @param[in] desces
				void entryTexture(DX12Resource& target, const std::vector<TextureDesc>& desces);

				/// @brief WICファイルからテクスチャを読み込みます
				/// @param[in] pDevice
				/// @param[in] target
				/// @param[in] filepath
				void loadTextureFromWICFile(ID3D12Device* pDevice, DX12Resource& target, const char* filepath);

				/// @brief WICファイルからテクスチャを読み込みます
				/// @param[in] pDevice
				/// @param[in] target
				/// @param[in] filepath
				void loadTextureFromWICFile(ID3D12Device* pDevice, DX12Resource& target, const std::string& filepath);

				/// @brief DDSファイルからテクスチャを読み込みます
				/// @param[in] pDevice
				/// @param[in] target
				/// @param[in] filepath
				void loadTextureFromDDS(ID3D12Device* pDevice, DX12Resource& target, const char* filepath);

				/// @brief DDSファイルからテクスチャを読み込みます
				/// @param[in] pDevice
				/// @param[in] target
				/// @param[in] filepath
				void loadTextureFromDDS(ID3D12Device* pDevice, DX12Resource& target, const std::string& filepath);

				/// @brief 登録されたデータをGPU上に転送します
				void upload()noexcept;

			private:
				/// @brief
				/// @exception hinode::graphics::RunTimeErrorException
				void startRecording();

			private:
				bool mIsRecording;
				DX12CommandQueue mCmdQueue;
				DX12CommandAllocator mCmdAllocator;
				DX12Fence mFence;

				DX12GraphicsCommandList mCmdList;
				std::list<DX12Resource> mCopySrc;
			};
		}
	}
}