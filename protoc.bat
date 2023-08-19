:: echo "path:"%~dp0
SET CUR_PATH=%~dp0
:: force_gen_all 强制全部生成,因为有可能你漏了一个.pb.h, .pb.cc提交会导致增量更新生成不出来,预期去检查不如强制全部生成
cd %~dp0/tools/protogen/ && protogentool.exe --proto_path=../../protocols/proto --csharp_out=../../protocols/csharp --cpp_out=../../protocols/cplusplus --base_path=../../ --cpp_protoc=../../tools/protobuf/bin/protoc.exe --force_gen_all=1 --protocols_path=../../protocols/ --google_proto_include_path=../../3rd/protobuf/include/
::cd %~dp0/tools/protogen/ && protogentool.exe --proto_path=../../protocols/proto --csharp_out=../../protocols/csharp --cpp_out=../../protocols/cplusplus --base_path=../../ --cpp_protoc=../../tools/protobuf/bin/protoc.exe --protocols_path=../../protocols/ --google_proto_include_path=../../3rd/protobuf/include/

::%~dp0/tools/protobuf-net/ProtoGen/protoc.exe --proto_path=%~dp0/protocols/proto --cpp_out=%~dp0/protocols/cplusplus %~dp0/protocols/proto/com.proto

cd %CUR_PATH%/tools/protobuf/bin
for %%i in (../../../protocols/proto/*.proto) do (
    protoc.exe --js_out=import_style=es6,binary:../../../protocols/js/ --proto_path=../../../protocols/proto %%i
    echo Exchange %%i To Javascript file successfully!
)

pause

if %errorlevel% equ 0 (
	echo Sussess generate protobuf files all!
	exit 1
) else (
    echo Failed to generate protobuf files error: %errorlevel%
    pause
)

