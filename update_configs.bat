@echo off

SET CUR_PATH=%~dp0

cd %CUR_PATH%/tools/ConfigExporter/ 
ConfigExporter.exe --config=xlsx --lang=S:cpp@C:csharp,lua  --source_dir=../../service/TestService/config/xlsx --target_dir=../../service/TestService/config/code --data=../../service/TestService/config/data --meta=../../service/TestService/config/meta
pause