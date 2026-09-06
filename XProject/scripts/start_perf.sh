# @file start_perf.sh
# @author EricYonng<120453674@qq.com>
# @brief start perf...
#!/usr/bin/env bash

GREP_FLAG=$1
SAMPLE_FREQ=$2

# 脚本路径
SCRIPT_PATH="$(cd $(dirname $0); pwd)"
# perf path
PERF_PATH=${SCRIPT_PATH}/perf_dir/

IS_EXISTS_PATH=$(ls ${SCRIPT_PATH} | grep 'perf_dir')
if [ -z "${IS_EXISTS_PATH}" ]
then
    echo "create perf dir..."
    mkdir ${PERF_PATH}
else
    echo "exist perf dir:${PERF_PATH}"
fi

if [ -z ${GREP_FLAG} ]
then
    echo "have no any grep flag"
    exit 1
fi
if [ -z ${SAMPLE_FREQ} ]
then
    SAMPLE_FREQ=99
fi

# 获取进程id
TARGET_PIDS=$(ps -aux | grep "${GREP_FLAG}" | sed '/grep/d' | sed '/start_perf/d'  | sed '/stop_perf/d' | awk '{print $2}')

# TARGET_PIDS="$(ps -aux |grep "${GREP_FLAG}" | sed '/grep/d' | sed '/start_perf/d' | sed '/stop_perf/d' | sed 's/^[^ ]* //' | sed 's/^ *//' | sed 's/ .*$//')"

echo "start_perf GREP_FLAG:${GREP_FLAG} TARGET_PIDS:${TARGET_PIDS}"

# 遍历并启动perf 收集性能信息
for pid in $TARGET_PIDS
do
    EXE_PATH=$(ls -l /proc/${pid}/exe)
    echo "start perf target pid:${pid} exe path:${EXE_PATH}" 

    # 先创建目录
    cd ${PERF_PATH} && rm -rf ${PERF_PATH}/perf_record_${GREP_FLAG}_${pid}
    mkdir ${PERF_PATH}/perf_record_${GREP_FLAG}_${pid}
    cd ${PERF_PATH}/perf_record_${GREP_FLAG}_${pid}
    nohup perf record -e cpu-clock -a -F ${SAMPLE_FREQ} -g -p ${pid} --call-graph dwarf > /dev/null 2>&1 &
done
