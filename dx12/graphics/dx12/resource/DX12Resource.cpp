#include "stdafx.h"

#include "DX12Resource.h"

#include <vector>
#include "../common/Log.h"
#include "../common/Exception.h"

#include "../descriptorHeap/DX12DescriptorHeap.h"

#undef min
#undef max

namespace hinode
{
	namespace graphics
	{
		/// @brief DX12Resourceを内部に持つクラス
		class DX12Resource::_impl : private ImplInterfacePointer<DX12Resource>
		{
		public:
			UNCOPYABLE_IMPL_MOVE_CONSTRUCTOR(DX12Resource);

			ID3D12Resource* mpResource;
			std::vector<View> mViews;
			D3D12_RESOURCE_STATES mCurrentStates;
			D3D12_CLEAR_VALUE mClearValue;
			DX12DescriptorHeap mDescriptorHeapForClear;

			_impl(DX12Resource* pInterface)
				: ImplInterfacePointer(pInterface)
				, mpResource(nullptr)
				, mCurrentStates(D3D12_RESOURCE_STATE_COMMON)
			{}

			_impl& operator=(_impl&& right)noexcept
			{
				this->mpResource = right.mpResource;
				this->mViews = std::move(right.mViews);
				this->mCurrentStates = right.mCurrentStates;
				this->mClearValue = right.mClearValue;
				this->mDescriptorHeapForClear = std::move(right.mDescriptorHeapForClear);
				right.mpResource = nullptr;
				return *this;
			}

			~_impl()noexcept
			{
				this->clear();
			}

			void clear()noexcept
			{
				this->mViews.clear();
				this->mViews.shrink_to_fit();
				this->mCurrentStates = D3D12_RESOURCE_STATE_COMMON;
				this->mClearValue = D3D12_CLEAR_VALUE();
				this->mDescriptorHeapForClear.clear();
				safeRelease(&this->mpResource);
			}

			void create(
				ID3D12Device* pDevice,
				const D3D12_HEAP_PROPERTIES *pHeapProperties,
				D3D12_HEAP_FLAGS heapFlags,
				const D3D12_RESOURCE_DESC* pResourceDesc,
				D3D12_RESOURCE_STATES initialStates,
				const D3D12_CLEAR_VALUE* pClearValue)
			{
				this->clear();
				if (FAILED(pDevice->CreateCommittedResource(pHeapProperties, heapFlags, pResourceDesc, initialStates, pClearValue, IID_PPV_ARGS(&this->mpResource)))) {
					throw HINODE_GRAPHICS_CREATE_EXCEPTION(FailedToCreateException, "DX12Resource", "create")
						<< "DX12Resourceの作成に失敗しました";
				}
				this->mCurrentStates = initialStates;
				if (pClearValue) {
					this->mClearValue = *pClearValue;
				}
			}

			void create(
				ID3D12Device* pDevice,
				const DX12ResourceHeapDesc& heapDesc,
				const DX12ResourceDesc& resourceDesc)
			{
				this->clear();

				const D3D12_CLEAR_VALUE* pClearValue = &resourceDesc.clearValue;
				unless(resourceDesc.enableClearValue()) {
					pClearValue = nullptr;
				}

				if (FAILED(pDevice->CreateCommittedResource(&heapDesc.properties, heapDesc.flags, &resourceDesc.desc, resourceDesc.initialStates, pClearValue, IID_PPV_ARGS(&this->mpResource)))) {
					throw HINODE_GRAPHICS_CREATE_EXCEPTION(FailedToCreateException, "DX12Resource", "create")
						<< "DX12Resourceの作成に失敗しました";
				}
				this->mCurrentStates = resourceDesc.initialStates;
				this->mClearValue = resourceDesc.clearValue;
			}

			void create(
				ID3D12Device* pDevice,
				ID3D12Heap* pHeap,
				UINT heapOffset,
				const D3D12_RESOURCE_DESC* pResourceDesc,
				D3D12_RESOURCE_STATES initialStates,
				const D3D12_CLEAR_VALUE* pClearValue)
			{
				assert(nullptr != pHeap);
				this->clear();
				if (FAILED(pDevice->CreatePlacedResource(pHeap, heapOffset, pResourceDesc, initialStates, pClearValue, IID_PPV_ARGS(&this->mpResource)))) {
					throw HINODE_GRAPHICS_CREATE_EXCEPTION(FailedToCreateException, "DX12Resource", "create")
						<< "Placed Resourceの作成に失敗しました";
				}
				this->mCurrentStates = initialStates;
				if (pClearValue) {
					this->mClearValue = *pClearValue;
				}
			}

