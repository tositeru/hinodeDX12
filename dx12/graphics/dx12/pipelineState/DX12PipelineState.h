#pragma once

#include <vector>
#include <memory>
#include <array>

#include "../common/Common.h"
#include "../IDX12Interface.h"

namespace hinode
{
	namespace graphics
	{
		/// @brief ID3D12PipelineStateを内部に持つクラス
		class DX12PipelineState : public IDX12Interface
		{
			UNCOPYABLE_PIMPL_IDIOM_TEMPLATE(DX12PipelineState)
		public:
			static UINT sCalDispatchCount(UINT value, UINT threadGroupCount)noexcept;
		public:
			enum TYPE
			{
				eTYPE_UNKNOWN,
				eTYPE_GRAPHICS,
				eTYPE_COMPUTE,
			};
		public:
			/// @brief メモリ開放
			void clear()noexcept;

			/// @brief グラフィックスパイプラインステートを作成する
			///
			/// 作成に成功した場合はdesc.pAdapterを内部で保持します。
			/// @param[in] pDevice
			/// @param[in] pDesc
			/// @exception hinode::graphics::InvalidArgumentsException
			void create(ID3D12Device* pDevice, const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc);

			/// @brief コンピュートパイプラインステートを作成する
			///
			/// 作成に成功した場合はdesc.pAdapterを内部で保持します。
			/// @param[in] pDevice
			/// @param[in] pDesc
			/// @exception hinode::graphics::InvalidArgumentsException
			void create(ID3D12Device* pDevice, const D3D12_COMPUTE_PIPELINE_STATE_DESC* pDesc);

			/// @brief すでに作成しているID3D12GraphicsPieplineStateを設定する
			///
			/// 廃止予定
			/// @param[in] pState
			void set(ID3D12PipelineState* pState);

			/// @brief 内部のID3D12PipelineStateに名前をつける
			/// @param[in] const wchar_t* name
			/// @retval HRESULT
			HRESULT setName(const wchar_t* name) noexcept override;

			/// @brief 使用できる状態か?
			/// @retval bool
			bool isGood()const noexcept override;

		accessor_declaration:
			TYPE type()const noexcept;
			ID3D12PipelineState* operator->()noexcept;

			ID3D12PipelineState* pipelineState()noexcept;
			operator ID3D12PipelineState*()noexcept { return this->pipelineState(); }
		};
	}

	namespace graphics
	{
		/// @brief D3D12_GRAPHICS_PIPELINE_STATE_DESCに設定用メンバ関数を追加したもの
		struct DX12_GRAPHICS_PIPELINE_STATE_DESC : public D3D12_GRAPHICS_PIPELINE_STATE_DESC
		{
			std::vector<D3D12_INPUT_ELEMENT_DESC> mInputElements;
			DX12_GRAPHICS_PIPELINE_STATE_DESC();

			DX12_GRAPHICS_PIPELINE_STATE_DESC& setRootSignature(ID3D12RootSignature* pRootSignature)noexcept;
			DX12_GRAPHICS_PIPELINE_STATE_DESC& setVS(D3D12_SHADER_BYTECODE* pByteCode)noexcept;
			DX12_GRAPHICS_PIPELINE_STATE_DESC& setHS(D3D12_SHADER_BYTECODE* pByteCode)noexcept;
			DX12_GRAPHICS_PIPELINE_STATE_DESC& setDS(D3D12_SHADER_BYTECODE* pByteCode)noexcept;
			DX12_GRAPHICS_PIPELINE_STATE_DESC& setGS(D3D12_SHADER_BYTECODE* pByteCode)noexcept;
			DX12_GRAPHICS_PIPELINE_STATE_DESC& setPS(D3D12_SHADER_BYTECODE* pByteCode)noexcept;
			DX12_GRAPHICS_PIPELINE_STATE_DESC& setVS(ID3DBlob* pBlob)noexcept;
			DX12_GRAPHICS_PIPELINE_STATE_DESC& setHS(ID3DBlob* pBlob)noexcept;
			DX12_GRAPHICS_PIPELINE_STATE_DESC& setDS(ID3DBlob* pBlob)noexcept;
			DX12_GRAPHICS_PIPELINE_STATE_DESC& setGS(ID3DBlob* pBlob)noexcept;
			DX12_GRAPHICS_PIPELINE_STATE_DESC& setPS(ID3DBlob* pBlob)noexcept;
			DX12_GRAPHICS_PIPELINE_STATE_DESC& setBlend(const D3D12_BLEND_DESC* pDesc, UINT sampleMask = UINT_MAX)noexcept;
			DX12_GRAPHICS_PIPELINE_STATE_DESC& setRasterizer(const D3D12_RASTERIZER_DESC* pDesc)noexcept;
			DX12_GRAPHICS_PIPELINE_STATE_DESC& setDepthStencil(D3D12_DEPTH_STENCIL_DESC* pDesc)noexcept;
			DX12_GRAPHICS_PIPELINE_STATE_DESC& setPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE type)noexcept;
			DX12_GRAPHICS_PIPELINE_STATE_DESC& setIndexBufferStripCut(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE value)noexcept;
			DX12_GRAPHICS_PIPELINE_STATE_DESC& setStreamOutput(const D3D12_STREAM_OUTPUT_DESC* pDesc)noexcept;
			DX12_GRAPHICS_PIPELINE_STATE_DESC& setSampling(UINT count, UINT quality)noexcept;
			DX12_GRAPHICS_PIPELINE_STATE_DESC& setNodeMask(UINT mask)noexcept;
			DX12_GRAPHICS_PIPELINE_STATE_DESC& setDSV(DXGI_FORMAT format)noexcept;
			DX12_GRAPHICS_PIPELINE_STATE_DESC& setCached(const D3D12_CACHED_PIPELINE_STATE* pState)noexcept;
			DX12_GRAPHICS_PIPELINE_STATE_DESC& onDebugTool()noexcept;

