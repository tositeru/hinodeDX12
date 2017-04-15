#pragma once

#include <vector>

#include "../common/Common.h"
#include "../IDX12Interface.h"

namespace hinode
{
	namespace graphics
	{
		struct DX12ResourceHeapDesc;
		struct DX12ResourceDesc;

		class DX12DescriptorHeap;

		/// @brief ID3D12Resourceを内部に持つクラス
		class DX12Resource : public IDX12Interface
		{
			UNCOPYABLE_PIMPL_IDIOM_TEMPLATE(DX12Resource)
		public:
			/// @brief リソースのビューをまとめたもの
			class View {
				friend DX12Resource;
			public:
				enum TYPE {
					eUNKNONW,
					eVERTEX,
					eINDEX,
					eSTREAM_OUTPUT,
					eSRV,
					eUAV,
					eRTV,
					eDSV,
				};

			public:
				/// @brief コンストラクタ
				View()noexcept;

				/// @brief 頂点バッファビューの設定
				/// @param[in] strideInBytes
				/// @param[in] sizeInBytes
				/// @retval View&
				View& setVertex(UINT strideInBytes, UINT sizeInBytes)noexcept;
				/// @brief インデックスバッファビューの設定
				/// @param[in] format
				/// @param[in] sizeInBytes
				/// @retval View&
				View& setIndex(DXGI_FORMAT format, UINT sizeInBytes)noexcept;
				/// @brief ストリームアウトプットビューの設定
				/// @param[in] sizeInBytes
				/// @param[in] filledSizeLocation
				/// @retval View&
				View& setStreamOutput(UINT sizeInBytes, D3D12_GPU_VIRTUAL_ADDRESS filledSizeLocation)noexcept;
				/// @brief SRVの設定
				/// @param[in] srv
				/// @retval View&
				View& setSRV(D3D12_SHADER_RESOURCE_VIEW_DESC& srv)noexcept;
				/// @brief UAVの設定
				/// @param[in] uav
				/// @retval View&
				View& setUAV(D3D12_UNORDERED_ACCESS_VIEW_DESC& uav)noexcept;
				/// @brief RTVの設定
				/// @param[in] uav
				/// @retval View&
				View& setRTV(D3D12_RENDER_TARGET_VIEW_DESC& rtv)noexcept;
				/// @brief DSVの設定
				/// @param[in] uav
				/// @retval View&
				View& setDSV(D3D12_DEPTH_STENCIL_VIEW_DESC& dsv)noexcept;

			accessor_declaration:
				TYPE type()const noexcept;
				const D3D12_VERTEX_BUFFER_VIEW& vertex()const noexcept;
				const D3D12_INDEX_BUFFER_VIEW& index()const noexcept;
				const D3D12_STREAM_OUTPUT_BUFFER_VIEW& streamOutput()const noexcept;
				const D3D12_SHADER_RESOURCE_VIEW_DESC& srv()const noexcept;
				const D3D12_UNORDERED_ACCESS_VIEW_DESC& uav()const noexcept;
				const D3D12_RENDER_TARGET_VIEW_DESC& rtv()const noexcept;
				const D3D12_DEPTH_STENCIL_VIEW_DESC& dsv()const noexcept;

			private:
				TYPE mType = eUNKNONW;
				union
				{
					D3D12_VERTEX_BUFFER_VIEW mVertexView;
					D3D12_INDEX_BUFFER_VIEW mIndexView;
					D3D12_STREAM_OUTPUT_BUFFER_VIEW mStreamOutputView;
					D3D12_SHADER_RESOURCE_VIEW_DESC mSRV;
					D3D12_UNORDERED_ACCESS_VIEW_DESC mUAV;
					D3D12_RENDER_TARGET_VIEW_DESC mRTV;
					D3D12_DEPTH_STENCIL_VIEW_DESC mDSV;
				};
			};

			/// @brief ビューを作成するときに使用するパラメータ
			struct MakeViewDesc
			{
				bool isCubemap;

				bool isRawBuffer = false;		///< Bufferのときに使う
				UINT elementCount = 0;			///< Bufferのときに使う
				UINT structureByteStride = 0;	///< Bufferのときに使う

				MakeViewDesc()noexcept;

				MakeViewDesc& setTexture2D(bool isCubemap)noexcept;
			};
		public:
			static UINT16 sCalMipLevel(UINT16 w)noexcept;
			static UINT16 sCalMipLevel(UINT16 w, UINT16 h)noexcept;
			static UINT16 sCalMipLevel(UINT16 w, UINT16 h, UINT16 d)noexcept;

		public:
			/// @brief メモリ開放
			void clear()noexcept;

