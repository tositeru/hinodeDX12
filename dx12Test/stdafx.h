// stdafx.h : �W���̃V�X�e�� �C���N���[�h �t�@�C���̃C���N���[�h �t�@�C���A�܂���
// �Q�Ɖ񐔂������A�����܂�ύX����Ȃ��A�v���W�F�N�g��p�̃C���N���[�h �t�@�C��
// ���L�q���܂��B
//

#pragma once

//websocketpp��asio�𓱓��������ƂŃC���N���[�h���ɂ����WINAPI���݂̍Ē�`�ȂǃG���[���o��悤�ɂȂ����̂ŁA���̉����
//�Q�l�T�C�g:http://kouma.hatenablog.com/entry/2012/08/15/232143
#define _WINSOCKAPI_

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <new>


// TODO: �v���O�����ɕK�v�Ȓǉ��w�b�_�[�������ŎQ�Ƃ��Ă�������
#if defined(_DEBUG)
#define NEW  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define NEW ::new
#endif
