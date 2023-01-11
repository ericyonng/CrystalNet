# @author EricYonng<120453674@qq.com>
# @brief 3rd scripts
#!/usr/bin/env bash

# 路径
SCRIPT_PATH="$(cd $(dirname $0); pwd)"

GREP_FLAG="CrystalNet"
if [ $1 ]
then
    echo "param 1:$1"
    GREP_FLAG=$1
fi

echo "will stop process with GREP_FLAG:${GREP_FLAG}"

PID_LIST="$(ps -aux |grep ${GREP_FLAG} | sed '/grep/d' | sed '/stop/d' | sed 's/^[^ ]* //' | sed 's/^ *//' | sed 's/ .*$//')"

echo "will stop PID_LIST:${PID_LIST}"

for pid in $PID_LIST
do
    echo "stop process $pid"
    kill 2 $pid
done
