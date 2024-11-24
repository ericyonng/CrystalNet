call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
mkdir dll_output
mkdir bin
mkdir static_lib_output
cd src
::Compile all C -> obj
cl /MD /O2 /c /DLUA_BUILD_AS_DLL *.c
:: Rename obj
ren lua.obj lua.o
ren luac.obj luac.o
::Generate lib and DLL
link /DLL /IMPLIB:lua5.4.7.lib /OUT:lua5.4.7.dll *.obj
::Generate Executer
link /OUT:lua.exe lua.o lua5.4.7.lib
::Generate static library
lib /OUT:lua5.4.7-static.lib *.obj
::Generate Lua compiler
link /OUT:luac.exe luac.o lua5.4.7-static.lib
xcopy lua5.4.7.dll ..\dll_output\
xcopy lua5.4.7.lib ..\dll_output\
xcopy lua5.4.7-static.lib ..\static_lib_output
xcopy lua.exe ..\bin
xcopy luac.exe ..\bin
xcopy lua5.4.7.lib ..\bin
xcopy lua5.4.7.dll ..\bin
xcopy lua5.4.7-static.lib ..\bin

::Clean tmp files
del *.obj
del *.exe
del *.exp
del *.lib
del *.o
del *.dll
cd ..