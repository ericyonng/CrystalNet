# @author EricYonng<120453674@qq.com>
# @brief proto generator ...
#!/usr/bin/env bash

SCRIPT_PATH="$(cd $(dirname $0); pwd)"
ROOT_PATH=${SCRIPT_PATH}/../../
ROOT_PATH_BY_PROTOGEN=../../
XPROJ_PATH_BY_PROTOGEN=../../XProject/

WHOSME="$(echo `whoami`)"
echo $WHOSME

echo "current gcc version:"`gcc --version`

sudo chmod a+x ${ROOT_PATH}tools/protogen/protogentool

# force_gen_all 强制全部生成,因为有可能你漏了一个.pb.h, .pb.cc提交会导致增量更新生成不出来,预期去检查不如强制全部生成
sudo ${ROOT_PATH}tools/protogen/protogentool --proto_path=${XPROJ_PATH_BY_PROTOGEN}protocols/proto --ts_out=${XPROJ_PATH_BY_PROTOGEN}protocols/ts_out --csharp_out=${XPROJ_PATH_BY_PROTOGEN}protocols/csharp --cpp_out=${XPROJ_PATH_BY_PROTOGEN}protocols/cplusplus --orm_out=${XPROJ_PATH_BY_PROTOGEN}protocols/orm_out --base_path=${ROOT_PATH_BY_PROTOGEN} --cpp_protoc=${ROOT_PATH_BY_PROTOGEN}tools/protobuf/bin/protoc --protocols_path=${XPROJ_PATH_BY_PROTOGEN}protocols/ --force_gen_all=1 --google_proto_include_path=${ROOT_PATH}3rd/protobuf/include/
# sudo $SCRIPT_PATH/tools/protogen/protogentool --proto_path=../../protocols/proto --csharp_out=../../protocols/csharp --cpp_out=../../protocols/cplusplus --base_path=../../ --cpp_protoc=../../tools/protobuf/bin/protoc --protocols_path=../../protocols/ --google_proto_include_path=../../3rd/protobuf/include/

if [ $? = 0 ]
then
    echo "proto gen success."
else
    echo "proto gen failure."
fi

