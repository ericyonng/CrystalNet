
@echo off

SET CUR_PATH=%~dp0

:: msbuild路径
SET MSBUILD_PATH=C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\MSBuild\\Current\\Bin\\

:: 生成vs2019 sln
cd %CUR_PATH%\tools\premake && win_premake5.exe vs2019

:: 解决方案路径
SET SLN_PATH=%CUR_PATH%\\build\\vs2019\\CrystalNet_vs2019.sln
:: 判断文件是否存在
if not exist %SLN_PATH% (
    echo sln not exist %SLN_PATH%
    pause
    exit 1
)

:: 执行rebuild release
cd %MSBUILD_PATH%
MSBuild.exe %SLN_PATH%  /t:build /p:Configuration=Release