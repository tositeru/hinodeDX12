#include "stdafx.h"

#include "Window.h"

#include <iostream>
#include <tchar.h>
#include "../../common/Common.h"

using namespace hinode::graphics;
using namespace std;

namespace hinode
{
	namespace winapi
	{
		LRESULT CALLBACK Window::sWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			Window* pWindow = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
			try {

				switch (message)
				{
				case WM_CREATE: {
					// Save the DXSample* passed in to CreateWindow.
					LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
					SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
					return 0;
				}
				case WM_SIZE:
					return pWindow->onResize(wParam, static_cast<UINT>(LOWORD(lParam)), static_cast<UINT>(HIWORD(lParam)));
				case WM_KEYDOWN: {
					KeyInfo keyInfo;
					keyInfo.set(lParam);
					return pWindow->onKeyDown(static_cast<UINT8>(wParam), keyInfo);
				}
				case WM_KEYUP: {
					KeyInfo keyInfo;
					keyInfo.set(lParam);
					return pWindow->onKeyUp(static_cast<UINT8>(wParam), keyInfo);
				}
				case WM_PAINT:
					pWindow->onPaint();
					return 0;

				case WM_DESTROY:
					PostQuitMessage(0);
					return 0;
				}

				// Handle any messages the switch statement didn't.
				return DefWindowProc(hWnd, message, wParam, lParam);
			} catch (hinode::graphics::Exception& e) {
				e.writeLog();
				abort();
			} catch (...) {
				cerr << "不明の例外が発生しました。終了します。" << endl;
				abort();
			}
		}

		void Window::sDisableConsoleCloseButton()noexcept
		{
			HMENU hmenu = GetSystemMenu(GetConsoleWindow(), FALSE);
			RemoveMenu(hmenu, SC_CLOSE, MF_BYCOMMAND);
		}

		Window::Window()
			: mHWnd(NULL)
		{}

		Window::~Window()
		{

		}

		void Window::create(const InitParam& initParam)
		{
			WNDCLASSEX wcex;
			wcex.lpfnWndProc = (initParam.pWindowProc) ? initParam.pWindowProc : Window::sWndProc;
			wcex.hInstance = GetModuleHandleW(NULL);
			wcex.lpszClassName = _T("winapi::Window");
			wcex.cbSize = sizeof(WNDCLASSEX);
			wcex.style = CS_HREDRAW | CS_VREDRAW;
			wcex.cbClsExtra = 0;
			wcex.cbWndExtra = 0;
			wcex.hIcon = LoadIcon(wcex.hInstance, (LPCTSTR)107);
			wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
			wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
			wcex.lpszMenuName = NULL;
			wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)107);

			if (!RegisterClassEx(&wcex)) {
				throw HINODE_GRAPHICS_CREATE_EXCEPTION(FailedToCreateException, "Window", "create") << "ウィンドウ作成に失敗しました。";
			}

			// ウィンドウ作成
			RECT rc = { 0, 0, initParam.windowWidth, initParam.windowHeight };
			AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

			DWORD style = WS_OVERLAPPEDWINDOW;
			this->mHWnd = CreateWindowW(wcex.lpszClassName, initParam.pWindowTitle, style,
				CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, wcex.hInstance,
				this);
			if (!this->mHWnd) {
				throw HINODE_GRAPHICS_CREATE_EXCEPTION(FailedToCreateException, "Window", "create") << "ウィンドウ作成に失敗しました。";
			}
		}

		void Window::show(int showCommad)
		{
			::ShowWindow(this->mHWnd, showCommad);
		}

		int Window::mainLoop()
		{
			MSG msg = { 0 };
			//Main message loop
			while (WM_QUIT != msg.message)
			{
				if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
			return static_cast<int>(msg.wParam);
		}

		LRESULT Window::onResize(WPARAM wParam, UINT width, UINT height)
		{
			return 0;
		}

		LRESULT Window::onKeyDown(UINT8 key, const KeyInfo& keyInfo)
		{
			return 0;
		}

		LRESULT Window::onKeyUp(UINT8 key, const KeyInfo& keyInfo)
		{
			return 0;
		}

		void Window::onPaint()
		{
		}

		HWND Window::HWnd()noexcept
		{
			return this->mHWnd;
		}


		Window::InitParam::InitParam()noexcept
			: InitParam(1280, 720, L"winapi::Window")
		{}

		Window::InitParam::InitParam(LONG width, LONG height, const TCHAR* pTitle)noexcept
			: windowWidth(width)
			, windowHeight(height)
			, pWindowTitle(pTitle)
			, pWindowProc(NULL)
		{}

		Window::KeyInfo& Window::KeyInfo::set(LPARAM lParam)noexcept
		{
			this->repeatCount = lParam & 0xffff;
			this->scanCode = (lParam >> 16) & 0x7f;
			this->isExtendeKey = (0 != ((lParam >> 24) & 0x1));
			this->contextCode = (lParam >> 29) & 0x1;
			this->prevKeyState = (lParam >> 30) & 0x1;
			this->transitionState = (lParam >> 31) & 0x1;
			return *this;
		}
	}
}