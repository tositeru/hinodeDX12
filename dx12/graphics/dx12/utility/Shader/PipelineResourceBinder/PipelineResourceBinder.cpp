#include "stdafx.h"

#include "PipelineResourceBinder.h"

#include "../RootSignatureInfo/RootSignatureInfo.h"
#include "../Pipeline/Pipeline.h"
#include "../../../common/Log.h"


namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			PipelineResourceBinder::PipelineResourceBinder()
			{
			}

			PipelineResourceBinder::~PipelineResourceBinder()
			{
				this->clear();
			}

			void PipelineResourceBinder::clear()
			{
				this->mpRootElements.clear();
				this->mType = DX12PipelineState::eTYPE_UNKNOWN;
				this->mReplaceIndices.reset();
			}

			void PipelineResourceBinder::create(ID3D12Device* pDevice, DX12PipelineState::TYPE type, const RootSignatureInfo& rootSignatureInfo, UINT nodeMask)
			{
				this->clear();

				this->mpRootElements.reserve(rootSignatureInfo.rootElements().size());
				for (auto& pElement : rootSignatureInfo.rootElements()) {
					if (D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE == pElement->type) {
						auto rootDescriptor = pElement->cast<RootSignatureInfo::RootDescriptorTable>();
						auto sp = std::make_shared<DescriptorTable>();
						auto descriptorType = (D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER == rootDescriptor.ranges[0].RangeType) ? DescriptorTable::eDESCRIPTOR_SAMPLER : DescriptorTable::eDESCRIPTOR_CBV_SRV_UAV;
						auto initialDescriptorCount = rootDescriptor.calDescriptorCount();
						sp->create(rootDescriptor.rootIndex, descriptorType, initialDescriptorCount);
						this->mpRootElements.push_back(sp);
					} else if (D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS == pElement->type) {
						auto rootConstants = pElement->cast<RootSignatureInfo::RootConstants>();
						auto sp = std::make_shared<RootConstants>();
						sp->create(rootConstants.rootIndex, rootConstants.count32BitValue);
						this->mpRootElements.push_back(sp);
					} else if (D3D12_ROOT_PARAMETER_TYPE_CBV == pElement->type) {
						auto sp = std::make_shared<RootCBV>();
						sp->create(pElement->rootIndex);
						this->mpRootElements.push_back(sp);
					} else if (D3D12_ROOT_PARAMETER_TYPE_SRV == pElement->type) {
						auto sp = std::make_shared<RootSRV>();
						sp->create(pElement->rootIndex);
						this->mpRootElements.push_back(sp);
					} else if (D3D12_ROOT_PARAMETER_TYPE_UAV == pElement->type) {
						auto sp = std::make_shared<RootUAV>();
						sp->create(pElement->rootIndex);
						this->mpRootElements.push_back(sp);
					} else {
						assert(false && "未実装");
					}
				}
				this->mType = type;
			}

			void PipelineResourceBinder::clone(PipelineResourceBinder* pOut)
			{
				assert(nullptr != pOut);

				pOut->mpRootElements = this->mpRootElements;
				pOut->mType = this->mType;
				pOut->mReplaceIndices = this->mReplaceIndices;
			}

			void PipelineResourceBinder::replace(size_t index)
			{
				auto& src = this->mpRootElements.at(index);
				this->mpRootElements.at(index) = src->clone();
			}

			void PipelineResourceBinder::replace(std::initializer_list<size_t> indices)
			{
				for (auto index : indices) {
					auto& src = this->mpRootElements.at(index);
					this->mpRootElements.at(index) = src->clone();
				}
			}

			void PipelineResourceBinder::replaceAuto()
			{
				for (auto i = 0u; i < this->mReplaceIndices.size(); ++i) {
					if (!this->mReplaceIndices[i]) continue;

					auto& src = this->mpRootElements.at(i);
					this->mpRootElements.at(i) = src->clone();
				}
			}

			void PipelineResourceBinder::setAutoReplaceIndex(std::initializer_list<size_t> indices)
			{
				this->mReplaceIndices.reset();
				for (auto index : indices) {
					this->mReplaceIndices.set(index);
				}
			}

			void PipelineResourceBinder::bind(ID3D12GraphicsCommandList* pCommandList)
			{
				switch (this->mType) {
				case DX12PipelineState::eTYPE_GRAPHICS:
					for (auto& sp : this->mpRootElements) {
						sp->bindForGraphics(pCommandList);
					}
					break;
				case DX12PipelineState::eTYPE_COMPUTE:
					for (auto& sp : this->mpRootElements) {
						sp->bindForCompute(pCommandList);
					}
					break;
				default:
					assert(false);
				}
			}

			bool PipelineResourceBinder::validate(const Pipeline& pipeline)noexcept
			{
				bool isValid = true;
				for (auto& sp : this->mpRootElements) {
					unless(sp->validate()) {
						isValid = false;
						break;
					}
				}
				try {
					unless(isValid) {
						//以下、ログ出力
						auto log = HINODE_GRAPHICS_ERROR_LOG("PipelineResourceBinder", "validate");
						log << "ルートパラメータに設定されていない要素があります。";
						log.setAutoWrote(false);
						for (auto& sp : this->mpRootElements) {
							if (sp->validate())
								continue;
							log << Log::eBR;
							RootSignatureInfo::BindingInfo bindingInfo;
							bindingInfo.type = sp->sToDX12RootParameterType(sp->type());
							bindingInfo.rootIndex = sp->rootIndex();
							auto& rootParam = pipeline.rootSignatureInfo().rootElement(bindingInfo);
							log << "  RootParameter type=" << rootParam.toTypeStr() << " rootIndex=" << sp->rootIndex();
							if (IRootElement::eDESCRIPTOR_TABLE == sp->type()) {
								auto& descriptorTable = sp->cast<DescriptorTable>();
								auto& rootDescriptor = rootParam.cast<RootSignatureInfo::RootDescriptorTable>();
								for (auto i = 0u; i < rootDescriptor.ranges.size(); ++i) {
									if (descriptorTable.isSetValidation()[i]) {
										continue;
									}
									auto& ranges = rootDescriptor.ranges[i];
									auto& info = pipeline.shaderResourceInfo().find(ranges.RangeType, ranges.BaseShaderRegister, ranges.RegisterSpace);
									log << Log::eBR;
									log << "   table index=" << i << " name=\"" << info.Name << "\""
										<< " register(" << info.BindPoint << ", " << info.Space << ")"
										<< " type=" << DX12RootSignature::sToStr(info.Type);
								}
							} else {
								auto key = pipeline.rootSignatureInfo().findKey(bindingInfo);
								auto& info = pipeline.shaderResourceInfo().find(
									RootSignatureInfo::Key::sToD3D12DescriptorRangeType(key.type),
									key.slot, key.space);
								log << Log::eBR;
								log << "   name=\"" << info.Name << "\""
									<< " register(" << info.BindPoint << ", " << info.Space << ")"
									<< " type=" << DX12RootSignature::sToStr(info.Type);
							}
						}
						log.setAutoWrote(true);
					}
					return isValid;
				} catch (Exception& e) {
					e.writeLog();
					return false;
				} catch (...) {
					HINODE_GRAPHICS_ERROR_LOG("PipelineResourceBinder", "validate")
						<< "不明な例外が発生しました";
					return false;
				}
			}

			void PipelineResourceBinder::updateCBV(size_t rootIndex, size_t descriptorTableIndex, DescriptorHeapPoolSet& dhPoolSet, const void* pData, uint byteSize, ConstantBufferPool& cbPool)
			{
				auto& element = this->rootElement(rootIndex);
				switch (element.type()) {
				case IRootElement::eDESCRIPTOR_TABLE:
				{
					auto& descriptorTable = element.cast<PipelineResourceBinder::DescriptorTable>();
					if (!dhPoolSet.resource().isSameAllocateFrameID(descriptorTable.dhAllocateInfo())) {
						//確保したフレームが異なるなら確保し直す
						descriptorTable.allocate(dhPoolSet.resource());
					}
					auto cpuHandle = descriptorTable.getCPUHandle(static_cast<int>(descriptorTableIndex));
					auto allocateInfo = cbPool.allocate(pData, byteSize);
					cpuHandle.createConstantBufferView(dhPoolSet.resource().getDevice(), &allocateInfo.cbViewDesc);

					descriptorTable.setValidation(descriptorTableIndex);
					break;
				}
				case IRootElement::eROOT_CONSTANTS:
				{
					auto& constants = element.cast<PipelineResourceBinder::RootConstants>();
					constants.update(pData, byteSize / sizeof(uint32_t), 0);
					break;
				}
				case IRootElement::eROOT_CBV:
				{
					auto& rootCBV = element.cast<PipelineResourceBinder::RootCBV>();
					auto allocateInfo = cbPool.allocate(pData, byteSize);
					rootCBV.updateGPUAddress(allocateInfo.cbViewDesc.BufferLocation);
					break;
				}
				default:
					assert(false);
				}
			}

			void PipelineResourceBinder::updateCBV(size_t rootIndex, size_t descriptorTableIndex, DescriptorHeapPoolSet& dhPoolSet, ConstantBufferPool::AllocateInfo& allocateInfo)
			{
				auto& element = this->rootElement(rootIndex);
				switch (element.type()) {
				case IRootElement::eDESCRIPTOR_TABLE:
				{
					auto& descriptorTable = element.cast<PipelineResourceBinder::DescriptorTable>();
					if (!dhPoolSet.resource().isSameAllocateFrameID(descriptorTable.dhAllocateInfo())) {
						//確保したフレームが異なるなら確保し直す
						descriptorTable.allocate(dhPoolSet.resource());
					}
					auto cpuHandle = descriptorTable.getCPUHandle(static_cast<int>(descriptorTableIndex));
					cpuHandle.createConstantBufferView(dhPoolSet.resource().getDevice(), &allocateInfo.cbViewDesc);

					descriptorTable.setValidation(descriptorTableIndex);
					break;
				}
				case IRootElement::eROOT_CONSTANTS:
				{
					auto& constants = element.cast<PipelineResourceBinder::RootConstants>();			
					constants.update(allocateInfo.cpuMemory, allocateInfo.cbViewDesc.SizeInBytes / sizeof(uint32_t), 0);
					break;
				}
				case IRootElement::eROOT_CBV:
				{
					auto& rootCBV = element.cast<PipelineResourceBinder::RootCBV>();
					rootCBV.updateGPUAddress(allocateInfo.cbViewDesc.BufferLocation);
					break;
				}
				default:
					assert(false);
				}
			}

			void PipelineResourceBinder::updateSRV(size_t rootIndex, size_t descriptorTableIndex, DescriptorHeapPoolSet& dhPoolSet, ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pSRVDesc)
			{
				auto& element = this->rootElement(rootIndex);

				switch (element.type()) {
				case IRootElement::eDESCRIPTOR_TABLE:
				{
					auto& descriptorTable = element.cast<PipelineResourceBinder::DescriptorTable>();
					if (!dhPoolSet.resource().isSameAllocateFrameID(descriptorTable.dhAllocateInfo())) {
						//確保したフレームが異なるなら確保し直す
						descriptorTable.allocate(dhPoolSet.resource());
					}
					auto cpuHandle = descriptorTable.getCPUHandle(static_cast<int>(descriptorTableIndex));
					cpuHandle.createShaderResourceView(dhPoolSet.resource().getDevice(), pResource, pSRVDesc);

					descriptorTable.setValidation(descriptorTableIndex);
					break;
				}
				case IRootElement::eROOT_SRV:
				{
					auto& rootSRV = element.cast<PipelineResourceBinder::RootSRV>();
					rootSRV.updateGPUAddress(pResource->GetGPUVirtualAddress());
					break;
				}
				default:
					assert(false);
				}
			}

			void PipelineResourceBinder::updateUAV(size_t rootIndex, size_t descriptorTableIndex, DescriptorHeapPoolSet& dhPoolSet, ID3D12Resource* pResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* pUAVDesc, ID3D12Resource* pCounterResource)
			{
				auto& element = this->rootElement(rootIndex);
				switch (element.type()) {
				case IRootElement::eDESCRIPTOR_TABLE:
				{
					auto& descriptorTable = element.cast<PipelineResourceBinder::DescriptorTable>();
					if (!dhPoolSet.resource().isSameAllocateFrameID(descriptorTable.dhAllocateInfo())) {
						//確保したフレームが異なるなら確保し直す
						descriptorTable.allocate(dhPoolSet.resource());
					}
					auto cpuHandle = descriptorTable.getCPUHandle(static_cast<int>(descriptorTableIndex));
					cpuHandle.createUnorderedAccessView(dhPoolSet.resource().getDevice(), pResource, pUAVDesc, pCounterResource);

					descriptorTable.setValidation(descriptorTableIndex);
					break;
				}
				case IRootElement::eROOT_UAV:
				{
					auto& rootUAV = element.cast<PipelineResourceBinder::RootUAV>();
					rootUAV.updateGPUAddress(pResource->GetGPUVirtualAddress());
					break;
				}
				default:
					assert(false);
				}
			}

			void PipelineResourceBinder::updateSampler(size_t rootIndex, size_t descriptorTableIndex, DescriptorHeapPoolSet& dhPoolSet, const D3D12_SAMPLER_DESC* pSamplerDesc)
			{
				auto& element = this->rootElement(rootIndex);
				switch (element.type()) {
				case IRootElement::eDESCRIPTOR_TABLE:
				{
					auto& descriptorTable = element.cast<PipelineResourceBinder::DescriptorTable>();
					if (!dhPoolSet.sampler().isSameAllocateFrameID(descriptorTable.dhAllocateInfo())) {
						//確保したフレームが異なるなら確保し直す
						descriptorTable.allocate(dhPoolSet.sampler());
					}
					auto cpuHandle = descriptorTable.getCPUHandle(static_cast<int>(descriptorTableIndex));
					cpuHandle.createSampler(dhPoolSet.sampler().getDevice(), pSamplerDesc);

					descriptorTable.setValidation(descriptorTableIndex);
					break;
				}
				default:
					assert(false);
				}
			}

			DX12PipelineState::TYPE PipelineResourceBinder::type()const noexcept
			{
				return this->mType;
			}

			std::vector<std::shared_ptr<PipelineResourceBinder::IRootElement>>& PipelineResourceBinder::rootElements()noexcept
			{
				return this->mpRootElements;
			}

			PipelineResourceBinder::IRootElement& PipelineResourceBinder::rootElement(size_t index)
			{
				return *this->mpRootElements.at(index);
			}

			//
			//	GraphicsResourceBinder::IRootElement
			//

			D3D12_ROOT_PARAMETER_TYPE PipelineResourceBinder::IRootElement::sToDX12RootParameterType(TYPE type)noexcept
			{
				switch (type) {
				case eDESCRIPTOR_TABLE: return D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				case eROOT_CONSTANTS:	return D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
				case eROOT_CBV:			return D3D12_ROOT_PARAMETER_TYPE_CBV;
				case eROOT_SRV:			return D3D12_ROOT_PARAMETER_TYPE_SRV;
				case eROOT_UAV:			return D3D12_ROOT_PARAMETER_TYPE_UAV;
				default:
					assert(false && "未対応");
				}
				return D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			}

			PipelineResourceBinder::IRootElement::IRootElement(TYPE type)
				: mRootIndex(-1)
				, mType(type)
			{}

			int PipelineResourceBinder::IRootElement::rootIndex()const noexcept
			{
				return this->mRootIndex;
			}

			PipelineResourceBinder::IRootElement::TYPE PipelineResourceBinder::IRootElement::type()const noexcept
			{
				return this->mType;
			}

			void PipelineResourceBinder::IRootElement::clear()
			{
				this->mRootIndex = -1;
			}

			//
			// GraphicsResourceBinder::DescriptorTable
			//

			PipelineResourceBinder::DescriptorTable::DescriptorTable()
				: DescriptorTable(eDESCRIPTOR_UNKNOWN)
			{}

			PipelineResourceBinder::DescriptorTable::DescriptorTable(DESCRIPTOR_TYPE type)
				: IRootElement(eDESCRIPTOR_TABLE)
				, mDescriptorType(type)
				, mDescriptorCount(-1)
				, mDHAllocateInfo()
			{}

			void PipelineResourceBinder::DescriptorTable::clear()
			{
				IRootElement::clear();
				//this->mDescriptors.clear();
				this->mDescriptorCount = -1;
				this->mDescriptorType = DescriptorTable::eDESCRIPTOR_UNKNOWN;
				this->mDHAllocateInfo.clear();
				this->mIsSetValidations.clear();
				this->mIsSetValidations.shrink_to_fit();
			}

			void PipelineResourceBinder::DescriptorTable::create(int rootIndex, DESCRIPTOR_TYPE descriptorType, int initialDescriptorCount)noexcept
			{
				this->clear();
				this->mRootIndex = rootIndex;
				this->mDescriptorType = descriptorType;
				this->mDescriptorCount = initialDescriptorCount;

				this->mIsSetValidations.resize(initialDescriptorCount);
			}

			void PipelineResourceBinder::DescriptorTable::allocate(DescriptorHeapPool& dhPool)
			{
				this->mDHAllocateInfo = dhPool.allocate(this->mDescriptorCount);
				std::fill(this->mIsSetValidations.begin(), this->mIsSetValidations.end(), false);
			}

			void PipelineResourceBinder::DescriptorTable::bindForGraphics(ID3D12GraphicsCommandList* pCommandList)
			{
				pCommandList->SetGraphicsRootDescriptorTable(this->mRootIndex, this->mDHAllocateInfo.gpuHandle);
			}

			void PipelineResourceBinder::DescriptorTable::bindForCompute(ID3D12GraphicsCommandList* pCommandList)
			{
				pCommandList->SetComputeRootDescriptorTable(this->mRootIndex, this->mDHAllocateInfo.gpuHandle);
			}

			std::shared_ptr<PipelineResourceBinder::IRootElement> PipelineResourceBinder::DescriptorTable::clone()
			{
				auto sp = std::make_shared<PipelineResourceBinder::DescriptorTable>(*this);
				sp->create(this->mRootIndex, this->mDescriptorType, this->mDescriptorCount);
				return sp;
			}

			bool PipelineResourceBinder::DescriptorTable::validate()const noexcept
			{
				for (const auto& isSet : this->mIsSetValidations) {
					unless(isSet) {
						return false;
					}
				}
				return true;
			}

			void PipelineResourceBinder::DescriptorTable::setValidation(size_t offset, size_t count)
			{
				assert(offset + (count-1) < this->mIsSetValidations.size());
				for (auto i = 0u; i < count; ++i) {
					this->mIsSetValidations[offset + i] = true;
				}
			}

			DX12CPUDescriptorHandle PipelineResourceBinder::DescriptorTable::getCPUHandle(int offset)
			{
				return this->dhAllocateInfo().cpuHandle.makeHandle(offset);
			}

			int PipelineResourceBinder::DescriptorTable::descriptorCount()const noexcept
			{
				return this->mDescriptorCount;
			}

			const std::vector<bool>& PipelineResourceBinder::DescriptorTable::isSetValidation()const noexcept
			{
				return this->mIsSetValidations;
			}

			PipelineResourceBinder::DescriptorTable::DESCRIPTOR_TYPE PipelineResourceBinder::DescriptorTable::descriptorType()const noexcept
			{
				return this->mDescriptorType;
			}

			DescriptorHeapPool::AllocateInfo& PipelineResourceBinder::DescriptorTable::dhAllocateInfo()noexcept
			{
				return this->mDHAllocateInfo;
			}

			const DescriptorHeapPool::AllocateInfo& PipelineResourceBinder::DescriptorTable::dhAllocateInfo()const noexcept
			{
				return this->mDHAllocateInfo;
			}

			//
			// GraphicsResourceBinder::RootConstants
			//

			PipelineResourceBinder::RootConstants::RootConstants()
				: IRootElement(IRootElement::eROOT_CONSTANTS)
				, mCount(0)
			{}

			void PipelineResourceBinder::RootConstants::clear()
			{
				IRootElement::clear();
				this->mCount = 0;
				this->mData.clear();
			}

			void PipelineResourceBinder::RootConstants::create(int rootIndex, int count32BitValue)
			{
				this->clear();
				this->mRootIndex = rootIndex;
				this->mCount = count32BitValue;
				this->mData.resize(this->mCount);
			}

			void PipelineResourceBinder::RootConstants::bindForGraphics(ID3D12GraphicsCommandList* pCommandList)
			{
				pCommandList->SetGraphicsRoot32BitConstants(this->mRootIndex, this->mCount, this->mData.data(), 0);
			}

			void PipelineResourceBinder::RootConstants::bindForCompute(ID3D12GraphicsCommandList* pCommandList)
			{
				pCommandList->SetComputeRoot32BitConstants(this->mRootIndex, this->mCount, this->mData.data(), 0);
			}

			std::shared_ptr<PipelineResourceBinder::IRootElement> PipelineResourceBinder::RootConstants::clone()
			{
				auto sp = std::make_shared<PipelineResourceBinder::RootConstants>();
				sp->create(this->mRootIndex, this->count());
				return sp;
			}

			bool PipelineResourceBinder::RootConstants::validate()const noexcept
			{
				return true;
			}

			void PipelineResourceBinder::RootConstants::update(const void* pData, int count, int offset)noexcept
			{
				assert(static_cast<size_t>(offset + count) <= this->mData.size());
				memcpy(this->mData.data() + offset, pData, sizeof(uint32_t) * count);
			}

			int PipelineResourceBinder::RootConstants::count()const noexcept
			{
				return this->mCount;
			}

			//
			// GraphicsResourceBinder::RootCBV
			//

			PipelineResourceBinder::RootCBV::RootCBV()
				: IRootElement(IRootElement::eROOT_CBV)
			{ }

			void PipelineResourceBinder::RootCBV::clear()
			{
				IRootElement::clear();
				this->mpGPUAddress = 0;
			}

			void PipelineResourceBinder::RootCBV::create(int rootIndex)
			{
				this->clear();
				this->mRootIndex = rootIndex;
			}

			void PipelineResourceBinder::RootCBV::bindForGraphics(ID3D12GraphicsCommandList* pCommandList)
			{
				assert(0 != this->mpGPUAddress);
				pCommandList->SetGraphicsRootConstantBufferView(this->mRootIndex, this->mpGPUAddress);
			}

			void PipelineResourceBinder::RootCBV::bindForCompute(ID3D12GraphicsCommandList* pCommandList)
			{
				assert(0 != this->mpGPUAddress);
				pCommandList->SetComputeRootConstantBufferView(this->mRootIndex, this->mpGPUAddress);
			}

			std::shared_ptr<PipelineResourceBinder::IRootElement> PipelineResourceBinder::RootCBV::clone()
			{
				auto sp = std::make_shared<PipelineResourceBinder::RootCBV>();
				sp->create(this->mRootIndex);
				return sp;
			}

			bool PipelineResourceBinder::RootCBV::validate()const noexcept
			{
				return 0 != this->mpGPUAddress;
			}

			void PipelineResourceBinder::RootCBV::updateGPUAddress(D3D12_GPU_VIRTUAL_ADDRESS pGPUAddress)noexcept
			{
				this->mpGPUAddress = pGPUAddress;
			}

			//
			// GraphicsResourceBinder::RootSRV
			//

			PipelineResourceBinder::RootSRV::RootSRV()
				: IRootElement(IRootElement::eROOT_SRV)
			{
			}

			void PipelineResourceBinder::RootSRV::clear()
			{
				IRootElement::clear();
				this->mpGPUAddress = 0;
			}

			void PipelineResourceBinder::RootSRV::create(int rootIndex)
			{
				this->clear();
				this->mRootIndex = rootIndex;
			}

			void PipelineResourceBinder::RootSRV::bindForGraphics(ID3D12GraphicsCommandList* pCommandList)
			{
				assert(0 != this->mpGPUAddress);
				pCommandList->SetGraphicsRootShaderResourceView(this->mRootIndex, this->mpGPUAddress);
			}

			void PipelineResourceBinder::RootSRV::bindForCompute(ID3D12GraphicsCommandList* pCommandList)
			{
				assert(0 != this->mpGPUAddress);
				pCommandList->SetComputeRootShaderResourceView(this->mRootIndex, this->mpGPUAddress);
			}

			std::shared_ptr<PipelineResourceBinder::IRootElement> PipelineResourceBinder::RootSRV::clone()
			{
				auto sp = std::make_shared<PipelineResourceBinder::RootSRV>();
				sp->create(this->mRootIndex);
				return sp;
			}

			bool PipelineResourceBinder::RootSRV::validate()const noexcept
			{
				return 0 != this->mpGPUAddress;
			}

			void PipelineResourceBinder::RootSRV::updateGPUAddress(D3D12_GPU_VIRTUAL_ADDRESS pGPUAddress)noexcept
			{
				this->mpGPUAddress = pGPUAddress;
			}

			//
			// GraphicsResourceBinder::RootUAV
			//

			PipelineResourceBinder::RootUAV::RootUAV()
				: IRootElement(IRootElement::eROOT_UAV)
			{
			}

			void PipelineResourceBinder::RootUAV::clear()
			{
				IRootElement::clear();
				this->mpGPUAddress = 0;
			}

			void PipelineResourceBinder::RootUAV::create(int rootIndex)
			{
				this->clear();
				this->mRootIndex = rootIndex;
			}

			void PipelineResourceBinder::RootUAV::bindForGraphics(ID3D12GraphicsCommandList* pCommandList)
			{
				assert(0 != this->mpGPUAddress);
				pCommandList->SetGraphicsRootUnorderedAccessView(this->mRootIndex, this->mpGPUAddress);
			}

			void PipelineResourceBinder::RootUAV::bindForCompute(ID3D12GraphicsCommandList* pCommandList)
			{
				assert(0 != this->mpGPUAddress);
				pCommandList->SetComputeRootUnorderedAccessView(this->mRootIndex, this->mpGPUAddress);
			}

			std::shared_ptr<PipelineResourceBinder::IRootElement> PipelineResourceBinder::RootUAV::clone()
			{
				auto sp = std::make_shared<PipelineResourceBinder::RootUAV>();
				sp->create(this->mRootIndex);
				return sp;
			}

			bool PipelineResourceBinder::RootUAV::validate()const noexcept
			{
				return 0 != this->mpGPUAddress;
			}

			void PipelineResourceBinder::RootUAV::updateGPUAddress(D3D12_GPU_VIRTUAL_ADDRESS pGPUAddress)noexcept
			{
				this->mpGPUAddress = pGPUAddress;
			}

		}
	}
}