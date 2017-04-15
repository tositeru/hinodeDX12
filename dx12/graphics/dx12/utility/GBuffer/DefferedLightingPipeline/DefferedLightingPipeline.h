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
			/// @brief ディファードライティング用のパイプラインを作成するためのクラス
			class DefferedLightingPipeline
			{
			public:
				/// @brief 平行光ライティングを行うパイプラインを作成する
				///
				/// pDescに設定されたシェーダ、ルートシグネチャは無視されます
				/// @param[in] pDevice
				/// @param[in] pDesc default=nullptr
				/// @param[in] nodeMask default=0u
				static PipelineSet sCreateDirectionLighting(ID3D12Device* pDevice, const DX12_COMPUTE_PIPELINE_STATE_DESC* pDesc = nullptr, UINT nodeMask = 0u);

				/// @brief シャドウマップ付き平行光ライティングを行うパイプラインを作成する
				///
				/// pDescに設定されたシェーダ、ルートシグネチャは無視されます
				/// @param[in] pDevice
				/// @param[in] pDesc default=nullptr
				/// @param[in] nodeMask default=0u
				static PipelineSet sCreateDirectionLightingWithShadow(ID3D12Device* pDevice, const DX12_COMPUTE_PIPELINE_STATE_DESC* pDesc = nullptr, UINT nodeMask = 0u);

			};
		}
	}
}