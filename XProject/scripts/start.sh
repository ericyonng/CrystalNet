# @author EricYonng<120453674@qq.com>
# @brief run svr...
#!/usr/bin/env bash

SCRIPT_PATH="$(cd $(dirname $0); pwd)"
# $1:debug/release $2:is_block $3:wait seconds to check start 
# 启动testsuit
sh $SCRIPT_PATH/linux_run.sh build testsuit $1 "$2" $3
