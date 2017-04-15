#include <stdafx.h>

#include "DebugCommandServer.h"

#include <functional>

#include <json11/json11.hpp>

#include <graphics/dx12/common/Log.h>

#include "JPEncode.h"

namespace debug
{
	DebugCommandServer::DebugCommandServer()
	{
		this->mEndpoint.set_error_channels(websocketpp::log::elevel::all);
		this->mEndpoint.set_access_channels(websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload);

		this->mEndpoint.init_asio();

		this->mEndpoint.set_open_handler(std::bind(&DebugCommandServer::onOpen, this, std::placeholders::_1));
		this->mEndpoint.set_message_handler(std::bind(
			&DebugCommandServer::onMessage, this,
			std::placeholders::_1, std::placeholders::_2
		));
	}

	DebugCommandServer::~DebugCommandServer()
	{
	}

	void DebugCommandServer::start(uint16_t port, OnOpenPred onOpenPred, OnMessagePred predMessage)
	{
		if (onOpenPred) {
			this->mOnOpenPred = onOpenPred;
		}
		if (predMessage) {
			this->mOnMessagePred = predMessage;
		}

		this->mEndpoint.listen(port);
		this->mEndpoint.start_accept();
	}

	void DebugCommandServer::end()
	{
		//TODO デバッグ目的のためいい加減な処理なので、あとで終了処理を作る. 
	}

	void DebugCommandServer::onOpen(websocketpp::connection_hdl hdl)
	{
		unless(this->mOnOpenPred(this->mEndpoint, hdl)) {
			//適当にメッセージを送る
			auto msg = jp_encode::multi_to_utf8_cppapi("接続しました。");
			this->mEndpoint.send(hdl, msg, websocketpp::frame::opcode::text);
		}
	}

	void DebugCommandServer::onMessage(websocketpp::connection_hdl hdl, server::message_ptr msg)
	{
		unless (this->mOnMessagePred(this->mEndpoint, hdl, msg)) {
			//とりあえず、echoサーバーにしておく
			this->mEndpoint.send(hdl, msg->get_payload(), msg->get_opcode());
		}
	}

	void DebugCommandServer::poll()
	{
		this->mEndpoint.poll();
	}

}