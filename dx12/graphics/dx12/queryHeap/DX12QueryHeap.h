#pragma once

#include "../common/Common.h"
#include "../IDX12Interface.h"

namespace hinode
{
	namespace graphics
	{
		class DX12QueryHeap : IDX12Interface
		{
			UNCOPYABLE_PIMPL_IDIOM_TEMPLATE(DX12QueryHeap)
		public:
			void clear()noexcept;

			/// @brief �N�G���q�[�v�̍쐬
			/// @param[in] pDevice
			/// @param[in] pDesc
			/// @exception hinode::graphics::InvalidArgumentsException
			void create(ID3D12Device* pDevice, D3D12_QUERY_HEAP_DESC* pDesc);

			/// @brief ������DX12CommandQueue�ɖ��O������
			/// @param[in] const wchar_t* name
			/// @retval HRESULT
			HRESULT setName(const wchar_t* name) noexcept override;

			/// @brief �g�p�ł����Ԃ�?
			/// @retval bool
			bool isGood()const noexcept override;

		accessor_declaration:
			ID3D12QueryHeap* operator->()noexcept;

			ID3D12QueryHeap* queryHeap()noexcept;
			operator ID3D12QueryHeap*()noexcept { return this->queryHeap(); }

		};
	}
}