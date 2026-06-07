@echo off
setlocal enabledelayedexpansion

:: 取得当前路径（注意：引号在外面包裹，不在值里）
SET "CUR_PATH=%~dp0"
SET "VER=%1"
SET "SUFFIX=%2"

echo CUR_PATH:%CUR_PATH%
echo VER:%VER%
echo SUFFIX:%SUFFIX%

set "SRC_FILE=%CUR_PATH%..\..\output\%VER%\build\PluginTmp\libTestServicePlugin%SUFFIX%.dll"
set "DEST_DIR=%CUR_PATH%..\..\output\%VER%\build\"

:: 检查源文件是否存在
if not exist "%SRC_FILE%" (
    echo [ERROR] 文件不存在: %SRC_FILE%
    exit /b 1
)

:: 获取当前时间（yyyyMMddHHmmss）
for /f "tokens=2 delims==" %%a in ('wmic os get localdatetime /value') do set "datetime=%%a"
set "TIMESTAMP=!datetime:~0,14!"

:: 提取文件名和扩展名
for %%f in ("%SRC_FILE%") do (
    set "FILENAME=%%~nf"
    set "EXT=%%~xf"
)

set "NEW_NAME=%FILENAME%.%TIMESTAMP%%EXT%"

:: 确保目标目录存在
if not exist "%DEST_DIR%" mkdir "%DEST_DIR%"

:: 如果目标文件已存在则删除
if exist "%DEST_DIR%%NEW_NAME%" del /q "%DEST_DIR%%NEW_NAME%"

:: 拷贝（加 \ 让 copy 明确目标是目录，加 /Y 静默覆盖）
copy /Y "%SRC_FILE%" "%DEST_DIR%%NEW_NAME%"

echo [OK] %SRC_FILE% --^> %DEST_DIR%%NEW_NAME%
echo [OK] 已拷贝完成
