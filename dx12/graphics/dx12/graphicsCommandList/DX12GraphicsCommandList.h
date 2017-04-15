#pragma once

#include <array>

#include "../common/Common.h"
#include "../IDX12Interface.h"

namespace hinode
{
	namespace graphics
	{
		struct DX12GraphicsCommandListDesc;

		/// @brief DX12GraphicsCommandListを内部に持つクラス
		class DX12GraphicsCommandList : public IDX12Interface
		{
			UNCOPYABLE_PIMPL_IDIOM_TEMPLATE(DX12GraphicsCommandList)
		public:
			/// @brief メモリ開放
			void clear()_noexcept;

			/// @brief コマンドキューを作成する
			///
			///	作成に成功した場合は中でID3D12GraphicsCommandList::Close()を呼び出してます。
			/// @param[in] pDevice
			/// @param[in] desc
			/// @exception hinode::graphics::InvalidArgumentsException
			void create(ID3D12Device* pDevice, DX12GraphicsCommandListDesc& desc);

			/// @brief 記録を開始する
			/// @param[in] pAllocator
			/// @param[in] pState
			bool reset(ID3D12CommandAllocator* pAllocator, ID3D12PipelineState* pState)noexcept;

			/// @brief 記録を終了する
			bool close()noexcept;

			/// @brief コマンドを記録する
			///
			/// この関数の初めにDX12GraphicsCommandListDesc::Resetが、終わりにDX12GraphicsCommandListDesc::Closeが呼びだされます。
			/// @param[in] pAllocator
			/// @param[in] pState
			/// @param[in] pred
			bool record(ID3D12CommandAllocator* pAllocator, ID3D12PipelineState* pState, std::function<void(DX12GraphicsCommandList& cmdList)> pred);

			/// @brief ID3D12DescriptorHeapを設定する
			/// @param[in] std::initializer_list<ID3D12DescriptorHeap*>
			void setDescriptorHeaps(std::initializer_list<ID3D12DescriptorHeap*> heaps);

			/// @brief リソースのバリアーを設定する
			///
			/// 同じステート間の遷移を見つけた場合はそのバリアーは設定されません
			/// @tparam[in] Args
			/// @parma[in] args const D3D12_RESOURCE_BARRIER&を想定しています
			template<typename... Args> void setBarriers(Args&... args)
			{
				std::array<D3D12_RESOURCE_BARRIER, sizeof...(Args)> barriers;
				UINT count = 0;

				setBarriersPredication<decltype(barriers)> pred;
				pred(barriers, count, args...);
				if (1 <= count) {
					this->cmdList()->ResourceBarrier(count, barriers.data());
				}
			}

			/// @brief 内部のDX12CommandQueueに名前をつける
			/// @param[in] const wchar_t* name
			/// @retval HRESULT
			HRESULT setName(const wchar_t* name) _noexcept override;

			/// @brief 使用できる状態か?
			/// @retval bool
			bool isGood()const noexcept override;

		accessor_declaration:
			ID3D12GraphicsCommandList* operator->()_noexcept;

			ID3D12GraphicsCommandList* cmdList()_noexcept;
			operator ID3D12GraphicsCommandList*()_noexcept { return this->cmdList(); }

		private:
			template<typename ArrayType> struct setBarriersPredication {
				template<typename... Args>
				void operator()(ArrayType& outBarriers, UINT& outCounts, const D3D12_RESOURCE_BARRIER& first, Args&... args)
				{
					if (D3D12_RESOURCE_BARRIER_TYPE_TRANSITION == first.Type) {
						unless(first.Transition.StateBefore == first.Transition.StateAfter) {
							outBarriers[outCounts] = first;
							++outCounts;
						}
					} else {
						outBarriers[outCounts] = first;
						++outCounts;
					}
					return (*this)(outBarriers, outCounts, args...);
				}
				void operator()(ArrayType& outBarriers, UINT& outCounts)
				{}
			};

		};
	}
	namespace graphics
	{
		struct DX12GraphicsCommandListDesc
		{
			UINT node = 0;
			D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			ID3D12CommandAllocator* pAllocator = nullptr;
			ID3D12PipelineState* pPipelineState = nullptr;

			DX12GraphicsCommandListDesc() = default;
			DX12GraphicsCommandListDesc(UINT node, D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator* pAllocator, ID3D12PipelineState* pPipelineState)noexcept;
		};
	}
}