#pragma once

#include "../../../pipelineState/DX12PipelineState.h"
#include "../../Shader/PipelineSet/PipelineSet.h"

#include "../../math/SimpleMath.h"
#include "../Shader/DefferedLighting/CSDefferedDirectionLighting.hlsl"
#include "../Shader/DefferedLighting/CSDefferedDirectionLightingWithShadow.hlsl"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			/// @brief �f�B�t�@�[�h���C�e�B���O�p�̃p�C�v���C�����쐬���邽�߂̃N���X
			class DefferedLightingPipeline
			{
			public:
				/// @brief ���s�����C�e�B���O���s���p�C�v���C�����쐬����
				///
				/// pDesc�ɐݒ肳�ꂽ�V�F�[�_�A���[�g�V�O�l�`���͖�������܂�
				/// @param[in] pDevice
				/// @param[in] pDesc default=nullptr
				/// @param[in] nodeMask default=0u
				static PipelineSet sCreateDirectionLighting(ID3D12Device* pDevice, const DX12_COMPUTE_PIPELINE_STATE_DESC* pDesc = nullptr, UINT nodeMask = 0u);

				/// @brief �V���h�E�}�b�v�t�����s�����C�e�B���O���s���p�C�v���C�����쐬����
				///
				/// pDesc�ɐݒ肳�ꂽ�V�F�[�_�A���[�g�V�O�l�`���͖�������܂�
				/// @param[in] pDevice
				/// @param[in] pDesc default=nullptr
				/// @param[in] nodeMask default=0u
				static PipelineSet sCreateDirectionLightingWithShadow(ID3D12Device* pDevice, const DX12_COMPUTE_PIPELINE_STATE_DESC* pDesc = nullptr, UINT nodeMask = 0u);

			};
		}
	}
}