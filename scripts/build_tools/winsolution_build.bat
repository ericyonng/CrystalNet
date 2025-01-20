@echo off

Rem Generate visual studio project files

SET CUR_PATH="%~dp0"
SET ROOT_PATH="%CUR_PATH%..\..\"

echo ROOT PATH:%ROOT_PATH%
echo Visual Studio solution and project files generate tool.
echo For now supported Visual Studio versions:
echo    vs2015
echo    vs2017
echo    vs2019
echo    vs2022
set /p choose=Please input:

cd %ROOT_PATH%tools\premake && win_premake5.exe --file=premake5_tools.lua %choose%

if errorlevel 1 (
    echo Failed to generate Visual Studio solution and project files, error: %errorlevel%
    pause
    exit 1
) else (
    echo Succcess to generate Visual Studio solution and project files
    echo Solution file path: build_tools/%choose%/CrystalNet_%choose%.sln
	Rem cancel support c++17
	Rem win_premake5.exe --file=fixcpp17_forwin.lua %choose%
   if "%1"=="" (
        explorer ..\..\build_tools\%choose%
    )
    if "%1"=="1" (
        explorer ..\..\build_tools\%choose%
    )
)

