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

PID_LIST="$(ps -aux |grep ${GREP_FLAG} | sed 's/^[^ ]* //' | sed 's/^ *//' | sed 's/ .*$//')"

FILTER_LIST="$(ps -aux |grep ${GREP_FLAG} | grep stop | sed 's/^[^ ]* //' | sed 's/^ *//' | sed 's/ .*$//')"

echo "will stop PID_LIST:${PID_LIST}"
echo "FILTER_LIST:${FILTER_LIST}"

for pid in $PID_LIST
do
    IS_FILTER_PID=0
    for filter_pid in $FILTER_LIST
    do
        if [ $pid -eq $filter_pid ]
        then
            IS_FILTER_PID=1
            break
        fi
    done

    if [ $pid -eq 1 ]
    then
        continue
    fi

    echo "stop process $pid"
    kill 2 $pid
done
