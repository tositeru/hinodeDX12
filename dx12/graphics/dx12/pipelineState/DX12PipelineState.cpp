#include "stdafx.h"

#include "DX12PipelineState.h"

#include "../common/Log.h"
#include "../common/Exception.h"

namespace hinode
{
	namespace graphics
	{
		/// @brief DX12GraphicsPipelineStateを内部に持つクラス
		class DX12PipelineState::_impl : private ImplInterfacePointer<DX12PipelineState>
		{
		public:
			UNCOPYABLE_IMPL_MOVE_CONSTRUCTOR(DX12PipelineState);

			ID3D12PipelineState* mpPipeline;
			TYPE mType;

			_impl(DX12PipelineState* pInterface)
				: ImplInterfacePointer(pInterface)
				, mpPipeline(nullptr)
				, mType(eTYPE_UNKNOWN)
			{}

			_impl& operator=(_impl&& right)_noexcept
			{
				this->mpPipeline = right.mpPipeline;
				right.mpPipeline = nullptr;
				this->mType = right.mType;
				return *this;
			}

			~_impl()_noexcept
			{
				this->clear();
			}

			void clear()_noexcept
			{
				safeRelease(&this->mpPipeline);
				this->mType = eTYPE_UNKNOWN;
			}

			void create(ID3D12Device* pDevice, const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc)
			{
				this->clear();
				if (FAILED(pDevice->CreateGraphicsPipelineState(pDesc, IID_PPV_ARGS(&this->mpPipeline)))) {
					throw HINODE_GRAPHICS_CREATE_EXCEPTION(InvalidArgumentsException, "DX12GraphicsPipelineState", "create")
						<< "DX12GraphicsPipelineStateの作成に失敗しました";
				}
				this->mType = eTYPE_GRAPHICS;
			}

			void create(ID3D12Device* pDevice, const D3D12_COMPUTE_PIPELINE_STATE_DESC* pDesc)
			{
				this->clear();
				if (FAILED(pDevice->CreateComputePipelineState(pDesc, IID_PPV_ARGS(&this->mpPipeline)))) {
					throw HINODE_GRAPHICS_CREATE_EXCEPTION(InvalidArgumentsException, "DX12GraphicsPipelineState", "create")
						<< "DX12GraphicsPipelineStateの作成に失敗しました";
				}
				this->mType = eTYPE_COMPUTE;
			}

			void set(ID3D12PipelineState* pState)
			{
				assert(nullptr != pState);
				this->clear();

				this->mpPipeline = pState;
			}

			HRESULT setName(const wchar_t* name) _noexcept
			{
				assert(nullptr != this->mpPipeline || "まだ初期化されていません");
				return this->mpPipeline->SetName(name);
			}

			bool isGood()const noexcept
			{
				return nullptr != this->mpPipeline;
			}

		accessor_declaration:
			TYPE type()const noexcept
			{
				return this->mType;
			}

			ID3D12PipelineState* operator->()_noexcept
			{
				assert(nullptr != this->mpPipeline || "まだ初期化されていません");
				return this->pipelineState();
			}

			ID3D12PipelineState* pipelineState()_noexcept
			{
				assert(nullptr != this->mpPipeline || "まだ初期化されていません");
				return this->mpPipeline;
			}

		};

		UINT DX12PipelineState::sCalDispatchCount(UINT value, UINT threadGroupCount)noexcept
		{
			auto v = value / threadGroupCount;
			v += (value % threadGroupCount != 0 || v == 0) ? 1 : 0;
			return v;
		}

		//===========================================================================
		//
		//	IMPLイディオムのインターフェイスクラスの関数定義
		//

		UNCOPYABLE_PIMPL_IDIOM_CPP_TEMPLATE(DX12PipelineState);

		void DX12PipelineState::clear()_noexcept
		{
			this->impl().clear();
		}

		void DX12PipelineState::create(ID3D12Device* pDevice, const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc)
		{
			this->impl().create(pDevice, pDesc);
		}

		void DX12PipelineState::create(ID3D12Device* pDevice, const D3D12_COMPUTE_PIPELINE_STATE_DESC* pDesc)
		{
			this->impl().create(pDevice, pDesc);
		}

