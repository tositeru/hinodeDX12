#include "stdafx.h"

#include "DX12Device.h"

#include "../common/Log.h"
#include "../common/Exception.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

namespace hinode
{
	namespace graphics
	{
		class DX12Device::_impl : private ImplInterfacePointer<DX12Device>
		{
		public:
			UNCOPYABLE_IMPL_MOVE_CONSTRUCTOR(DX12Device);

			ID3D12Device* mpDevice;
			Microsoft::WRL::ComPtr<IDXGIAdapter1> mpAdapter;

			_impl(DX12Device* pInterface)
				: ImplInterfacePointer(pInterface)
				, mpDevice(nullptr)
				, mpAdapter(nullptr)
			{}

			_impl& operator=(_impl&& right)_noexcept
			{
				this->mpDevice = right.mpDevice;
				this->mpAdapter = right.mpAdapter;
				right.mpDevice = nullptr;
				right.mpAdapter = nullptr;
				return *this;
			}

			~_impl()_noexcept
			{
				this->clear();
			}

			void clear()_noexcept
			{
				safeRelease(&this->mpDevice);
				this->mpAdapter.Reset();
			}

			void create(DX12DeviceDesc& desc)
			{
				this->clear();

				auto hr = D3D12CreateDevice(desc.pAdapter.Get(), desc.minimumLevel, IID_PPV_ARGS(&this->mpDevice));
				if (FAILED(hr)) {
					throw HINODE_GRAPHICS_CREATE_EXCEPTION(InvalidArgumentsException, "DX12Device", "create")
						<< "ID3D12Deviceの作成に失敗しました";
				}
				this->mpAdapter = desc.pAdapter;
			}

			HRESULT setName(const wchar_t* name)
			{
				assert(nullptr != this->mpDevice || "まだ初期化されていません");
				return this->mpDevice->SetName(name);
			}

			bool isGood()const noexcept
			{
				return nullptr != this->mpDevice;
			}

		accessor_declaration:
			ID3D12Device* operator->()_noexcept
			{
				return this->device();
			}

			ID3D12Device* device()_noexcept
			{
				return this->mpDevice;
			}

			IDXGIAdapter1* adapter()_noexcept
			{
				return this->mpAdapter.Get();
			}

			HRESULT getDebugDevice(ID3D12DebugDevice** ppOut)_noexcept
			{
				assert(nullptr != this->mpDevice || "まだ初期化されていません");
				auto hr = this->mpDevice->QueryInterface(IID_PPV_ARGS(ppOut));
				if (FAILED(hr)) {
					Log(Log::eERROR) << "ID3D12DebugDeviceの作成に失敗";
				}
				return hr;
			}
		};

		//===========================================================================
		//
		//	IMPLイディオムのインターフェイスクラスの関数定義
		//

		void DX12Device::sCreateDevice(DX12Device* pOut, DX12DeviceDesc* pDesc, IDXGIFactory4* pDXGIFactory, bool isIgnoreSoftware)
		{
			Microsoft::WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
			for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pDXGIFactory->EnumAdapters1(adapterIndex, &hardwareAdapter); ++adapterIndex) {
				DXGI_ADAPTER_DESC1 desc;
				hardwareAdapter->GetDesc1(&desc);

				if (isIgnoreSoftware && (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) {
					continue;
				}

				auto hr = D3D12CreateDevice(hardwareAdapter.Get(), pDesc->minimumLevel, _uuidof(ID3D12Device), nullptr);
				if (SUCCEEDED(hr)) {
					break;
				}
			}

			pDesc->pAdapter = hardwareAdapter;
			pOut->create(*pDesc);
		}

		UNCOPYABLE_PIMPL_IDIOM_CPP_TEMPLATE(DX12Device);

		void DX12Device::clear()_noexcept
		{
			this->impl().clear();
		}

		void DX12Device::create(DX12DeviceDesc& desc)
		{
			this->impl().create(desc);
		}

		HRESULT DX12Device::setName(const wchar_t* name)_noexcept
		{
			return this->impl().setName(name);
		}

		bool DX12Device::isGood()const noexcept
		{
			return this->impl().isGood();
		}

		ID3D12Device* DX12Device::operator->()_noexcept
		{
			return this->impl().device();
		}

		ID3D12Device* DX12Device::device()_noexcept
		{
			return this->impl().device();
		}

		IDXGIAdapter1* DX12Device::adapter()_noexcept
		{
			return this->impl().adapter();
		}

		HRESULT DX12Device::getDebugDevice(ID3D12DebugDevice** ppOut)_noexcept
		{
			return this->impl().getDebugDevice(ppOut);
		}
	}
}