			void create(
				ID3D12Device* pDevice,
				ID3D12Heap* pHeap,
				UINT heapOffset,
				const DX12ResourceDesc& resourceDesc)
			{
				assert(nullptr != pHeap);
				this->clear();

				const D3D12_CLEAR_VALUE* pClearValue = &resourceDesc.clearValue;
				unless(resourceDesc.enableClearValue()) {
					pClearValue = nullptr;
				}

				if (FAILED(pDevice->CreatePlacedResource(pHeap, heapOffset, &resourceDesc.desc, resourceDesc.initialStates, pClearValue, IID_PPV_ARGS(&this->mpResource)))) {
					throw HINODE_GRAPHICS_CREATE_EXCEPTION(FailedToCreateException, "DX12Resource", "create")
						<< "Placed Resourceの作成に失敗しました";
				}
				this->mCurrentStates = resourceDesc.initialStates;
				this->mClearValue = resourceDesc.clearValue;
			}

			void create(ID3D12Resource* pTransferredResource, D3D12_RESOURCE_STATES state, bool isMove = true)
			{
				assert(nullptr != pTransferredResource);

				this->clear();

				this->mpResource = pTransferredResource;
				this->mpResource->AddRef();
				this->mCurrentStates = state;
				if (isMove) {
					pTransferredResource->Release();
				}
			}

			void createForCopy(ID3D12Device* pDevice, DX12Resource& target, const void* pInitData, UINT64 initDataByteSize)
			{
				assert(target.isGood());

				auto desc = target->GetDesc();

				//CPUからGPUへデータを転送するときは、一度Bufferに転送する必要がある
				if (D3D12_RESOURCE_DIMENSION_BUFFER != desc.Dimension) {
					desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
					desc.Width = initDataByteSize;
					desc.Height = desc.DepthOrArraySize = 1;
					desc.Format = DXGI_FORMAT_UNKNOWN;
					desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
					desc.MipLevels = 1;
				}
				this->create(pDevice, &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC(desc), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr);
				if (pInitData) {
					this->update(0, nullptr, [&](const D3D12_RANGE* pRange, void* pData, D3D12_RANGE* pWritedRange) {
						memcpy(pData, pInitData, static_cast<size_t>(initDataByteSize));
						return true;
					});
				}
			}

			HRESULT setSwapChianBuffer(IDXGISwapChain* pSwapChain, UINT index)noexcept
			{
				this->clear();
				return pSwapChain->GetBuffer(index, IID_PPV_ARGS(&this->mpResource));
			}

			void appendView(const View& view)noexcept
			{
				assert(nullptr != this->mpResource && "まだ初期化されていません");
				
				auto v = view;
				switch (view.type()) {
				case View::eVERTEX:			v.mVertexView.BufferLocation = this->mpResource->GetGPUVirtualAddress(); break;
				case View::eINDEX:			v.mIndexView.BufferLocation = this->mpResource->GetGPUVirtualAddress(); break;
				case View::eSTREAM_OUTPUT:	v.mStreamOutputView.BufferLocation = this->mpResource->GetGPUVirtualAddress(); break;
				}
				this->mViews.push_back(v);
			}

			HRESULT update(UINT subresourceIndex, const D3D12_RANGE* pRange, std::function<bool(const D3D12_RANGE* pRange, void* pData, D3D12_RANGE* pWritedRange)> predicate)noexcept
			{
				assert(nullptr != this->mpResource && "まだ初期化されていません");

				void* pData = nullptr;
				auto hr = this->mpResource->Map(subresourceIndex, pRange, &pData);
				if (FAILED(hr)) {
					return hr;
				}
				D3D12_RANGE writedRange;
				bool mayAllWrited = predicate(pRange, pData, &writedRange);
				this->mpResource->Unmap(subresourceIndex, mayAllWrited ? nullptr : &writedRange);
				return hr;
			}
			
			D3D12_RESOURCE_BARRIER makeBarrier(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, UINT subresource, D3D12_RESOURCE_BARRIER_FLAGS flags)
			{
				assert(nullptr != this->mpResource && "まだ初期化されていません");
				return CD3DX12_RESOURCE_BARRIER::Transition(this->mpResource, before, after, subresource, flags);
			}