			template<size_t count>
			DX12_GRAPHICS_PIPELINE_STATE_DESC& setRTV(const std::array<DXGI_FORMAT, count> formats)noexcept
			{
				static_assert(count < _countof(this->RTVFormats), "");
				memcpy(this->RTVFormats, formats.data(), sizeof(formats[0]) * formats.size());
				this->NumRenderTargets = static_cast<UINT>(formats.size());
				return *this;
			}

			DX12_GRAPHICS_PIPELINE_STATE_DESC& setRTV(std::initializer_list<DXGI_FORMAT> formats)noexcept;

			template<size_t count>
			DX12_GRAPHICS_PIPELINE_STATE_DESC& setInputLayout(const std::array<D3D12_INPUT_ELEMENT_DESC, count>& layouts)noexcept
			{
				this->mInputElements.resize(layouts.size());
				for (size_t i = 0; i < layouts.size(); ++i) {
					this->mInputElements[i] = layouts[i];
				}

				this->InputLayout.pInputElementDescs = this->mInputElements.data();
				this->InputLayout.NumElements = static_cast<UINT>(this->mInputElements.size());
				return *this;
			}

			DX12_GRAPHICS_PIPELINE_STATE_DESC& setInputLayout(const std::vector<D3D12_INPUT_ELEMENT_DESC>& layouts)noexcept
			{
				this->mInputElements = layouts;
				this->InputLayout.pInputElementDescs = this->mInputElements.data();
				this->InputLayout.NumElements = static_cast<UINT>(this->mInputElements.size());
				return *this;
			}
		};
	}
	namespace graphics
	{
		/// @brief D3D12_COMPUTE_PIPELINE_STATE_DESCに設定用メンバ関数を追加したもの
		struct DX12_COMPUTE_PIPELINE_STATE_DESC : public D3D12_COMPUTE_PIPELINE_STATE_DESC
		{
			DX12_COMPUTE_PIPELINE_STATE_DESC();

			DX12_COMPUTE_PIPELINE_STATE_DESC& setRootSignature(ID3D12RootSignature* pRootSignature)noexcept;
			DX12_COMPUTE_PIPELINE_STATE_DESC& setCS(D3D12_SHADER_BYTECODE* pByteCode)noexcept;
			DX12_COMPUTE_PIPELINE_STATE_DESC& setCS(ID3DBlob* pBlob)noexcept;
			DX12_COMPUTE_PIPELINE_STATE_DESC& setNodeMask(UINT mask)noexcept;
			DX12_COMPUTE_PIPELINE_STATE_DESC& setCached(const D3D12_CACHED_PIPELINE_STATE* pState)noexcept;
			DX12_COMPUTE_PIPELINE_STATE_DESC& onDebugTool()noexcept;
		};
	}
	namespace graphics
	{
		struct DX12DepthStencilDesc : public CD3DX12_DEPTH_STENCIL_DESC
		{
			DX12DepthStencilDesc();

			DX12DepthStencilDesc& setDepth(bool isEnable, D3D12_COMPARISON_FUNC compareFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL, D3D12_DEPTH_WRITE_MASK writeMask= D3D12_DEPTH_WRITE_MASK_ALL)noexcept;
			DX12DepthStencilDesc& isEnableStencil(bool isEnable)noexcept;
			DX12DepthStencilDesc& setStencilMask(UINT8 readMask, UINT8 writeMask)noexcept;
			DX12DepthStencilDesc& setStencilFront(D3D12_COMPARISON_FUNC compareFunc, D3D12_STENCIL_OP passOp, D3D12_STENCIL_OP depthFailOp, D3D12_STENCIL_OP failOp)noexcept;
			DX12DepthStencilDesc& setStencilBack(D3D12_COMPARISON_FUNC compareFunc, D3D12_STENCIL_OP passOp, D3D12_STENCIL_OP depthFailOp, D3D12_STENCIL_OP failOp)noexcept;
		};
	}
}