		void DX12PipelineState::set(ID3D12PipelineState* pState)
		{
			this->impl().set(pState);
		}


		HRESULT DX12PipelineState::setName(const wchar_t* name) _noexcept
		{
			return this->impl().setName(name);
		}

		bool DX12PipelineState::isGood()const noexcept
		{
			return this->impl().isGood();
		}

		ID3D12PipelineState* DX12PipelineState::operator->()_noexcept
		{
			return this->impl().pipelineState();
		}

		ID3D12PipelineState* DX12PipelineState::pipelineState()_noexcept
		{
			return this->impl().pipelineState();
		}
	}

	namespace graphics
	{
		DX12_GRAPHICS_PIPELINE_STATE_DESC::DX12_GRAPHICS_PIPELINE_STATE_DESC()
			: D3D12_GRAPHICS_PIPELINE_STATE_DESC({})
		{
			this->setRasterizer(&CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT))
				.setBlend(&CD3DX12_BLEND_DESC(D3D12_DEFAULT), UINT_MAX)
				.setDepthStencil(&DX12DepthStencilDesc())
				.setPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
				.setRTV({ DXGI_FORMAT_R8G8B8A8_UNORM })
				.setDSV(DXGI_FORMAT_D32_FLOAT)
				.setSampling(1, 0)
			;
		}

		DX12_GRAPHICS_PIPELINE_STATE_DESC& DX12_GRAPHICS_PIPELINE_STATE_DESC::setRootSignature(ID3D12RootSignature* pRootSignature)_noexcept
		{
			this->pRootSignature = pRootSignature;
			return *this;
		}

		DX12_GRAPHICS_PIPELINE_STATE_DESC& DX12_GRAPHICS_PIPELINE_STATE_DESC::setVS(D3D12_SHADER_BYTECODE* pByteCode)_noexcept
		{
			this->VS = *pByteCode;
			return *this;
		}

		DX12_GRAPHICS_PIPELINE_STATE_DESC& DX12_GRAPHICS_PIPELINE_STATE_DESC::setHS(D3D12_SHADER_BYTECODE* pByteCode)_noexcept
		{
			this->HS = *pByteCode;
			return *this;
		}

		DX12_GRAPHICS_PIPELINE_STATE_DESC& DX12_GRAPHICS_PIPELINE_STATE_DESC::setDS(D3D12_SHADER_BYTECODE* pByteCode)_noexcept
		{
			this->DS = *pByteCode;
			return *this;
		}

		DX12_GRAPHICS_PIPELINE_STATE_DESC& DX12_GRAPHICS_PIPELINE_STATE_DESC::setGS(D3D12_SHADER_BYTECODE* pByteCode)_noexcept
		{
			this->GS = *pByteCode;
			return *this;
		}

		DX12_GRAPHICS_PIPELINE_STATE_DESC& DX12_GRAPHICS_PIPELINE_STATE_DESC::setPS(D3D12_SHADER_BYTECODE* pByteCode)_noexcept
		{
			this->PS = *pByteCode;
			return *this;
		}

		DX12_GRAPHICS_PIPELINE_STATE_DESC& DX12_GRAPHICS_PIPELINE_STATE_DESC::setVS(ID3DBlob* pBlob)_noexcept
		{
			return this->setVS(&CD3DX12_SHADER_BYTECODE(pBlob));
		}

		DX12_GRAPHICS_PIPELINE_STATE_DESC& DX12_GRAPHICS_PIPELINE_STATE_DESC::setHS(ID3DBlob* pBlob)_noexcept
		{
			return this->setHS(&CD3DX12_SHADER_BYTECODE(pBlob));
		}

		DX12_GRAPHICS_PIPELINE_STATE_DESC& DX12_GRAPHICS_PIPELINE_STATE_DESC::setDS(ID3DBlob* pBlob)_noexcept
		{
			return this->setDS(&CD3DX12_SHADER_BYTECODE(pBlob));
		}