			D3D12_RESOURCE_BARRIER makeBarrier(D3D12_RESOURCE_STATES next, UINT subresource, D3D12_RESOURCE_BARRIER_FLAGS flags)
			{
				assert(this->isGood() && "まだ初期化されていません");
				auto ret = CD3DX12_RESOURCE_BARRIER::Transition(this->mpResource, this->mCurrentStates, next, subresource, flags);
				this->mCurrentStates = next;
				return ret;
			}

			D3D12_SHADER_RESOURCE_VIEW_DESC makeSRV(const MakeViewDesc& viewDesc)noexcept
			{
				assert(this->isGood() && "まだ初期化されていません");
				auto desc = this->mpResource->GetDesc();
				assert(!(D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE & desc.Flags) && "このリソースではSRVは無効化されています。");

				D3D12_SHADER_RESOURCE_VIEW_DESC result;
				switch (desc.Dimension) {
				case D3D12_RESOURCE_DIMENSION_BUFFER:
					//assert(false && "未実装　作ってね");
					result.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
					result.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
					result.Format = desc.Format;
					result.Buffer.FirstElement = 0;
					result.Buffer.Flags = viewDesc.isRawBuffer ? D3D12_BUFFER_SRV_FLAG_RAW : D3D12_BUFFER_SRV_FLAG_NONE;
					result.Buffer.NumElements = viewDesc.elementCount;
					result.Buffer.StructureByteStride = viewDesc.structureByteStride;
					break;
				case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
					result.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
					result.Format = desc.Format;
					result.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
					if (1 <= desc.DepthOrArraySize) {
						result.Texture1D.MipLevels = desc.MipLevels;
						result.Texture1D.MostDetailedMip = 0;
						result.Texture1D.ResourceMinLODClamp = 0;
					} else {
						result.Texture1DArray.ArraySize = desc.DepthOrArraySize;
						result.Texture1DArray.FirstArraySlice = 0;
						result.Texture1DArray.MipLevels = desc.MipLevels;
						result.Texture1DArray.MostDetailedMip = 0;
						result.Texture1DArray.ResourceMinLODClamp = 0;
					}
					break;
				case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
					result.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
					result.Format = desc.Format;
					result.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
					if (1 <= desc.DepthOrArraySize) {
						if (viewDesc.isCubemap) {
							result.TextureCube.MipLevels = desc.MipLevels;
							result.TextureCube.MostDetailedMip = 0;
							result.TextureCube.ResourceMinLODClamp = 0;
						}  else if (1 < desc.SampleDesc.Count) {
							result.Texture2DMS.UnusedField_NothingToDefine;
						} else {
							result.Texture2D.MipLevels = desc.MipLevels;
							result.Texture2D.MostDetailedMip = 0;
							result.Texture2D.PlaneSlice = 0;
							result.Texture2D.ResourceMinLODClamp = 0.f;
						}
					} else {
						if (viewDesc.isCubemap) {
							result.TextureCubeArray.First2DArrayFace = 0;
							result.TextureCubeArray.NumCubes = desc.DepthOrArraySize / 6;
							result.TextureCubeArray.MipLevels = desc.MipLevels;
							result.TextureCubeArray.MostDetailedMip = 0;
							result.TextureCubeArray.ResourceMinLODClamp = 0;
						} else if (1 < desc.SampleDesc.Count) {
							result.Texture2DMSArray.ArraySize = desc.DepthOrArraySize;
							result.Texture2DMSArray.FirstArraySlice = 0;
						} else {
							result.Texture2DArray.ArraySize = desc.DepthOrArraySize;
							result.Texture2DArray.FirstArraySlice = 0;
							result.Texture2DArray.MipLevels = desc.MipLevels;
							result.Texture2DArray.MostDetailedMip = 0;
							result.Texture2DArray.PlaneSlice = 0;
							result.Texture2DArray.ResourceMinLODClamp = 0.f;
						}
					}
					break;
				case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
					result.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
					result.Format = desc.Format;
					result.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
					result.Texture3D.MipLevels = desc.MipLevels;
					result.Texture3D.MostDetailedMip = 0;
					result.Texture3D.ResourceMinLODClamp = 0.f;
					break;
				default:
					assert(false && "未実装");
				}
				return result;
			}

