@echo off
::for /f "tokens=2" %%a in ('tasklist ^| findstr /C:"testsuit"') do (
::    echo "%%a"
::    taskkill /F /pid %%a /t
::)

::@echo off

SET CUR_PATH=%~dp0
SET ROOT_PATH=%CUR_PATH%../../

cd %ROOT_PATH%/tools/CloseProcess/ 

:: is_match_path:匹配目录 例如: vs2019/testsuit 会匹配vs2019
::CloseWindowsProcess.exe vs2019/testsuit is_match_path=0 is_likely=1 is_waiting_close=1
CloseProcess.exe LogicServer is_waiting_close=1
