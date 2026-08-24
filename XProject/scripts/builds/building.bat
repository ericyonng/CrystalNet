:: 取得当前路径
SET CUR_PATH=%~dp0
SET VER="%1"
SET OUTPUT_DIR=%2

if %VER% == "debug" (
    del /q %CUR_PATH%..\..\3rd\kernel\libCrystalKernel_debug.pdb
    del /q %CUR_PATH%..\..\3rd\kernel\libCrystalKernel_debug.lib
    del /q %CUR_PATH%..\..\3rd\kernel\libCrystalKernel_debug.dll
    xcopy /s /y %CUR_PATH%..\..\\%OUTPUT_DIR%\libCrystalKernel_debug.pdb %CUR_PATH%..\..\3rd\kernel\
    xcopy /s /y %CUR_PATH%..\..\\%OUTPUT_DIR%\libCrystalKernel_debug.lib %CUR_PATH%..\..\3rd\kernel\
    xcopy /s /y %CUR_PATH%..\..\\%OUTPUT_DIR%\libCrystalKernel_debug.dll %CUR_PATH%..\..\3rd\kernel\
    echo Done debug!
)

if %VER% == "release" (
    del /q %CUR_PATH%\..\..\3rd\kernel\libCrystalKernel.pdb
    del /q %CUR_PATH%\..\..\3rd\kernel\libCrystalKernel.lib
    del /q %CUR_PATH%\..\..\3rd\kernel\libCrystalKernel.dll
    xcopy /s /y %CUR_PATH%\..\..\%OUTPUT_DIR%\libCrystalKernel.pdb %CUR_PATH%\..\..\3rd\kernel\
    xcopy /s /y %CUR_PATH%\..\..\%OUTPUT_DIR%\libCrystalKernel.lib %CUR_PATH%\..\..\3rd\kernel\
    xcopy /s /y %CUR_PATH%\..\..\%OUTPUT_DIR%\libCrystalKernel.dll %CUR_PATH%\..\..\3rd\kernel\
    echo Done release!
)

exit 0