			D3D12_UNORDERED_ACCESS_VIEW_DESC makeUAV(const MakeViewDesc& viewDesc)noexcept
			{
				assert(this->isGood() && "まだ初期化されていません");

				auto desc = this->mpResource->GetDesc();
				assert(D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS & desc.Flags && "このリソースではUAVは無効化されています。");

				D3D12_UNORDERED_ACCESS_VIEW_DESC result;
				result.Format = desc.Format;
				switch (desc.Dimension) {
				case D3D12_RESOURCE_DIMENSION_BUFFER:
					assert(false && "未実装　作ってね");
					result.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
					result.Buffer.CounterOffsetInBytes;
					result.Buffer.FirstElement = 0;
					result.Buffer.NumElements;
					result.Buffer.StructureByteStride;
					result.Buffer.Flags;
					break;
				case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
					if (1 == desc.DepthOrArraySize) {
						result.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
						result.Texture1D.MipSlice = 0;
					} else {
						result.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
						result.Texture1DArray.ArraySize = desc.DepthOrArraySize;
						result.Texture1DArray.FirstArraySlice = 0;
						result.Texture1DArray.MipSlice = 0;
					}
					break;
				case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
					if (1 == desc.DepthOrArraySize) {
						result.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
						result.Texture2D.MipSlice = 0;
						result.Texture2D.PlaneSlice = 0;
					} else {
						result.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
						result.Texture2DArray.ArraySize = desc.DepthOrArraySize;
						result.Texture2DArray.FirstArraySlice = 0;
						result.Texture2DArray.MipSlice = 0;
						result.Texture2DArray.PlaneSlice = 0;
					}
					break;
				case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
					result.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
					result.Texture3D.MipSlice = 0;
					result.Texture3D.FirstWSlice = 0;
					result.Texture3D.WSize = 0;
					break;
				default:
					assert(false && "未実装");
				}
				return result;
			}

			D3D12_RENDER_TARGET_VIEW_DESC makeRTV(const MakeViewDesc& viewDesc)noexcept
			{
				assert(this->isGood() && "まだ初期化されていません");
				auto desc = this->mpResource->GetDesc();
				assert(D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET & desc.Flags && "このリソースではRTVは無効化されています。");

				D3D12_RENDER_TARGET_VIEW_DESC result;
				result.Format = desc.Format;
				switch (desc.Dimension) {
				case D3D12_RESOURCE_DIMENSION_BUFFER:
					assert(false && "未実装　作ってね");
					result.ViewDimension = D3D12_RTV_DIMENSION_BUFFER;
					result.Buffer.FirstElement = 0;
					result.Buffer.NumElements = 0;
					break;
				case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
					if (1 == desc.DepthOrArraySize) {
						result.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
						result.Texture1D.MipSlice = 0;
					} else {
						result.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
						result.Texture1DArray.MipSlice = 0;
						result.Texture1DArray.ArraySize = desc.DepthOrArraySize;
						result.Texture1DArray.FirstArraySlice = 0;
					}
					break;
				case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
					if (1 == desc.SampleDesc.Count) {
						if (1 == desc.DepthOrArraySize) {
							result.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
							result.Texture2D.MipSlice = 0;
							result.Texture2D.PlaneSlice = 0;
						} else {
							result.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
							result.Texture2DArray.MipSlice = 0;
							result.Texture2DArray.PlaneSlice = 0;
							result.Texture2DArray.ArraySize = desc.DepthOrArraySize;
							result.Texture2DArray.FirstArraySlice = 0;
						}
					} else {
						if (1 == desc.DepthOrArraySize) {
							result.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
						} else {
							result.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
							result.Texture2DMSArray.ArraySize = desc.DepthOrArraySize;
							result.Texture2DMSArray.FirstArraySlice = 0;
						}
					}
					break;
				case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
					result.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
					result.Texture3D.FirstWSlice = 0;
					result.Texture3D.MipSlice = 0;
					result.Texture3D.WSize = -1;
					break;
				default:
					assert(false && "未実装");
					break;
				}
				return result;
			}

