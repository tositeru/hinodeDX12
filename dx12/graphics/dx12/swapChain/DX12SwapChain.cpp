#include "stdafx.h"

#include "DX12SwapChain.h"

#include "../common/Log.h"
#include "../common/Exception.h"

namespace hinode
{
	namespace graphics
	{
		class DX12SwapChain::_impl : private ImplInterfacePointer<DX12SwapChain>
		{
		public:
			UNCOPYABLE_IMPL_MOVE_CONSTRUCTOR(DX12SwapChain);

			IDXGISwapChain3* mpSwapChain;
			UINT mBufferCount;
			_impl(DX12SwapChain* pInterface)
				: ImplInterfacePointer(pInterface)
				, mpSwapChain(nullptr)
				, mBufferCount(0)
			{}

			_impl& operator=(_impl&& right)_noexcept
			{
				this->mpSwapChain = right.mpSwapChain;
				this->mBufferCount = right.mBufferCount;
				right.mpSwapChain = nullptr;
				right.mBufferCount = 0;
				return *this;
			}

			~_impl()_noexcept
			{
				this->clear();
			}

			void clear()_noexcept
			{
				safeRelease(&this->mpSwapChain);
				this->mBufferCount = 0;
			}

			void create(ID3D12CommandQueue* pQueue, IDXGIFactory4* pDXGIFactory, DXGI_SWAP_CHAIN_DESC& desc)
			{
				this->clear();

				Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
				auto hr = pDXGIFactory->CreateSwapChain(pQueue, &desc, &swapChain);
				if (FAILED(hr)) {
					throw HINODE_GRAPHICS_CREATE_EXCEPTION(InvalidArgumentsException, "DX12SwapChain", "create")
						<< "IDXGISwapChainの作成に失敗しました";
				}
				hr = swapChain.Get()->QueryInterface(IID_PPV_ARGS(&this->mpSwapChain));
				if (FAILED(hr)) {
					throw HINODE_GRAPHICS_CREATE_EXCEPTION(InvalidArgumentsException, "DX12SwapChain", "create")
						<< "IDXGISwapChain3インターフェイス取得に失敗しました";
				}
				this->mBufferCount = desc.BufferCount;
			}

			HRESULT setName(const wchar_t* name)
			{
				return S_OK;
			}

			bool isGood()const noexcept
			{
				return nullptr != this->mpSwapChain;
			}

		accessor_declaration:
			IDXGISwapChain3* operator->()_noexcept
			{
				return this->swapChain();
			}

			IDXGISwapChain3* swapChain()_noexcept
			{
				return this->mpSwapChain;
			}

			UINT bufferCount()const _noexcept
			{
				return this->mBufferCount;
			}

			UINT width()const noexcept
			{
				assert(this->isGood() && "初期化されていません。");
				DXGI_SWAP_CHAIN_DESC desc;
				this->mpSwapChain->GetDesc(&desc);
				return desc.BufferDesc.Width;
			}

			UINT height()const noexcept
			{
				assert(this->isGood() && "初期化されていません。");
				DXGI_SWAP_CHAIN_DESC desc;
				this->mpSwapChain->GetDesc(&desc);
				return desc.BufferDesc.Height;
			}
		};

		//===========================================================================
		//
		//	IMPLイディオムのインターフェイスクラスの関数定義
		//

		DXGI_SWAP_CHAIN_DESC DX12SwapChain::sMakeDesc(UINT frameCount, UINT width, UINT height, HWND hWnd, bool isWindow)
		{
			DXGI_SWAP_CHAIN_DESC result = {};
			result.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			result.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			result.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			result.SampleDesc.Count = 1;

			result.BufferCount = frameCount;
			result.BufferDesc.Width = width;
			result.BufferDesc.Height = height;
			result.OutputWindow = hWnd;
			result.Windowed = isWindow;
			return result;
		}

		UNCOPYABLE_PIMPL_IDIOM_CPP_TEMPLATE(DX12SwapChain);

		void DX12SwapChain::clear()_noexcept
		{
			this->impl().clear();
		}

		void DX12SwapChain::create(ID3D12CommandQueue* pQueue, IDXGIFactory4* pDXGIFactory, DXGI_SWAP_CHAIN_DESC& desc)
		{
			this->impl().create(pQueue, pDXGIFactory, desc);
		}

		HRESULT DX12SwapChain::setName(const wchar_t* name) _noexcept
		{
			return this->impl().setName(name);
		}

		bool DX12SwapChain::isGood()const noexcept
		{
			return this->impl().isGood();
		}

		IDXGISwapChain3* DX12SwapChain::operator->()_noexcept
		{
			return this->impl().swapChain();
		}

		IDXGISwapChain3* DX12SwapChain::swapChain()_noexcept
		{
			return this->impl().swapChain();
		}

		UINT DX12SwapChain::bufferCount()const _noexcept
		{
			return this->impl().bufferCount();
		}

		UINT DX12SwapChain::width()const noexcept
		{
			return this->impl().width();
		}

		UINT DX12SwapChain::height()const noexcept
		{
			return this->impl().height();
		}

	}
}