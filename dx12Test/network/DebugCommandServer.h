#pragma once

#pragma warning(disable: 4503)
//#define ASIO_STANDALONE // <�[ �v���W�F�N�g�ݒ�Œ�`���Ă܂�
#define _WEBSOCKETPP_CPP11_STL_

//Windows.h�̌�ɃC���N���[�h����ƃG���[���o��̂ŁA�O�����ăC���N���[�h���Ă���B
#include <winsock.h>
//���̃w�b�_�[���C���N���[�h����Ƃ���Windows.h����ɃC���N���[�h���Ă��������B
#include <cstdint>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
//���M�f�[�^�̈��k���s���Ƃ��Ɏg���w�b�_�[ zlib���K�v
//#include <websocketpp/extensions/permessage_deflate/enabled.hpp>

#include <Windows.h>

namespace debug
{
	/// @brief �f�o�b�O�R�}���h����M����T�[�o�[�N���X
	///
	/// TODO �T�[�o�[���I������Ƃ��ڑ����̃N���C�A���g�֒ʒm���Ă��Ȃ��̂ŁA�C������������
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