			D3D12_DEPTH_STENCIL_VIEW_DESC makeDSV(const MakeViewDesc& viewDesc)noexcept
			{
				assert(this->isGood() && "まだ初期化されていません");
				auto desc = this->mpResource->GetDesc();
				assert(D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL & desc.Flags && "このリソースではDSVは無効化されています。");

				D3D12_DEPTH_STENCIL_VIEW_DESC result;
				result.Format = desc.Format;
				result.Flags = D3D12_DSV_FLAG_NONE;
				switch (desc.Dimension) {
				case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
					if (1 == desc.DepthOrArraySize) {
						result.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
						result.Texture1D.MipSlice = 0;
					} else {
						result.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
						result.Texture1DArray.MipSlice = 0;
						result.Texture1DArray.FirstArraySlice = 0;
						result.Texture1DArray.ArraySize = desc.DepthOrArraySize;
					}
					break;
				case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
					if (1 == desc.SampleDesc.Count) {
						if (1 == desc.DepthOrArraySize) {
							result.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
							result.Texture2D.MipSlice = 0;
						} else {
							result.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
							result.Texture2DArray.MipSlice = 0;
							result.Texture2DArray.ArraySize = desc.DepthOrArraySize;
							result.Texture2DArray.FirstArraySlice = 0;
						}
					} else {
						if (1 == desc.DepthOrArraySize) {
							result.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
						} else {
							result.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
							result.Texture2DMSArray.ArraySize = desc.DepthOrArraySize;
							result.Texture2DMSArray.FirstArraySlice = 0;
						}
					}
					break;
				case D3D12_RESOURCE_DIMENSION_BUFFER:
				case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
					assert(false && "バッファとTexture3Dは深度ステンシルとして使えません。");
					break;
				default:
					assert(false && "未実装");
					break;
				}

				return result;
			}

			void createDescriptorHeapForClear(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT nodeMask)
			{
				assert(this->isGood() && "まだ初期化されていません");
				auto pDevice = this->getDevice();
				D3D12_DESCRIPTOR_HEAP_DESC desc;
				desc.Type = type;
				desc.NodeMask = nodeMask;
				desc.NumDescriptors = 1;
				desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
				this->mDescriptorHeapForClear.create(pDevice, &desc);
				auto handle = this->mDescriptorHeapForClear.makeCpuHandle(0);
				
				switch (type) {
				case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
					assert((this->mpResource->GetDesc().Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) && "このリソースはUnorderedAccessViewを使用できません。");
					handle.createUnorderedAccessView(pDevice, this->mpResource, nullptr, nullptr);
					break;
				case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
					assert((this->mpResource->GetDesc().Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) && "このリソースはRenderTargetViewを使用できません。");
					handle.createRenderTargetView(pDevice, this->mpResource, nullptr);
					break;
				case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
					assert((this->mpResource->GetDesc().Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) && "このリソースはDepthStencilViewを使用できません。");
					handle.createDepthStencilView(pDevice, this->mpResource, nullptr);
					break;
				default:
					assert("対応していないDescriptorHeapType");
				}
			}

			HRESULT setName(const wchar_t* name) noexcept
			{
				assert(this->isGood() && "まだ初期化されていません");
				return this->mpResource->SetName(name);
			}

			bool isGood()const noexcept
			{
				return nullptr != this->mpResource;
			}

		accessor_declaration:
			ID3D12Resource* operator->()noexcept
			{
				assert(this->isGood() && "まだ初期化されていません");
				return this->resource();
			}

			ID3D12Resource* resource()noexcept
			{
				assert(this->isGood() && "まだ初期化されていません");
				return this->mpResource;
			}

			ID3D12Resource** resourcePointer()noexcept
			{
				assert(this->isGood() && "まだ初期化されていません");
				return &this->mpResource;
			}

			const View& view(int index)const
			{
				assert(this->isGood() && "まだ初期化されていません");
				assert(static_cast<size_t>(index) < this->mViews.size() && "指定されたインデックスのビューはありません。");
				return this->mViews[index];
			}

			D3D12_RESOURCE_STATES currentState()const noexcept
			{
				return this->mCurrentStates;
			}

			const D3D12_CLEAR_VALUE& clearValue()const noexcept
			{
				return this->mClearValue;
			}

			DX12DescriptorHeap& descriptorHeapForClear()noexcept
			{
				assert(this->isGood() && "まだ初期化されていません");
				assert(this->mDescriptorHeapForClear.isGood() && "Descriptor Heapは初期化されていません。");
				return this->mDescriptorHeapForClear;
			}

			ID3D12Device* getDevice()noexcept
			{
				ID3D12Device* pDevice;
				auto hr = this->mpResource->GetDevice(IID_PPV_ARGS(&pDevice));
				assert(SUCCEEDED(hr));
				pDevice->Release();
				return pDevice;
			}

			D3D12_PLACED_SUBRESOURCE_FOOTPRINT getCopyableFootprint(UINT subresourceIndex, UINT64 baseOffset, UINT64* pOutTotalByteSize = nullptr)noexcept
			{
				assert(this->isGood() && "まだ初期化されていません");
				D3D12_PLACED_SUBRESOURCE_FOOTPRINT result;
				this->getDevice()->GetCopyableFootprints(
					&this->mpResource->GetDesc(),
					subresourceIndex,
					1,
					baseOffset,
					&result, nullptr, nullptr, pOutTotalByteSize);
				return result;
			}

