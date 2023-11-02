@echo off

SET CUR_PATH=%~dp0
echo For now supported Visual Studio versions:
echo    vs2015
echo    vs2017
echo    vs2019
echo    vs2022
set /p choose=Please input:

call stop.bat

cd %CUR_PATH%/output/%choose% && start testsuit_debug.exe