		DX12_GRAPHICS_PIPELINE_STATE_DESC& DX12_GRAPHICS_PIPELINE_STATE_DESC::setGS(ID3DBlob* pBlob)_noexcept
		{
			return this->setGS(&CD3DX12_SHADER_BYTECODE(pBlob));
		}

		DX12_GRAPHICS_PIPELINE_STATE_DESC& DX12_GRAPHICS_PIPELINE_STATE_DESC::setPS(ID3DBlob* pBlob)_noexcept
		{
			return this->setPS(&CD3DX12_SHADER_BYTECODE(pBlob));
		}



		DX12_GRAPHICS_PIPELINE_STATE_DESC& DX12_GRAPHICS_PIPELINE_STATE_DESC::setBlend(const D3D12_BLEND_DESC* pDesc, UINT sampleMask)_noexcept
		{
			this->BlendState = *pDesc;
			this->SampleMask = sampleMask;
			return *this;
		}

		DX12_GRAPHICS_PIPELINE_STATE_DESC& DX12_GRAPHICS_PIPELINE_STATE_DESC::setRasterizer(const D3D12_RASTERIZER_DESC* pDesc)_noexcept
		{
			this->RasterizerState = *pDesc;
			return *this;
		}

		DX12_GRAPHICS_PIPELINE_STATE_DESC& DX12_GRAPHICS_PIPELINE_STATE_DESC::setDepthStencil(D3D12_DEPTH_STENCIL_DESC* pDesc)_noexcept
		{
			this->DepthStencilState = *pDesc;
			return *this;
		}

