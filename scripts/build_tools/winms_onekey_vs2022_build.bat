@echo off
setlocal

SET CUR_PATH=%~dp0
SET ROOT_PATH="%CUR_PATH%..\.."
SET VSWHERE_PATH=%ROOT_PATH%\tools\vswhere

:: 使用 vswhere 查找 Visual Studio 2022 的安装路径
cd %VSWHERE_PATH%
for /f "usebackq tokens=*" %%i in (`vswhere -version "[17.0,18.0)" -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
    set VS_INSTALL_PATH=%%i
)

:: 检查是否找到 Visual Studio 路径
if "%VS_INSTALL_PATH%"=="" (
    echo Visual Studio 2022 not found.
    pause
    exit /b 1
)

:: 获取 MSBuild 路径
set MSBUILD_PATH=%VS_INSTALL_PATH%\MSBuild\Current\Bin
set MSBUILD_EXE_PATH=%VS_INSTALL_PATH%\MSBuild\Current\Bin\MSBuild.exe

:: 检查 MSBuild 是否存在
if not exist "%MSBUILD_EXE_PATH%" (
    echo MSBuild not found at %MSBUILD_EXE_PATH%.
    pause
    exit /b 1
)

:: 输出 MSBuild 路径
echo MSBuild Path: %MSBUILD_EXE_PATH%

:: 将路径添加到 PATH 环境变量
set PATH=%MSBUILD_PATH%;%PATH%

:: 验证 PATH 是否已更新
:: echo Updated PATH: %PATH%

:: 选择vs版本
echo Visual Studio solution and project files generate tool.
echo For now supported Visual Studio versions:
set choose=vs2022
echo choose:%choose%

:: 选择编译版本
echo choose Release?(y for release, others for debug) y?:
set /p VsBuildVer=Please input:

SET FINAL_VER=Debug
if '%VsBuildVer%' == 'y' SET FINAL_VER=Release
echo FINAL_VER:%FINAL_VER%

:: 重编或者增量编译
echo choose Rebuild (y for rebuild, others for increament build)y?:
set /p VsBuildType=Please input:

SET FINAL_BUILD=build
if '%VsBuildType%' == 'y' SET FINAL_BUILD=Rebuild
echo FINAL_BUILD:%FINAL_BUILD%

:: 生成sln
cd %ROOT_PATH%\tools\premake && win_premake5.exe --file=premake5_tools.lua %choose%

:: sln路径
SET SLN_PATH=%ROOT_PATH%\\build_tools\\%choose%\\CrystalNet_%choose%.sln
:: 判断sln文件是否存在
if not exist %SLN_PATH% (
    echo sln not exist %SLN_PATH%
    pause
    exit 1
)

:: 执行
MSBuild.exe %SLN_PATH%  /t:%FINAL_BUILD% /p:Configuration=%FINAL_VER%

if errorlevel 1 (
    echo Build Failed...
    pause
) else (
    echo Build Succcess!
)

endlocal