@echo off

:: 取得当前路径
SET CUR_PATH="%~dp0"
SET ROOT_PATH="%CUR_PATH%..\..\"
SET VER="%1"

echo root_path:%ROOT_PATH%

:: 获取管理员权限
:: if not "%1"=="am_admin" (powershell start -verb runas '%0' am_admin & exit /b)

:: -------------------------------------- 链接配置 -------------------------------------------
if not exist %ROOT_PATH%output\%VER%\Cfgs (
    mklink /d %ROOT_PATH%output\%VER%\Cfgs %ROOT_PATH%Service\TestService\config\data\cpp\
	
)
if not exist %ROOT_PATH%output\%VER%\ini (
    mklink /d %ROOT_PATH%output\%VER%\ini %ROOT_PATH%doc\ini
	
)

:: mysql dll拷贝
del /q %ROOT_PATH%output\%VER%\libmysql.dll
xcopy /s /y %ROOT_PATH%\3rd\mysql\win\lib\libmysql.dll %ROOT_PATH%\output\%VER%\

:: ssl拷贝
del /q %ROOT_PATH%output\%VER%\libssl-1_1-x64.dll
xcopy /s /y %ROOT_PATH%\3rd\openssl\staticlib\dlls\libssl-1_1-x64.dll %ROOT_PATH%\output\%VER%\

del /q %ROOT_PATH%output\%VER%\libcrypto-1_1-x64.dll
xcopy /s /y %ROOT_PATH%\3rd\openssl\staticlib\dlls\libcrypto-1_1-x64.dll %ROOT_PATH%\output\%VER%\

::脚本拷贝
xcopy /s /y %ROOT_PATH%\toolbox\scripts\* %ROOT_PATH%\output\%VER%\

:: lua脚本
if not exist %ROOT_PATH%output\%VER%\TestServiceLuaScript (
    mklink /d %ROOT_PATH%output\%VER%\TestServiceLuaScript %ROOT_PATH%service\TestService\TestServiceLuaScript
	
)

:: ------------------------------------ 结束杂项链接 -----------------------------------------
echo Done!
ping -n 1 -w 1618 11.11.11.11 > nul
exit 0

:: -------------------------------------- 函数定义 -------------------------------------------
:: relink目录函数定义
:relink_dir
if exist %2 (
    rd /s/q %2
)
mklink /d %2 %1
goto EOF


:: relink文件函数定义
:relink_file
if exist %2 (
    del /q %2
)
mklink %2 %1
goto EOF


::remove_dir 删除目录
:remove_dir
set DelDir=%1
if exist %delDir% (
    rd /s/q %delDir%
)
goto EOF
:: ------------------------------------ 结束函数定义 -----------------------------------------

:EOF

