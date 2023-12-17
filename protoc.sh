# @author EricYonng<120453674@qq.com>
# @brief proto generator ...
#!/usr/bin/env bash

SCRIPT_PATH="$(cd $(dirname $0); pwd)"

WHOSME="$(echo `whoami`)"
echo $WHOSME

echo "current gcc version:"`gcc --version`

sudo chmod a+x $SCRIPT_PATH/tools/protogen/protogentool

# force_gen_all 强制全部生成,因为有可能你漏了一个.pb.h, .pb.cc提交会导致增量更新生成不出来,预期去检查不如强制全部生成
sudo $SCRIPT_PATH/tools/protogen/protogentool --proto_path=../../protocols/proto --ts_out=../../protocols/ts_out --csharp_out=../../protocols/csharp --cpp_out=../../protocols/cplusplus --orm_out=../../protocols/orm_out --base_path=../../ --cpp_protoc=../../tools/protobuf/bin/protoc --protocols_path=../../protocols/ --force_gen_all=1 --google_proto_include_path=../../3rd/protobuf/include/
# sudo $SCRIPT_PATH/tools/protogen/protogentool --proto_path=../../protocols/proto --csharp_out=../../protocols/csharp --cpp_out=../../protocols/cplusplus --base_path=../../ --cpp_protoc=../../tools/protobuf/bin/protoc --protocols_path=../../protocols/ --google_proto_include_path=../../3rd/protobuf/include/

if [ $? = 0 ]
then
    echo "proto gen success."
else
    echo "proto gen failure."
fi