			/// @brief ID3D12Resourceを作成する
			///
			/// @param[in] pDevice
			/// @param[in] pHeapProperties
			/// @param[in] heapFlags
			/// @param[in] pResourceDesc
			/// @param[in] resourceStates
			/// @param[in, optional] pClearValue
			/// @exception hinode::graphics::FailedToCreateException
			void create(
				ID3D12Device* pDevice,
				const D3D12_HEAP_PROPERTIES *pHeapProperties,
				D3D12_HEAP_FLAGS heapFlags,
				const D3D12_RESOURCE_DESC* pResourceDesc,
				D3D12_RESOURCE_STATES initialStates,
				const D3D12_CLEAR_VALUE* pClearValue);

			/// @brief ID3D12Resourceを作成する
			///
			/// @param[in] pDevice
			/// @param[in] heapDesc
			/// @param[in] resourceDesc
			/// @exception hinode::graphics::FailedToCreateException
			void create(
				ID3D12Device* pDevice,
				const DX12ResourceHeapDesc& heapDesc,
				const DX12ResourceDesc& resourceDesc);

			/// @brief Placed Resourceを作成する
			///
			/// @param[in] pDevice
			/// @param[in] pHeap
			/// @param[in] heapOffset
			/// @param[in] pResourceDesc
			/// @param[in] resourceStates
			/// @param[in, optional] pClearValue
			/// @exception hinode::graphics::FailedToCreateException
			void create(
				ID3D12Device* pDevice,
				ID3D12Heap* pHeap,
				UINT heapOffset,
				const D3D12_RESOURCE_DESC* pResourceDesc,
				D3D12_RESOURCE_STATES initialStates,
				const D3D12_CLEAR_VALUE* pClearValue);

			/// @brief Placed Resourceを作成する
			///
			/// @param[in] pDevice
			/// @param[in] pHeap
			/// @param[in] heapOffset
			/// @param[in] resourceDesc
			/// @exception hinode::graphics::FailedToCreateException
			void create(
				ID3D12Device* pDevice,
				ID3D12Heap* pHeap,
				UINT heapOffset,
				const DX12ResourceDesc& resourceDesc);

			/// @brief 指定されたID3D12Resourceの所有権をもらう
			/// 
			/// @param[in] pTransferredResource
			/// @param[in] state pSrcの現在の状態
			/// @param[in] isMove default=true もらった後、pSrcをリリースするか？
			void create(ID3D12Resource* pTransferredResource, D3D12_RESOURCE_STATES state, bool isMove=true);

			/// @brief コピー用のリソースを作成する
			///
			/// この関数で作成したリソースはバッファ型として作成されます。
			/// @param[in] pDevice
			/// @param[in] target
			/// @param[in] pData
			/// @param[in] initDataByteSize
			/// @exception hinode::graphics::FailedToCreateException
			void createForCopy(ID3D12Device* pDevice, DX12Resource& target, const void* pInitData, UINT64 initDataByteSize);

			/// @brief スワップチェインのバッファを設定する
			/// @param[in] pSwapChain
			/// @param[in] index
			/// @retval HRESULT
			HRESULT setSwapChianBuffer(IDXGISwapChain* pSwapChain, UINT index)noexcept;

			/// @brief ビューを追加する
			/// @param[in] view
			void appendView(const View& view)noexcept;

			/// @brief データの更新を行う
			/// @param[in] subresourceIndex
			/// @param[in] pRange
			/// @param[in] predicate trueを返した場合はリソース全体を書き換えたものだとみなします。
			/// @retval HRESULT
			HRESULT update(UINT subresourceIndex, const D3D12_RANGE* pRange, std::function<bool(const D3D12_RANGE* pRange, void* pData, D3D12_RANGE* pWritedRange)> predicate)noexcept;

			/// @brief 指定した内容でD3D12_RESOURCE_BARRIERを作る
			/// @param[in] before
			/// @param[in] after
			/// @param[in] subresource defalut=D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES
			/// @param[in] flags default=D3D12_RESOURCE_BARRIER_FLAG_NONE
			/// @retval D3D12_RESOURCE_BARRIER
			D3D12_RESOURCE_BARRIER makeBarrier(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);

			/// @brief 指定した内容でD3D12_RESOURCE_BARRIERを作る
			///
			/// この関数はcurrentState関数で得られる値を更新します
			/// currentState関数で得られる値がnextで指定した値と同じ場合でもD3D12_RESOURCE_BARRIERを作成します
			/// @param[in] next
			/// @param[in] subresource defalut=D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES
			/// @param[in] flags default=D3D12_RESOURCE_BARRIER_FLAG_NONE
			/// @retval D3D12_RESOURCE_BARRIER
			D3D12_RESOURCE_BARRIER makeBarrier(D3D12_RESOURCE_STATES next, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);

			/// @brief リソースの内容をもとにシェーダリソースビューを作成する
			/// @retval D3D12_SHADER_RESOURCE_VIEW_DESC
			D3D12_SHADER_RESOURCE_VIEW_DESC makeSRV(const MakeViewDesc& viewDesc = MakeViewDesc())noexcept;

