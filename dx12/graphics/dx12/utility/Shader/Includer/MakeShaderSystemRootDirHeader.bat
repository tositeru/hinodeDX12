set _VAR=%1
set MY_SYSTEM_ROOT_PATH=%_VAR:\=/%
echo #pargma once > %2ShaderSystemRootPath.h
echo std::string gShaderSystemRootPath = "%MY_SYSTEM_ROOT_PATH%"; > %2ShaderSystemRootPath.h
echo std::wstring gShaderSystemRootPathW = L"%MY_SYSTEM_ROOT_PATH%"; >> %2ShaderSystemRootPath.h
echo // graphics/dx12/utility/Shader/shaderにあるhlsliファイルを自前のシェーダにインクルードかつCPP側でも使用したいときは> SunriseShaderSystemHeaderDir.txt
echo // 下のパスをプロジェクトのインクルードパスに追加してください>> SunriseShaderSystemHeaderDir.txt
echo %MY_SYSTEM_ROOT_PATH%>> SunriseShaderSystemHeaderDir.txt