		DX12_GRAPHICS_PIPELINE_STATE_DESC& DX12_GRAPHICS_PIPELINE_STATE_DESC::setPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE type)_noexcept
		{
			this->PrimitiveTopologyType = type;
			return *this;
		}

		DX12_GRAPHICS_PIPELINE_STATE_DESC& DX12_GRAPHICS_PIPELINE_STATE_DESC::setIndexBufferStripCut(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE value)_noexcept
		{
			this->IBStripCutValue = value;
			return *this;
		}

		DX12_GRAPHICS_PIPELINE_STATE_DESC& DX12_GRAPHICS_PIPELINE_STATE_DESC::setStreamOutput(const D3D12_STREAM_OUTPUT_DESC* pDesc)_noexcept
		{
			this->StreamOutput = *pDesc;
			return *this;
		}

		DX12_GRAPHICS_PIPELINE_STATE_DESC& DX12_GRAPHICS_PIPELINE_STATE_DESC::setSampling(UINT count, UINT quality)_noexcept
		{
			this->SampleDesc.Count = count;
			this->SampleDesc.Quality = quality;
			return *this;
		}

		DX12_GRAPHICS_PIPELINE_STATE_DESC& DX12_GRAPHICS_PIPELINE_STATE_DESC::setNodeMask(UINT mask)_noexcept
		{
			this->NodeMask = mask;
			return *this;
		}

		DX12_GRAPHICS_PIPELINE_STATE_DESC& DX12_GRAPHICS_PIPELINE_STATE_DESC::setDSV(DXGI_FORMAT format)_noexcept
		{
			this->DSVFormat = format;
			return *this;
		}

		DX12_GRAPHICS_PIPELINE_STATE_DESC& DX12_GRAPHICS_PIPELINE_STATE_DESC::setCached(const D3D12_CACHED_PIPELINE_STATE* pState)_noexcept
		{
			this->CachedPSO = *pState;
			return *this;
		}

		DX12_GRAPHICS_PIPELINE_STATE_DESC& DX12_GRAPHICS_PIPELINE_STATE_DESC::onDebugTool()_noexcept
		{
			this->Flags = D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG;
			return *this;
		}

		DX12_GRAPHICS_PIPELINE_STATE_DESC& DX12_GRAPHICS_PIPELINE_STATE_DESC::setRTV(std::initializer_list<DXGI_FORMAT> formats)_noexcept
		{
			assert(formats.size() < _countof(this->RTVFormats));

			if (0 == formats.size()) {
				for (auto& format : this->RTVFormats) {
					format = DXGI_FORMAT_UNKNOWN;
				}
			} else {
				memcpy(this->RTVFormats, formats.begin(), sizeof(DXGI_FORMAT) * formats.size());
			}
			this->NumRenderTargets = static_cast<UINT>(formats.size());
			return *this;
		}
	}
	namespace graphics
	{
		DX12_COMPUTE_PIPELINE_STATE_DESC::DX12_COMPUTE_PIPELINE_STATE_DESC()
			: D3D12_COMPUTE_PIPELINE_STATE_DESC({})
		{ }

		DX12_COMPUTE_PIPELINE_STATE_DESC& DX12_COMPUTE_PIPELINE_STATE_DESC::setRootSignature(ID3D12RootSignature* pRootSignature)noexcept
		{
			this->pRootSignature = pRootSignature;
			return *this;
		}

		DX12_COMPUTE_PIPELINE_STATE_DESC& DX12_COMPUTE_PIPELINE_STATE_DESC::setCS(D3D12_SHADER_BYTECODE* pByteCode)noexcept
		{
			this->CS = *pByteCode;
			return *this;
		}

		DX12_COMPUTE_PIPELINE_STATE_DESC& DX12_COMPUTE_PIPELINE_STATE_DESC::setCS(ID3DBlob* pBlob)noexcept
		{
			return this->setCS(&CD3DX12_SHADER_BYTECODE(pBlob));
		}

		DX12_COMPUTE_PIPELINE_STATE_DESC& DX12_COMPUTE_PIPELINE_STATE_DESC::setNodeMask(UINT mask)noexcept
		{
			this->NodeMask = mask;
			return *this;
		}

		DX12_COMPUTE_PIPELINE_STATE_DESC& DX12_COMPUTE_PIPELINE_STATE_DESC::setCached(const D3D12_CACHED_PIPELINE_STATE* pState)noexcept
		{
			this->CachedPSO = *pState;
			return *this;
		}

		DX12_COMPUTE_PIPELINE_STATE_DESC& DX12_COMPUTE_PIPELINE_STATE_DESC::onDebugTool()noexcept
		{
			this->Flags = D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG;
			return *this;
		}
	}

	namespace graphics
	{
		DX12DepthStencilDesc::DX12DepthStencilDesc()
			: CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT)
		{}

		DX12DepthStencilDesc& DX12DepthStencilDesc::setDepth(bool isEnable, D3D12_COMPARISON_FUNC compareFunc, D3D12_DEPTH_WRITE_MASK writeMask)noexcept
		{
			this->DepthEnable = isEnable;
			this->DepthFunc = compareFunc;
			this->DepthWriteMask = writeMask;
			return *this;
		}

		DX12DepthStencilDesc& DX12DepthStencilDesc::isEnableStencil(bool isEnable)noexcept
		{
			this->StencilEnable = isEnable;
			return *this;
		}

		DX12DepthStencilDesc& DX12DepthStencilDesc::setStencilMask(UINT8 readMask, UINT8 writeMask)noexcept
		{
			this->StencilReadMask = readMask;
			this->StencilWriteMask = writeMask;
			return *this;
		}

		DX12DepthStencilDesc& DX12DepthStencilDesc::setStencilFront(D3D12_COMPARISON_FUNC compareFunc, D3D12_STENCIL_OP passOp, D3D12_STENCIL_OP depthFailOp, D3D12_STENCIL_OP failOp)noexcept
		{
			this->FrontFace.StencilFunc = compareFunc;
			this->FrontFace.StencilPassOp = passOp;
			this->FrontFace.StencilDepthFailOp = depthFailOp;
			this->FrontFace.StencilFailOp = failOp;
			return *this;
		}

		DX12DepthStencilDesc& DX12DepthStencilDesc::setStencilBack(D3D12_COMPARISON_FUNC compareFunc, D3D12_STENCIL_OP passOp, D3D12_STENCIL_OP depthFailOp, D3D12_STENCIL_OP failOp)noexcept
		{
			this->BackFace.StencilFunc = compareFunc;
			this->BackFace.StencilPassOp = passOp;
			this->BackFace.StencilDepthFailOp = depthFailOp;
			this->BackFace.StencilFailOp = failOp;
			return *this;
		}

	}
}