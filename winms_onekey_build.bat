@echo off

Rem Generate visual studio project files

SET CUR_PATH=%~dp0

:: 需要手动设置msbuild.exe路径 TODO:
SET MSBUILD_PATH=C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\MSBuild\\Current\\Bin\\

:: 选择vs版本
echo Visual Studio solution and project files generate tool.
echo For now supported Visual Studio versions:
echo    vs2015
echo    vs2017
echo    vs2019
echo    vs2022
set /p choose=Please input:
echo choose:%choose%

:: 编译版本
echo choose Debug/Release:
set /p Ver=Please input:
echo Ver:%Ver%

:: 重编或者增量编译
echo choose Rebuild y/n:
set /p VsBuildType=Please input:
pause

SET FINAL_BUILD=build
if '%VsBuildType%' == 'y' SET FINAL_BUILD=Rebuild
echo FINAL_BUILD:%FINAL_BUILD%

:: 生成sln
cd %CUR_PATH%\tools\premake && win_premake5.exe %choose%

:: sln路径
SET SLN_PATH=%CUR_PATH%\\build\\%choose%\\CrystalNet_%choose%.sln
:: 判断sln文件是否存在
if not exist %SLN_PATH% (
    echo sln not exist %SLN_PATH%
    pause
    exit 1
)

:: 执行
cd %MSBUILD_PATH%
MSBuild.exe %SLN_PATH%  /t:%FINAL_BUILD% /p:Configuration=%Ver%

