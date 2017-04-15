#pragma once

#include <array>

#include "../../resource/DX12Resource.h"
#include "../../descriptorHeap/DX12DescriptorHeap.h"

namespace hinode
{
	namespace graphics
	{
		class DX12GraphicsCommandList;

		namespace utility
		{
			class GBufferManager
			{
			public:
				enum GBUFFER_TYPE {
					eGBUFFER_ALBEDO,
					eGBUFFER_NORMAL,
					eGBUFFER_SPECULER,
					eGBUFFER_DEPTH_FOR_DSV,
					eGBUFFER_DEPTH_FOR_SRV_UAV,
					eGBUFFER_TYPE_COUNT,
					eGBUFFER_RTV_COUNT = eGBUFFER_SPECULER + 1,
					eGBUFFER_DSV_COUNT = 1,
				};

				enum VIEW_TYPE {
					eVIEW_SRV,
					eVIEW_UAV,
					eVIEW_RTV,
					eVIEW_DSV = 0, // for eGBUFFER_DEPTH_FOR_DSV
				};

			public:
				static std::array<DXGI_FORMAT, eGBUFFER_TYPE_COUNT> sDefaultFormats();

			public:
				GBufferManager();
				~GBufferManager();

				/// @brief メモリ解放
				void clear();

				/// @brief 作成
				/// @param[in] width
				/// @param[in] height
				void create(ID3D12Device* pDevice, UINT width, UINT height);

				/// @brief 各GBufferのフォーマットを指定した作成
				///
				/// こちらが想定しているデフォルトのフォーマットはGBufferManager::sDefaultFormats関数で取得できます。
				/// @param[in] width
				/// @param[in] height
				/// @parma[in] formats 順番は GBUFFER_TYPEを参考にして下さい
				void create(ID3D12Device* pDevice, UINT width, UINT height, const std::array<DXGI_FORMAT, eGBUFFER_TYPE_COUNT>& formats);

				/// @brief GBufferへの書き込みを開始する
				///
				/// この関数を呼出した後は必ず、GBufferManager::end関数を呼び出してください
				/// @param[in] cmdList
				void begin(DX12GraphicsCommandList& cmdList);

				/// @brief GBufferへの書き込みを終了する
				///
				/// @param[in] cmdList
				void end(DX12GraphicsCommandList& cmdList);

			accessor_declaration:
				DX12Resource& get(GBUFFER_TYPE type)noexcept;
				const DX12Resource::View& getView(GBUFFER_TYPE type, VIEW_TYPE viewType)noexcept;

			private:
				std::array<DX12Resource, eGBUFFER_TYPE_COUNT> mGBuffer;
				DX12DescriptorHeap mGBufferRTVHeap;
				DX12DescriptorHeap mGBufferDSVHeap;
			};
		}
	}
}