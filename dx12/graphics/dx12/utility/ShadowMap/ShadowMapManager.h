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
			class ShadowMapManager
			{
			public:
				enum TYPE
				{
					eTYPE_DSV,
					eTYPE_SRV,
					eTYPE_COUNT,
				};

				enum VIEW_TYPE
				{
					eVIEW_DSV,
					eVIEW_SRV = 0,
					eVIEW_UAV = 1,
					eVIEW_TYPE_COUNT,
				};

			public:
				ShadowMapManager();
				~ShadowMapManager();

				/// @brief ���������
				void clear();

				/// @brief �쐬
				/// @param[in] pDevice
				/// @parma[in] width
				/// @param[in] height
				/// @param[in] formatDSV
				/// @param[in] formatSRV
				/// @param[in] nodeMask default=0
				void create(ID3D12Device* pDevice, UINT width, UINT height, DXGI_FORMAT formatDSV, DXGI_FORMAT formatSRV, UINT nodeMask=0);

				/// @brief �V���h�E�}�b�v�ւ̏������݂��J�n����
				///
				/// ���̊֐����ďo������͕K���AShadowMapManager::end�֐����Ăяo���Ă�������
				/// @param[in] cmdList
				void begin(DX12GraphicsCommandList& cmdList);

				/// @brief �V���h�E�}�b�v�ւ̏������݂��I������
				///
				/// @param[in] cmdList
				void end(DX12GraphicsCommandList& cmdList);

			accessor_declaration:
				DX12Resource& shadowMap()noexcept;
				DX12Resource& getShadowMap(TYPE type)noexcept;

			private:
				std::array<DX12Resource, eTYPE_COUNT> mShadowMaps;
				DX12DescriptorHeap mDSVHeap;
			};
		}
	}
}