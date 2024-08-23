@echo off

SET CUR_PATH=%~dp0

cd %CUR_PATH%/tools/ConfigExporter/ 
ConfigExporter.exe --config=xlsx --lang=S:cpp@C:csharp,lua  --source_dir=../../service/TestService/config/xlsx --target_dir=../../service/TestService/config/code --data=../../service/TestService/config/data --meta=../../service/TestService/config/meta

if %errorlevel% equ 0 (
	echo Sussess generate testservice configs!
) else (
    echo Failed to generate testservice configs error: %errorlevel%
    pause
    exit 1
)

:: Client
ConfigExporter.exe --config=xlsx --lang=S:cpp@C:csharp,lua  --source_dir=../../service/Client/config/xlsx --target_dir=../../service/Client/config/code --data=../../service/Client/config/data --meta=../../service/Client/config/meta

if %errorlevel% equ 0 (
	echo Sussess generate Client configs!
) else (
    echo Failed to generate Client configs error: %errorlevel%
    pause
    exit 1
)

:: Gateway
ConfigExporter.exe --config=xlsx --lang=S:cpp@C:csharp,lua  --source_dir=../../service/GateService/config/xlsx --target_dir=../../service/GateService/config/code --data=../../service/GateService/config/data --meta=../../service/GateService/config/meta
if %errorlevel% equ 0 (
	echo Sussess generate GateService configs!
) else (
    echo Failed to generate GateService configs error: %errorlevel%
    pause
    exit 1
)

:: CenterServer
ConfigExporter.exe --config=xlsx --lang=S:cpp@C:csharp,lua  --source_dir=../../service/CenterService/config/xlsx --target_dir=../../service/CenterService/config/code --data=../../service/CenterService/config/data --meta=../../service/CenterService/config/meta

pause

if %errorlevel% equ 0 (
	echo Sussess generate CenterService configs!
) else (
    echo Failed to generate CenterService configs error: %errorlevel%
    pause
    exit 1
)