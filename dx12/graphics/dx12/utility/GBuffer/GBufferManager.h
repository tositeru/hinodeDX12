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

				/// @brief ���������
				void clear();

				/// @brief �쐬
				/// @param[in] width
				/// @param[in] height
				void create(ID3D12Device* pDevice, UINT width, UINT height);

				/// @brief �eGBuffer�̃t�H�[�}�b�g���w�肵���쐬
				///
				/// �����炪�z�肵�Ă���f�t�H���g�̃t�H�[�}�b�g��GBufferManager::sDefaultFormats�֐��Ŏ擾�ł��܂��B
				/// @param[in] width
				/// @param[in] height
				/// @parma[in] formats ���Ԃ� GBUFFER_TYPE���Q�l�ɂ��ĉ�����
				void create(ID3D12Device* pDevice, UINT width, UINT height, const std::array<DXGI_FORMAT, eGBUFFER_TYPE_COUNT>& formats);

				/// @brief GBuffer�ւ̏������݂��J�n����
				///
				/// ���̊֐����ďo������͕K���AGBufferManager::end�֐����Ăяo���Ă�������
				/// @param[in] cmdList
				void begin(DX12GraphicsCommandList& cmdList);

				/// @brief GBuffer�ւ̏������݂��I������
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