@echo off

SET CUR_PATH=%~dp0

cd %CUR_PATH%/tools/ConfigExporter/ 
ConfigExporter.exe --config=xlsx --lang=S:cpp@C:csharp,lua  --source_dir=../../service/TestService/config/xlsx --target_dir=../../service/TestService/config/code --data=../../service/TestService/config/data --meta=../../service/TestService/config/meta

:: Gateway
ConfigExporter.exe --config=xlsx --lang=S:cpp@C:csharp,lua  --source_dir=../../service/GateService/config/xlsx --target_dir=../../service/GateService/config/code --data=../../service/GateService/config/data --meta=../../service/GateService/config/meta

:: CenterServer
ConfigExporter.exe --config=xlsx --lang=S:cpp@C:csharp,lua  --source_dir=../../service/CenterService/config/xlsx --target_dir=../../service/CenterService/config/code --data=../../service/CenterService/config/data --meta=../../service/CenterService/config/meta