			std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> getCopyableFootprints(UINT firstSubresource, UINT subresourceCount, UINT64 baseOffset, UINT64* pOutTotalByteSize = nullptr)noexcept
			{
				assert(this->isGood() && "まだ初期化されていません");
				std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> results;
				results.resize(subresourceCount);
				std::vector<UINT64> totalBytes;
				UINT64* pTotalBytes = nullptr;
				if (pOutTotalByteSize) {
					totalBytes.resize(subresourceCount);
					pTotalBytes = &totalBytes[0];
				}

				this->getDevice()->GetCopyableFootprints(
					&this->mpResource->GetDesc(),
					firstSubresource,
					subresourceCount,
					baseOffset,
					&results[0], nullptr, nullptr, pTotalBytes);

				if (pOutTotalByteSize) {
					for (auto& t : totalBytes) *pOutTotalByteSize += t;
				}

				return results;
			}
		};

		UINT16 DX12Resource::sCalMipLevel(UINT16 w)noexcept
		{
			return static_cast<UINT16>(ceil(log2(std::max<UINT16>(1u, w)))) + 1u;
		}

		UINT16 DX12Resource::sCalMipLevel(UINT16 w, UINT16 h)noexcept
		{
			return std::max(sCalMipLevel(w), sCalMipLevel(h));
		}

		UINT16 DX12Resource::sCalMipLevel(UINT16 w, UINT16 h, UINT16 d)noexcept
		{
			return std::max(std::max(sCalMipLevel(w), sCalMipLevel(h)), sCalMipLevel(d));
		}

		//===========================================================================
		//
		//	IMPLイディオムのインターフェイスクラスの関数定義
		//

		UNCOPYABLE_PIMPL_IDIOM_CPP_TEMPLATE(DX12Resource);

		void DX12Resource::clear()noexcept
		{
			this->impl().clear();
		}

		void DX12Resource::create(ID3D12Device* pDevice, const D3D12_HEAP_PROPERTIES *pHeapProperties, D3D12_HEAP_FLAGS heapFlags, const D3D12_RESOURCE_DESC* pResourceDesc, D3D12_RESOURCE_STATES initialStates, const D3D12_CLEAR_VALUE* pClearValue)
		{
			this->impl().create(pDevice, pHeapProperties, heapFlags, pResourceDesc, initialStates, pClearValue);
		}

		void DX12Resource::create(ID3D12Device* pDevice, const DX12ResourceHeapDesc& heapDesc, const DX12ResourceDesc& resourceDesc)
		{
			this->impl().create(pDevice, heapDesc, resourceDesc);
		}

		void DX12Resource::create(ID3D12Device* pDevice, ID3D12Heap* pHeap, UINT heapOffset, const D3D12_RESOURCE_DESC* pResourceDesc, D3D12_RESOURCE_STATES initialStates, const D3D12_CLEAR_VALUE* pClearValue)
		{
			this->impl().create(pDevice, pHeap, heapOffset, pResourceDesc, initialStates, pClearValue);
		}

		void DX12Resource::create(ID3D12Device* pDevice, ID3D12Heap* pHeap, UINT heapOffset, const DX12ResourceDesc& resourceDesc)
		{
			this->impl().create(pDevice, pHeap, heapOffset, resourceDesc);
		}

		void DX12Resource::create(ID3D12Resource* pTransferredResource, D3D12_RESOURCE_STATES state, bool isMove)
		{
			this->impl().create(pTransferredResource, state, isMove);
		}

		void DX12Resource::createForCopy(ID3D12Device* pDevice, DX12Resource& target, const void* pInitData, UINT64 initDataByteSize)
		{
			this->impl().createForCopy(pDevice, target, pInitData, initDataByteSize);
		}

		HRESULT DX12Resource::setSwapChianBuffer(IDXGISwapChain* pSwapChain, UINT index)noexcept
		{
			return this->impl().setSwapChianBuffer(pSwapChain, index);
		}

		void DX12Resource::appendView(const View& view)noexcept
		{
			this->impl().appendView(view);
		}

		HRESULT DX12Resource::update(UINT subresourceIndex, const D3D12_RANGE* pRange, std::function<bool(const D3D12_RANGE* pRange, void* pData, D3D12_RANGE* pWritedRange)> predicate)noexcept
		{
			return this->impl().update(subresourceIndex, pRange, predicate);
		}

