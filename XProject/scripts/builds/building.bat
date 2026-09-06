:: 取得当前路径
SET CUR_PATH=%~dp0
SET VER="%1"
SET OUTPUT_DIR=%2
SET ROOT_PATH=%CUR_PATH%..\..\..\

if %VER% == "debug" (
    del /q %ROOT_PATH%3rd\kernel\libCrystalKernel_debug.pdb
    del /q %ROOT_PATH%3rd\kernel\libCrystalKernel_debug.lib
    del /q %ROOT_PATH%3rd\kernel\libCrystalKernel_debug.dll
    xcopy /s /y %CUR_PATH%..\..\\%OUTPUT_DIR%\libCrystalKernel_debug.pdb %ROOT_PATH%3rd\kernel\
    xcopy /s /y %CUR_PATH%..\..\\%OUTPUT_DIR%\libCrystalKernel_debug.lib %ROOT_PATH%3rd\kernel\
    xcopy /s /y %CUR_PATH%..\..\\%OUTPUT_DIR%\libCrystalKernel_debug.dll %ROOT_PATH%3rd\kernel\
    echo Done debug!
)

if %VER% == "release" (
    del /q %ROOT_PATH%3rd\kernel\libCrystalKernel.pdb
    del /q %ROOT_PATH%3rd\kernel\libCrystalKernel.lib
    del /q %ROOT_PATH%3rd\kernel\libCrystalKernel.dll
    xcopy /s /y %CUR_PATH%\..\..\%OUTPUT_DIR%\libCrystalKernel.pdb %ROOT_PATH%3rd\kernel\
    xcopy /s /y %CUR_PATH%\..\..\%OUTPUT_DIR%\libCrystalKernel.lib %ROOT_PATH%3rd\kernel\
    xcopy /s /y %CUR_PATH%\..\..\%OUTPUT_DIR%\libCrystalKernel.dll %ROOT_PATH%3rd\kernel\
    echo Done release!
)

exit 0

