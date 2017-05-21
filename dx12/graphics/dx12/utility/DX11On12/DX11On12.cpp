#include "stdafx.h"

#include "DX11On12.h"

#include <wrl/client.h>

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			DX11On12::DX11On12()
			{ }

			DX11On12::~DX11On12()
			{
				this->clear();
			}

			void DX11On12::clear()noexcept
			{
				safeRelease(&this->mpDX11Device);
				safeRelease(&this->mpDX11DeviceContext);
				safeRelease(&this->mpWriteFactory);
				safeRelease(&this->mpD2dFactory);
				safeRelease(&this->mpD2dDevice);
				safeRelease(&this->mpD2dDeviceContext);
			}

			void DX11On12::create(ID3D12Device* pDevice, std::initializer_list<ID3D12CommandQueue*> pCmdQueues, UINT nodeMask)
			{
				this->clear();

				{
					UINT dx11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
					dx11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

					Microsoft::WRL::ComPtr<ID3D11Device> pDX11Device;
					auto hr = D3D11On12CreateDevice(
						pDevice,
						dx11DeviceFlags, nullptr, 0,
						reinterpret_cast<IUnknown*const*>(pCmdQueues.begin()), static_cast<UINT>(pCmdQueues.size()),
						nodeMask,
						&pDX11Device, &this->mpDX11DeviceContext, nullptr);
					unless(SUCCEEDED(hr)) {
						throw HINODE_GRAPHICS_CREATE_EXCEPTION(FailedToCreateException, "DX11On12", "create")
							<< "DX11Device�̍쐬�Ɏ��s";
					}

					hr = pDX11Device->QueryInterface(&this->mpDX11Device);
					unless(SUCCEEDED(hr)) {
						throw HINODE_GRAPHICS_CREATE_EXCEPTION(FailedToCreateException, "DX11On12", "create")
							<< "ID3D11On12Device�̃N�G���Ɏ��s";
					}

				}

				{
					D2D1_FACTORY_OPTIONS d2dFactoryOptions;
#if defined(_DEBUG)
					d2dFactoryOptions |= D2D1_DEBUG_LEVEL_INFORMATION;
#endif
					D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, d2dFactoryOptions,  &this->mpD2dFactory);

					Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
					auto hr = this->mpDX11Device->QueryInterface(dxgiDevice.GetAddressOf());
					unless(SUCCEEDED(hr)) {
						throw HINODE_GRAPHICS_CREATE_EXCEPTION(FailedToCreateException, "DX11On12", "create")
							<< "IDXGIDevice�̃N�G���Ɏ��s";
					}
					hr = this->mpD2dFactory->CreateDevice(dxgiDevice.Get(), &this->mpD2dDevice);
					unless (SUCCEEDED(hr)) {
						throw HINODE_GRAPHICS_CREATE_EXCEPTION(FailedToCreateException, "DX11On12", "create")
							<< "ID2D1Device2�̃N�G���Ɏ��s";
					}
					D2D1_DEVICE_CONTEXT_OPTIONS deviceOption = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
					hr = this->mpD2dDevice->CreateDeviceContext(deviceOption, &this->mpD2dDeviceContext);
					unless(SUCCEEDED(hr)) {
						throw HINODE_GRAPHICS_CREATE_EXCEPTION(FailedToCreateException, "DX11On12", "create")
							<< "ID2D1DeviceContext2�̃N�G���Ɏ��s";
					}

					hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&this->mpWriteFactory);
					unless(SUCCEEDED(hr)) {
						throw HINODE_GRAPHICS_CREATE_EXCEPTION(FailedToCreateException, "DX11On12", "create")
							<< "IDWriteFactory�̃N�G���Ɏ��s";
					}
				}
			}

		}
	}
}