set _VAR=%1
set MY_SYSTEM_ROOT_PATH=%_VAR:\=/%
echo #pargma once > %2ShaderSystemRootPath.h
echo std::string gShaderSystemRootPath = "%MY_SYSTEM_ROOT_PATH%"; > %2ShaderSystemRootPath.h
echo std::wstring gShaderSystemRootPathW = L"%MY_SYSTEM_ROOT_PATH%"; >> %2ShaderSystemRootPath.h
echo // graphics/dx12/utility/Shader/shader�ɂ���hlsli�t�@�C�������O�̃V�F�[�_�ɃC���N���[�h����CPP���ł��g�p�������Ƃ���> SunriseShaderSystemHeaderDir.txt
echo // ���̃p�X���v���W�F�N�g�̃C���N���[�h�p�X�ɒǉ����Ă�������>> SunriseShaderSystemHeaderDir.txt
echo %MY_SYSTEM_ROOT_PATH%>> SunriseShaderSystemHeaderDir.txt