		D3D12_RESOURCE_BARRIER DX12Resource::makeBarrier(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, UINT subresource, D3D12_RESOURCE_BARRIER_FLAGS flags)
		{
			return this->impl().makeBarrier(before, after, subresource, flags);
		}

		D3D12_RESOURCE_BARRIER DX12Resource::makeBarrier(D3D12_RESOURCE_STATES next, UINT subresource, D3D12_RESOURCE_BARRIER_FLAGS flags)
		{
			return this->impl().makeBarrier(next, subresource, flags);
		}

		D3D12_SHADER_RESOURCE_VIEW_DESC DX12Resource::makeSRV(const MakeViewDesc& desc)noexcept
		{
			return this->impl().makeSRV(desc);
		}

		D3D12_UNORDERED_ACCESS_VIEW_DESC DX12Resource::makeUAV(const MakeViewDesc& desc)noexcept
		{
			return this->impl().makeUAV(desc);
		}

		D3D12_RENDER_TARGET_VIEW_DESC DX12Resource::makeRTV(const MakeViewDesc& desc)noexcept
		{
			return this->impl().makeRTV(desc);
		}

		D3D12_DEPTH_STENCIL_VIEW_DESC DX12Resource::makeDSV(const MakeViewDesc& desc)noexcept
		{
			return this->impl().makeDSV(desc);
		}

		void DX12Resource::createDescriptorHeapForClear(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT nodeMask)
		{
			this->impl().createDescriptorHeapForClear(type, nodeMask);
		}

		HRESULT DX12Resource::setName(const wchar_t* name) noexcept
		{
			return this->impl().setName(name);
		}

		bool DX12Resource::isGood()const noexcept
		{
			return this->impl().isGood();
		}

		ID3D12Resource* DX12Resource::operator->()noexcept
		{
			return this->impl().resource();
		}

		ID3D12Resource* DX12Resource::resource()noexcept
		{
			return this->impl().resource();
		}

		ID3D12Resource** DX12Resource::resourcePointer()noexcept
		{
			return this->impl().resourcePointer();
		}

		const DX12Resource::View& DX12Resource::view(int index)const
		{
			return this->impl().view(index);
		}

		D3D12_RESOURCE_STATES DX12Resource::currentState()const noexcept
		{
			return this->impl().currentState();
		}

		const D3D12_CLEAR_VALUE& DX12Resource::clearValue()const noexcept
		{
			return this->impl().clearValue();
		}

		DX12DescriptorHeap& DX12Resource::descriptorHeapForClear()noexcept
		{
			return this->impl().descriptorHeapForClear();
		}

		ID3D12Device* DX12Resource::getDevice()noexcept
		{
			return this->impl().getDevice();
		}

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT DX12Resource::getCopyableFootprint(UINT subresourceIndex, UINT64 baseOffset, UINT64* pOutTotalByteSize)noexcept
		{
			return this->impl().getCopyableFootprint(subresourceIndex, baseOffset, pOutTotalByteSize);
		}

		std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> DX12Resource::getCopyableFootprints(UINT firstSubresource, UINT subresourceCount, UINT64 baseOffset, UINT64* pOutTotalByteSize)noexcept
		{
			return this->impl().getCopyableFootprints(firstSubresource, subresourceCount, baseOffset, pOutTotalByteSize);
		}
	}

	namespace graphics
	{
		DX12Resource::View::View()noexcept
		{}

		DX12Resource::View& DX12Resource::View::setVertex(UINT strideInBytes, UINT sizeInBytes)noexcept
		{
			this->mType = eVERTEX;
			this->mVertexView.SizeInBytes = sizeInBytes;
			this->mVertexView.StrideInBytes = strideInBytes;
			return *this;
		}

		DX12Resource::View& DX12Resource::View::setIndex(DXGI_FORMAT format, UINT sizeInBytes)noexcept
		{
			this->mType = eINDEX;
			this->mIndexView.Format = format;
			this->mIndexView.SizeInBytes = sizeInBytes;
			return *this;
		}

		DX12Resource::View& DX12Resource::View::setStreamOutput(UINT sizeInBytes, D3D12_GPU_VIRTUAL_ADDRESS filledSizeLocation)noexcept
		{
			this->mType = eSTREAM_OUTPUT;
			this->mStreamOutputView.SizeInBytes = sizeInBytes;
			this->mStreamOutputView.BufferFilledSizeLocation = filledSizeLocation;
			return *this;
		}

