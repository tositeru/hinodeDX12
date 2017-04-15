#pragma once

#include <graphics\dx12\utility\winapi\Window.h>
#include <graphics\dx12\utility\DX12Utility.h>

#include "../scene/IScene.h"
#include "../network/DebugCommandServer.h"
#include "../scene/SceneInfo/SceneInfo.h"
#include "../scene/SceneInfo/SceneInfoMediator.h"

class DXWindow : public hinode::winapi::Window
{
public:
	struct InitParam : public hinode::winapi::Window::InitParam
	{
		UINT frameCount;

		InitParam();
		InitParam(LONG width, LONG height, const TCHAR* pTitle)noexcept;
	};

public:
	DXWindow();

	virtual ~DXWindow();

	void clear();
	void create(const InitParam& initParam);

	virtual LRESULT onResize(WPARAM wParam, UINT width, UINT height)override;

	virtual LRESULT onKeyDown(UINT8 key, const KeyInfo& keyInfo)override;

	virtual LRESULT onKeyUp(UINT8 key, const KeyInfo& keyInfo)override;

	virtual void onPaint()override;

private:
	void serverThread();

	bool onOpenPred(debug::DebugCommandServer::server& endpoint, websocketpp::connection_hdl hdl);
	bool onClientMessagePred(debug::DebugCommandServer::server& endpoint, websocketpp::connection_hdl hdl, debug::DebugCommandServer::server::message_ptr msg);

private:
	hinode::graphics::utility::Graphics mGraphics;

	std::unique_ptr<IScene> mpScene;
	bool mIsWaitPrevFrame;

	std::thread mThread;
	debug::DebugCommandServer mDebugCommandServer;
	bool mIsCloseThread;

	scene::SceneInfo mSceneInfo;
	scene::SceneInfoMediator mSceneInfoMediator;
};

