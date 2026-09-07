@echo off

SET CUR_PATH=%~dp0
SET ROOT_PATH=%CUR_PATH%../../
SET XPROJ_PATH=%CUR_PATH%../

cd %ROOT_PATH%/tools/ConfigExporter/ 
ConfigExporter.exe --config=xlsx --lang=S:cpp@C:csharp,lua  --source_dir=%XPROJ_PATH%Config/xlsx --target_dir=%XPROJ_PATH%Config/code --data=%XPROJ_PATH%Config/data --meta=%XPROJ_PATH%Config/meta

if %errorlevel% equ 0 (
	echo Sussess generate testservice configs!
) else (
    echo Failed to generate testservice configs error: %errorlevel%
    pause
    exit 1
)

pause