		DX12Resource::View& DX12Resource::View::setSRV(D3D12_SHADER_RESOURCE_VIEW_DESC& srv)noexcept
		{
			this->mType = eSRV;
			this->mSRV = srv;
			return *this;
		}

		DX12Resource::View& DX12Resource::View::setUAV(D3D12_UNORDERED_ACCESS_VIEW_DESC& uav)noexcept
		{
			this->mType = eUAV;
			this->mUAV = uav;
			return *this;
		}

		DX12Resource::View& DX12Resource::View::setRTV(D3D12_RENDER_TARGET_VIEW_DESC& rtv)noexcept
		{
			this->mType = eRTV;
			this->mRTV = rtv;
			return *this;
		}

		DX12Resource::View& DX12Resource::View::setDSV(D3D12_DEPTH_STENCIL_VIEW_DESC& dsv)noexcept
		{
			this->mType = eDSV;
			this->mDSV = dsv;
			return *this;
		}

		DX12Resource::View::TYPE DX12Resource::View::type()const noexcept
		{
			return this->mType;
		}

		const D3D12_VERTEX_BUFFER_VIEW& DX12Resource::View::vertex()const noexcept
		{
			assert(eVERTEX == this->mType);
			return this->mVertexView;
		}

		const D3D12_INDEX_BUFFER_VIEW& DX12Resource::View::index()const noexcept
		{
			assert(eINDEX == this->mType);
			return this->mIndexView;
		}

		const D3D12_STREAM_OUTPUT_BUFFER_VIEW& DX12Resource::View::streamOutput()const noexcept
		{
			assert(eSTREAM_OUTPUT == this->mType);
			return this->mStreamOutputView;
		}

		const D3D12_SHADER_RESOURCE_VIEW_DESC& DX12Resource::View::srv()const noexcept
		{
			assert(eSRV == this->mType);
			return this->mSRV;
		}

		const D3D12_UNORDERED_ACCESS_VIEW_DESC& DX12Resource::View::uav()const noexcept
		{
			assert(eUAV == this->mType);
			return this->mUAV;
		}

		const D3D12_RENDER_TARGET_VIEW_DESC& DX12Resource::View::rtv()const noexcept
		{
			assert(eRTV == this->mType);
			return this->mRTV;
		}

		const D3D12_DEPTH_STENCIL_VIEW_DESC& DX12Resource::View::dsv()const noexcept
		{
			assert(eDSV == this->mType);
			return this->mDSV;
		}


	}

	namespace graphics
	{
		DX12Resource::MakeViewDesc::MakeViewDesc()noexcept
			: isCubemap(false)
		{}

		DX12Resource::MakeViewDesc& DX12Resource::MakeViewDesc::setTexture2D(bool isCubemap)noexcept
		{
			this->isCubemap = isCubemap;
			return *this;
		}

	}
	namespace graphics
	{
		DX12ResourceHeapDesc::DX12ResourceHeapDesc()
			: DX12ResourceHeapDesc(D3D12_HEAP_TYPE_DEFAULT, D3D12_HEAP_FLAG_NONE)
		{ }

		DX12ResourceHeapDesc::DX12ResourceHeapDesc(D3D12_HEAP_TYPE heapType, D3D12_HEAP_FLAGS heapFlags)
			: properties(heapType)
			, flags(heapFlags)
		{}
	}

	namespace graphics
	{
		DX12ResourceDesc::DX12ResourceDesc()
			: desc()
			, initialStates(D3D12_RESOURCE_STATE_COMMON)
			, clearValue(DXGI_FORMAT_UNKNOWN, 1, 0)
		{}

		void DX12ResourceDesc::setClearValueForColor(DXGI_FORMAT format, float r, float g, float b, float a)noexcept
		{
			this->clearValue.Format = format;
			this->clearValue.Color[0] = r;
			this->clearValue.Color[1] = g;
			this->clearValue.Color[2] = b;
			this->clearValue.Color[3] = a;
		}

		void DX12ResourceDesc::setClearValueForDepth(DXGI_FORMAT format, float depth, UINT8 stencil)noexcept
		{
			this->clearValue.Format = format;
			this->clearValue.DepthStencil.Depth = depth;
			this->clearValue.DepthStencil.Stencil;
		}

		bool DX12ResourceDesc::enableClearValue()const noexcept
		{
			return 
				   D3D12_RESOURCE_DIMENSION_BUFFER != this->desc.Dimension
				&& (this->desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL || this->desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
		}
	}
}