#pragma once

#pragma warning(disable: 4503)
//#define ASIO_STANDALONE // <ー プロジェクト設定で定義してます
#define _WEBSOCKETPP_CPP11_STL_

//Windows.hの後にインクルードするとエラーが出るので、前もってインクルードしている。
#include <winsock.h>
//このヘッダーをインクルードするときはWindows.hより先にインクルードしてください。
#include <cstdint>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
//送信データの圧縮を行うときに使うヘッダー zlibが必要
//#include <websocketpp/extensions/permessage_deflate/enabled.hpp>

#include <Windows.h>

namespace debug
{
	/// @brief デバッグコマンドを受信するサーバークラス
	///
	/// TODO サーバーを終了するとき接続中のクライアントへ通知していないので、気が向いたら作る
	class DebugCommandServer
	{
	public:
		//struct deflate_server_config : public websocketpp::config::asio {
		//	struct permessage_deflate_config { };
		//	typedef websocketpp::extensions::permessage_deflate::enabled<permessage_deflate_config> permessage_deflate_type;
		//};
		using server = websocketpp::server<websocketpp::config::asio>;
		using OnOpenPred = std::function<bool(server& mEndpoint, websocketpp::connection_hdl hdl)>;
		using OnMessagePred = std::function<bool(server& mEndpoint, websocketpp::connection_hdl hdl, server::message_ptr msg)>;
	public:
		DebugCommandServer();
		~DebugCommandServer();

		void start(uint16_t port, OnOpenPred onOpenPred = nullptr, OnMessagePred onMessagePred = nullptr);
		void end();

		void poll();

	private:
		void onOpen(websocketpp::connection_hdl hdl);
		void onMessage(websocketpp::connection_hdl hdl, server::message_ptr msg);

	private:
		server mEndpoint;
		OnOpenPred mOnOpenPred;
		OnMessagePred mOnMessagePred;
	};
}