			/// @brief リソースの内容をもとにアンオーダードアクセスビューを作成する
			/// @retval D3D12_UNORDERED_ACCESS_VIEW_DESC
			D3D12_UNORDERED_ACCESS_VIEW_DESC makeUAV(const MakeViewDesc& viewDesc = MakeViewDesc())noexcept;

			/// @brief リソースの内容をもとにレンダーターゲットビューを作成する
			/// @retval D3D12_RENDER_TARGET_VIEW_DESC
			D3D12_RENDER_TARGET_VIEW_DESC makeRTV(const MakeViewDesc& viewDesc = MakeViewDesc())noexcept;

			/// @brief リソースの内容をもとにデプスステンシルビューを作成する
			/// @retval D3D12_DEPTH_STENCIL_VIEW_DESC
			D3D12_DEPTH_STENCIL_VIEW_DESC makeDSV(const MakeViewDesc& viewDesc = MakeViewDesc())noexcept;

			/// @brief クリアー用のdescriptor heapを作成する
			/// @param[in] type
			void createDescriptorHeapForClear(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT nodeMask=0u);

			/// @brief 内部のID3D12Resourceに名前をつける
			/// @param[in] const wchar_t* name
			/// @retval HRESULT
			HRESULT setName(const wchar_t* name) noexcept override;

			/// @brief 使用できる状態か?
			/// @retval bool
			bool isGood()const noexcept override;

		accessor_declaration:
			ID3D12Resource* operator->()noexcept;

			ID3D12Resource* resource()noexcept;
			operator ID3D12Resource* ()noexcept { return this->resource(); }
			ID3D12Resource** resourcePointer()noexcept;
			const View& view(int index=0)const;
			D3D12_RESOURCE_STATES currentState()const noexcept;
			const D3D12_CLEAR_VALUE& clearValue()const noexcept;
			DX12DescriptorHeap& descriptorHeapForClear()noexcept;

			/// @brief 関連しているID3D12Deviceを取得する
			/// @retval ID3D12Device*
			ID3D12Device* getDevice()noexcept;

			/// @brief 指定したサブリソースのD3D12_PLACED_SUBRESOURCE_FOOTPRINTを取得する
			/// @param[in] subresourceIndex
			/// @param[in] baseOffset
			/// @retval D3D12_PLACED_SUBRESOURCE_FOOTPRINT
			/// @param[out option] pOutTotalByteSize (default=nullptr) サブリソースのバイトサイズ
			D3D12_PLACED_SUBRESOURCE_FOOTPRINT getCopyableFootprint(UINT subresourceIndex, UINT64 baseOffset, UINT64* pOutTotalByteSize=nullptr)noexcept;

			/// @brief 指定したサブリソースのD3D12_PLACED_SUBRESOURCE_FOOTPRINTを取得する
			/// @param[in] firstSubresource
			/// @param[in] subresourceCount
			/// @param[in] baseOffset
			/// @retval D3D12_PLACED_SUBRESOURCE_FOOTPRINT
			/// @param[out option] pOutTotalByteSize (default=nullptr) サブリソースのバイトサイズ
			std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> getCopyableFootprints(UINT firstSubresource, UINT subresourceCount, UINT64 baseOffset, UINT64* pOutTotalByteSize = nullptr)noexcept;
		};
	}

	namespace graphics
	{
		struct DX12ResourceHeapDesc
		{
			CD3DX12_HEAP_PROPERTIES properties;
			D3D12_HEAP_FLAGS flags;

			DX12ResourceHeapDesc();
			DX12ResourceHeapDesc(D3D12_HEAP_TYPE heapType, D3D12_HEAP_FLAGS heapFlags = D3D12_HEAP_FLAG_NONE);
		};

		struct DX12ResourceDesc
		{
			CD3DX12_RESOURCE_DESC desc;
			D3D12_RESOURCE_STATES initialStates;
			CD3DX12_CLEAR_VALUE clearValue;	//クリアー関数で指定する値。この値以外でクリアーすると遅くなると警告が出る

			DX12ResourceDesc();

			/// @brief clearValueの値を設定する
			/// @param[in] format
			/// @param[in] r
			/// @param[in] g
			/// @param[in] b
			/// @param[in] a
			void setClearValueForColor(DXGI_FORMAT format, float r, float g, float b, float a)noexcept;

			/// @brief clearValueの値を設定する
			/// @param[in] format
			/// @param[in] r
			/// @param[in] g
			/// @param[in] b
			/// @param[in] a
			void setClearValueForDepth(DXGI_FORMAT format, float depth, UINT8 stencil)noexcept;

			/// @brief 作成時にD3D12_CLEAR_VALUEを使用するかどうか?
			/// @retval bool
			bool enableClearValue()const noexcept;
		};
	}
}
