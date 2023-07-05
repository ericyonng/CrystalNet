@echo off
::for /f "tokens=2" %%a in ('tasklist ^| findstr /C:"testsuit"') do (
::    echo "%%a"
::    taskkill /F /pid %%a /t
::)

::@echo off

SET CUR_PATH=%~dp0

cd %CUR_PATH%/tools/CloseProcess/ 

:: is_match_path:匹配目录 例如: vs2019/testsuit 会匹配vs2019
::CloseWindowsProcess.exe vs2019/testsuit is_match_path=0
CloseProcess.exe testsuit

pause