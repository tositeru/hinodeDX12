#include "stdafx.h"

#include "DXWindow.h"

#include <iostream>
#include <chrono>

#include "../network/JPEncode.h"

#include <graphics\dx12\common\Common.h>

using namespace hinode::graphics;

DXWindow::DXWindow()
	: mIsWaitPrevFrame(false)
{
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
}

DXWindow::~DXWindow()
{
	this->clear();
	if (this->mThread.joinable()) {
		this->mIsCloseThread = true;
		this->mThread.join();
	}
}

void DXWindow::clear()
{
	this->mGraphics.waitPrevFrame();

	this->mpScene.reset();
	this->mSceneInfo.clear();
	this->mGraphics.clear();
}

void DXWindow::create(const InitParam& initParam)
{
	Window::create(initParam);

	hinode::graphics::Log::sStandbyLogFile();

	DX12DeviceDesc deviceDesc;
	deviceDesc.minimumLevel = D3D_FEATURE_LEVEL_11_0;
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	auto swapChainDesc = DX12SwapChain::sMakeDesc(initParam.frameCount, initParam.windowWidth, initParam.windowHeight, this->HWnd(), true);
	this->mGraphics.init(&deviceDesc, queueDesc, &swapChainDesc);

	if (this->mSceneInfo.load("data/debug/scene.json")) {
		this->mSceneInfo.initResource(this->mGraphics.device());
	}
	this->mSceneInfoMediator.init(&this->mSceneInfo);

	this->mpScene = IScene::sBuild(IScene::sGetStartSceneNo());
	this->mpScene->init(this->mGraphics, this->mSceneInfo);
	std::cout << "0キーを押すとキー操作をコマンドプロンプトに表示します" << std::endl;

	this->mIsCloseThread = false;
	//this->mThread = std::thread(std::bind(&DXWindow::serverThread, this));

	this->show();
}

void DXWindow::serverThread()
{
	auto onOpen = std::bind(&DXWindow::onOpenPred, this, std::placeholders::_1, std::placeholders::_2);
	auto onMessage = std::bind(
		&DXWindow::onClientMessagePred, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3
	);
	this->mDebugCommandServer.start(9002, onOpen, onMessage);

	while (!this->mIsCloseThread) {
		this->mDebugCommandServer.poll();
		std::this_thread::sleep_for(std::chrono::milliseconds(16));
	}
}

bool DXWindow::onOpenPred(debug::DebugCommandServer::server& endpoint, websocketpp::connection_hdl hdl)
{
	std::unordered_map<std::string, std::vector<json11::Json>> jsonData;
	unless(this->mSceneInfo.objectHash().empty()) {
		auto& objects = jsonData["objects"];
		for (auto& it : this->mSceneInfo.objectHash()) {
			auto objJson = it.second.makeJson();
			objects.push_back(objJson);
		}
	}

	auto json = json11::Json(jsonData);
	auto msg = json.dump();
	endpoint.send(hdl, msg, websocketpp::frame::opcode::text);
	return true;
}

bool DXWindow::onClientMessagePred(debug::DebugCommandServer::server& endpoint, websocketpp::connection_hdl hdl, debug::DebugCommandServer::server::message_ptr msg)
{
	unless (websocketpp::frame::opcode::text == msg->get_opcode()) {
		return false;
	}
	auto json_str = msg->get_payload();
	std::string error;
	auto json = json11::Json::parse(json_str, error);
	unless (error.empty()) {
		auto msg = jp_encode::multi_to_utf8_cppapi("error: 送信されてきた文字列がjson形式ではありません.");
		endpoint.send(hdl, msg, websocketpp::frame::opcode::text);
		return true;
	}

	//jsonデータをSceneInfoMediatorで解析する
	auto lock = this->mSceneInfoMediator.getLock();

	std::string returnMessage = "";
	if (json["add_objects"].is_array()) {
		unless(lock) {
			lock.lock();
		}
		//追加するオブジェクトを確認する
		auto& items = json["add_objects"].array_items();
		for (auto& o : items) {
			auto result = this->mSceneInfoMediator.addObject(o);
			unless((result.isSuccess)) {
				returnMessage += result.message + "\n";
			}
		}
	}
	if (json["del_objects"].is_array()) {
		unless(lock) {
			lock.lock();
		}
		auto& items = json["del_objects"].array_items();
		for (auto& o : items) {
			this->mSceneInfoMediator.addDeleteElement(o.string_value(), scene::ELEMENT_TYPE::eOBJECT);
		}
	}
	if (json["update_objects"].is_array()) {
		unless(lock) {
			lock.lock();
		}
		auto& items = json["update_objects"].array_items();
		for (auto& o : items) {
			auto& name = o["name"];
			if (name.is_string()) {
				this->mSceneInfoMediator.addUpdateElement(name.string_value(), scene::ELEMENT_TYPE::eOBJECT, o);
			}
		}
	}

	if (lock) {
		lock.unlock();
	}

	if (returnMessage.empty()) {
		returnMessage = "成功";
	}

	endpoint.send(hdl, jp_encode::multi_to_utf8_cppapi(returnMessage), websocketpp::frame::opcode::text);
	return true;
}

LRESULT DXWindow::onResize(WPARAM wParam, UINT width, UINT height)
{
	return 0;
}

LRESULT DXWindow::onKeyDown(UINT8 key, const KeyInfo& keyInfo)
{
	if (static_cast<UINT8>(key) == VK_ESCAPE) {
		SendMessage(this->HWnd(), WM_DESTROY, 0, 0);
	}
	return 0;
}

LRESULT DXWindow::onKeyUp(UINT8 key, const KeyInfo& keyInfo)
{
	return 0;
}

void DXWindow::onPaint()
{
	//----------------------------------------------------------------------
	//	ここに描画処理を書いてください
	//

	if (this->mSceneInfoMediator.isExistData()) {
		this->mSceneInfoMediator.deliver();
	}

	this->mpScene->paint(this->mGraphics, this->mSceneInfo);

	//----------------------------------------------------------------------
}

//-------------------------------------------------------------------------------------------------------------------
//
//	DXWindow::InitParam
//
//-------------------------------------------------------------------------------------------------------------------


DXWindow::InitParam::InitParam()
	: hinode::winapi::Window::InitParam()
	, frameCount(2)
{}

DXWindow::InitParam::InitParam(LONG width, LONG height, const TCHAR* pTitle)noexcept
	: hinode::winapi::Window::InitParam(width, height, pTitle)
	, frameCount(2)
{}
