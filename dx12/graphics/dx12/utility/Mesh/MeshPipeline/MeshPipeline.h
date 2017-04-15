#pragma once

#include "..\..\..\pipelineState\DX12PipelineState.h"
#include "../../Shader/PipelineSet/PipelineSet.h"

#include "../../math/SimpleMath.h"
#include "../Shader/PVertex/PVertexPipeline.hlsl"
#include "../Shader/PCVertex/PCVertexPipeline.hlsl"
#include "../Shader/PTVertex/PTVertexPipeline.hlsl"
#include "../Shader/PNVertex/PNVertexPipeline.hlsl"
#include "../Shader/PTNVertex/PTNVertexPipeline.hlsl"
#include "../Shader/PTNCVertex/PTNCVertexPipeline.hlsl"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			/// @brief Forwawrdレンダリング用のパイプラインを作成するためのクラス
			class ForwardPipeline
			{
			public:
				/// @brief 位置のみの頂点を描画するためのパイプラインを作成します
				///
				/// pDescに設定されたシェーダ、ルートシグネチャ、入力レイアウトは無視されます
				/// @param[in] pDevice
				/// @param[in] pDesc
				static PipelineSet sCreatePVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc=nullptr, UINT nodeMask=0u);

				/// @brief 位置と色情報を持つ頂点を描画するためのパイプラインを作成します
				///
				/// pDescに設定されたシェーダ、ルートシグネチャ、入力レイアウトは無視されます
				/// @param[in] pDevice
				/// @param[in] pDesc
				static PipelineSet sCreatePCVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc = nullptr, UINT nodeMask = 0u);

				/// @brief 位置とテクスチャ座標情報を持つ頂点を描画するためのパイプラインを作成します
				///
				/// pDescに設定されたシェーダ、ルートシグネチャ、入力レイアウトは無視されます
				/// @param[in] pDevice
				/// @param[in] pDesc
				static PipelineSet sCreatePTVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc = nullptr, UINT nodeMask = 0u);

				/// @brief 位置と法線情報を持つ頂点を描画するためのパイプラインを作成します
				///
				/// pDescに設定されたシェーダ、ルートシグネチャ、入力レイアウトは無視されます
				/// @param[in] pDevice
				/// @param[in] pDesc
				static PipelineSet sCreatePNVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc = nullptr, UINT nodeMask = 0u);

				/// @brief 位置とテクスチャ座標、法線情報を持つ頂点を描画するためのパイプラインを作成します
				///
				/// pDescに設定されたシェーダ、ルートシグネチャ、入力レイアウトは無視されます
				/// @param[in] pDevice
				/// @param[in] pDesc
				static PipelineSet sCreatePTNVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc = nullptr, UINT nodeMask = 0u);

				/// @brief 位置とテクスチャ座標、法線、色情報を持つ頂点を描画するためのパイプラインを作成します
				///
				/// pDescに設定されたシェーダ、ルートシグネチャ、入力レイアウトは無視されます
				/// @param[in] pDevice
				/// @param[in] pDesc
				static PipelineSet sCreatePTNCVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc = nullptr, UINT nodeMask = 0u);

			};

			/// @brief Defferedレンダリング用のパイプラインを作成するためのクラス
			class DefferedPipeline
			{
			public:
				/// @brief 位置のみの頂点を描画するためのパイプラインを作成します
				///
				/// pDescに設定されたシェーダ、ルートシグネチャ、入力レイアウトは無視されます
				/// @param[in] pDevice
				/// @param[in] pDesc
				static PipelineSet sCreatePVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc = nullptr, UINT nodeMask = 0u);

				/// @brief 位置と色情報を持つ頂点を描画するためのパイプラインを作成します
				///
				/// pDescに設定されたシェーダ、ルートシグネチャ、入力レイアウトは無視されます
				/// @param[in] pDevice
				/// @param[in] pDesc
				static PipelineSet sCreatePCVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc = nullptr, UINT nodeMask = 0u);

				/// @brief 位置とテクスチャ座標情報を持つ頂点を描画するためのパイプラインを作成します
				///
				/// pDescに設定されたシェーダ、ルートシグネチャ、入力レイアウトは無視されます
				/// @param[in] pDevice
				/// @param[in] pDesc
				static PipelineSet sCreatePTVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc = nullptr, UINT nodeMask = 0u);

				/// @brief 位置と法線情報を持つ頂点を描画するためのパイプラインを作成します
				///
				/// pDescに設定されたシェーダ、ルートシグネチャ、入力レイアウトは無視されます
				/// @param[in] pDevice
				/// @param[in] pDesc
				static PipelineSet sCreatePNVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc = nullptr, UINT nodeMask = 0u);

				/// @brief 位置とテクスチャ座標、法線情報を持つ頂点を描画するためのパイプラインを作成します
				///
				/// pDescに設定されたシェーダ、ルートシグネチャ、入力レイアウトは無視されます
				/// @param[in] pDevice
				/// @param[in] pDesc
				static PipelineSet sCreatePTNVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc = nullptr, UINT nodeMask = 0u);

				/// @brief 位置とテクスチャ座標、法線、色情報を持つ頂点を描画するためのパイプラインを作成します
				///
				/// pDescに設定されたシェーダ、ルートシグネチャ、入力レイアウトは無視されます
				/// @param[in] pDevice
				/// @param[in] pDesc
				static PipelineSet sCreatePTNCVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc = nullptr, UINT nodeMask = 0u);

			};

			/// @brief ZPass用のパイプラインを作成するためのクラス
			class ZPassPipeline
			{
			public:
				/// @brief 位置のみの頂点を描画するためのパイプラインを作成します
				///
				/// pDescに設定されたシェーダ、ルートシグネチャ、入力レイアウトは無視されます
				/// @param[in] pDevice
				/// @param[in] pDesc
				static PipelineSet sCreatePVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc = nullptr, UINT nodeMask = 0u);

				/// @brief 位置と色情報を持つ頂点を描画するためのパイプラインを作成します
				///
				/// pDescに設定されたシェーダ、ルートシグネチャ、入力レイアウトは無視されます
				/// @param[in] pDevice
				/// @param[in] pDesc
				static PipelineSet sCreatePCVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc = nullptr, UINT nodeMask = 0u);

				/// @brief 位置とテクスチャ座標情報を持つ頂点を描画するためのパイプラインを作成します
				///
				/// pDescに設定されたシェーダ、ルートシグネチャ、入力レイアウトは無視されます
				/// @param[in] pDevice
				/// @param[in] pDesc
				static PipelineSet sCreatePTVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc = nullptr, UINT nodeMask = 0u);

				/// @brief 位置と法線情報を持つ頂点を描画するためのパイプラインを作成します
				///
				/// pDescに設定されたシェーダ、ルートシグネチャ、入力レイアウトは無視されます
				/// @param[in] pDevice
				/// @param[in] pDesc
				static PipelineSet sCreatePNVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc = nullptr, UINT nodeMask = 0u);

				/// @brief 位置とテクスチャ座標、法線情報を持つ頂点を描画するためのパイプラインを作成します
				///
				/// pDescに設定されたシェーダ、ルートシグネチャ、入力レイアウトは無視されます
				/// @param[in] pDevice
				/// @param[in] pDesc
				static PipelineSet sCreatePTNVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc = nullptr, UINT nodeMask = 0u);

				/// @brief 位置とテクスチャ座標、法線、色情報を持つ頂点を描画するためのパイプラインを作成します
				///
				/// pDescに設定されたシェーダ、ルートシグネチャ、入力レイアウトは無視されます
				/// @param[in] pDevice
				/// @param[in] pDesc
				static PipelineSet sCreatePTNCVertex(ID3D12Device* pDevice, const DX12_GRAPHICS_PIPELINE_STATE_DESC* pDesc = nullptr, UINT nodeMask = 0u);

			};

		}
	}
}