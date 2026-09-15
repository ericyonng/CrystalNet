@echo off

SET CUR_PATH=%~dp0
SET ROOT_PATH=%CUR_PATH%../../
echo For now supported Visual Studio versions:
echo    vs2015
echo    vs2017
echo    vs2019
echo    vs2022
set /p choose=Please input:

:: call stop.bat

cd %ROOT_PATH%/output/%choose%/build_x && start LogicServer_debug.exe

