@echo off

Rem Generate visual studio project files

echo Visual Studio solution and project files generate tool.
echo For now supported Visual Studio versions:
echo    vs2015
echo    vs2017
echo    vs2019
echo    vs2022
set /p choose=Please input:

cd tools\premake && win_premake5.exe --file=onlytest_premake5.lua %choose%

if errorlevel 1 (
    echo Failed to generate Visual Studio solution and project files, error: %errorlevel%
    pause
    exit 1
) else (
    echo Succcess to generate Visual Studio solution and project files
    echo Solution file path: build/%choose%/CrystalNet_%choose%.sln
	Rem cancel support c++17
	Rem win_premake5.exe --file=fixcpp17_forwin.lua %choose%
   if "%1"=="" (
        explorer ..\..\build\%choose%
    )
    if "%1"=="1" (
        explorer ..\..\build\%choose%
    )
)

